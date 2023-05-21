#include "MUSI6106Config.h"

#ifdef WITH_TESTS
#include "Synthesis.h"
#include "Vector.h"
#include "Vibrato.h"

#include "gtest/gtest.h"


namespace vibrato_test {
    void CHECK_ARRAY_CLOSE(float* buffer1, float* buffer2, int iLength, float fTolerance)
    {
        for (int i = 0; i < iLength; i++)
        {
            EXPECT_NEAR(buffer1[i], buffer2[i], fTolerance);
        }
    }

    class Vibrato : public testing::Test
    {
    protected:
        void SetUp() override
        {

            CVibrato::create(m_pVibrato);
            m_pVibrato->init(m_fMaxModWidth, m_fSampleRate, m_iNumChannels);

            m_ppfInputData = new float* [m_iNumChannels];
            m_ppfOutputData = new float* [m_iNumChannels];
            m_ppfInputTmp = new float* [m_iNumChannels];
            m_ppfOutputTmp = new float* [m_iNumChannels];
            for (int i = 0; i < m_iNumChannels; i++)
            {
                m_ppfInputData[i] = new float[m_kiDataLength];
                CSynthesis::generateSine(m_ppfInputData[i], 800, m_fSampleRate, m_kiDataLength, .6F, 0);
                m_ppfOutputData[i] = new float[m_kiDataLength];
                CVector::setZero(m_ppfOutputData[i], m_kiDataLength);
            }
        }

        virtual void TearDown()
        {
            for (int i = 0; i < m_iNumChannels; i++)
            {
                delete[] m_ppfOutputData[i];
                delete[] m_ppfInputData[i];
            }
            delete[] m_ppfOutputTmp;
            delete[] m_ppfInputTmp;
            delete[] m_ppfOutputData;
            delete[] m_ppfInputData;

            CVibrato::destroy(m_pVibrato);
        }

        void process ()
        {
            int iNumFramesRemaining = m_kiDataLength;
            while (iNumFramesRemaining > 0)
            {
                int iNumFrames = std::min(iNumFramesRemaining, m_iBlockLength);

                for (int c = 0; c < m_iNumChannels; c++)
                {
                    m_ppfInputTmp[c] = &m_ppfInputData[c][m_kiDataLength - iNumFramesRemaining];
                    m_ppfOutputTmp[c] = &m_ppfOutputData[c][m_kiDataLength - iNumFramesRemaining];
                }
                m_pVibrato->process(m_ppfInputTmp, m_ppfOutputTmp, iNumFrames);

                iNumFramesRemaining -= iNumFrames;
            }
        }

        CVibrato *m_pVibrato = 0;
        float **m_ppfInputData = 0,
            **m_ppfOutputData = 0,
            **m_ppfInputTmp = 0,
            **m_ppfOutputTmp = 0;
        int m_kiDataLength = 35131;
        float m_fMaxModWidth = .5F;
        int m_iBlockLength = 171;
        int m_iNumChannels = 3;
        float m_fSampleRate = 31271;
        float m_fModWidth = .1F,
            m_fModFreq = 1.F;
    };

    TEST_F(Vibrato, VibModWidthZero)
    {
        m_pVibrato->setParam(CVibrato::kParamModFreqInHz, 20);
        m_pVibrato->setParam(CVibrato::kParamModWidthInS, 0);

        process();

        int iDelay = CUtil::float2int<int>(m_fMaxModWidth*m_fSampleRate+1);
        for (int c = 0; c < m_iNumChannels; c++)
            CHECK_ARRAY_CLOSE(m_ppfInputData[c], &m_ppfOutputData[c][iDelay], m_kiDataLength-iDelay, 1e-3F);
    }

    TEST_F(Vibrato, VibDc)
    {
        m_pVibrato->setParam(CVibrato::kParamModFreqInHz, 2);
        m_pVibrato->setParam(CVibrato::kParamModWidthInS, .1F);
        for (int c = 0; c < m_iNumChannels; c++)
        {
            CSynthesis::generateDc(m_ppfInputData[c], m_kiDataLength, (c+1)*.1F);
        }

        process();

        int iDelay = CUtil::float2int<int>(m_fMaxModWidth*m_fSampleRate+1);
        for (int c = 0; c < m_iNumChannels; c++)
            CHECK_ARRAY_CLOSE(m_ppfInputData[c], &m_ppfOutputData[c][iDelay], m_kiDataLength-iDelay, 1e-3F);
    }

    TEST_F(Vibrato, VibVaryingBlocksize)
    {
        m_pVibrato->setParam(CVibrato::kParamModFreqInHz, 2);
        m_pVibrato->setParam(CVibrato::kParamModWidthInS, .1F);

        process();

        m_pVibrato->reset();
        m_pVibrato->init(m_fMaxModWidth,m_fSampleRate,m_iNumChannels);
        m_pVibrato->setParam(CVibrato::kParamModFreqInHz, 2);
        m_pVibrato->setParam(CVibrato::kParamModWidthInS, .1F);
        {
            int iNumFramesRemaining = m_kiDataLength;
            while (iNumFramesRemaining > 0)
            {

                int iNumFrames = std::min(iNumFramesRemaining, static_cast<int>(rand() * 17000. /RAND_MAX));

                for (int c = 0; c < m_iNumChannels; c++)
                {
                    m_ppfInputTmp[c] = &m_ppfInputData[c][m_kiDataLength - iNumFramesRemaining];
                }
                m_pVibrato->process(m_ppfInputTmp, m_ppfInputTmp, iNumFrames);

                iNumFramesRemaining -= iNumFrames;
            }
        }

        for (int c = 0; c < m_iNumChannels; c++)
            CHECK_ARRAY_CLOSE(m_ppfInputData[c], m_ppfOutputData[c], m_kiDataLength, 1e-3F);
    }

    TEST_F(Vibrato, VibParamRange)
    {
        EXPECT_EQ(Error_t::kFunctionInvalidArgsError, m_pVibrato->setParam(CVibrato::kParamModFreqInHz, -1));
        EXPECT_EQ(Error_t::kFunctionInvalidArgsError, m_pVibrato->setParam(CVibrato::kParamModWidthInS, -0.001F));

        EXPECT_EQ(Error_t::kFunctionInvalidArgsError, m_pVibrato->setParam(CVibrato::kParamModFreqInHz, m_fSampleRate));
        EXPECT_EQ(Error_t::kFunctionInvalidArgsError, m_pVibrato->setParam(CVibrato::kParamModWidthInS, m_fMaxModWidth+.1F));
    }

    TEST_F(Vibrato, VibZeroInput)
    {
        m_pVibrato->setParam(CVibrato::kParamModFreqInHz, 20);
        m_pVibrato->setParam(CVibrato::kParamModWidthInS, 0);
        
        for (int c = 0; c < m_iNumChannels; c++)
            CVector::setZero(m_ppfInputData[c], m_kiDataLength);

        process();

        int iDelay = CUtil::float2int<int>(m_fMaxModWidth*m_fSampleRate+1);
        for (int c = 0; c < m_iNumChannels; c++)
            CHECK_ARRAY_CLOSE(m_ppfInputData[c], &m_ppfOutputData[c][iDelay], m_kiDataLength-iDelay, 1e-3F);
    }

}

#endif //WITH_TESTS
