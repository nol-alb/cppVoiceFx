
// standard headers

// project headers
#include "MUSI6106Config.h"

#include "ErrorDef.h"
#include "Util.h"
#include "RingBuffer.h"

#include "CombFilterIf.h"
#include "CombFilter.h"

static const char*  kCMyProjectBuildDate             = __DATE__;


CCombFilterIf::CCombFilterIf () :
    m_bIsInitialized(false),
    m_pCCombFilter(0),
    m_fSampleRate(0)
{
    // this never hurts
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

Error_t CCombFilterIf::create( CCombFilterIf*& pCCombFilterIf )
{
    pCCombFilterIf = new CCombFilterIf ();

    if (!pCCombFilterIf)
        return Error_t::kUnknownError;


    return Error_t::kNoError;
}

Error_t CCombFilterIf::destroy (CCombFilterIf*& pCCombFilterIf)
{
    if (!pCCombFilterIf)
        return Error_t::kUnknownError;
    
    pCCombFilterIf->reset ();
    
    delete pCCombFilterIf;
    pCCombFilterIf = 0;

    return Error_t::kNoError;

}

Error_t CCombFilterIf::init( CombFilterType_t eFilterType, float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels )
{
    reset();

    if (fMaxDelayLengthInS <= 0 ||
        fSampleRateInHz <= 0 ||
        iNumChannels <= 0)
        return Error_t::kFunctionInvalidArgsError;

    switch (eFilterType)
    {
    case kCombFIR:
        m_pCCombFilter  = static_cast<CCombFilterBase*> (new CCombFilterFir (CUtil::float2int<int>(fMaxDelayLengthInS * fSampleRateInHz), iNumChannels));
        break;
    case kCombIIR:
        m_pCCombFilter  = static_cast<CCombFilterBase*> (new CCombFilterIir (CUtil::float2int<int>(fMaxDelayLengthInS * fSampleRateInHz), iNumChannels));
        break;
    }

    m_fSampleRate       = fSampleRateInHz;
    m_bIsInitialized    = true;

    return Error_t::kNoError;
}

Error_t CCombFilterIf::reset ()
{
    delete m_pCCombFilter;
    m_pCCombFilter      = 0;

    m_fSampleRate       = 0;
    m_bIsInitialized    = false;

    return Error_t::kNoError;
}

Error_t CCombFilterIf::process(float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames )
{
    if (!m_bIsInitialized)
        return Error_t::kNotInitializedError;

    return m_pCCombFilter->process(ppfInputBuffer, ppfOutputBuffer, iNumberOfFrames);
}

Error_t CCombFilterIf::setParam( FilterParam_t eParam, float fParamValue )
{
    if (!m_bIsInitialized)
        return Error_t::kNotInitializedError;

    switch (eParam)
    {
    case kParamDelay:
        return m_pCCombFilter->setParam(eParam, fParamValue * m_fSampleRate);
    default:
    case kParamGain:
        return m_pCCombFilter->setParam(eParam, fParamValue);
    }
}

float CCombFilterIf::getParam( FilterParam_t eParam ) const
{
    if (!m_bIsInitialized)
        return -1;

    switch (eParam)
    {
    case kParamDelay:
        return m_pCCombFilter->getParam(eParam) / m_fSampleRate;
    default:
    case kParamGain:
        return m_pCCombFilter->getParam (eParam);
    }
}
