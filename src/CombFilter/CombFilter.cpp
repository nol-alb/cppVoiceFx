//
//  CombFilter.cpp
//  MUSI6106
//
//  Created by Noel  Alben on 2/7/22.
//
// standard headers
#include <limits>

// project headers
#include "MUSI6106Config.h"

#include "ErrorDef.h"
#include "Util.h"
#include "RingBuffer.h"

#include "CombFilterIf.h"
#include "CombFilter.h"

CCombFilterBase::CCombFilterBase(int delayLengthInS, int iNumChannels, float gain): m_RingBuffptr(0)
{
    m_DelayLengthIns=delayLengthInS;
    m_iNumChannels = iNumChannels;
    gain=gain;
    m_RingBuffptr = new CRingBuffer<float>*[iNumChannels];
    for (int i=0; i<iNumChannels;i++)
    {
        m_RingBuffptr[i] = new CRingBuffer<float>(m_DelayLengthIns);
        m_RingBuffptr[i]->reset();
        
    }
    
   
    
    
}



CCombFilterBase::~CCombFilterBase()
{
    for (int i=0; i<m_iNumChannels;i++)
    {
        m_RingBuffptr[i]->~CRingBuffer();
    }
    {
        for (int i = 0; i < m_iNumChannels; ++i) {
            delete m_RingBuffptr[i];
        }
        delete [] m_RingBuffptr;
        m_RingBuffptr = 0;
    }
}

Error_t CCombFilterBase::setGain(float gain)
{
    if (gain > 1)
        return Error_t::kFunctionInvalidArgsError;
    else if (gain < -1)
        return Error_t::kFunctionInvalidArgsError;
    else
    m_gain = gain;
}
float CCombFilterBase::getGain()
{
    return m_gain;
}
Error_t CCombFilterBase::setDelay(int DelayLengthinSamples)
{
    if(DelayLengthinSamples<=0)
    {
        return Error_t::kFunctionInvalidArgsError;
    }
    m_DelayLengthIns=DelayLengthinSamples;
    
}
int CCombFilterBase::getDelay()
{
    return m_DelayLengthIns;
    
}


Error_t CCombFilterBase::resetComb()
{
    m_DelayLengthIns=0;
    m_gain=0;
    
    return Error_t::kNoError;
}
//Error_t CCombFilterBase::setParam(CCombFilterIf::FilterParam_t eParam, float fParamValue)
//{
//    return Error_t::kNoError;
//
//}
//Error_t CCombFilterBase::getParam(CCombFilterIf::FilterParam_t eParam)
//{
//    return Error_t::kNoError;
//}

Error_t CCombFilterFir::process(float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames)
{
    // For loop outside for each channel
    //for loop inside to go through the audui file
    //Ring buffer is my delayline
    //i get from ringbuffer and post increment
    //i put into ringbuffer and post increment
    //Repeat loop
    /*
     x = [1,2,3,4,5]
     buff=[0,0]
     y = [1,2,4,6,8]
     */
    for  (int j=0; j<m_iNumChannels;j++)
    {
        for (int i=0; i<iNumberOfFrames;i++)
        {
            ppfOutputBuffer[j][i]= ppfInputBuffer[j][i]+m_gain*m_RingBuffptr[j]->getPostInc();
            m_RingBuffptr[j]->putPostInc(ppfInputBuffer[j][i]);
            
        }
    }
    return Error_t::kNoError;
}
Error_t CCombFilterIir::process(float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames)
{
    for  (int j=0; j<m_iNumChannels;j++)
    {
        for (int i=0; i<iNumberOfFrames;i++)
        {
            ppfOutputBuffer[j][i] = ppfInputBuffer[j][i] + m_gain * m_RingBuffptr[j]->getPostInc();
            m_RingBuffptr[j]->putPostInc(ppfOutputBuffer[j][i]);
            
        }
    }
    return Error_t::kNoError;
    
}
