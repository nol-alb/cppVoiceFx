#include "MUSI6106Config.h"

#ifdef WITH_TESTS
#include "Vector.h"
#include "Vibrato.h"
#include "Synthesis.h"
#include "Tremolo.h"

#include "gtest/gtest.h"


namespace vibrato_test {
    void CHECK_ARRAY_CLOSE(float* buffer1, float* buffer2, int iLength, float fTolerance)
    {
        for (int i = 0; i < iLength; i++)
        {
            EXPECT_NEAR(buffer1[i], buffer2[i], fTolerance);
        }
    }


    class CVibratoTests : public testing::Test
    {
    public:
        CVibrato *p_CVibratoTest=0;
        float** m_ppfInBuffer = 0;
        float** m_ppfOutBuffer = 0;
        int m_iBufferChannels = 2;
        int m_iBufferLength = 1000;
        int m_iSampleFreq = 44100;

        CVibratoTests() 
        {

        }

        ~CVibratoTests() override 
        {

        }

        void SetUp() override {

            CVibrato::create(p_CVibratoTest);
            m_ppfInBuffer = new float*[m_iBufferChannels];
            m_ppfOutBuffer = new float*[m_iBufferChannels];
            for (int channel = 0; channel < m_iBufferChannels; channel++)
            {
                m_ppfInBuffer[channel] = new float[m_iBufferLength] {};
                m_ppfOutBuffer[channel] = new float[m_iBufferLength] {};
            }

        }

        virtual void TearDown() override{
            CVibrato::destroy(p_CVibratoTest);
            for (int channel = 0; channel < m_iBufferChannels; channel++)
            {
                delete[] m_ppfInBuffer[channel];
                delete[] m_ppfOutBuffer[channel];
            }
            delete[] m_ppfInBuffer;
            delete[] m_ppfOutBuffer;
            m_ppfInBuffer = 0;
            m_ppfOutBuffer = 0;
        }

        /**
        * Process one block of data given a starting sample and block size
        * @param ppfInputBuffer : entire buffer of input values
        * @param ppfOutputBuffer : entire buffer to place outputted values
        * @param pVibrato : pointer to an initialized CVibrato instantiation
        * @param iNumChannels : number of channels in input/output buffer
        * @param iStartSample : the sample at which to begin the block processing
        * @param iBlockSize : the number of samples to process
        * @return void
        */
        void processBlock(float** ppfInputBuffer, float** ppfOutputBuffer, CVibrato* pVibrato, int iNumChannels, int iStartSample, int iBlockSize)
        {
            float** ppfTempInBuffer = new float* [iNumChannels];
            float** ppfTempOutBuffer = new float* [iNumChannels];
            for (int channel = 0; channel < iNumChannels; channel++)
            {
                ppfTempInBuffer[channel] = new float[iBlockSize] {0};
                ppfTempOutBuffer[channel] = new float[iBlockSize] {0};
                for (int sample = 0; sample < iBlockSize; sample++)
                    ppfTempInBuffer[channel][sample] = ppfInputBuffer[channel][sample + iStartSample];
            }

            pVibrato->process(ppfTempInBuffer, ppfTempOutBuffer, iBlockSize);

            for (int channel = 0; channel < iNumChannels; channel++)
            {
                for (int sample = 0; sample < iBlockSize; sample++)
                    ppfOutputBuffer[channel][sample + iStartSample] = ppfTempOutBuffer[channel][sample];
                delete[] ppfTempInBuffer[channel];
                delete[] ppfTempOutBuffer[channel];
            }
            delete[] ppfTempInBuffer;
            delete[] ppfTempOutBuffer;
        }
    };
    TEST_F(CVibratoTests, CorrectInputOutputHandles)
    {
        int iNumChannels = 1;
        float fTestSampleRateInHz= 44100;
        float fTestDelay=0.1f;
        float fTestModInHz= 15;
        float fTestWidth=0.01f;
        EXPECT_EQ(p_CVibratoTest->init(fTestDelay,fTestWidth,fTestModInHz,fTestSampleRateInHz,iNumChannels),Error_t::kNoError);
        EXPECT_EQ(p_CVibratoTest->getParam(CVibrato::ParamVibrato::kLFOFreqInHz),15);
        EXPECT_EQ(p_CVibratoTest->getParam(CVibrato::ParamVibrato::kSampleRate),44100);
        EXPECT_EQ(p_CVibratoTest->getParam(CVibrato::ParamVibrato::kWidthInSec),0.01f);
        iNumChannels = 2;
        fTestSampleRateInHz= 44100;
        fTestDelay=1.5f;
        fTestModInHz= 15;
        fTestWidth=0.015f;
        EXPECT_EQ(p_CVibratoTest->init(fTestDelay,fTestWidth,fTestModInHz,fTestSampleRateInHz,iNumChannels),Error_t::kNoError);
        EXPECT_EQ(p_CVibratoTest->getParam(CVibrato::ParamVibrato::kWidthInSec),0.015f);
    }
    TEST_F(CVibratoTests, OutBoundsInputOutputHandles)
    {
        int iNumChannels = 1;
        float fTestSampleRateInHz= -44100;
        float fTestDelay=-0.1f;
        float fTestModInHz= -15;
        float fTestWidth=0.01f;
        EXPECT_EQ(p_CVibratoTest->init(fTestDelay,fTestWidth,fTestModInHz,fTestSampleRateInHz,iNumChannels),Error_t::kFunctionInvalidArgsError);
        iNumChannels = 1;
        fTestSampleRateInHz= 44100;
        fTestDelay=0.1f;
        fTestModInHz= 15;
        fTestWidth=1.24f;
        EXPECT_EQ(p_CVibratoTest->init(fTestDelay,fTestWidth,fTestModInHz,fTestSampleRateInHz,iNumChannels),Error_t::kFunctionInvalidArgsError);

    }

    TEST_F(CVibratoTests, DCstaysDC)
    {
        for (int channel = 0; channel < m_iBufferChannels; channel++)
            CSynthesis::generateDc(m_ppfInBuffer[channel], m_iBufferLength, 1);
        float fWidth = 0.02;
        float fDelayInSec = 0.0001;

        p_CVibratoTest->init(fDelayInSec, fWidth, 1, 44100, m_iBufferChannels);
        p_CVibratoTest->process(m_ppfInBuffer, m_ppfOutBuffer, m_iBufferLength);
        int check_start_post = static_cast<int>(2 + (fWidth * 2 * 44100));
        for (int channel = 0; channel < m_iBufferChannels; channel++)
            CHECK_ARRAY_CLOSE(m_ppfInBuffer[channel] + check_start_post, m_ppfOutBuffer[channel], m_iBufferLength - check_start_post, 1e-3);

    }

    TEST_F(CVibratoTests, ZeroWidthTest)
    {
        for (int channel = 0; channel < m_iBufferChannels; channel++)
            CSynthesis::generateSine(m_ppfInBuffer[channel], 440, 44100, m_iBufferLength, 0);
        float fWidth = 0;
        float fDelayInSec = 0.0025;
        p_CVibratoTest->init(fDelayInSec, fWidth, 24, 44100, m_iBufferChannels);
        p_CVibratoTest->process(m_ppfInBuffer, m_ppfOutBuffer, m_iBufferLength);
        int delayInSamples = fDelayInSec*m_iSampleFreq;
        for (int channel = 0; channel < m_iBufferChannels; channel++)
        {
                CHECK_ARRAY_CLOSE(m_ppfInBuffer[channel], m_ppfOutBuffer[channel]+delayInSamples,m_iBufferLength-delayInSamples,1e-3);
        }
    }
    TEST_F(CVibratoTests, ZeroInputSignal)
    {
        for (int channel = 0; channel < m_iBufferChannels; channel++)
            CSynthesis::generateSine(m_ppfInBuffer[channel], 440.f,1.f,m_iBufferLength);
        float fWidth = 0.f;
        float fDelayInSec = 0.25;
        p_CVibratoTest->init(fDelayInSec, fWidth, 24, 44100, m_iBufferChannels);
        p_CVibratoTest->process(m_ppfInBuffer, m_ppfOutBuffer, m_iBufferLength);
        int delayInSamples = fDelayInSec*m_iSampleFreq;
        for (int channel = 0; channel < m_iBufferChannels; channel++)
        {
            for (int val = 0; val<m_iBufferLength; val++)
            {
                EXPECT_NEAR(m_ppfOutBuffer[channel][val],0,1e-3);
            }

        }

    }

    TEST_F(CVibratoTests, VaryingInputBlockSize)
    {
        float** ppfBlockOutput = new float* [m_iBufferChannels];
        for (int channel = 0; channel < m_iBufferChannels; channel++)
        {
            ppfBlockOutput[channel] = new float[m_iBufferLength] {0};
            CSynthesis::generateNoise(m_ppfInBuffer[channel], m_iBufferLength);
        }

        p_CVibratoTest->init(0.01, 0.002, 2, 44100, 2);
        p_CVibratoTest->process(m_ppfInBuffer, m_ppfOutBuffer, m_iBufferLength);

        p_CVibratoTest->init(0.01, 0.002, 2, 44100, 2);
        processBlock(m_ppfInBuffer, ppfBlockOutput, p_CVibratoTest, m_iBufferChannels, 0, 100);
        processBlock(m_ppfInBuffer, ppfBlockOutput, p_CVibratoTest, m_iBufferChannels, 100, 500);
        processBlock(m_ppfInBuffer, ppfBlockOutput, p_CVibratoTest, m_iBufferChannels, 600, 400);

        for (int channel = 0; channel < m_iBufferChannels; channel++)
        {
            CHECK_ARRAY_CLOSE(ppfBlockOutput[channel], m_ppfOutBuffer[channel], m_iBufferLength, 1E-3);
            delete[] ppfBlockOutput[channel];
        }
        delete[] ppfBlockOutput;
    }

    class Lfo : public testing::Test
    {
    protected:
        void SetUp() override
        {
            m_pLfo = new CLfo();
            m_pfLfoBuffer = new float[m_iBufferLength] {};
            m_pfSynthesisBuffer = new float[m_iBufferLength] {};
        }

        virtual void TearDown()
        {
            delete m_pLfo;
            delete[] m_pfLfoBuffer;
            delete[] m_pfSynthesisBuffer;
            m_pLfo = 0;
            m_pfLfoBuffer = 0;
            m_pfSynthesisBuffer = 0;
        }

        /**
        * Generates and compares sinusoidal buffers from CLfo and CSynthesis based on a set of values
        * @param fSampleRate : sample rate in Hz
        * @param fFrequency : frequency in Hz
        * @param fAmplitude : amplitude of sinusoid
        * @return void
        */
        void compareSinusoids(float fSampleRate, float fFrequency, float fAmplitude)
        {
            m_pLfo->resetPhase();

            m_pLfo->setParam(CLfo::CLfo::kSampleRate, fSampleRate);
            m_pLfo->setParam(CLfo::CLfo::kFrequency, fFrequency);
            m_pLfo->setParam(CLfo::CLfo::kAmplitude, fAmplitude);

            CSynthesis::generateSine(m_pfSynthesisBuffer, fFrequency, fSampleRate, m_iBufferLength, fAmplitude);

            for (int i = 0; i < m_iBufferLength; i++)
                m_pfLfoBuffer[i] = m_pLfo->process();

            CHECK_ARRAY_CLOSE(m_pfLfoBuffer, m_pfSynthesisBuffer, m_iBufferLength, 1E-3);
        }

        CLfo* m_pLfo = 0;
        float* m_pfLfoBuffer = 0;
        float* m_pfSynthesisBuffer = 0;
        int m_iBufferLength = 1000;
    };

    TEST_F(Lfo, HandlesOutOfBoundsInput)
    {
        EXPECT_EQ(m_pLfo->setParam(CLfo::LfoParam_t::kSampleRate, -1) , Error_t::kFunctionInvalidArgsError);
        EXPECT_EQ(m_pLfo->setParam(CLfo::LfoParam_t::kSampleRate, -11020), Error_t::kFunctionInvalidArgsError);

        EXPECT_EQ(m_pLfo->setParam(CLfo::LfoParam_t::kFrequency, -1), Error_t::kFunctionInvalidArgsError);
        EXPECT_EQ(m_pLfo->setParam(CLfo::LfoParam_t::kFrequency, -28381), Error_t::kFunctionInvalidArgsError);

        EXPECT_EQ(m_pLfo->setParam(CLfo::LfoParam_t::kAmplitude, -2), Error_t::kFunctionInvalidArgsError);
        EXPECT_EQ(m_pLfo->setParam(CLfo::LfoParam_t::kAmplitude, -1293), Error_t::kFunctionInvalidArgsError);
    }

    TEST_F(Lfo, SetParametersCorrectly)
    {
        EXPECT_EQ(m_pLfo->setParam(CLfo::LfoParam_t::kSampleRate, 44100), Error_t::kNoError);
        EXPECT_EQ(m_pLfo->getParam(CLfo::LfoParam_t::kSampleRate), 44100);

        EXPECT_EQ(m_pLfo->setParam(CLfo::LfoParam_t::kFrequency, 440), Error_t::kNoError);
        EXPECT_EQ(m_pLfo->getParam(CLfo::LfoParam_t::kFrequency), 440);

        EXPECT_EQ(m_pLfo->setParam(CLfo::LfoParam_t::kAmplitude, 0.5), Error_t::kNoError);
        EXPECT_EQ(m_pLfo->getParam(CLfo::LfoParam_t::kAmplitude), 0.5);
    }

    TEST_F(Lfo, ReturnCorrectSinusoid)
    {
        // 
        int freqs[10]{ 1,2,3,4,5,6,7,8,9,10 };
        for (int i = 0; i < 10; i++)
            compareSinusoids(44100, freqs[i], 1.0);

        float amps[7]{ 0, 0.25, 0.5, 1.0, 1.5, 2.0, 2.5 };
        for (int i = 0; i < 7; i++)
            compareSinusoids(44100, 440, amps[i]);

        int sampleRates[4]{ 11025, 22050, 44100, 48000};
        for (int i = 0; i < 4; i++)
            compareSinusoids(sampleRates[i], 440, 1.0);
    }


    class RingBuffer : public testing::Test
    {
    protected:
        void SetUp() override
        {
            m_pRingBuffer = new CRingBuffer<float>(m_iMaxBufferSize);
        }

        virtual void TearDown()
        {
            delete m_pRingBuffer;
        }

        CRingBuffer<float>* m_pRingBuffer = 0;
        int m_iMaxBufferSize = 10;
    };

    TEST_F(RingBuffer, WrapAround)
    {
        int expectedValues[] = { 0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9 };
        for (int i = 0; i < m_iMaxBufferSize; i++)
            m_pRingBuffer->putPostInc(static_cast<float>(i));

        for (int i = 0; i < m_iMaxBufferSize * 2; i++)
            EXPECT_EQ(m_pRingBuffer->getPostInc(), expectedValues[i]);
    }

    TEST_F(RingBuffer, OutOfBoundsIndexing)
    {
        for (int i = 0; i < m_iMaxBufferSize; i++)
            m_pRingBuffer->putPostInc(static_cast<float>(i));

        EXPECT_EQ(m_pRingBuffer->get(-1), m_pRingBuffer->get(9));
        EXPECT_EQ(m_pRingBuffer->get(-11), m_pRingBuffer->get(9));
        EXPECT_EQ(m_pRingBuffer->get(10), m_pRingBuffer->get(0));
        EXPECT_EQ(m_pRingBuffer->get(21), m_pRingBuffer->get(1));

        m_pRingBuffer->setReadIdx(-5);
        m_pRingBuffer->setWriteIdx(-5);
        EXPECT_EQ(m_pRingBuffer->getReadIdx(), 5);
        EXPECT_EQ(m_pRingBuffer->getWriteIdx(), 5);

        m_pRingBuffer->setReadIdx(12);
        m_pRingBuffer->setWriteIdx(12);
        EXPECT_EQ(m_pRingBuffer->getReadIdx(), 2);
        EXPECT_EQ(m_pRingBuffer->getWriteIdx(), 2);
    }

    TEST_F(RingBuffer, FractionalIndexing)
    {
        for (int i = 0; i < m_iMaxBufferSize; i++)
            m_pRingBuffer->putPostInc(static_cast<float>(i));

        float indexValues[5] = { 0.5, 3.25, 4.44, 6.43, 8.77 };
        for (const float index : indexValues)
            EXPECT_EQ(m_pRingBuffer->get(index), index);
    }

    TEST_F(RingBuffer, NumValuesInBuffer)
    {
        for (int i = 0; i < 5; i++)
            m_pRingBuffer->putPostInc(1);

        EXPECT_EQ(m_pRingBuffer->getLength(), m_iMaxBufferSize);
        EXPECT_EQ(m_pRingBuffer->getNumValuesInBuffer(), 5);

        for (int i = 0; i < m_iMaxBufferSize; i++)
        {
            m_pRingBuffer->putPostInc(m_pRingBuffer->getPostInc());
            EXPECT_EQ(m_pRingBuffer->getNumValuesInBuffer(), 5);
        }
    }

    TEST_F(RingBuffer, CorrectDelay)
    {
        m_pRingBuffer->put(1);

        m_pRingBuffer->setWriteIdx(5);
        int expectedValues[5]{ 0,0,0,0,1 };
        for (int i = 0; i < 5; i++)
        {
            m_pRingBuffer->putPostInc(m_pRingBuffer->getPostInc());
            EXPECT_EQ(m_pRingBuffer->get(), expectedValues[i]);
        }
    }

}
class TremoloTests: public testing::Test
{
public:
    CTremolo *p_CTremolo=0;
    float** m_ppfInBuffer = 0;
    float** m_ppfOutBuffer = 0;
    int m_iBufferChannels = 2;
    int m_iBufferLength = 400;



    void SetUp( ) {
        CTremolo::create(p_CTremolo);
        m_ppfInBuffer = new float*[m_iBufferChannels];
        m_ppfOutBuffer = new float*[m_iBufferChannels];
        for (int channel = 0; channel < m_iBufferChannels; channel++)
        {
            m_ppfInBuffer[channel] = new float[m_iBufferLength] {};
            m_ppfOutBuffer[channel] = new float[m_iBufferLength] {};
        }

        // code here will execute just before the test ensues
    }

    void TearDown( ) {
            CTremolo::destroy(p_CTremolo);
            for (int channel = 0; channel < m_iBufferChannels; channel++)
            {
                delete[] m_ppfInBuffer[channel];
                delete[] m_ppfOutBuffer[channel];
            }
            delete[] m_ppfInBuffer;
            delete[] m_ppfOutBuffer;
            m_ppfInBuffer = 0;
            m_ppfOutBuffer = 0;
        // code here will be called just after the test completes
        // ok to through exceptions from here if need be
    }
};

#endif //WITH_TESTS
