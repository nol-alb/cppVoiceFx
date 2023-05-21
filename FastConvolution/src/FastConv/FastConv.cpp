//
// Created by Noel  Alben on 4/28/22.
//

#include "FastConv.h"
#include "Util.h"
#include "CConvbase.h"

CFastConv::CFastConv() {
}

CFastConv::~CFastConv() {
    reset();
}

Error_t CFastConv::init(float *pfImpulseResponse, int iLengthOfIr,int iBlockLength /*= 8192*/, ConvCompMode_t eCompMode /*= kFreqDomain*/)
{
    this->reset();
    if (eCompMode==kTimeDomain)
    {
        m_pCConv =  static_cast<CConvbase*>(new CConvTime());
        return m_pCConv->init(pfImpulseResponse,iLengthOfIr,iBlockLength);
    }
    else if(eCompMode==kFreqDomain)
    {
        m_pCConv = static_cast<CConvbase*>(new CConvFFT());
        return m_pCConv->init(pfImpulseResponse, iLengthOfIr, iBlockLength);
    }
    return Error_t::kNoError;

}
Error_t CFastConv::reset()
{
    if (m_pCConv)
        delete m_pCConv;
    m_pCConv = 0;

   return Error_t::kNoError;
}

Error_t CFastConv::process(float *pfOutputBuffer, const float *pfInputBuffer, int iLengthOfBuffers)
{
    m_pCConv->process(pfOutputBuffer,pfInputBuffer,iLengthOfBuffers);
    return Error_t::kNoError;
}
Error_t CFastConv::flushBuffer(float *pfOutputBuffer)
{
    m_pCConv->flushBuffer(pfOutputBuffer);
    return Error_t::kNoError;
}
