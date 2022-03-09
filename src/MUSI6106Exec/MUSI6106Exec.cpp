
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "Vibrato.h"
#include "Tremolo.h"
using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string                 sInputFilePath,                 //!< file paths
    sOutputFilePath;

    static const int            kBlockSize = 1024;
    long long                   iNumFrames = kBlockSize;
    int                         iNumChannels;

    float                       fSampleRate;
    float                       fModFrequencyInHz;
    float                       fBasicDelayInSec;
    float                       fModWidthInSec;

    clock_t                     time = 0;

    float                       **ppfAudioData = 0;
    float                       **ppfAudioOutputData = 0;

    CAudioFileIf                *phAudioFile = 0;
    CAudioFileIf                *phAudioOutputFile = 0;

    CAudioFileIf::FileSpec_t    stFileSpec;

    CVibrato                    *pCVibrato = 0;
    CTremolo                    *pCTremolo = 0;
    showClInfo();


    if(argc < 7)
    {
        cout << "Incorrect number of arguments!" << endl;
        return -1;
    }


    sInputFilePath = argv[1];
    sOutputFilePath = argv[2];
    fSampleRate = atof(argv[3]);
    fModFrequencyInHz = atof(argv[4]);
    fBasicDelayInSec = atof(argv[5]);
    fModWidthInSec = atof(argv[6]);

    cout << "Delay: " << fBasicDelayInSec << endl;
    cout << "Width: " << fModWidthInSec << endl;
    cout << "LFO Freq: " << fModFrequencyInHz << endl;
    ///////////////////////////////////////////////////////////////////////////
    CAudioFileIf::create(phAudioFile);
    CAudioFileIf::create(phAudioOutputFile);

    phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    phAudioFile->getFileSpec(stFileSpec);
    phAudioOutputFile->openFile(sOutputFilePath, CAudioFileIf::kFileWrite, &stFileSpec);
    iNumChannels = stFileSpec.iNumChannels;
    if (!phAudioFile->isOpen())
    {
        cout << "Input file open error!";

        CAudioFileIf::destroy(phAudioFile);
        CAudioFileIf::destroy(phAudioOutputFile);
        return -1;
    }
    else if (!phAudioOutputFile->isOpen())
    {
        cout << "Output file cannot be initialized!";

        CAudioFileIf::destroy(phAudioFile);
        CAudioFileIf::destroy(phAudioOutputFile);
        return -1;
    }
////////////////////////////////////////////////////////////////////////////
    CVibrato::create(pCVibrato);
    CTremolo::create(pCTremolo);
    pCTremolo->init(fModWidthInSec,fModFrequencyInHz,fSampleRate,iNumChannels);
    pCVibrato->init(fBasicDelayInSec,fModWidthInSec,fModFrequencyInHz,fSampleRate,iNumChannels);


    // Set parameters of vibrato
    pCVibrato->setParam(CVibrato::kLFOFreqInHz, fModFrequencyInHz);
    pCVibrato->setParam(CVibrato::kWidthInSec, fModWidthInSec);

    // allocate memory
    ppfAudioData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfAudioData[i] = new float[kBlockSize];

    ppfAudioOutputData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfAudioOutputData[i] = new float[kBlockSize];


    while (!phAudioFile->isEof())
    {
        phAudioFile->readData(ppfAudioData, iNumFrames);
        pCTremolo->process(ppfAudioData,ppfAudioOutputData,iNumFrames);
       // pCVibrato->process(ppfAudioData, ppfAudioOutputData, iNumFrames);
        phAudioOutputFile->writeData(ppfAudioOutputData, iNumFrames);
    }

    cout << "\nreading/writing done in: \t" << (clock() - time)*1.F / CLOCKS_PER_SEC << " seconds." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // clean-up
    CAudioFileIf::destroy(phAudioFile);
    CAudioFileIf::destroy(phAudioOutputFile);
    CVibrato::destroy(pCVibrato);

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        delete[] ppfAudioData[i];
        delete[] ppfAudioOutputData[i];
    }
    delete[] ppfAudioData;
    delete[] ppfAudioOutputData;
    ppfAudioData = 0;
    ppfAudioOutputData = 0;

    return 0;
    // all done

}


void     showClInfo()
{
    cout << "Based off of MUSI6106 Assignment Executables" << endl;
    cout << "Written in the Spring of 2022 By Noel Alben" << endl;
    cout  << endl;

    return;
}
