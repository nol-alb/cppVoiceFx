
// standard headers

// project headers
#include "MUSI6106Config.h"
#include <iostream>

#include "ErrorDef.h"
#include "Util.h"

#include "CombFilterIf.h"
#include "CombFilter.h"
using std::cout;
using std::endl;



// Check for m)bIsInitialized state in each function?


static const char*  kCMyProjectBuildDate = __DATE__;


CCombFilterIf::CCombFilterIf () :
    m_bIsInitialized(false),
    m_pCCombFilter(0),
    m_fSampleRate(0)
{
    // this should never hurt
    this->reset ();
}


CCombFilterIf::~CCombFilterIf ()
{
    this->reset ();
}

const int  CCombFilterIf::getVersion (const Version_t eVersionIdx)
{
    int iVersion = 0;

    switch (eVersionIdx)
    {
    case kMajor:
        iVersion    = MUSI6106_VERSION_MAJOR; 
        break;
    case kMinor:
        iVersion    = MUSI6106_VERSION_MINOR; 
        break;
    case kPatch:
        iVersion    = MUSI6106_VERSION_PATCH; 
        break;
    case kNumVersionInts:
        iVersion    = -1;
        break;
    }

    return iVersion;
}
const char*  CCombFilterIf::getBuildDate ()
{
    return kCMyProjectBuildDate;
}

Error_t CCombFilterIf::create (CCombFilterIf*& pCCombFilter)
{
    pCCombFilter = new CCombFilterIf();
    return Error_t::kNoError;
}

Error_t CCombFilterIf::destroy (CCombFilterIf*& pCCombFilter)
{
    //take everything back to the initial stage and delete
    pCCombFilter->reset();
    delete pCCombFilter;
    pCCombFilter=nullptr;
    
    return Error_t::kNoError;
}

Error_t CCombFilterIf::init (CombFilterType_t eFilterType, float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels)
{
    m_fSampleRate=fSampleRateInHz;
    int delaylength = (int)(fMaxDelayLengthInS*fSampleRateInHz);
    if(delaylength<=0)
    {
        std::cout<<"Illegal Values Entered, Please check"<<std::endl;
        return Error_t::kFunctionInvalidArgsError;
    }
    if (eFilterType == kCombFIR)
    {
        m_pCCombFilter  = (new CCombFilterFir (delaylength, iNumChannels));
    }
    else if (eFilterType == kCombIIR)
    {
        m_pCCombFilter  = (new CCombFilterIir (delaylength, iNumChannels));
    }
    m_bIsInitialized=true;
    
    return Error_t::kNoError;
}

Error_t CCombFilterIf::reset ()
{
    m_pCCombFilter      = 0;

    m_fSampleRate       = 0;
    m_bIsInitialized    = false;
    return Error_t::kNoError;
}

Error_t CCombFilterIf::process (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames)
{
    if(!m_bIsInitialized)
        return Error_t::kNotInitializedError;
    m_pCCombFilter->process(ppfInputBuffer, ppfOutputBuffer, iNumberOfFrames);
    return Error_t::kNoError;
}

Error_t CCombFilterIf::setParam (FilterParam_t eParam, float fParamValue)
{
    if(!m_bIsInitialized)
        return Error_t::kNotInitializedError;
    switch(eParam){
        case kParamGain:
            m_pCCombFilter->setGain(fParamValue);
            break;
        
        case kParamDelay:
            m_pCCombFilter->setDelay(fParamValue*m_fSampleRate);
            
            break;
            
        case kNumFilterParams:
            //don't know what to do
            break;
    }
    return Error_t::kNoError;
}

float CCombFilterIf::getParam (FilterParam_t eParam) const
{


    switch(eParam){
        case kParamGain:
            return m_pCCombFilter->getGain();
            break;
        
        case kParamDelay:
            return m_pCCombFilter->getDelay();
            
            break;
            
        case kNumFilterParams:
            //don't know what to do
            break;
    }
    
}
