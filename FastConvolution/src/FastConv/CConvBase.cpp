//
// Created by Noel  Alben on 4/28/22.
//


#include "CConvbase.h"
#include "Vector.h"
#include "Util.h"

CConvbase::CConvbase() {
}
CConvbase::~CConvbase() {
}

Error_t CConvbase::reset()
{
    m_iIrLength =0;
    m_iBlockLength = 0;
    m_bIsInitialized = false;

    return Error_t::kNoError;

}

Error_t CConvTime::init(float *pfImpulseResponse, int iLengthOfIr, int iBlockLength)
{
    if (iLengthOfIr<= 0)
        return Error_t::kFunctionInvalidArgsError;
    if(!pfImpulseResponse)
        return Error_t::kFunctionInvalidArgsError;
    this->reset();

    m_iIrLength = iLengthOfIr;
    m_iBlockLength = iBlockLength;
    long long int l_iLenghtOfIR;
    l_iLenghtOfIR = m_iIrLength;


    m_pCRingBuff = new CRingBuffer<float>(m_iIrLength);
    m_pfImpulseResponse = new float[m_iIrLength];
    CVectorFloat::copy(m_pfImpulseResponse, pfImpulseResponse,l_iLenghtOfIR);

    m_bIsInitialized = true;
    return Error_t::kNoError;

}

Error_t CConvTime::reset()
{
    delete[] m_pfImpulseResponse;
    m_pfImpulseResponse = 0;
    delete m_pCRingBuff;
    m_pCRingBuff = 0;

    return Error_t::kNoError;

}
Error_t CConvTime::process(float *pfOutputBuffer, const float *pfInputBuffer, int iLengthOfBuffers)
{

    if (!m_bIsInitialized)
        return Error_t::kFunctionIllegalCallError;

    if (!pfInputBuffer)
        return Error_t::kFunctionInvalidArgsError;

    if (!pfOutputBuffer)
        return Error_t::kFunctionInvalidArgsError;

    if (iLengthOfBuffers <= 0)
        return Error_t::kFunctionInvalidArgsError;

        CVectorFloat::setZero(pfOutputBuffer,static_cast<long long int>(iLengthOfBuffers));
        for (int i = 0; i < iLengthOfBuffers; i++)
        {
            //Add input to the ring buffer in circular manner
            m_pCRingBuff->putPostInc(pfInputBuffer[i]);
            float acc = 0;
            for (int j =0; j < m_iIrLength; j++)
            {
                //fill the accumulator and read the ring buffer backwards for the flip
                acc += m_pfImpulseResponse[j] * m_pCRingBuff->get(-j);
            }
            pfOutputBuffer[i]=acc;
            m_pCRingBuff->getPostInc();
            assert(m_pCRingBuff->getReadIdx() == m_pCRingBuff->getWriteIdx());
        }
    return Error_t::kNoError;
    }

Error_t CConvTime::flushBuffer(float* pfOutputBuffer)
{
    float* pfFlushInputBuffer = new float[m_iIrLength- 1]();
    CVectorFloat::setZero(pfOutputBuffer, m_iIrLength-1);
    process(pfOutputBuffer, pfFlushInputBuffer, m_iIrLength - 1);
    delete[] pfFlushInputBuffer;

    return Error_t::kNoError;
}

Error_t CConvFFT::init(float *pfImpulseResponse, int iLengthOfIr, int iBlockLength)
{
    if(!CUtil::isPowOf2(iBlockLength) || iBlockLength<=0)
        return Error_t::kFunctionInvalidArgsError;
    if (iLengthOfIr<=0)
        return Error_t::kFunctionInvalidArgsError;
    if(!pfImpulseResponse)
        return Error_t::kFunctionInvalidArgsError;
    this->reset();
    //Initialize fft

    m_iBlockLength=iBlockLength;
    m_iFftLength = iBlockLength << 1;
    m_iIrLength = iLengthOfIr;
    m_iIRBlockNum = static_cast<int>(std::ceil(static_cast<float>(m_iIrLength) / static_cast<float>(m_iBlockLength)));
    CFft::createInstance(m_pcFFT);
    m_pcFFT->initInstance(m_iFftLength, 1, CFft::kWindowHann,CFft::kNoWindow);
    m_iFftLength=m_pcFFT->getLength(CFft::kLengthFft);
    // Inputbuffer processor memory
    pfFFTRealTemp = new float[m_iBlockLength + 1]();
    pfFFTImagTemp = new float[m_iBlockLength + 1]();
    pfCurrentBlockFFTReal = new float[m_iBlockLength + 1]();
    pfCurrentBlockFFTImag = new float[m_iBlockLength + 1]();
    m_pfProcessedBlockBuffer = new float [(m_iIRBlockNum + 1) * m_iBlockLength]();
    m_pfInputBlockBuffer = new float[m_iFftLength]();

    m_iTailLength = m_iBlockLength * m_iIRBlockNum + m_iBlockLength;
    m_pfTail = new float[m_iTailLength]();

    // Memory required for Impulse Response
    m_pfImpulseResponse=pfImpulseResponse;
    pfComplexTemp = new CFft::complex_t[m_iFftLength];
    pfIRTemp = new float[m_iFftLength]();
    pfIFFTTemp= new float [m_iFftLength]();
    m_ppfIRFreqDomainReal = new float* [m_iIRBlockNum];
    m_ppfIRFreqDomainImag = new float* [m_iIRBlockNum];
    for(int i=0;i<m_iIRBlockNum;i++) {
        //Impulse Processing
        //Fill IRtemp with zeros and then move blocklength of Impulseresponse into IRTemp, IR temp is now 2blockLength
        for (int j = 0; j < m_iBlockLength; j++) {
            if (i * iBlockLength + j < m_iIrLength)
                pfIRTemp[j] = pfImpulseResponse[i * iBlockLength + j];
            else
                pfIRTemp[j] = 0;
        }
        //Creat real and Imaginary memory for IR
        m_ppfIRFreqDomainReal[i] = new float[m_iBlockLength + 1]();
        m_ppfIRFreqDomainImag[i] = new float[m_iBlockLength + 1]();
        m_pcFFT->doFft(pfComplexTemp, pfIRTemp);
        m_pcFFT->splitRealImag(m_ppfIRFreqDomainReal[i], m_ppfIRFreqDomainImag[i], pfComplexTemp);
    }
    m_bIsInitialized=true;
    return Error_t::kNoError;
}

Error_t CConvFFT::reset()
{
    if(!m_bIsInitialized)
        return Error_t::kNotInitializedError;
    for (int i =0; i<m_iBlockLength;i++)
    {
        delete[] m_ppfIRFreqDomainReal[i];
        delete[] m_ppfIRFreqDomainImag[i];
    }
    delete[] m_ppfIRFreqDomainImag;
    delete[] m_ppfIRFreqDomainReal;
    delete[] m_pfInputBlockBuffer;
    delete[] m_pfProcessedBlockBuffer;

    m_ppfIRFreqDomainReal=0;
    m_ppfIRFreqDomainImag=0;
    m_pfProcessedBlockBuffer=0;
    m_pfInputBlockBuffer=0;

    delete[] pfIRTemp;
    delete[] pfFFTRealTemp;
    delete[] pfFFTImagTemp;
    delete[] pfCurrentBlockFFTImag;
    delete[] pfCurrentBlockFFTReal;
    delete[] pfIFFTTemp;
    delete[] m_pfImpulseResponse;
    delete[] m_pfTail;
    m_pfTail=0;
    m_pfImpulseResponse=0;
    pfIFFTTemp=0;
    pfIRTemp=0;
    pfFFTImagTemp=0;
    pfFFTRealTemp=0;
    pfCurrentBlockFFTReal=0;
    pfCurrentBlockFFTImag=0;

    delete[] pfComplexTemp;
    pfComplexTemp = 0;

    CFft::destroyInstance(m_pcFFT);
    m_pcFFT=0;

    return Error_t::kNoError;

}

Error_t CConvFFT::process(float *pfOutputBuffer, const float *pfInputBuffer, int iLengthBuffers)
{
    if (!m_bIsInitialized)
        return Error_t::kFunctionIllegalCallError;

    if (!pfInputBuffer)
        return Error_t::kFunctionInvalidArgsError;

    if (!pfOutputBuffer)
        return Error_t::kFunctionInvalidArgsError;

    if (iLengthBuffers <= 0)
        return Error_t::kFunctionInvalidArgsError;

    const int mod = (m_iIRBlockNum + 1) * m_iBlockLength;

    // fill the inputprocessing buffer until the minimum length (blockLength)
    for (int i=0; i<iLengthBuffers;i++)
    {
        if(!m_iInputWriteIdx) // one block full
        {
            //Perform FFT on the blocked Input Buffer
            m_pcFFT->doFft(pfComplexTemp, m_pfInputBlockBuffer);
            //Split the Real and Imaginary parts
            m_pcFFT->splitRealImag(pfCurrentBlockFFTReal, pfCurrentBlockFFTImag, pfComplexTemp);
            //Iterate through all the IR blocks
            for (int iIrBlock = 0; iIrBlock < m_iIRBlockNum; iIrBlock++)
            {
                //perform complex multiplication
                complexMultiplication(pfFFTRealTemp, pfFFTImagTemp,
                                      pfCurrentBlockFFTReal, pfCurrentBlockFFTImag,
                                      m_ppfIRFreqDomainReal[iIrBlock], m_ppfIRFreqDomainImag[iIrBlock]);
                //Reusing complexTemp: Merge Real and Imaginary parts
                m_pcFFT->mergeRealImag(pfComplexTemp, pfFFTRealTemp, pfFFTImagTemp);

                m_pcFFT->doInvFft(pfIFFTTemp, pfComplexTemp);
                //Overlap and add into the ptocessedBlock buffer
                //for h1,h2,h3,h4
                // --> x1*h1 -->WriteIdx, ReadIdx   --> 0+x2*h4                                 --> x2*h4+x3*h3                                     -->x2*h4+x3*h3+x4*h2
                //     x1*h2                            x1*h2+x2*h1 --> WriteBlockIdx, ReadIdx      0+x3*h4                                            x3*h4+x4*h3
                //     x1*h3                            x1*h3+x2*h2                                 x1*h3+x2*h2+x3*h1 -->WriteBlocIdx, ReadBlockIdx    0+x4*h4
                //     x1*h4                            x1*h4+x2*h3                                 x1*h4+x2*h3+x3*h2                                  x1*h4+x2*h3+x3*h2+x4*h1
                for (int k = 0; k < m_iFftLength; k++)
                {
                    m_pfProcessedBlockBuffer[(iIrBlock * m_iBlockLength + m_iOutputWriteIdx + k) % mod] += pfIFFTTemp[k];
                }
            }
            (m_iOutputWriteIdx += m_iBlockLength) %= mod;
        }

        //Set zeros to m_pfInputProcess buffer
        //fill the inputblockbuffer of size 2BlockSize until the processing size is met
        m_pfInputBlockBuffer[m_iInputWriteIdx++]= pfInputBuffer[i];
        if (m_iInputWriteIdx == m_iBlockLength) m_iInputWriteIdx = 0;

        //Fill the output buffer with values from the current readBlock
        pfOutputBuffer[i] = m_pfProcessedBlockBuffer[m_iOutputReadIdx];
        m_pfProcessedBlockBuffer[m_iOutputReadIdx++] = 0;
        if (m_iOutputReadIdx == mod) m_iOutputReadIdx = 0;
    }

    return Error_t::kNoError;
}
Error_t CConvFFT::flushBuffer(float *pfOutputBuffer)
{
    float* pfFlushInputBuffer = new float[m_iBlockLength + m_iIrLength - 1];
    CVectorFloat::setZero(pfFlushInputBuffer,static_cast<long long int>(m_iBlockLength + m_iIrLength - 1));
    process(pfOutputBuffer, pfFlushInputBuffer, m_iBlockLength + m_iIrLength - 1);
    delete[] pfFlushInputBuffer;
    return Error_t::kNoError;
}
