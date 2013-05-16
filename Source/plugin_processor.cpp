/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2013 Martin Zuther (http://www.mzuther.de/)

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

    pAverageLevelFiltered = NULL;
    pPluginParameters = new KmeterPluginParameters();

    // depends on "KmeterPluginParameters"!
    nAverageAlgorithm = getParameterAsInt(KmeterPluginParameters::selAverageAlgorithm);

    fProcessedSeconds = 0.0f;

    fPeakLevels = NULL;
    fRmsLevels = NULL;
    fAverageLevelsFiltered = NULL;

    nOverflows = NULL;
}


KmeterAudioProcessor::~KmeterAudioProcessor()
{
    removeAllActionListeners();

    // call function "releaseResources()" by force to make sure all
    // allocated memory is freed
    releaseResources();

    delete pPluginParameters;
    pPluginParameters = NULL;

    delete audioFilePlayer;
    audioFilePlayer = NULL;
}


void KmeterAudioProcessor::addActionListenerParameters(ActionListener* listener) throw()
{
    pPluginParameters->addActionListener(listener);
}


void KmeterAudioProcessor::removeActionListenerParameters(ActionListener* listener) throw()
{
    pPluginParameters->removeActionListener(listener);
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

    if (index == KmeterPluginParameters::selAverageAlgorithm)
    {
        setAverageAlgorithm(getParameterAsInt(index));
    }
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
    else if (index == KmeterPluginParameters::selCrestFactor)
    {
        if (isValidating())
        {
            audioFilePlayer->setCrestFactor(nValue);
        }
    }

    beginParameterChangeGesture(index);

    float newValue = pPluginParameters->translateParameterToFloat(index, nValue);
    setParameterNotifyingHost(index, newValue);

    endParameterChangeGesture(index);

    if (index == KmeterPluginParameters::selAverageAlgorithm)
    {
        setAverageAlgorithm(nValue);
    }
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


const String KmeterAudioProcessor::getInputChannelName(int channelIndex) const
{
    return "Input " + String(channelIndex + 1);
}


const String KmeterAudioProcessor::getOutputChannelName(int channelIndex) const
{
    return "Output " + String(channelIndex + 1);
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


bool KmeterAudioProcessor::silenceInProducesSilenceOut() const
{
    return true;
}


double KmeterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
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
        Logger::outputDebugString("[K-Meter] WARNING: sample rate of " + String(sampleRate) + " Hz not supported");
        bSampleRateIsValid = false;
        return;
    }
    else
    {
        bSampleRateIsValid = true;
    }

    nNumInputChannels = getNumInputChannels();
    isStereo = (nNumInputChannels == 2);

    DBG("[K-Meter] number of input channels: " + String(nNumInputChannels));

    pMeterBallistics = new MeterBallistics(nNumInputChannels, nAverageAlgorithm, false, false);

    fPeakLevels = new float[nNumInputChannels];
    fRmsLevels = new float[nNumInputChannels];
    fAverageLevelsFiltered = new float[nNumInputChannels];

    nOverflows = new int[nNumInputChannels];

    for (int nChannel = 0; nChannel < nNumInputChannels; nChannel++)
    {
        fPeakLevels[nChannel] = 0.0f;
        fRmsLevels[nChannel] = 0.0f;
        fAverageLevelsFiltered[nChannel] = MeterBallistics::getMeterMinimumDecibel();

        nOverflows[nChannel] = 0;
    }

    pAverageLevelFiltered = new AverageLevelFiltered(this, nNumInputChannels, KMETER_BUFFER_SIZE, (int) sampleRate, nAverageAlgorithm);

    // make sure that ring buffer can hold at least KMETER_BUFFER_SIZE
    // samples and is large enough to receive a full block of audio
    nSamplesInBuffer = 0;
    unsigned int uRingBufferSize = (samplesPerBlock > KMETER_BUFFER_SIZE) ? samplesPerBlock : KMETER_BUFFER_SIZE;

    pRingBufferInput = new AudioRingBuffer("Input ring buffer", nNumInputChannels, uRingBufferSize, KMETER_BUFFER_SIZE, KMETER_BUFFER_SIZE);
    pRingBufferInput->setCallbackClass(this);

    pRingBufferOutput = new AudioRingBuffer("Output ring buffer", nNumInputChannels, uRingBufferSize, KMETER_BUFFER_SIZE, KMETER_BUFFER_SIZE);
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

    delete pAverageLevelFiltered;
    pAverageLevelFiltered = NULL;

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

        // make sure we'll clear all output channels
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
        Logger::outputDebugString("[K-Meter] nNumInputChannels < 1");
        return;
    }

    int nNumSamples = buffer.getNumSamples();

    // In case we have more outputs than inputs, we'll clear any
    // output channels that didn't contain input data, because these
    // aren't guaranteed to be empty -- they may contain garbage.

    for (int i = nNumInputChannels; i < getNumOutputChannels(); i++)
    {
        buffer.clear(i, 0, nNumSamples);
    }

    if (audioFilePlayer)
    {
        audioFilePlayer->fillBufferChunk(&buffer);
    }

    bool bMono = getParameterAsBool(KmeterPluginParameters::selMono);

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
    pAverageLevelFiltered->copyFromBuffer(*pRingBufferInput, 0, (int) getSampleRate());

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
            // (please note that this level has already been converted
            // to decibels!)
            fAverageLevelsFiltered[nChannel] = pAverageLevelFiltered->getLevel(nChannel);

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
        else if ((fRmsLevels[0] >= 0.0001f) || (fRmsLevels[1] >= 0.0001f))
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
        if ((fRmsLevels[0] < 0.0001f) && (fRmsLevels[1] < 0.0001f))
        {
            fStereoMeterValue = 0.0f;
        }
        else if (fRmsLevels[1] >= fRmsLevels[0])
        {
            fStereoMeterValue = 1.0f - fRmsLevels[0] / fRmsLevels[1];
        }
        else
        {
            fStereoMeterValue = fRmsLevels[1] / fRmsLevels[0] - 1.0f;
        }

        pMeterBallistics->setStereoMeterValue(fProcessedSeconds, fStereoMeterValue);
    }

    // "UM" --> update meters
    sendActionMessage("UM");

    // To hear the audio source after average filtering, simply set
    // DEBUG_FILTER to 1.  Please remember to disable this setting
    // before committing your changes.
    if (DEBUG_FILTER)
    {
        pAverageLevelFiltered->copyToBuffer(*pRingBufferOutput, 0, uChunkSize);
    }
    else
    {
        AudioSampleBuffer TempAudioBuffer = AudioSampleBuffer(nNumInputChannels, uChunkSize);
        pRingBufferInput->copyToBuffer(TempAudioBuffer, 0, uChunkSize, 0);
        pRingBufferOutput->addSamples(TempAudioBuffer, 0, uChunkSize);
    }
}


void KmeterAudioProcessor::startValidation(File fileAudio, int nSelectedChannel, bool bReportCSV, bool bAverageMeterLevel, bool bPeakMeterLevel, bool bMaximumPeakLevel, bool bStereoMeterValue, bool bPhaseCorrelation)
{
    // reset all meters before we start the validation
    pMeterBallistics->reset();

    int nCrestFactor = getParameterAsInt(KmeterPluginParameters::selCrestFactor);
    audioFilePlayer = new AudioFilePlayer(fileAudio, (int) getSampleRate(), pMeterBallistics, nCrestFactor);
    audioFilePlayer->setReporters(nSelectedChannel, bReportCSV, bAverageMeterLevel, bPeakMeterLevel, bMaximumPeakLevel, bStereoMeterValue, bPhaseCorrelation);

    // refresh editor; "V+" --> validation started
    sendActionMessage("V+");
}


void KmeterAudioProcessor::stopValidation()
{
    delete audioFilePlayer;
    audioFilePlayer = NULL;

    // refresh editor; "V-" --> validation stopped
    sendActionMessage("V-");
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

        // in the 16-bit domain, full scale corresponds to an absolute
        // integer value of 32'767 or 32'768, so we'll treat absolute
        // levels of 32'767 and above as overflows; this corresponds
        // to a floating-point level of 32'767 / 32'768 = 0.9999694
        // (approx. -0.001 dBFS).
        if ((fSampleValue < -0.9999f) || (fSampleValue > 0.9999f))
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


int KmeterAudioProcessor::getAverageAlgorithm()
{
    return nAverageAlgorithm;
}


void KmeterAudioProcessor::setAverageAlgorithm(const int average_algorithm)
{
    if (average_algorithm != nAverageAlgorithm)
    {
        pAverageLevelFiltered->setAlgorithm(average_algorithm);
    }
}


void KmeterAudioProcessor::setAverageAlgorithmFinal(const int average_algorithm)
{
    nAverageAlgorithm = average_algorithm;
    pMeterBallistics->setAverageAlgorithm(nAverageAlgorithm);

    //  the level averaging alghorithm has been changed, so update the
    // "RMS" and "ITU-R" buttons to make sure that the correct button
    // is lit
    //
    // "AC" --> algorithm changed
    sendActionMessage("AC");
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

// This creates new instances of the plug-in.
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KmeterAudioProcessor();
}


AudioProcessor* JUCE_CALLTYPE createPluginFilterOfType(AudioProcessor::WrapperType)
{
    return new KmeterAudioProcessor();
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
