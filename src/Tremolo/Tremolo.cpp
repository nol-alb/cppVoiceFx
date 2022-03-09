//
// Created by Noel  Alben on 3/6/22.
//

#include "Tremolo.h"
#include "RingBuffer.h"
#include "Lfo.h"
#include "MUSI6106Config.h"
#include <iostream>

using std::cout;
using std::endl;

CTremolo::CTremolo() :
        m_bIsInitialised(false),
        m_fFrequency(0),
        m_iNumChannels(0),
        m_ptBuff(0),
        m_pLFO(0),
        m_fModFrequencyinSamples(0),
        m_fSampleRateinSamples(0),
        m_fWidthInVal(0)
{
    // this should never hurt
    this->reset ();
}
CTremolo::~CTremolo() {
    this->reset();
}

Error_t CTremolo::create(CTremolo*& pCTremolo)
{
    pCTremolo = new CTremolo() ;
    return Error_t::kNoError;
}

Error_t CTremolo::destroy(CTremolo*& pCTremolo)
{
    if (!pCTremolo)
        return Error_t::kUnknownError;
    delete pCTremolo;
    pCTremolo=0;

    return Error_t::kNoError;


}

Error_t CTremolo::reset()
{
    delete m_pLFO;
    m_pLFO=0;

    for (int i=0; i<m_iNumChannels; i++) {
        delete m_ptBuff[i];
    }
    delete [] m_ptBuff;
    m_ptBuff=0;

    m_fSampleRateinSamples =0;
    m_fWidthInVal=0;
    m_fModFrequencyinSamples=0;
    m_bIsInitialised= false;
    return Error_t::kNoError;
}
Error_t CTremolo::init(float fmodWidthval, float flfoFreqInHz, float fSampleFreqInHz, int iNumChannels)
{
    this->reset();
    if (fSampleFreqInHz <= 0|| flfoFreqInHz <= 0 || iNumChannels <= 0) {
        cout << "Invalid Argument Please check Param Bounds" << endl;
        return Error_t::kFunctionInvalidArgsError;
    }
    m_fSampleRateinSamples = fSampleFreqInHz;
    m_fModFrequencyinSamples = (flfoFreqInHz);
    m_fWidthInVal = fmodWidthval;
    m_iNumChannels=iNumChannels;

    m_pLFO = new CLfo(m_fSampleRateinSamples,m_fWidthInVal,m_fModFrequencyinSamples);
    m_bIsInitialised = true;
    return Error_t::kNoError;

}
Error_t CTremolo::setParam(ParamTremolo vParam, float fParamValue) {
    return Error_t::kNoError;

}
float CTremolo::getParam(ParamTremolo vParam) const {

}
Error_t CTremolo::process(float **ppfInputBuffer, float **ppfOutputbuffer, int pBlockSize) {
    if (!m_bIsInitialised)
        return Error_t::kNotInitializedError;
    for (int j=0; j<m_iNumChannels; j++) {
        for (int i = 0; i < pBlockSize; i++) {
            float Lfo_Offset = m_pLFO->process();
            ppfOutputbuffer[j][i] = Lfo_Offset*ppfInputBuffer[j][i];
        }
    }
    return Error_t::kNoError;

}