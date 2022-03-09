//
// Created by Noel  Alben on 3/6/22.
//

#ifndef MUSI6106_CVOICEFXIF_H
#define MUSI6106_CVOICEFXIF_H

#include "Vibrato.h"
#include "CombFilterIf.h"
#include "Lfo.h"
#include "ErrorDef.h"

class CVoicefxIf
{
public:
    /*! feedforward or recursive comb filter */
    enum Voicefx_Type
    {
        kVibrato,           //!< finite impulse response filter
        kCombFilter,           //!< infinite impulse response filter
        kTremolo,

        kNumFilterTypes
    };

    /*! list of parameters for the comb filters */
    enum VoicefxParam_t
    {
        kParamGain,         //!< gain as factor (usually -1...1)
        kParamDelay,        //!< delay in seconds for specification of comb width
        kWidthInSec,
        kWidthInAmp,
        kMaxDelay,
        kLFOFreqInHz,
        kSampleRate,

        kNumFxParams
    };



    /*! creates a new comb filter instance
    \param pCCombFilterIf pointer to the new class
    \return Error_t
    */
    static Error_t create (CVoicefxIf*& pVoicefxIf);

    /*! destroys a comb filter instance
    \param pCCombFilterIf pointer to the class to be destroyed
    \return Error_t
    */
    static Error_t destroy (CVoicefxIf*& pVoicefxIf);

    /*! initializes a comb filter instance
    \param eFilterType FIR or IIR
    \param fMaxDelayLengthInS maximum allowed delay in seconds
    \param fSampleRateInHz sample rate in Hz
    \param iNumChannels number of audio channels
    \return Error_t
    */
    Error_t init (Voicefx_Type FxType,float fSampleRateInHz, int iNumChannels);

    /*! resets the internal variables (requires new call of init)
    \return Error_t
    */
    Error_t reset ();

    /*! sets a comb filter parameter
    \param eParam what parameter (see ::FilterParam_t)
    \param fParamValue value of the parameter
    \return Error_t
    */
    Error_t setParam (Voicefx_Type FxType, VoicefxParam_t eParam, float fParamValue);

    /*! return the value of the specified parameter
    \param eParam
    \return float
    */
    float   getParam (VoicefxParam_t eParam) const;

    /*! processes one block of audio
    \param ppfInputBuffer input buffer [numChannels][iNumberOfFrames]
    \param ppfOutputBuffer output buffer [numChannels][iNumberOfFrames]
    \param iNumberOfFrames buffer length (per channel)
    \return Error_t
    */
    Error_t process (Voicefx_Type FxType, float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames);

protected:
    CVoicefxIf ();
    virtual ~CVoicefxIf();

private:
    bool            m_bIsInitialized;   //!< internal bool to check whether the init function has been called
                                          //!< handle of the comb filter

    float           m_fSampleRate;      //!< audio sample rate in Hz
};
#endif //MUSI6106_CVOICEFXIF_H
