
#include "Vibrato.h"
#include "RingBuffer.h"
#include "Lfo.h"
#include "MUSI6106Config.h"
#include <iostream>

using std::cout;
using std::endl;

CVibrato::CVibrato() :
    m_bIsInitialised(false),
    m_fFrequency(0),
    m_iNumChannels(0),
    m_ptBuff(0),
    m_pLFO(0),
    m_fDelayinSamples(0),
    m_fModFrequencyinSamples(0),
    m_fSampleRateinSamples(0),
    m_fWidthInSamples(0)
{
    // this should never hurt
    this->reset ();
}
CVibrato::~CVibrato() {
    this->reset();
}

Error_t CVibrato::create(CVibrato*& pCVibrato)
{
    pCVibrato = new CVibrato() ;
    return Error_t::kNoError;
}

Error_t CVibrato::destroy(CVibrato*& pCVibrato)
{
    if (!pCVibrato)
        return Error_t::kUnknownError;
    delete pCVibrato;
    pCVibrato=0;

    return Error_t::kNoError;


}

Error_t CVibrato::reset()
{
    delete m_pLFO;
    m_pLFO=0;

    for (int i=0; i<m_iNumChannels; i++) {
        delete m_ptBuff[i];
    }
    delete [] m_ptBuff;
    m_ptBuff=0;

    m_fSampleRateinSamples =0;
    m_fWidthInSamples=0;
    m_fDelayinSamples=0;
    m_fModFrequencyinSamples=0;
    m_bIsInitialised= false;
    return Error_t::kNoError;
}
Error_t CVibrato::init(float fdelayInSec, float fmodWidthInSec, float flfoFreqInHz, float fSampleFreqInHz,
                       int iNumChannels){
    this->reset();
    if (fSampleFreqInHz <= 0|| flfoFreqInHz <= 0 || iNumChannels <= 0 || fmodWidthInSec>fdelayInSec) {
        cout << "Invalid Argument Please check Param Bounds" << endl;
        return Error_t::kFunctionInvalidArgsError;
    }
    m_fSampleRateinSamples = fSampleFreqInHz;
    m_fModFrequencyinSamples = (flfoFreqInHz/m_fSampleRateinSamples);
    m_fWidthInSamples = (fmodWidthInSec*m_fSampleRateinSamples);
    m_fDelayinSamples =(fdelayInSec*m_fSampleRateinSamples);
    m_iNumChannels=iNumChannels;

    m_pLFO = new CLfo(m_fSampleRateinSamples,m_fWidthInSamples,m_fModFrequencyinSamples);

    m_ptBuff = new CRingBuffer<float>* [iNumChannels];
    for (int i=0; i<iNumChannels;i++) {
        //2+DELAY+WIDTH*2
        int delayLength = static_cast<int>(2 + m_fDelayinSamples + m_fWidthInSamples * 2);
        m_ptBuff[i] = new CRingBuffer<float>(delayLength);
    }

        m_bIsInitialised = true;
    return Error_t::kNoError;



}

Error_t CVibrato::setParam(ParamVibrato vParam, float fParamValue)
{
    if(!m_bIsInitialised)
        return Error_t::kNotInitializedError;
    switch (vParam)
    {
        case kDelayInSec:
            m_fDelayinSamples = fParamValue*m_fSampleRateinSamples;
            break;
        case kWidthInSec:
            m_fWidthInSamples = fParamValue*m_fSampleRateinSamples;
            m_pLFO->setParam(CLfo::LfoParam_t::kAmplitude,m_fWidthInSamples);

            break;
        case kLFOFreqInHz:
            m_fModFrequencyinSamples = fParamValue;
            m_pLFO->setParam(CLfo::LfoParam_t::kFrequency, m_fModFrequencyinSamples);
            break;
        case kSampleRate:
            m_fSampleRateinSamples = fParamValue;
            break;
        case  kNumVibratoParams:
            return Error_t::kFunctionInvalidArgsError;

    }

    return Error_t::kNoError;
}
float CVibrato::getParam(ParamVibrato vParam) const {
    if (!m_bIsInitialised)
        return 0;


    switch (vParam)
    {
        case(CVibrato::ParamVibrato::kDelayInSec):
            return m_fDelayinSamples/m_fSampleRateinSamples;
            break;
        case (CVibrato::ParamVibrato::kWidthInSec):
            return m_fWidthInSamples/m_fSampleRateinSamples;
            break;
        case (CVibrato::ParamVibrato::kLFOFreqInHz):
            return m_fModFrequencyinSamples*m_fSampleRateinSamples;
            break;
        case (CVibrato::ParamVibrato::kSampleRate):
            return m_fSampleRateinSamples;
            break;

    }
}
Error_t CVibrato::process(float **ppfInputBuffer, float **ppfOutputbuffer, int pBlockSize) {
    //TAP=1+DELAY+WIDTH*MOD;
    if (!m_bIsInitialised)
        return Error_t::kNotInitializedError;
    for (int j=0; j<m_iNumChannels; j++)
    {
        for (int i=0; i<pBlockSize; i++)
        {
            float Lfo_Offset = m_pLFO->process();
            //John's process code already handles the amplitude width
            float buf_fOffset = 1 + m_fDelayinSamples +Lfo_Offset;
            m_ptBuff[j]->putPostInc(ppfInputBuffer[j][i]);
            ppfOutputbuffer[j][i] = m_ptBuff[j]->get(buf_fOffset);
            m_ptBuff[j]->getPostInc();
        }
    }
    return Error_t::kNoError;

}


