/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2011 Martin Zuther (http://www.mzuther.de/)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Thank you for using free software!

---------------------------------------------------------------------------- */

#include "plugin_processor.h"
#include "plugin_editor.h"

//==============================================================================

KmeterAudioProcessor::KmeterAudioProcessor()
{
    if (DEBUG_FILTER)
    {
        DBG("********************************************************************************");
        DBG("** Debugging average filtering.  Please reset DEBUG_FILTER before committing! **");
        DBG("********************************************************************************");
    }

    bSampleRateIsValid = false;
    audioFilePlayer = NULL;

    pRingBufferInput = NULL;
    pRingBufferOutput = NULL;

    nNumInputChannels = 0;
    pMeterBallistics = NULL;

    setLatencySamples(KMETER_BUFFER_SIZE);

    pAverageLevelFilteredRms = NULL;
    pPluginParameters = new KmeterPluginParameters();

    fProcessedSeconds = 0.0f;

    fPeakLevels = NULL;
    fRmsLevels = NULL;
    fAverageLevelsFiltered = NULL;

    nOverflows = NULL;
}


KmeterAudioProcessor::~KmeterAudioProcessor()
{
    removeAllChangeListeners();

    // call function "releaseResources()" by force to make sure all
    // allocated memory is freed
    releaseResources();

    delete pPluginParameters;
    pPluginParameters = NULL;

    delete audioFilePlayer;
    audioFilePlayer = NULL;
}


void KmeterAudioProcessor::addChangeListenerParameters(ChangeListener* listener) throw()
{
    pPluginParameters->addChangeListener(listener);
}


void KmeterAudioProcessor::removeChangeListenerParameters(ChangeListener* listener) throw()
{
    pPluginParameters->removeChangeListener(listener);
}


//==============================================================================

const String KmeterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}


int KmeterAudioProcessor::getNumParameters()
{
    return pPluginParameters->getNumParameters(false);
}


float KmeterAudioProcessor::getParameter(int index)
{
    // This method will be called by the host, probably on the audio
    // thread, so it's absolutely time-critical. Don't use critical
    // sections or anything GUI-related, or anything at all that may
    // block in any way!

    return pPluginParameters->getParameterAsFloat(index);
}


void KmeterAudioProcessor::setParameter(int index, float newValue)
{
    // This method will be called by the host, probably on the audio
    // thread, so it's absolutely time-critical. Don't use critical
    // sections or anything GUI-related, or anything at all that may
    // block in any way!

    // Please use this method for non-automatable values only!

    pPluginParameters->setParameterFromFloat(index, newValue);
}


bool KmeterAudioProcessor::getParameterAsBool(int nIndex)
{
    // This method will be called by the host, probably on the audio
    // thread, so it's absolutely time-critical. Don't use critical
    // sections or anything GUI-related, or anything at all that may
    // block in any way!

    return pPluginParameters->getParameterAsBool(nIndex);
}


File KmeterAudioProcessor::getParameterValidationFile()
{
    // This method will be called by the host, probably on the audio
    // thread, so it's absolutely time-critical. Don't use critical
    // sections or anything GUI-related, or anything at all that may
    // block in any way!

    return pPluginParameters->getValidationFile();
}


void KmeterAudioProcessor::setParameterValidationFile(File& fileValidation)
{
    // This method will be called by the host, probably on the audio
    // thread, so it's absolutely time-critical. Don't use critical
    // sections or anything GUI-related, or anything at all that may
    // block in any way!

    pPluginParameters->setValidationFile(fileValidation);
}


const String KmeterAudioProcessor::getParameterName(int index)
{
    return pPluginParameters->getParameterName(index);
}


const String KmeterAudioProcessor::getParameterText(int index)
{
    return pPluginParameters->getParameterText(index);
}


int KmeterAudioProcessor::getParameterAsInt(int index)
{
    return pPluginParameters->getParameterAsInt(index);
}


void KmeterAudioProcessor::changeParameter(int index, int nValue)
{
    if (index == KmeterPluginParameters::selMono)
    {
        if (nNumInputChannels < 2)
        {
            nValue = true;
        }
        else if (nNumInputChannels > 2)
        {
            nValue = false;
        }
    }

    beginParameterChangeGesture(index);

    float newValue = pPluginParameters->translateParameterToFloat(index, nValue);
    setParameterNotifyingHost(index, newValue);

    endParameterChangeGesture(index);
}


void KmeterAudioProcessor::MarkParameter(int nIndex)
{
    pPluginParameters->MarkParameter(nIndex);
}


void KmeterAudioProcessor::UnmarkParameter(int nIndex)
{
    pPluginParameters->UnmarkParameter(nIndex);
}


bool KmeterAudioProcessor::isParameterMarked(int nIndex)
{
    return pPluginParameters->isParameterMarked(nIndex);
}


const String KmeterAudioProcessor::getInputChannelName(const int channelIndex) const
{
    return String("Input ") + String(channelIndex + 1);
}


const String KmeterAudioProcessor::getOutputChannelName(const int channelIndex) const
{
    return String("Output ") + String(channelIndex + 1);
}


bool KmeterAudioProcessor::isInputChannelStereoPair(int index) const
{
    return true;
}


bool KmeterAudioProcessor::isOutputChannelStereoPair(int index) const
{
    return true;
}


bool KmeterAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}


bool KmeterAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}


int KmeterAudioProcessor::getNumChannels()
{
    return nNumInputChannels;
}


int KmeterAudioProcessor::getNumPrograms()
{
    return 0;
}


int KmeterAudioProcessor::getCurrentProgram()
{
    return 0;
}


void KmeterAudioProcessor::setCurrentProgram(int index)
{
}


const String KmeterAudioProcessor::getProgramName(int index)
{
    return String::empty;
}


void KmeterAudioProcessor::changeProgramName(int index, const String& newName)
{
}

//==============================================================================

void KmeterAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    DBG("[K-Meter] in method KmeterAudioProcessor::prepareToPlay()");

    if ((sampleRate < 44100) || (sampleRate > 192000))
    {
        Logger::outputDebugString(String("[K-Meter] WARNING: sample rate of ") + String(sampleRate) + T(" Hz not supported"));
        bSampleRateIsValid = false;
        return;
    }
    else
    {
        bSampleRateIsValid = true;
    }

    nNumInputChannels = getNumInputChannels();
    isStereo = (nNumInputChannels == 2);

    DBG(String("[K-Meter] number of input channels: ") + String(nNumInputChannels));

    fPeakLevels = new float[nNumInputChannels];
    fRmsLevels = new float[nNumInputChannels];
    fAverageLevelsFiltered = new float[nNumInputChannels];

    nOverflows = new int[nNumInputChannels];

    for (int nChannel = 0; nChannel < nNumInputChannels; nChannel++)
    {
        fPeakLevels[nChannel] = 0.0f;
        fRmsLevels[nChannel] = 0.0f;
        fAverageLevelsFiltered[nChannel] = 0.0f;

        nOverflows[nChannel] = 0;
    }

    pAverageLevelFilteredRms = new AverageLevelFilteredRms(nNumInputChannels, KMETER_BUFFER_SIZE);

    pMeterBallistics = new MeterBallistics(nNumInputChannels, (int) sampleRate, false, false);

    // make sure that ring buffer can hold at least KMETER_BUFFER_SIZE
    // samples and is large enough to receive a full block of audio
    nSamplesInBuffer = 0;
    unsigned int uRingBufferSize = (samplesPerBlock > KMETER_BUFFER_SIZE) ? samplesPerBlock : KMETER_BUFFER_SIZE;

    pRingBufferInput = new AudioRingBuffer(T("Input ring buffer"), nNumInputChannels, uRingBufferSize, KMETER_BUFFER_SIZE, KMETER_BUFFER_SIZE);
    pRingBufferInput->setCallbackClass(this);

    pRingBufferOutput = new AudioRingBuffer(T("Output ring buffer"), nNumInputChannels, uRingBufferSize, KMETER_BUFFER_SIZE, KMETER_BUFFER_SIZE);
}


void KmeterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free
    // up any spare memory, etc.

    DBG("[K-Meter] in method KmeterAudioProcessor::releaseResources()");

    if (!bSampleRateIsValid)
    {
        return;
    }

    delete pAverageLevelFilteredRms;
    pAverageLevelFilteredRms = NULL;

    delete pMeterBallistics;
    pMeterBallistics = NULL;

    delete pRingBufferOutput;
    pRingBufferOutput = NULL;

    delete pRingBufferInput;
    pRingBufferInput = NULL;

    delete [] fPeakLevels;
    fPeakLevels = NULL;

    delete [] fRmsLevels;
    fRmsLevels = NULL;

    delete [] fAverageLevelsFiltered;
    fAverageLevelsFiltered = NULL;

    delete [] nOverflows;
    nOverflows = NULL;

    delete audioFilePlayer;
    audioFilePlayer = NULL;
}


void KmeterAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    // This is the place where you'd normally do the guts of your
    // plug-in's audio processing...

    if (!bSampleRateIsValid)
    {
        int nNumSamples = buffer.getNumSamples();
        int nNumChannels = getNumInputChannels();

        // In case we have more outputs than inputs, we'll clear any
        // output channels that didn't contain input data, because these
        // aren't guaranteed to be empty -- they may contain garbage.

        if (getNumOutputChannels() > nNumChannels)
        {
            nNumChannels = getNumOutputChannels();
        }

        for (int i = 0; i < nNumChannels; i++)
        {
            buffer.clear(i, 0, nNumSamples);
        }

        return;
    }

    if (nNumInputChannels < 1)
    {
        DBG("[K-Meter] nNumInputChannels < 1");
        return;
    }

    if (audioFilePlayer)
    {
        audioFilePlayer->fillBufferChunk(&buffer);
    }

    bool bMono = getParameterAsBool(KmeterPluginParameters::selMono);
    int nNumSamples = buffer.getNumSamples();

    // convert stereo input to mono if "Mono" button has been pressed
    if (isStereo && bMono)
    {
        float* output_left = buffer.getSampleData(0);
        float* output_right = buffer.getSampleData(1);

        for (int i = 0; i < nNumSamples; i++)
        {
            output_left[i] = 0.5f * (output_left[i] + output_right[i]);
            output_right[i] = output_left[i];
        }
    }

    pRingBufferInput->addSamples(buffer, 0, nNumSamples);

    nSamplesInBuffer += nNumSamples;
    nSamplesInBuffer %= KMETER_BUFFER_SIZE;

    pRingBufferOutput->copyToBuffer(buffer, 0, nNumSamples, KMETER_BUFFER_SIZE - nSamplesInBuffer);

    // In case we have more outputs than inputs, we'll clear any
    // output channels that didn't contain input data, because these
    // aren't guaranteed to be empty -- they may contain garbage.

    for (int i = nNumInputChannels; i < getNumOutputChannels(); i++)
    {
        buffer.clear(i, 0, nNumSamples);
    }
}


void KmeterAudioProcessor::processBufferChunk(AudioSampleBuffer& buffer, const unsigned int uChunkSize, const unsigned int uBufferPosition, const unsigned int uProcessedSamples)
{
    unsigned int uPreDelay = uChunkSize / 2;
    bool bMono = getParameterAsBool(KmeterPluginParameters::selMono);

    // length of buffer chunk in fractional seconds
    // (1024 samples / 44100 samples/s = 23.2 ms)
    fProcessedSeconds = (float) uChunkSize / (float) getSampleRate();

    // copy ring buffer to determine average level (FIR filter already
    // adds delay of (uChunkSize / 2) samples)
    pAverageLevelFilteredRms->copyFromBuffer(*pRingBufferInput, 0, (int) getSampleRate());

    for (int nChannel = 0; nChannel < nNumInputChannels; nChannel++)
    {
        if (bMono && (nChannel == 1))
        {
            fPeakLevels[nChannel] = fPeakLevels[0];
            fRmsLevels[nChannel] = fRmsLevels[0];
            fAverageLevelsFiltered[nChannel] = fAverageLevelsFiltered[0];
            nOverflows[nChannel] = nOverflows[0];
        }
        else
        {
            // determine peak level for uChunkSize samples (use pre-delay)
            fPeakLevels[nChannel] = pRingBufferInput->getMagnitude(nChannel, uChunkSize, uPreDelay);

            // determine peak level for uChunkSize samples (use pre-delay)
            fRmsLevels[nChannel] = pRingBufferInput->getRMSLevel(nChannel, uChunkSize, uPreDelay);

            // determine filtered average level for uChunkSize samples
            fAverageLevelsFiltered[nChannel] = pAverageLevelFilteredRms->getLevel(nChannel);

            // determine overflows for uChunkSize samples (use pre-delay)
            nOverflows[nChannel] = countOverflows(pRingBufferInput, nChannel, uChunkSize, uPreDelay);
        }

        // apply meter ballistics and store values so that the editor
        // can access them
        pMeterBallistics->updateChannel(nChannel, fProcessedSeconds, fPeakLevels[nChannel], fRmsLevels[nChannel], fAverageLevelsFiltered[nChannel], nOverflows[nChannel]);
    }

    // phase correlation is only defined for stereo signals
    if (isStereo)
    {
        float fPhaseCorrelation = 1.0f;

        // check whether the stereo signal has been mixed down to mono
        if (bMono)
        {
            fPhaseCorrelation = 1.0f;
        }
        // otherwise, process only levels at or above -80 dB
        else if ((fAverageLevelsFiltered[0] >= 0.0001f) || (fAverageLevelsFiltered[1] >= 0.0001f))
        {
            float sum_of_product = 0.0f;
            float sum_of_squares_left = 0.0f;
            float sum_of_squares_right = 0.0f;

            // determine correlation for uChunkSize samples (use pre-delay)
            for (unsigned int uSample = 0; uSample < uChunkSize; uSample++)
            {
                float ringbuffer_left = pRingBufferInput->getSample(0, uSample, uPreDelay);
                float ringbuffer_right = pRingBufferInput->getSample(1, uSample, uPreDelay);

                sum_of_product += ringbuffer_left * ringbuffer_right;
                sum_of_squares_left += ringbuffer_left * ringbuffer_left;
                sum_of_squares_right += ringbuffer_right * ringbuffer_right;
            }

            float fSumsOfSquares = sum_of_squares_left * sum_of_squares_right;

            // prevent division by zero and taking the square root of
            // a negative number
            if (fSumsOfSquares > 0.0f)
            {
                fPhaseCorrelation = sum_of_product / sqrt(fSumsOfSquares);
            }
            else
            {
                // this is mathematically incorrect, but "musically"
                // correct (i.e. signal is mono-compatible)
                fPhaseCorrelation = 1.0f;
            }
        }

        pMeterBallistics->setPhaseCorrelation(fProcessedSeconds, fPhaseCorrelation);

        float fStereoMeterValue = 0.0f;

        // do not process levels below -80 dB
        if ((fAverageLevelsFiltered[0] < 0.0001f) && (fAverageLevelsFiltered[1] < 0.0001f))
        {
            fStereoMeterValue = 0.0f;
        }
        else if (fAverageLevelsFiltered[1] >= fAverageLevelsFiltered[0])
        {
            fStereoMeterValue = (fAverageLevelsFiltered[1] - fAverageLevelsFiltered[0]) / fAverageLevelsFiltered[1];
        }
        else
        {
            fStereoMeterValue = (fAverageLevelsFiltered[1] - fAverageLevelsFiltered[0]) / fAverageLevelsFiltered[0];
        }

        pMeterBallistics->setStereoMeterValue(fProcessedSeconds, fStereoMeterValue);
    }

    sendChangeMessage(this);

    // To hear the audio source after average filtering, simply set
    // DEBUG_FILTER to 1.  Please remember to disable this setting
    // before committing your changes.
    if (DEBUG_FILTER)
    {
        pAverageLevelFilteredRms->copyToBuffer(*pRingBufferOutput, 0, uChunkSize);
    }
    else
    {
        AudioSampleBuffer TempAudioBuffer = AudioSampleBuffer(nNumInputChannels, uChunkSize);
        pRingBufferInput->copyToBuffer(TempAudioBuffer, 0, uChunkSize, 0);
        pRingBufferOutput->addSamples(TempAudioBuffer, 0, uChunkSize);
    }
}


void KmeterAudioProcessor::startValidation(File fileAudio, int nSelectedChannel, bool bPeakMeterLevel, bool bAverageMeterLevel, bool bStereoMeterValue, bool bPhaseCorrelation)
{
    audioFilePlayer = new AudioFilePlayer(fileAudio, (int) getSampleRate(), pMeterBallistics);
    audioFilePlayer->setReporters(nSelectedChannel, bPeakMeterLevel, bAverageMeterLevel, bStereoMeterValue, bPhaseCorrelation);

    // refresh editor
    sendChangeMessage(NULL);
}


void KmeterAudioProcessor::stopValidation()
{
    delete audioFilePlayer;
    audioFilePlayer = NULL;

    // refresh editor
    sendChangeMessage(NULL);
}


bool KmeterAudioProcessor::isValidating()
{
    if (audioFilePlayer == NULL)
    {
        return false;
    }
    else
    {
        if (audioFilePlayer->isPlaying())
        {
            return true;
        }
        else
        {
            stopValidation();
            return false;
        }
    }
}


int KmeterAudioProcessor::countOverflows(AudioRingBuffer* ring_buffer, const unsigned int channel, const unsigned int length, const unsigned int pre_delay)
{
    // initialise number of overflows in this buffer
    int nOverflows = 0;

    // loop through samples of buffer
    for (unsigned int uSample = 0; uSample < length; uSample++)
    {
        // get current sample value
        float fSampleValue = ring_buffer->getSample(channel, uSample, pre_delay);

        // current sample reaches or exceeds digital full scale; treat as
        // overflow
        if ((fSampleValue <= -1.0f) || (fSampleValue >= 1.0f))
        {
            nOverflows++;
        }
    }

    // return number of overflows in this buffer
    return nOverflows;
}


MeterBallistics* KmeterAudioProcessor::getLevels()
{
    return pMeterBallistics;
}

//==============================================================================

AudioProcessorEditor* KmeterAudioProcessor::createEditor()
{
    if (nNumInputChannels > 0)
    {
        return new KmeterAudioProcessorEditor(this, nNumInputChannels);
    }
    else
    {
        return new KmeterAudioProcessorEditor(this, JucePlugin_MaxNumInputChannels);
    }
}

bool KmeterAudioProcessor::hasEditor() const
{
    return true;
}

//==============================================================================

void KmeterAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    copyXmlToBinary(pPluginParameters->storeAsXml(), destData);
}


void KmeterAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    ScopedPointer<XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    pPluginParameters->loadFromXml(xml);
}

//==============================================================================

// This creates new instances of the plug-in..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KmeterAudioProcessor();
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
