#include "MUSI6106Config.h"

#ifdef WITH_TESTS
#include "Synthesis.h"

#include "Vector.h"
#include "FastConv.h"

#include "gtest/gtest.h"

//TODO: [15] Implement three tests (in Test_FastConv.cpp)
// identity: generate a random IR of length 51 samples and an impulse as input signal at sample index 3; make the input signal
// 10 samples long. Check the correct values of the 10 samples of the output signal.
// flushbuffer: use the same signals as in identity, but now get the full tail of the impulse response and check for correctness
// blocksize: for an input signal of length 10000 samples, implement a test similar to identity with a succession of different
// input/output block sizes (1, 13, 1023, 2048,1,17, 5000, 1897)

//TODO: Duplicate your above tests so they now verify computation in the frequency domain and compensate appropriately for
// latency at different blocksizes.




namespace fastconv_test {
    void CHECK_ARRAY_CLOSE(float* buffer1, float* buffer2, int iLength, float fTolerance)
    {
        for (int i = 0; i < iLength; i++)
        {
            EXPECT_NEAR(buffer1[i], buffer2[i], fTolerance);
        }
    }

    class FastConv : public testing::Test
    {
    protected:
        virtual void SetUp()
        {
            m_pCFastConv = new CFastConv;
            TestImpulse[0]=0;

            for (int i = 1; i < 51; i++)
            {
                TestImpulse[i] = static_cast<float>(std::rand()) / (static_cast <float> (RAND_MAX));
               // TestImpulse[i] = i;//TestImpulse[i] * 2.0 - 1.0;
            }
        }

        virtual void TearDown()
        {
            delete m_pCFastConv;
            m_pCFastConv = 0;
            for (int i = 0; i < 51; i++) {
                TestImpulse[i] = 0;
            }
        }

        CFastConv* m_pCFastConv = 0;
        float TestImpulse[51] = { 0 };


    };


    ////////////////////////////////////////////////////
    // TIME DOMAIN TESTS
    ////////////////////////////////////////////////////
    TEST_F(FastConv, TimeDomainIdentityTest)
    {
        // float TestImpulse[51] = { 0 };
        float TestInput[10] = { 0 };
        float TestOutput[10] = { 0 };
        float CheckOutput[60] = { 0 };
        TestInput[3] = 1;
        for (int i = 0; i < 51; i++)
        {
        //    TestImpulse[i] = static_cast<float>(std::rand()) / (static_cast <float> (RAND_MAX));
          //  TestImpulse[i] = TestImpulse[i] * 2.0 - 1.0;
            CheckOutput[i + 3] = TestImpulse[i];
        }


        m_pCFastConv->init(TestImpulse, 51, 1024, CFastConv::kTimeDomain);
        m_pCFastConv->process(TestOutput, TestInput, 10);


        CHECK_ARRAY_CLOSE(CheckOutput, TestOutput, 10, 1e-3);

    }
    TEST_F(FastConv, TimeDomainFlushBufferTest)
    {
       // float TestImpulse[51] = { 0 };
        float TestInput[10] = { 0 };
        float TestOutput[10] = { 0 };
        float TestFlush[50] = { 0 };
        float CheckOutput[60] = { 0 };
        TestInput[3] = 1;
        for (int i = 0; i < 51; i++)
        {
       //     TestImpulse[i] = static_cast<float>(std::rand()) / (static_cast <float> (RAND_MAX));
         //   TestImpulse[i] = TestImpulse[i] * 2.0 - 1.0;
            CheckOutput[i + 3] = TestImpulse[i];
        }


        m_pCFastConv->init(TestImpulse, 51, 1024, CFastConv::kTimeDomain);
        m_pCFastConv->process(TestOutput, TestInput, 10);
        m_pCFastConv->flushBuffer(TestFlush);

        CHECK_ARRAY_CLOSE(TestOutput, CheckOutput, 10, 1e-3);
        CHECK_ARRAY_CLOSE(TestFlush, TestImpulse + 7, 51 - 7, 1e-3);

    }

    TEST_F(FastConv, TimeDomainBlockSizeTest)
    {
      //  float TestImpulse[51] = { 0 };
        float TestInput[10000] = { 0 };
        float TestOutput[10000] = { 0 };
        int BufferSize[8] = { 1, 13, 1023, 2048, 1, 17, 5000, 1897 };
        int InputStartIdx[8] = { 0 }; // All of the buffersizes add up to 10000, so we can just start the reading of the input at shifted positions


        m_pCFastConv->init(TestImpulse, 51, 1024, CFastConv::kTimeDomain);
        for (int i = 0; i < 8; i++)
        {
            if (i == 0) {
                InputStartIdx[i] = 0;
            }
            else {
                InputStartIdx[i] = InputStartIdx[i-1] + BufferSize[i-1];
            }

            // set all beginnings of new block sizes to 1 for easy testing of convolution
            TestInput[InputStartIdx[i]] = 1;
            // reinitialize the convolution every time bc this is the easiest way to reset the pointers in the IR
            m_pCFastConv->init(TestImpulse, 51, 1024, CFastConv::kTimeDomain);

            m_pCFastConv->process(TestOutput + InputStartIdx[i], TestInput + InputStartIdx[i], BufferSize[i]);
            CHECK_ARRAY_CLOSE(TestOutput + InputStartIdx[i], TestImpulse, std::min(BufferSize[i], 51), 1e-3);

        }

    }

    ////////////////////////////////////////////////////
    // FREQ DOMAIN TESTS
    ////////////////////////////////////////////////////

    TEST_F(FastConv, FreqDomainIdentityTest)
    {
        //float TestImpulse[51] = { 0 };
        float TestInput[128] = { 0 };
        float TestOutput[128] = { 0 };
        float CheckOutput[60] = { 0 };
        TestInput[1] = 1;
        float TestImpulse[64] = {0};

        for (int i = 0; i < 64; ++i) {
            TestImpulse[i] = static_cast<float>(std::rand()) / (static_cast <float> (RAND_MAX));
        }

        m_pCFastConv->init(TestImpulse, 64, 64, CFastConv::kFreqDomain);

        m_pCFastConv->process(TestOutput, TestInput, 128);
        CHECK_ARRAY_CLOSE(TestImpulse, TestOutput + 64 + 1, 63, 1e-3);

    }

    TEST_F(FastConv, FreqDomainFlushIdentifyTest)
    {
        float TestInput[128] = { 0 };
        float TestOutput[128] = { 0 };
        float TestFlush[82] = { 0 };
        TestInput[3] = 1;
        for (int i = 0; i < 51; i++)
        {
            TestImpulse[i] = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        }

        m_pCFastConv->init(TestImpulse, 51, 32, CFastConv::kFreqDomain);
        m_pCFastConv->process(TestOutput, TestInput, 4);
        m_pCFastConv->flushBuffer(TestFlush);

        CHECK_ARRAY_CLOSE(TestFlush + 31, TestImpulse, 51, 1e-3);
    }
//
    TEST_F(FastConv, FreqDomainDifferBlockSize)
    {
        float* TestImpulse = new float[16384]();
        float* TestInput = new float[10000]();
        float* TestOutput = new float[10000]();
        int Impulse_blockSize = 1024;
        //(L-1 + Initial delay)
        float* TestFlush = new float[16383 + Impulse_blockSize];
        long long int fullSize = 16383 + Impulse_blockSize;

        int BlockSizes[8] = { 1, 13, 1023, 2048, 1, 17, 5000, 1897 };
        int StartIdx[8] = { 0 };
        for (int i = 1; i < 8; i++)
        {
            StartIdx[i] = StartIdx[i - 1] + BlockSizes[i - 1];
        }
        TestInput[3] = 1;
        for (int i = 0; i < 16384; i++)
        {
            TestImpulse[i] = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        }
        m_pCFastConv->init(TestImpulse, 16384, 1024, CFastConv::kFreqDomain);

        for (int i = 0; i < 8; i++)
        {
            m_pCFastConv->process(TestOutput + StartIdx[i], TestInput + StartIdx[i], BlockSizes[i]);
            // CHECK_ARRAY_CLOSE(TestOutput + 3 + Impulse_blockSize + StartIdx[i], TestImpulse, BlockSizes[i] - 3 - Impulse_blockSize, 1e-3);
        }
        CHECK_ARRAY_CLOSE(TestOutput + 3 + Impulse_blockSize, TestImpulse, 10000 - 3 - Impulse_blockSize, 1e-6);

        m_pCFastConv->flushBuffer(TestFlush);
        CHECK_ARRAY_CLOSE(TestFlush + Impulse_blockSize, TestImpulse + 10000 - 3, 16384 - (10000 - 3), 1e-3);

        delete[] TestImpulse;
        delete[] TestInput;
        delete[] TestOutput;
        delete[] TestFlush;

    }

    class AlexFastConv: public testing::Test
    {
    protected:
        void SetUp() override
        {
            m_pfInput = new float[m_iInputLength];
            m_pfIr = new float[m_iIRLength];
            m_pfOutput = new float[m_iInputLength + m_iIRLength];

            CVectorFloat::setZero(m_pfInput, m_iInputLength);
            m_pfInput[0] = 1;

            CSynthesis::generateNoise(m_pfIr, m_iIRLength);
            m_pfIr[0] = 1;

            CVectorFloat::setZero(m_pfOutput, m_iInputLength + m_iIRLength);

            m_pCFastConv = new CFastConv();
        }

        virtual void TearDown()
        {
            m_pCFastConv->reset();
            delete m_pCFastConv;

            delete[] m_pfIr;
            delete[] m_pfOutput;
            delete[] m_pfInput;
        }

        float *m_pfInput = 0;
        float *m_pfIr = 0;
        float *m_pfOutput = 0;

        int m_iInputLength = 83099;
        int m_iIRLength = 60001;

        CFastConv *m_pCFastConv = 0;
    };

    TEST_F(AlexFastConv, Params)
    {
        EXPECT_EQ(false, Error_t::kNoError == m_pCFastConv->init(0, 1));
        EXPECT_EQ(false, Error_t::kNoError == m_pCFastConv->init(m_pfIr, 0));
        EXPECT_EQ(false, Error_t::kNoError == m_pCFastConv->init(m_pfIr, 10, -1));
        EXPECT_EQ(false, Error_t::kNoError == m_pCFastConv->init(m_pfIr, 10, 7));
        EXPECT_EQ(true, Error_t::kNoError == m_pCFastConv->init(m_pfIr, 10, 4));
        EXPECT_EQ(true, Error_t::kNoError == m_pCFastConv->reset());
    }

    TEST_F(AlexFastConv, Impulse)
    {
        // impulse with impulse
        int iBlockLength = 4;
        m_pCFastConv->init(m_pfIr, 1, iBlockLength);

        for (auto i = 0; i < 500; i++)
            m_pCFastConv->process(&m_pfOutput[i], &m_pfInput[i], 1);

        EXPECT_NEAR(1.F, m_pfOutput[iBlockLength], 1e-6F);
        EXPECT_NEAR(0.F, CVectorFloat::getMin(m_pfOutput, m_iInputLength), 1e-6F);
        EXPECT_NEAR(1.F, CVectorFloat::getMax(m_pfOutput, m_iInputLength), 1e-6F);

        // impulse with dc
        for (auto i = 0; i < 4; i++)
            m_pfOutput[i] = 1;
        iBlockLength = 8;
        m_pCFastConv->init(m_pfOutput, 4, iBlockLength);

        for (auto i = 0; i < 500; i++)
            m_pCFastConv->process(&m_pfOutput[i], &m_pfInput[i], 1);

        EXPECT_NEAR(0.F, CVectorFloat::getMean(m_pfOutput, 8), 1e-6F);
        EXPECT_NEAR(1.F, CVectorFloat::getMean(&m_pfOutput[8], 4), 1e-6F);
        EXPECT_NEAR(0.F, CVectorFloat::getMean(&m_pfOutput[12], 400), 1e-6F);

        // impulse with noise
        iBlockLength = 8;
        m_pCFastConv->init(m_pfIr, 27, iBlockLength);

        for (auto i = 0; i < m_iInputLength; i++)
            m_pCFastConv->process(&m_pfOutput[i], &m_pfInput[i], 1);

        CHECK_ARRAY_CLOSE(m_pfIr, &m_pfOutput[iBlockLength], 27, 1e-6F);
        CHECK_ARRAY_CLOSE(&m_pfInput[1], &m_pfOutput[iBlockLength + 27], 10, 1e-6F);

        // noise with impulse
        iBlockLength = 8;
        m_pCFastConv->init(m_pfInput, 27, iBlockLength);

        for (auto i = 0; i < m_iIRLength; i++)
            m_pCFastConv->process(&m_pfOutput[i], &m_pfIr[i], 1);

        CHECK_ARRAY_CLOSE(m_pfIr, &m_pfOutput[iBlockLength], m_iIRLength - iBlockLength, 1e-6F);
    }
    TEST_F(AlexFastConv, ImpulseTime)
    {
        // impulse with impulse
        int iBlockLength = 4;
        m_pCFastConv->init(m_pfIr, 1, iBlockLength, CFastConv::kTimeDomain);

        for (auto i = 0; i < 500; i++)
            m_pCFastConv->process(&m_pfOutput[i], &m_pfInput[i], 1);

        EXPECT_NEAR(1.F, m_pfOutput[0], 1e-6F);
        EXPECT_NEAR(0.F, CVectorFloat::getMin(m_pfOutput, m_iInputLength), 1e-6F);
        EXPECT_NEAR(1.F, CVectorFloat::getMax(m_pfOutput, m_iInputLength), 1e-6F);

        // impulse with dc
        for (auto i = 0; i < 4; i++)
            m_pfOutput[i] = 1;
        iBlockLength = 8;
        m_pCFastConv->init(m_pfOutput, 4, iBlockLength, CFastConv::kTimeDomain);

        for (auto i = 0; i < 500; i++)
            m_pCFastConv->process(&m_pfOutput[i], &m_pfInput[i], 1);

        EXPECT_NEAR(1.F, CVectorFloat::getMean(&m_pfOutput[0], 4), 1e-6F);
        EXPECT_NEAR(0.F, CVectorFloat::getMean(&m_pfOutput[4], 400), 1e-6F);

        // impulse with noise
        iBlockLength = 8;
        m_pCFastConv->init(m_pfIr, 27, iBlockLength, CFastConv::kTimeDomain);

        for (auto i = 0; i < m_iInputLength; i++)
            m_pCFastConv->process(&m_pfOutput[i], &m_pfInput[i], 1);

        CHECK_ARRAY_CLOSE(m_pfIr, &m_pfOutput[0], 27, 1e-6F);
        CHECK_ARRAY_CLOSE(&m_pfInput[1], &m_pfOutput[27], 10, 1e-6F);

        // noise with impulse
        iBlockLength = 8;
        m_pCFastConv->init(m_pfInput, 27, iBlockLength, CFastConv::kTimeDomain);

        for (auto i = 0; i < m_iIRLength; i++)
            m_pCFastConv->process(&m_pfOutput[i], &m_pfIr[i], 1);

        CHECK_ARRAY_CLOSE(m_pfIr, &m_pfOutput[0], m_iIRLength , 1e-6F);
    }

    TEST_F(AlexFastConv, BlockLengths)
    {
        // impulse with noise
        int iBlockLength = 4;

        for (auto j = 0; j < 10; j++)
        {
            m_pCFastConv->init(m_pfIr, 51, iBlockLength);

            for (auto i = 0; i < m_iInputLength; i++)
                m_pCFastConv->process(&m_pfOutput[i], &m_pfInput[i], 1);

            CHECK_ARRAY_CLOSE(m_pfIr, &m_pfOutput[iBlockLength], 51 - iBlockLength, 1e-6F);

            iBlockLength <<= 1;
        }
    }

    TEST_F(AlexFastConv, InputLengths)
    {
        // impulse with noise
        int iBlockLength = 4096;

        int iCurrIdx = 0,
            aiInputLength[] = {
            4095,
            17,
            32157,
            99,
            4097,
            1,
            42723
        };

        m_pCFastConv->init(m_pfIr, m_iIRLength, iBlockLength);

        for (auto i = 0; i < 7; i++)
        {
            m_pCFastConv->process(&m_pfOutput[iCurrIdx], &m_pfInput[iCurrIdx], aiInputLength[i]);
            iCurrIdx += aiInputLength[i];
        }

        CHECK_ARRAY_CLOSE(m_pfIr, &m_pfOutput[iBlockLength], m_iIRLength, 1e-6F);
        EXPECT_NEAR(0.F, CVectorFloat::getMean(&m_pfOutput[m_iIRLength + iBlockLength], 10000), 1e-6F);

    }

    TEST_F(AlexFastConv, FlushBuffer)
    {
        // impulse with noise
        int iBlockLength = 8;
        int iIrLength = 27;

        CVectorFloat::setZero(m_pfOutput, m_iInputLength + m_iIRLength);
        m_pCFastConv->init(m_pfIr, iIrLength, iBlockLength);

        m_pCFastConv->process(m_pfOutput, m_pfInput, 1);

        m_pCFastConv->flushBuffer(&m_pfOutput[1]);

        EXPECT_NEAR(0.F, CVectorFloat::getMean(m_pfOutput, iBlockLength), 1e-6F);
        CHECK_ARRAY_CLOSE(m_pfIr, &m_pfOutput[iBlockLength], iIrLength, 1e-6F);

        // same for time domain
        CVectorFloat::setZero(m_pfOutput, m_iInputLength + m_iIRLength);
        m_pCFastConv->init(m_pfIr, iIrLength, iBlockLength,CFastConv::kTimeDomain);

        m_pCFastConv->process(m_pfOutput, m_pfInput, 1);

        m_pCFastConv->flushBuffer(&m_pfOutput[1]);

        CHECK_ARRAY_CLOSE(m_pfIr, &m_pfOutput[0], iIrLength, 1e-6F);
    }
}

#endif //WITH_TESTS

