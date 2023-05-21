
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "Vibrato.h"
#include "FastConv.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{

    std::string             sInputFilePath,                 //!< file paths
        sOutputFilePath, sImpulsePath;

    static const int            iFastConvBlockLength = 2048;
    static const int            kBlockSize = 1024;
    long long                   iNumFrames = kBlockSize;
    int                         iNumChannels;

    float                       fModFrequencyInHz;
    float                       fModWidthInSec;
    bool                        bFreqConvCheck;


    clock_t                     time = 0;

    float** ppfInputAudio = 0;
    float** ppfOutputAudio = 0;
    float** ppfImpulseResponse=0;

    float** ppfFlushBuffer = 0;

    long long iImpulseLength = 0;

    int iFlushBufferLength = 0;

    CAudioFileIf* phAudioFile = 0;
    CAudioFileIf* phAudioOutputFile = 0;
    CAudioFileIf* phImpluseFile = 0;

    CAudioFileIf::FileSpec_t    stFileSpec, stImpulseSpec;

    CVibrato* pCVibrato = 0;
    CFastConv** m_pCFastConv = 0;
    CFastConv::ConvCompMode_t eCompMode;

    showClInfo();


    // command line args
    if (argc < 4)
    {
        cout << "Incorrect number of arguments!" << endl;
        return -1;
    }
    sInputFilePath = argv[1];
    sOutputFilePath = argv[2];
    sImpulsePath = argv[3];
    bFreqConvCheck = static_cast<bool>(std::stoi(argv[4]));
    if (bFreqConvCheck)
    {
        eCompMode = CFastConv::kFreqDomain;
    }
    else
    {
        eCompMode = CFastConv::kTimeDomain;
    }



//    fModFrequencyInHz = atof(argv[3]);
//    fModWidthInSec = atof(argv[4]);

    ///////////////////////////////////////////////////////////////////////////
    CAudioFileIf::create(phAudioFile);
    CAudioFileIf::create(phAudioOutputFile);
    CAudioFileIf::create(phImpluseFile);

    phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    phAudioFile->getFileSpec(stFileSpec);
    phAudioFile->openFile(sImpulsePath,CAudioFileIf::kFileRead);
    phAudioFile->getFileSpec(stImpulseSpec);
    phAudioOutputFile->openFile(sOutputFilePath, CAudioFileIf::kFileWrite, &stFileSpec);
    iNumChannels = stFileSpec.iNumChannels;
    if (!phAudioFile->isOpen())
    {
        cout << "Input file open error!";

        CAudioFileIf::destroy(phAudioFile);
        CAudioFileIf::destroy(phAudioOutputFile);
        CAudioFileIf::destroy(phImpluseFile);
        return -1;
    }
    else if (!phAudioOutputFile->isOpen())
    {
        cout << "Output file cannot be initialized!";

        CAudioFileIf::destroy(phAudioFile);
        CAudioFileIf::destroy(phAudioOutputFile);
        CAudioFileIf::destroy(phImpluseFile);
        return -1;
    }
    ////////////////////////////////////////////////////////////////////////////

//    CVibrato::create(pCVibrato);
//    pCVibrato->init(fModWidthInSec, stFileSpec.fSampleRateInHz, iNumChannels);
    phImpluseFile->getLength(iImpulseLength);

    ppfImpulseResponse = new float* [stImpulseSpec.iNumChannels];

    for (int i = 0; i < stImpulseSpec.iNumChannels; i++)
    {
        ppfImpulseResponse[i] = new float[iImpulseLength];
    }
    // allocate memory
    ppfInputAudio = new float* [stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfInputAudio[i] = new float[kBlockSize];

    ppfOutputAudio = new float* [stFileSpec.iNumChannels];
    ppfFlushBuffer = new float* [stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++) {
        ppfOutputAudio[i] = new float[kBlockSize];
        ppfFlushBuffer[i] = new float[iFlushBufferLength];
    }

    // Set parameters of vibrato
//    pCVibrato->setParam(CVibrato::kParamModFreqInHz, fModFrequencyInHz);
//    pCVibrato->setParam(CVibrato::kParamModWidthInS, fModWidthInSec);

    if (bFreqConvCheck)
    {
        iFlushBufferLength = iFastConvBlockLength + iImpulseLength - 1;
    }
    else
    {
        iFlushBufferLength = iImpulseLength - 1;
    }
    m_pCFastConv = new CFastConv* [stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        m_pCFastConv[i] = new CFastConv;
        m_pCFastConv[i]->init(ppfImpulseResponse[i], static_cast<int>(iImpulseLength), iFastConvBlockLength, eCompMode);
    }

    time = clock();
    // processing
    while (!phAudioFile->isEof())
    {
        phAudioFile->readData(ppfInputAudio, iNumFrames);
        for(int i = 0; i < stFileSpec.iNumChannels; i++){
            m_pCFastConv[i]->process(ppfOutputAudio[i], ppfInputAudio[i], iNumFrames);
        }

       // pCVibrato->process(ppfInputAudio, ppfOutputAudio, iNumFrames);
        phAudioOutputFile->writeData(ppfOutputAudio, iNumFrames);
    }
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        m_pCFastConv[i]->flushBuffer(ppfFlushBuffer[i]);
    }

    phAudioOutputFile->writeData(ppfFlushBuffer, iFlushBufferLength);
    phAudioFile->getFileSpec(stFileSpec);


    cout << "\nConvolution Process done in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // clean-up
    CAudioFileIf::destroy(phAudioFile);
    CAudioFileIf::destroy(phAudioOutputFile);
    CVibrato::destroy(pCVibrato);

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        delete m_pCFastConv[i];
        delete[] ppfInputAudio[i];
        delete[] ppfOutputAudio[i];
        delete[] ppfImpulseResponse[i];
        delete[] ppfFlushBuffer[i];
    }
    delete[] m_pCFastConv;
    delete[] ppfInputAudio;
    delete[] ppfOutputAudio;
    delete[] ppfImpulseResponse;
    delete[] ppfFlushBuffer;

    ppfInputAudio = 0;
    ppfOutputAudio = 0;
    m_pCFastConv=0;
    ppfImpulseResponse=0;
    ppfFlushBuffer=0;

    // all done
    return 0;

}


void     showClInfo()
{
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout << endl;

    return;
}

