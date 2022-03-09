//
// Created by Noel  Alben on 3/6/22.
//

#ifndef MUSI6106_TREMOLO_H
#define MUSI6106_TREMOLO_H
#include "ErrorDef.h"
#include "Lfo.h"
#include "RingBuffer.h"

class CTremolo
{
public:

    enum ParamTremolo{
        kAmplitudeWidth,
        kLFOFreqInHz,
        kSampleRate,


        kNumTremoloParams
    };
    /**
     * New pCTremolo Instance
     * Allocates memory for a pointer to the  Tremolo Class and creates a pointer for the LFO functionality
     * @param pCTremolo
     * @return Error_t
     */
    static Error_t create (CTremolo*& pCTremolo);
    /**
     *Resets and frees memory of the pCTremolo Instance
     * Resets the Tremolo is memory is deallocated and we free up the LFO pointer as well
     * @param pCTremolo
     * @return Error_t
     */
    static Error_t destroy (CTremolo*& pCTremolo);

    /**
     *  Initialises the required parameters for the Tremolo Functionality and sets up the ring buffer required for implementation
     *
     * @param fdelayInSec: Initial Delay In Seconds required (float value)
     * @param fmodWidthInSec: Width of the wobble required in the Tremolo (fmodWidthInSec<=fDelayInSec)
     * @param flfoFreqInHz : Frequency of the LFO wavetable sinusoid
     * @param fSampleFreqInHz : Sampling frequency for writing of the audio
     * @param iNumChannels : Num of channels required
     * @return Error_t
     */

    Error_t init (float fmodWidthval, float flfoFreqInHz, float fSampleFreqInHz, int iNumChannels);
    /**
     * Set the parameters of the Tremolo Enum
     * @param vParam (Required parameter from the Tremolo interface)
     * @param fParamValue (Float value of the parameter)
     * @return Error_t
     */
    Error_t setParam(ParamTremolo vParam, float fParamValue);

    /**
     * Get the parameter values User Specifies
     * @param vParam (Required parameter from the Tremolo interface)
     * @return float vParam value
     */

    float getParam(ParamTremolo vParam) const;


/**
 *  Apply the Tremolo Effect to inputBuffer and write the processed output to the outputBuffer
 * @param ppfInputBuffer
 * @param ppfOutputbuffer
 * @param pBlockSize
 * @return Error_t
 */
    Error_t process(float **ppfInputBuffer, float **ppfOutputbuffer, int pBlockSize);







protected:
    /**
     * CTremolo Constructor
     */
    CTremolo();
    /**
     * CTremolo Destructor
     */
    virtual ~CTremolo();
    Error_t reset();

private:
    /**
     * Pointer to the LFO Class
     */

    CLfo *m_pLFO; //Handle the LFO
    /**
     * Pointer to the CRingbuffer Class
     */
    CRingBuffer<float> **m_ptBuff; //Handle Ring Buffer
/**
 * Member variables required for Tremolo Implementation
 */
    float m_fFrequency;
    float m_fSampleRateinSamples;
    float m_fModFrequencyinSamples;
    float m_fWidthInVal;
    bool m_bIsInitialised;

    int m_iNumChannels;

};


#endif //MUSI6106_TREMOLO_H
