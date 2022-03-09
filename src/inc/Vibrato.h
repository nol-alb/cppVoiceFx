#if !defined(__Vibrato_hdr__)
#define __Vibrato_hdr__
#include "RingBuffer.h"
#include "Lfo.h"
#include "ErrorDef.h"

/**
*This is the Vibrato Interface:
* A user has control over
*   - What the initial delayInSeconds they need the effect for
*    - What width in seconds the Vibrato is processed
*   - The frequency of the LFO
* User methods:
* 1. Create (Static)
* 2. Destroy (Static)
* 3. Initialise all the Vibrato Parameters
* 4. Reset The buffer and Vibrato
* 5. Set and Get to receive the required parameters
* 6. Processes to apply the vibrato effect on an input buffer
* The interface is designed similar to the CCombFilterIf
 */



class CVibrato
{
public:

    enum ParamVibrato{
        kDelayInSec,
        kWidthInSec,
        kLFOFreqInHz,
        kSampleRate,


        kNumVibratoParams
    };
    /**
     * New pCVibrato Instance
     * Allocates memory for a pointer to the CVibrato Class and creates a pointer for the LFO functionality
     * @param pCVibrato
     * @return Error_t
     */
    static Error_t create (CVibrato*& pCVibrato);
    /**
     *Resets and frees memory of the pCVibrato Instance
     * Resets the Vibrato is memory is deallocated and we free up the LFO pointer as well
     * @param pCVibrato
     * @return Error_t
     */
    static Error_t destroy (CVibrato*& pCVibrato);

    /**
     *  Initialises the required parameters for the Vibrato Functionality and sets up the ring buffer required for implementation
     *
     * @param fdelayInSec: Initial Delay In Seconds required (float value)
     * @param fmodWidthInSec: Width of the wobble required in the Vibrato (fmodWidthInSec<=fDelayInSec)
     * @param flfoFreqInHz : Frequency of the LFO wavetable sinusoid
     * @param fSampleFreqInHz : Sampling frequency for writing of the audio
     * @param iNumChannels : Num of channels required
     * @return Error_t
     */

    Error_t init (float fdelayInSec, float fmodWidthInSec, float flfoFreqInHz, float fSampleFreqInHz, int iNumChannels);
    /**
     * Set the parameters of the Vibrato Enum
     * @param vParam (Required parameter from the Vibrato interface)
     * @param fParamValue (Float value of the parameter)
     * @return Error_t
     */
    Error_t setParam(ParamVibrato vParam, float fParamValue);

    /**
     * Get the parameter values User Specifies
     * @param vParam (Required parameter from the Vibrato interface)
     * @return float vParam value
     */

    float getParam(ParamVibrato vParam) const;


/**
 *  Apply the Vibrato Effect to inputBuffer and write the processed output to the outputBuffer
 * @param ppfInputBuffer
 * @param ppfOutputbuffer
 * @param pBlockSize
 * @return Error_t
 */
    Error_t process(float **ppfInputBuffer, float **ppfOutputbuffer, int pBlockSize);







protected:
    /**
     * CVibrato Constructor
     */
    CVibrato();
    /**
     * CVibrato Destructor
     */
    virtual ~CVibrato();
    Error_t reset();

private:
    bool m_bIsInitialised;
    /**
     * Pointer to the LFO Class
     */

    CLfo *m_pLFO; //Handle the LFO
    /**
     * Pointer to the CRingbuffer Class
     */
    CRingBuffer<float> **m_ptBuff; //Handle Ring Buffer
/**
 * Member variables required for Vibrato Implementation
 */
    float m_fFrequency;
    float m_fSampleRateinSamples;
    float m_fModFrequencyinSamples;
    float m_fWidthInSamples;
    float m_fDelayinSamples;

    int m_iNumChannels;

};

#endif // __Vibrato_hdr__
