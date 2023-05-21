#include "MUSI6106Config.h"

#ifdef WITH_TESTS
#include <cassert>

#include "Synthesis.h"
#include "Vector.h"
#include "CombFilterIf.h"
#include <algorithm>

#include "gtest/gtest.h"


namespace combfilter_test {
    void CHECK_ARRAY_CLOSE(float* buffer1, float* buffer2, int iLength, float fTolerance)
    {
        for (int i = 0; i < iLength; i++)
        {
            EXPECT_NEAR(buffer1[i], buffer2[i], fTolerance);
        }
    }

    class CombFilter : public testing::Test
    {
    protected:
        void SetUp() override
        {
            m_pCombFilter = 0;
            m_ppfInputData = 0;
            m_ppfOutputData = 0;
            m_iDataLength = 35131;
            m_fMaxDelayLength = 3.F;
            m_iBlockLength = 171; 
            m_iNumChannels = 3; 
            m_fSampleRate = 8000;
            m_fDelay = 0.1F; 
            m_fGain = 0.5F;

            CCombFilterIf::create(m_pCombFilter);

            m_ppfInputData = new float* [m_iNumChannels];
            m_ppfOutputData = new float* [m_iNumChannels];
            m_ppfInputTmp = new float* [m_iNumChannels];
            m_ppfOutputTmp = new float* [m_iNumChannels];
            for (int i = 0; i < m_iNumChannels; i++)
            {
                m_ppfInputData[i] = new float[m_iDataLength];
                CVectorFloat::setZero(m_ppfInputData[i], m_iDataLength);
                m_ppfOutputData[i] = new float[m_iDataLength];
                CVectorFloat::setZero(m_ppfOutputData[i], m_iDataLength);
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

            CCombFilterIf::destroy(m_pCombFilter);
        }

        void TestProcess() 
        {
            int iNumFramesRemaining = m_iDataLength;
            while (iNumFramesRemaining > 0)
            {
                int iNumFrames = std::min(iNumFramesRemaining, m_iBlockLength);

                for (int c = 0; c < m_iNumChannels; c++)
                {
                    m_ppfInputTmp[c]    = &m_ppfInputData[c][m_iDataLength - iNumFramesRemaining];
                    m_ppfOutputTmp[c]   = &m_ppfOutputData[c][m_iDataLength - iNumFramesRemaining];
                }
                m_pCombFilter->process(m_ppfInputTmp, m_ppfOutputTmp, iNumFrames);

                iNumFramesRemaining -= iNumFrames;
            }
        }
        void TestProcessInplace() 
        {
            int iNumFramesRemaining = m_iDataLength;
            while (iNumFramesRemaining > 0)
            {
                int iNumFrames = std::min(iNumFramesRemaining, m_iBlockLength);

                for (int c = 0; c < m_iNumChannels; c++)
                {
                    m_ppfInputTmp[c]    = &m_ppfInputData[c][m_iDataLength - iNumFramesRemaining];
                }
                m_pCombFilter->process(m_ppfInputTmp, m_ppfInputTmp, iNumFrames);

                iNumFramesRemaining -= iNumFrames;
            }
        }



        CCombFilterIf  *m_pCombFilter = 0;
        float       **m_ppfInputData = 0,
                    **m_ppfOutputData = 0,
                    **m_ppfInputTmp = 0,
                    **m_ppfOutputTmp = 0;
        int     m_iDataLength = 0;
        float   m_fMaxDelayLength = 0;
        int     m_iBlockLength = 0;
        int     m_iNumChannels = 0;
        float   m_fSampleRate = 0;
        float   m_fDelay = 0,
                m_fGain = 0;

    };

    TEST_F(CombFilter, ZeroInput)
    {
        //FIR
        m_pCombFilter->init(CCombFilterIf::kCombFIR, m_fMaxDelayLength, m_fSampleRate, m_iNumChannels);
        m_pCombFilter->setParam(CCombFilterIf::kParamGain, m_fGain);
        m_pCombFilter->setParam(CCombFilterIf::kParamDelay, m_fDelay);

        TestProcess();

        for (int c = 0; c < m_iNumChannels; c++)
            CHECK_ARRAY_CLOSE(m_ppfInputData[c], m_ppfOutputData[c], m_iDataLength, 1e-3F);

        m_pCombFilter->reset();

        //IIR
        m_pCombFilter->init(CCombFilterIf::kCombIIR, m_fMaxDelayLength, m_fSampleRate, m_iNumChannels);
        m_pCombFilter->setParam(CCombFilterIf::kParamGain, m_fGain);
        m_pCombFilter->setParam(CCombFilterIf::kParamDelay, m_fDelay);

        TestProcess();

        for (int c = 0; c < m_iNumChannels; c++)
            CHECK_ARRAY_CLOSE(m_ppfInputData[c], m_ppfOutputData[c], m_iDataLength, 1e-3F);
    }

    TEST_F(CombFilter, SetGetParam)
    {
       //different call orders
       m_pCombFilter->init(CCombFilterIf::kCombFIR, m_fMaxDelayLength, m_fSampleRate, m_iNumChannels);
       m_pCombFilter->setParam(CCombFilterIf::kParamGain, m_fGain);
       EXPECT_NEAR(m_fGain, m_pCombFilter->getParam(CCombFilterIf::kParamGain), 1e-3);

       m_pCombFilter->setParam(CCombFilterIf::kParamDelay, m_fDelay);
       EXPECT_NEAR(m_fDelay, m_pCombFilter->getParam(CCombFilterIf::kParamDelay), 1e-3);
       EXPECT_NEAR(m_fGain, m_pCombFilter->getParam(CCombFilterIf::kParamGain), 1e-3);
       
       m_pCombFilter->setParam(CCombFilterIf::kParamGain, m_fGain+.1F);
       EXPECT_NEAR(m_fGain+.1F, m_pCombFilter->getParam(CCombFilterIf::kParamGain), 1e-3);
       EXPECT_NEAR(m_fDelay, m_pCombFilter->getParam(CCombFilterIf::kParamDelay), 1e-3);

       // illegal values
       EXPECT_EQ(false, Error_t::kNoError == m_pCombFilter->setParam(CCombFilterIf::kParamDelay, m_fMaxDelayLength + 100));
       EXPECT_EQ(false, Error_t::kNoError == m_pCombFilter->setParam(CCombFilterIf::kParamDelay, -m_fDelay));

       // valid but potentially unexpected
       EXPECT_EQ(Error_t::kNoError, m_pCombFilter->setParam(CCombFilterIf::kParamGain, -5.F));
    }

    TEST_F(CombFilter, FirCancellation)
    {
        m_pCombFilter->init(CCombFilterIf::kCombFIR, m_fMaxDelayLength, m_fSampleRate, m_iNumChannels);
        m_pCombFilter->setParam(CCombFilterIf::kParamGain, m_fGain);
        m_pCombFilter->setParam(CCombFilterIf::kParamDelay, m_fDelay);
        
        // full period length
        for (int c = 0; c < m_iNumChannels; c++)
            CSynthesis::generateSine (m_ppfInputData[c], 1.F/m_fDelay, m_fSampleRate, m_iDataLength, .8F, static_cast<float>(c*M_PI_2));
        m_pCombFilter->setParam(CCombFilterIf::kParamGain, -1.F);

        TestProcess();

        for (int c = 0; c < m_iNumChannels; c++)
            for (int i = static_cast<int>(m_fDelay*m_fSampleRate+.5F); i < m_iDataLength; i++)
                EXPECT_NEAR(0.F, m_ppfOutputData[c][i], 1e-3F);

        // half period length
        m_pCombFilter->reset();
        m_pCombFilter->init(CCombFilterIf::kCombFIR, m_fMaxDelayLength, m_fSampleRate, m_iNumChannels);
        for (int c = 0; c < m_iNumChannels; c++)
            CSynthesis::generateSine (m_ppfInputData[c], .5F/m_fDelay, m_fSampleRate, m_iDataLength, .8F, static_cast<float>(c*M_PI_2));
        m_pCombFilter->setParam(CCombFilterIf::kParamGain, 1.F);
        m_pCombFilter->setParam(CCombFilterIf::kParamDelay, m_fDelay);

        TestProcess();

        for (int c = 0; c < m_iNumChannels; c++)
            for (int i = static_cast<int>(m_fDelay*m_fSampleRate+.5F); i < m_iDataLength; i++)
                EXPECT_NEAR(0.F, m_ppfOutputData[c][i], 1e-3F);
    }

    TEST_F(CombFilter, FilterOutput)
    {
        //m_fDelay = 2/m_fSampleRate;
        int iDelayInSamples = static_cast<int>(m_fDelay*m_fSampleRate + .5F);
        float fAmplitude    = .77F;

        //dirac
        for (int c = 0; c < m_iNumChannels; c++)
            m_ppfInputData[c][c]    = fAmplitude;

        //FIR
        m_pCombFilter->init(CCombFilterIf::kCombFIR, m_fMaxDelayLength, m_fSampleRate, m_iNumChannels);
        m_pCombFilter->setParam(CCombFilterIf::kParamGain, m_fGain);
        m_pCombFilter->setParam(CCombFilterIf::kParamDelay, m_fDelay);

        TestProcess();

        for (int c = 0; c < m_iNumChannels; c++)
        {
            for (int i = 0; i < m_iDataLength; i++)
            {
                if (i == c)
                    EXPECT_NEAR(fAmplitude, m_ppfOutputData[c][i], 1e-3F);
                else if (i == c + iDelayInSamples)
                    EXPECT_NEAR(fAmplitude * m_fGain, m_ppfOutputData[c][i], 1e-3F);
                else
                    EXPECT_NEAR(0.F, m_ppfOutputData[c][i], 1e-3F);
            }
        }

        m_pCombFilter->reset();

        //IIR
        m_pCombFilter->init(CCombFilterIf::kCombIIR, m_fMaxDelayLength, m_fSampleRate, m_iNumChannels);
        m_pCombFilter->setParam(CCombFilterIf::kParamGain, m_fGain);
        m_pCombFilter->setParam(CCombFilterIf::kParamDelay, m_fDelay);

        TestProcess();

        for (int c = 0; c < m_iNumChannels; c++)
        {
            for (int i= 0; i < m_iDataLength; i++)
            {
                if (i == c)
                    EXPECT_NEAR(fAmplitude, m_ppfOutputData[c][i], 1e-3F);
                else if (i == c+iDelayInSamples)
                    EXPECT_NEAR(fAmplitude*m_fGain, m_ppfOutputData[c][i], 1e-3F);
                else if (i == c+2*iDelayInSamples)
                    EXPECT_NEAR(fAmplitude*m_fGain*m_fGain, m_ppfOutputData[c][i], 1e-3F);
                else if (i == c+3*iDelayInSamples)
                    EXPECT_NEAR(fAmplitude*m_fGain*m_fGain*m_fGain, m_ppfOutputData[c][i], 1e-3F);
                else if (i == c+4*iDelayInSamples)
                    EXPECT_NEAR(fAmplitude*m_fGain*m_fGain*m_fGain*m_fGain, m_ppfOutputData[c][i], 1e-3F);
                else if (i == c+5*iDelayInSamples)
                    EXPECT_NEAR(fAmplitude*m_fGain*m_fGain*m_fGain*m_fGain*m_fGain, m_ppfOutputData[c][i], 1e-3F);
                else if ((i-c)%(iDelayInSamples) !=0)
                    EXPECT_NEAR(0.F, m_ppfOutputData[c][i], 1e-3F);
            }
        }
    }

    TEST_F(CombFilter, Inplace)
    {
        //Fir
        m_pCombFilter->init(CCombFilterIf::kCombFIR, m_fMaxDelayLength, m_fSampleRate, m_iNumChannels);

        for (int c = 0; c < m_iNumChannels; c++)
            CSynthesis::generateSine (m_ppfInputData[c], 387.F, m_fSampleRate, m_iDataLength, .8F, static_cast<float>(c*M_PI_2));
        m_pCombFilter->setParam(CCombFilterIf::kParamGain, m_fGain);
        m_pCombFilter->setParam(CCombFilterIf::kParamDelay, m_fDelay);

        TestProcess();
        m_pCombFilter->reset();
        m_pCombFilter->init(CCombFilterIf::kCombFIR, m_fMaxDelayLength, m_fSampleRate, m_iNumChannels);
        m_pCombFilter->setParam(CCombFilterIf::kParamGain, m_fGain);
        m_pCombFilter->setParam(CCombFilterIf::kParamDelay, m_fDelay);
        TestProcessInplace();

        for (int c = 0; c < m_iNumChannels; c++)
            CHECK_ARRAY_CLOSE(m_ppfInputData[c], m_ppfOutputData[c], m_iDataLength, 1e-3F);

        m_pCombFilter->reset();

        // Iir
        m_pCombFilter->init(CCombFilterIf::kCombIIR, m_fMaxDelayLength, m_fSampleRate, m_iNumChannels);
        for (int c = 0; c < m_iNumChannels; c++)
            CSynthesis::generateSine (m_ppfInputData[c], 387.F, m_fSampleRate, m_iDataLength, .8F, static_cast<float>(c*M_PI_2));
        m_pCombFilter->setParam(CCombFilterIf::kParamGain, m_fGain);
        m_pCombFilter->setParam(CCombFilterIf::kParamDelay, m_fDelay);

        TestProcess();
        m_pCombFilter->reset();
        m_pCombFilter->init(CCombFilterIf::kCombIIR, m_fMaxDelayLength, m_fSampleRate, m_iNumChannels);
        m_pCombFilter->setParam(CCombFilterIf::kParamGain, m_fGain);
        m_pCombFilter->setParam(CCombFilterIf::kParamDelay, m_fDelay);
        TestProcessInplace();

        for (int c = 0; c < m_iNumChannels; c++)
            CHECK_ARRAY_CLOSE(m_ppfInputData[c], m_ppfOutputData[c], m_iDataLength, 1e-3F);
    }
    TEST_F(CombFilter, VaryingBlocksize)
    {
        //Fir
        m_pCombFilter->init(CCombFilterIf::kCombFIR, m_fMaxDelayLength, m_fSampleRate, m_iNumChannels);

        for (int c = 0; c < m_iNumChannels; c++)
            CSynthesis::generateSine(m_ppfInputData[c], 387.F, m_fSampleRate, m_iDataLength, .8F, static_cast<float>(c * M_PI_2));
        m_pCombFilter->setParam(CCombFilterIf::kParamGain, m_fGain);
        m_pCombFilter->setParam(CCombFilterIf::kParamDelay, m_fDelay);

        TestProcess();

        m_pCombFilter->reset();
        m_pCombFilter->init(CCombFilterIf::kCombFIR, m_fMaxDelayLength, m_fSampleRate, m_iNumChannels);
        m_pCombFilter->setParam(CCombFilterIf::kParamGain, m_fGain);
        m_pCombFilter->setParam(CCombFilterIf::kParamDelay, m_fDelay);

        float** ppfTmp = new float* [m_iNumChannels];
        for (int c = 0; c < m_iNumChannels; c++)
            ppfTmp[c] = new float[17000];
        {
            int iNumFramesRemaining = m_iDataLength;

            while (iNumFramesRemaining > 0)
            {

                int iNumFrames = static_cast<int>(std::min(static_cast<float>(iNumFramesRemaining), static_cast<float>(rand()) / RAND_MAX * 17000.F));

                for (int c = 0; c < m_iNumChannels; c++)
                {
                    m_ppfInputTmp[c] = &m_ppfInputData[c][m_iDataLength - iNumFramesRemaining];
                }
                m_pCombFilter->process(m_ppfInputTmp, ppfTmp, iNumFrames);
                for (int c = 0; c < m_iNumChannels; c++)
                    CVectorFloat::copy(m_ppfInputTmp[c], ppfTmp[c], iNumFrames);

                iNumFramesRemaining -= iNumFrames;
            }
        }

        for (int c = 0; c < m_iNumChannels; c++)
            CHECK_ARRAY_CLOSE(m_ppfInputData[c], m_ppfOutputData[c], m_iDataLength, 1e-3F);

        //Iir
        m_pCombFilter->reset();
        m_pCombFilter->init(CCombFilterIf::kCombIIR, m_fMaxDelayLength, m_fSampleRate, m_iNumChannels);

        for (int c = 0; c < m_iNumChannels; c++)
            CSynthesis::generateSine(m_ppfInputData[c], 387.F, m_fSampleRate, m_iDataLength, .8F, static_cast<float>(c * M_PI_2));
        m_pCombFilter->setParam(CCombFilterIf::kParamGain, m_fGain);
        m_pCombFilter->setParam(CCombFilterIf::kParamDelay, m_fDelay);

        TestProcess();

        m_pCombFilter->reset();
        m_pCombFilter->init(CCombFilterIf::kCombIIR, m_fMaxDelayLength, m_fSampleRate, m_iNumChannels);
        m_pCombFilter->setParam(CCombFilterIf::kParamGain, m_fGain);
        m_pCombFilter->setParam(CCombFilterIf::kParamDelay, m_fDelay);
        {
            int iNumFramesRemaining = m_iDataLength;
            while (iNumFramesRemaining > 0)
            {

                int iNumFrames = static_cast<int>(std::min(static_cast<float>(iNumFramesRemaining), static_cast<float>(rand()) / RAND_MAX * 17000.F));

                for (int c = 0; c < m_iNumChannels; c++)
                {
                    m_ppfInputTmp[c] = &m_ppfInputData[c][m_iDataLength - iNumFramesRemaining];
                }
                m_pCombFilter->process(m_ppfInputTmp, ppfTmp, iNumFrames);
                for (int c = 0; c < m_iNumChannels; c++)
                    CVectorFloat::copy(m_ppfInputTmp[c], ppfTmp[c], iNumFrames);

                iNumFramesRemaining -= iNumFrames;
            }
        }

        for (int c = 0; c < m_iNumChannels; c++)
            CHECK_ARRAY_CLOSE(m_ppfInputData[c], m_ppfOutputData[c], m_iDataLength, 1e-3F);

        for (int c = 0; c < m_iNumChannels; c++)
            delete[]ppfTmp[c];
        delete[] ppfTmp;
    }
}

#endif //WITH_TESTS

