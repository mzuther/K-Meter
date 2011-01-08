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

    pRingBufferInput = NULL;
    pRingBufferOutput = NULL;

    nNumInputChannels = 0;
    pMeterBallistics = NULL;

    setLatencySamples(KMETER_BUFFER_SIZE);

    pAverageLevelFilteredRms = new AverageLevelFilteredRms(2, KMETER_BUFFER_SIZE);
    pPluginParameters = new KmeterPluginParameters();

    fProcessedSeconds = 0.0f;

    fPeakLevels = NULL;
    fAverageLevels = NULL;

    nOverflows = NULL;
    bOverflowsPreviousSample = NULL;
}


KmeterAudioProcessor::~KmeterAudioProcessor()
{
    removeAllChangeListeners();

    releaseResources();

    delete pAverageLevelFilteredRms;
    pAverageLevelFilteredRms = NULL;

    delete pMeterBallistics;
    pMeterBallistics = NULL;

    delete pPluginParameters;
    pPluginParameters = NULL;
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
    return pPluginParameters->getNumParameters();
}


float KmeterAudioProcessor::getParameter(int index)
{
    // This method will be called by the host, probably on the audio
    // thread, so it's absolutely time-critical. Don't use critical
    // sections or anything UI-related, or anything at all that may
    // block in any way!

    return pPluginParameters->getParameterAsFloat(index);
}


void KmeterAudioProcessor::setParameter(int index, float newValue)
{
    // This method will be called by the host, probably on the audio
    // thread, so it's absolutely time-critical. Don't use critical
    // sections or anything UI-related, or anything at all that may
    // block in any way!

    pPluginParameters->setParameterFromFloat(index, newValue);
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
    if ((index == KmeterPluginParameters::selMono) && (pMeterBallistics->getNumberOfChannels() < 2))
    {
        nValue = true;
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
    return String(channelIndex + 1);
}


const String KmeterAudioProcessor::getOutputChannelName(const int channelIndex) const
{
    return String(channelIndex + 1);
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

    nNumInputChannels = getNumInputChannels();
    isStereo = (nNumInputChannels == 2);

    DBG("[K-Meter] in method KmeterAudioProcessor::prepareToPlay()");
    DBG(String("[K-Meter] nNumInputChannels:  ") + String(nNumInputChannels));

    fPeakLevels = new float[nNumInputChannels];
    fAverageLevels = new float[nNumInputChannels];

    nOverflows = new int[nNumInputChannels];
    bOverflowsPreviousSample = new bool[nNumInputChannels];

    for (int nChannel = 0; nChannel < nNumInputChannels; nChannel++)
    {
        fPeakLevels[nChannel] = 0.0f;
        fAverageLevels[nChannel] = 0.0f;

        nOverflows[nChannel] = 0;
        bOverflowsPreviousSample[nChannel] = false;
    }

    pMeterBallistics = new MeterBallistics(nNumInputChannels, false, false);

    // make sure that ring buffer can hold at least KMETER_BUFFER_SIZE
    // samples and is large enough to receive a full block of audio
    nSamplesInBuffer = 0;
    unsigned int uRingBufferSize = (samplesPerBlock > KMETER_BUFFER_SIZE) ? samplesPerBlock : KMETER_BUFFER_SIZE;

    pRingBufferInput = new AudioRingBuffer(T("Input ring buffer"), 2, uRingBufferSize, KMETER_BUFFER_SIZE, KMETER_BUFFER_SIZE);
    pRingBufferInput->setCallbackClass(this);

    pRingBufferOutput = new AudioRingBuffer(T("Output ring buffer"), 2, uRingBufferSize, KMETER_BUFFER_SIZE, KMETER_BUFFER_SIZE);
}


void KmeterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free
    // up any spare memory, etc.

    DBG("[K-Meter] in method KmeterAudioProcessor::releaseResources()");

    delete pMeterBallistics;
    pMeterBallistics = NULL;

    delete pRingBufferOutput;
    pRingBufferOutput = NULL;

    delete pRingBufferInput;
    pRingBufferInput = NULL;

    delete [] fPeakLevels;
    fPeakLevels = NULL;

    delete [] fAverageLevels;
    fAverageLevels = NULL;

    delete [] nOverflows;
    nOverflows = NULL;

    delete [] bOverflowsPreviousSample;
    bOverflowsPreviousSample = NULL;
}


void KmeterAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    // This is the place where you'd normally do the guts of your
    // plugin's audio processing...

    if (nNumInputChannels < 1)
    {
        DBG("[K-Meter] nNumInputChannels < 1");
        return;
    }

    bool bMono = pPluginParameters->getParameterAsBool(KmeterPluginParameters::selMono);
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

    // In case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).

    for (int i = nNumInputChannels; i < getNumOutputChannels(); ++i)
    {
        buffer.clear(i, 0, nNumSamples);
    }
}


void KmeterAudioProcessor::processBufferChunk(AudioSampleBuffer& buffer, const unsigned int uChunkSize, const unsigned int uBufferPosition, const unsigned int uProcessedSamples)
{
    unsigned int uPreDelay = uChunkSize / 2;
    bool bMono = pPluginParameters->getParameterAsBool(KmeterPluginParameters::selMono);

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
            fAverageLevels[nChannel] = fAverageLevels[0];
            nOverflows[nChannel] = nOverflows[0];
        }
        else
        {
            // determine peak level for uChunkSize samples (use pre-delay)
            fPeakLevels[nChannel] = pRingBufferInput->getMagnitude(nChannel, uChunkSize, uPreDelay);

            // determine average level for uChunkSize samples
            fAverageLevels[nChannel] = pAverageLevelFilteredRms->getLevel(nChannel);

            // determine overflows for uChunkSize samples (use pre-delay)
            nOverflows[nChannel] = countOverflows(pRingBufferInput, nChannel, uChunkSize, uPreDelay, bOverflowsPreviousSample[nChannel]);
        }

        // apply meter ballistics and store values so that the editor
        // can access them
        pMeterBallistics->updateChannel(nChannel, fProcessedSeconds, fPeakLevels[nChannel], fAverageLevels[nChannel], nOverflows[nChannel]);
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
        else if ((fAverageLevels[0] >= 0.0001f) || (fAverageLevels[1] >= 0.0001f))
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

            // TODO: what about division by zero?
            fPhaseCorrelation = sum_of_product / sqrt(sum_of_squares_left * sum_of_squares_right);
        }

        pMeterBallistics->setPhaseCorrelation(fProcessedSeconds, fPhaseCorrelation);

        float fStereoMeterValue = 0.0f;

        // do not process levels below -80 dB
        if ((fAverageLevels[0] < 0.0001f) && (fAverageLevels[1] < 0.0001f))
        {
            fStereoMeterValue = 0.0f;
        }
        else if (fAverageLevels[1] >= fAverageLevels[0])
        {
            fStereoMeterValue = (fAverageLevels[1] - fAverageLevels[0]) / fAverageLevels[1];
        }
        else
        {
            fStereoMeterValue = (fAverageLevels[1] - fAverageLevels[0]) / fAverageLevels[0];
        }

        pMeterBallistics->setStereoMeterValue(fProcessedSeconds, fStereoMeterValue);
    }

    sendChangeMessage(this);

    // To hear the audio source after average filtering, simply set
    // DEBUG_FILTER to 1.  Please remember to disable this setting
    // before commiting your changes.
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


int KmeterAudioProcessor::countOverflows(AudioRingBuffer* ring_buffer, const unsigned int channel, const unsigned int length, const unsigned int pre_delay, bool& bPreviousSampleOver)
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
            // previous sample did not reach or exceed digital full scale,
            // so count current sample as overflow and remember this
            if (!bPreviousSampleOver)
            {
                nOverflows++;
                bPreviousSampleOver = true;
            }
        }
        // current sample does not reach digital full scale, so reset
        // bPreviousSampleOver
        else
        {
            bPreviousSampleOver = false;
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
    return new KmeterAudioProcessorEditor(this);
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

// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KmeterAudioProcessor();
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
