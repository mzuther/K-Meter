/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2018 Martin Zuther (http://www.mzuther.de/)

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


// To hear the audio source after average filtering, simply set
// DEBUG_FILTER to "true".  Please remember to revert this variable to
// "false" before committing your changes.
const bool DEBUG_FILTER = false;

/*==============================================================================

Flow of parameter processing:

  Editor:      buttonClicked(button) / sliderValueChanged(slider)
  Processor:   changeParameter(nIndex, fValue)
  Processor:   setParameter(nIndex, fValue)
  Parameters:  setFloat(nIndex, fValue)
  Editor:      actionListenerCallback(strMessage)
  Editor:      updateParameter(nIndex)

==============================================================================*/

KmeterAudioProcessor::KmeterAudioProcessor() :
#ifndef JucePlugin_PreferredChannelConfigurations
    AudioProcessor(getBusesProperties()),
#endif
    kmeterBufferSize_(1024)
{
    frut::Frut::printVersionNumbers();

    if (DEBUG_FILTER)
    {
        Logger::outputDebugString("********************************************************************************");
        Logger::outputDebugString("** Debugging average filtering.  Please reset DEBUG_FILTER before committing! **");
        Logger::outputDebugString("********************************************************************************");
    }

    sampleRateIsValid_ = false;
    isStereo_ = true;
    isSilent_ = false;

    attenuationLevel_ = 1.0f;

    setLatencySamples(kmeterBufferSize_);

    // depends on "KmeterPluginParameters"!
    averageAlgorithmId_ = getRealInteger(
                              KmeterPluginParameters::selAverageAlgorithm);

    processedSeconds_ = 0.0f;
}


KmeterAudioProcessor::~KmeterAudioProcessor()
{
    removeAllActionListeners();
}


AudioProcessor::BusesProperties KmeterAudioProcessor::getBusesProperties()
{
#ifdef KMETER_SURROUND

    return BusesProperties()
           .withInput("Main In",
                      AudioChannelSet::create5point1())
           .withOutput("Main Out",
                       AudioChannelSet::create5point1());

#else

    return BusesProperties()
           .withInput("Main In",
                      AudioChannelSet::stereo())
           .withOutput("Main Out",
                       AudioChannelSet::stereo());

#endif
}


#ifndef JucePlugin_PreferredChannelConfigurations
bool KmeterAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const
{
    // main bus: do not allow differing input and output layouts
    if (layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet())
    {
        return false;
    }

    // main bus: do not allow disabling of input channels
    if (layouts.getMainInputChannelSet().isDisabled())
    {
        return false;
    }

#ifdef KMETER_SURROUND

    // main bus with stereo input ==> okay
    if (layouts.getMainInputChannelSet() == AudioChannelSet::stereo())
    {
        return true;
    }

    // main bus with 5.0 input ==> okay
    if (layouts.getMainInputChannelSet() == AudioChannelSet::create5point0())
    {
        return true;
    }

    // main bus with 5.1 input ==> okay
    if (layouts.getMainInputChannelSet() == AudioChannelSet::create5point1())
    {
        return true;
    }

#else

    // main bus with mono input ==> okay
    if (layouts.getMainInputChannelSet() == AudioChannelSet::mono())
    {
        return true;
    }

    // main bus with stereo input ==> okay
    if (layouts.getMainInputChannelSet() == AudioChannelSet::stereo())
    {
        return true;
    }

#endif

    // current channel layout is not allowed
    return false;
}
#endif


const String KmeterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}


int KmeterAudioProcessor::getNumParameters()
{
    return pluginParameters_.getNumParameters(false);
}


const String KmeterAudioProcessor::getParameterName(
    int nIndex)
{
    return pluginParameters_.getName(nIndex);
}


const String KmeterAudioProcessor::getParameterText(
    int nIndex)
{
    return pluginParameters_.getText(nIndex);
}


float KmeterAudioProcessor::getParameter(
    int nIndex)
{
    // This method will be called by the host, probably on the audio
    // thread, so it's absolutely time-critical. Don't use critical
    // sections or anything GUI-related, or anything at all that may
    // block in any way!

    return pluginParameters_.getFloat(nIndex);
}


void KmeterAudioProcessor::changeParameter(
    int nIndex,
    float fValue)
{
    // This method will be called by the host, probably on the audio
    // thread, so it's absolutely time-critical. Don't use critical
    // sections or anything GUI-related, or anything at all that may
    // block in any way!

    if (nIndex == KmeterPluginParameters::selMono)
    {
        // automatically enable "Mono" button for mono channels
        if (getMainBusNumInputChannels() == 1)
        {
            fValue = 1.0f;
        }
        // automatically disable "Mono" button for multi-channel audio
        else if (getMainBusNumInputChannels() > 2)
        {
            fValue = 0.0f;
        }
    }

    // notify host of parameter change (this will automatically call
    // "setParameter"!)
    beginParameterChangeGesture(nIndex);
    setParameterNotifyingHost(nIndex, fValue);
    endParameterChangeGesture(nIndex);
}


void KmeterAudioProcessor::setParameter(
    int nIndex,
    float fValue)
{
    // This method will be called by the host, probably on the audio
    // thread, so it's absolutely time-critical. Don't use critical
    // sections or anything GUI-related, or anything at all that may
    // block in any way!

    // Please only call this method directly for non-automatable
    // values!

    pluginParameters_.setFloat(nIndex, fValue);

    // notify plug-in editor of parameter change
    if (pluginParameters_.hasChanged(nIndex))
    {
        // for visible parameters, notify the editor of changes (this
        // will also clear the change flag)
        if (nIndex < pluginParameters_.getNumParameters(false))
        {
            if (nIndex == KmeterPluginParameters::selCrestFactor)
            {
                if (audioFilePlayer_)
                {
                    audioFilePlayer_->setCrestFactor(getRealInteger(nIndex));
                }
            }
            else if (nIndex == KmeterPluginParameters::selAverageAlgorithm)
            {
                setAverageAlgorithm(getRealInteger(nIndex));
            }

            // "PC" ==> parameter changed, followed by a hash and the
            // parameter's ID
            sendActionMessage("PC#" + String(nIndex));
        }
        // for hidden parameters, we only have to clear the change
        // flag
        else
        {
            pluginParameters_.clearChangeFlag(nIndex);
        }
    }
}


void KmeterAudioProcessor::clearChangeFlag(
    int nIndex)
{
    pluginParameters_.clearChangeFlag(nIndex);
}


bool KmeterAudioProcessor::hasChanged(
    int nIndex)
{
    return pluginParameters_.hasChanged(nIndex);
}


void KmeterAudioProcessor::updateParameters(
    bool bIncludeHiddenParameters)
{
    int nNumParameters = pluginParameters_.getNumParameters(false);

    for (int nIndex = 0; nIndex < nNumParameters; ++nIndex)
    {
        if (pluginParameters_.hasChanged(nIndex))
        {
            float fValue = pluginParameters_.getFloat(nIndex);
            changeParameter(nIndex, fValue);
        }
    }

    if (bIncludeHiddenParameters)
    {
        // handle hidden parameters here!

        // the following parameters need no updating:
        //
        // * selValidationFileName
        // * selValidationSelectedChannel
        // * selValidationAverageMeterLevel
        // * selValidationPeakMeterLevel
        // * selValidationMaximumPeakLevel
        // * selValidationStereoMeterValue
        // * selValidationPhaseCorrelation
        // * selValidationCSVFormat
        // * selSkinName
    }
}


bool KmeterAudioProcessor::getBoolean(
    int nIndex)
{
    // This method will be called by the host, probably on the audio
    // thread, so it's absolutely time-critical. Don't use critical
    // sections or anything GUI-related, or anything at all that may
    // block in any way!

    return pluginParameters_.getBoolean(nIndex);
}


int KmeterAudioProcessor::getRealInteger(
    int nIndex)
{
    // This method will be called by the host, probably on the audio
    // thread, so it's absolutely time-critical. Don't use critical
    // sections or anything GUI-related, or anything at all that may
    // block in any way!

    return pluginParameters_.getRealInteger(nIndex);
}


File KmeterAudioProcessor::getParameterValidationFile()
{
    // This method will be called by the host, probably on the audio
    // thread, so it's absolutely time-critical. Don't use critical
    // sections or anything GUI-related, or anything at all that may
    // block in any way!

    return pluginParameters_.getValidationFile();
}


void KmeterAudioProcessor::setParameterValidationFile(
    const File &fileValidation)
{
    // This method will be called by the host, probably on the audio
    // thread, so it's absolutely time-critical. Don't use critical
    // sections or anything GUI-related, or anything at all that may
    // block in any way!

    pluginParameters_.setValidationFile(fileValidation);
}


String KmeterAudioProcessor::getParameterSkinName()
{
    // This method will be called by the host, probably on the audio
    // thread, so it's absolutely time-critical. Don't use critical
    // sections or anything GUI-related, or anything at all that may
    // block in any way!

    return pluginParameters_.getSkinName();
}


void KmeterAudioProcessor::setParameterSkinName(
    const String &strSkinName)
{
    // This method will be called by the host, probably on the audio
    // thread, so it's absolutely time-critical. Don't use critical
    // sections or anything GUI-related, or anything at all that may
    // block in any way!

    pluginParameters_.setSkinName(strSkinName);
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


double KmeterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}


int KmeterAudioProcessor::getNumPrograms()
{
    return 0;
}


int KmeterAudioProcessor::getCurrentProgram()
{
    return 0;
}


void KmeterAudioProcessor::setCurrentProgram(
    int nIndex)
{
    ignoreUnused(nIndex);
}


const String KmeterAudioProcessor::getProgramName(
    int nIndex)
{
    ignoreUnused(nIndex);

    return String();
}


void KmeterAudioProcessor::changeProgramName(
    int nIndex,
    const String &newName)
{
    ignoreUnused(nIndex, newName);
}


void KmeterAudioProcessor::prepareToPlay(
    double sampleRate,
    int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    Logger::outputDebugString("[K-Meter] preparing to play");

    if ((sampleRate < 44100) || (sampleRate > 192000))
    {
        Logger::outputDebugString("[K-Meter] WARNING: sample rate of " +
                                  String(sampleRate) + " Hz not supported");
        sampleRateIsValid_ = false;
        return;
    }
    else
    {
        sampleRateIsValid_ = true;
    }

    isSilent_ = false;

    if (getBoolean(KmeterPluginParameters::selMute))
    {
        attenuationLevel_ = 0.0f;
    }
    else if (getBoolean(KmeterPluginParameters::selDim))
    {
        attenuationLevel_ = 0.1f;
    }
    else
    {
        attenuationLevel_ = 1.0f;
    }

    int numInputChannels = getMainBusNumInputChannels();

    dither_.initialise(jmax(getMainBusNumInputChannels(),
                            getMainBusNumOutputChannels()),
                       24);

    Logger::outputDebugString("[K-Meter] number of input channels: " +
                              String(numInputChannels));
    Logger::outputDebugString("[K-Meter] number of output channels: " +
                              String(getMainBusNumOutputChannels()));

    isStereo_ = (numInputChannels == 2);

    meterBallistics_ = new MeterBallistics(
        numInputChannels, averageAlgorithmId_, false, false);

    peakLevels_.clear();
    rmsLevels_.clear();
    averageLevelsFiltered_.clear();
    truePeakLevels_.clear();

    overflowCounts_.clear();

    for (int channel = 0; channel < numInputChannels; ++channel)
    {
        peakLevels_.add(0.0f);
        rmsLevels_.add(0.0f);
        averageLevelsFiltered_.add(MeterBallistics::getMeterMinimumDecibel());
        truePeakLevels_.add(0.0f);

        overflowCounts_.add(0);
    }

    averageLevelFiltered_ = new AverageLevelFiltered(
        numInputChannels, (int) sampleRate, kmeterBufferSize_,
        averageAlgorithmId_);

    // maximum under-read of true peak measurement is 0.169 dB (see
    // Annex 2 of ITU-R BS.1770-4)
    int oversamplingFactor = 8;

    if (sampleRate >= 176400)
    {
        oversamplingFactor /= 4;
    }
    else if (sampleRate >= 88200)
    {
        oversamplingFactor /= 2;
    }

    truePeakMeter_ = new frut::dsp::TruePeakMeter(
        numInputChannels, kmeterBufferSize_, oversamplingFactor);

    // make sure that ring buffer can hold at least kmeterBufferSize_
    // samples and is large enough to receive a full block of audio
    int ringBufferSize = jmax(samplesPerBlock, kmeterBufferSize_);

    int preDelay = kmeterBufferSize_;
    int chunkSize = kmeterBufferSize_;

    ringBuffer_ = new frut::audio::RingBuffer<float>(
        numInputChannels,
        ringBufferSize,
        preDelay,
        chunkSize);

    ringBuffer_->setCallbackClass(this);
}


void KmeterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free
    // up any spare memory, etc.

    Logger::outputDebugString("[K-Meter] releasing resources");
    Logger::outputDebugString("");

    meterBallistics_ = nullptr;
    averageLevelFiltered_ = nullptr;
    truePeakMeter_ = nullptr;

    ringBuffer_ = nullptr;
}


void KmeterAudioProcessor::reset()
{
    // Use this method as the place to clear any delay lines, buffers,
    // etc, as it means there's been a break in the audio's
    // continuity.
}


void KmeterAudioProcessor::processBlock(
    AudioBuffer<float> &buffer,
    MidiBuffer &midiMessages)
{
    ignoreUnused(midiMessages);
    jassert(!isUsingDoublePrecision());

    // de-normalize samples
    dither_.denormalize(buffer);

    // process input samples
    process(buffer);
}


void KmeterAudioProcessor::processBlock(
    AudioBuffer<double> &buffer,
    MidiBuffer &midiMessages)
{
    ignoreUnused(midiMessages);
    jassert(isUsingDoublePrecision());

    int numberOfChannels = buffer.getNumChannels();
    int numberOfSamples = buffer.getNumSamples();

    AudioBuffer<float> processBuffer(numberOfChannels, numberOfSamples);

    // FIXME: copy ring buffer before processing

    // de-normalize samples
    dither_.denormalize(buffer);

    // dither input to float
    dither_.ditherToFloat(buffer, processBuffer);

    // process input samples
    process(processBuffer);

    // convert ouput to double
    dither_.convertToDouble(processBuffer, buffer);
}


void KmeterAudioProcessor::process(
    AudioBuffer<float> &buffer)
{
    int numberOfSamples = buffer.getNumSamples();

    if (!sampleRateIsValid_)
    {
        buffer.clear();
        return;
    }

    if (getMainBusNumInputChannels() < 1)
    {
        Logger::outputDebugString("[K-Meter] no input channels!");
        return;
    }

    // In case we have more outputs than inputs, we'll clear any
    // output channels that didn't contain input data, because these
    // aren't guaranteed to be empty -- they may contain garbage.

    for (int channel = getMainBusNumInputChannels(); channel < getMainBusNumOutputChannels(); ++channel)
    {
        buffer.clear(channel, 0, numberOfSamples);
    }

    if (audioFilePlayer_)
    {
        audioFilePlayer_->copyTo(buffer);
    }
    // silence input if validation window is open
    else if (isSilent_)
    {
        buffer.clear();
    }

    // process two channels only
    if (isStereo_)
    {
        float *inputLeft = buffer.getWritePointer(0);
        float *inputRight = buffer.getWritePointer(1);

        // "Mono" button has been pressed
        if (getBoolean(KmeterPluginParameters::selMono))
        {
            for (int i = 0; i < numberOfSamples; ++i)
            {
                // FIXME: dither
                inputLeft[i] = 0.5f * (inputLeft[i] + inputRight[i]);
                inputRight[i] = inputLeft[i];
            }
        }
        // "Flip" button has been pressed
        else if (getBoolean(KmeterPluginParameters::selFlip))
        {
            for (int i = 0; i < numberOfSamples; ++i)
            {
                float oldInputLeft = inputLeft[i];

                inputLeft[i] = inputRight[i];
                inputRight[i] = oldInputLeft;
            }
        }
    }

    // copy input buffer to ring buffer (applies pre-delay to input)
    //
    // calls "processBufferChunk" each time chunkSize samples have
    // been added!
    ringBuffer_->addFrom(buffer, 0, numberOfSamples);

    // copy ring buffer to output buffer
    ringBuffer_->removeTo(buffer, 0, numberOfSamples);

    // fade output attenuation from old to new value (JUCE takes care
    // of any optimizations)
    float oldAttenuationLevel = attenuationLevel_;

    if (getBoolean(KmeterPluginParameters::selMute))
    {
        attenuationLevel_ = 0.0f;
    }
    else if (getBoolean(KmeterPluginParameters::selDim))
    {
        attenuationLevel_ = 0.1f;
    }
    else
    {
        attenuationLevel_ = 1.0f;
    }

    // FIXME: dither
    // TODO: fade length depends on buffer size (1 ==> audible click)
    buffer.applyGainRamp(0, buffer.getNumSamples(),
                         oldAttenuationLevel, attenuationLevel_);
}


/// Called every time a certain number of samples have been added to a
/// RingBuffer.
///
/// @param buffer audio buffer with filled "chunk"
///
/// @return determines whether the audio buffer's contents should be
///         copied back to the original RingBuffer.
///
bool KmeterAudioProcessor::processBufferChunk(
    AudioBuffer<float> &buffer)
{
    int chunkSize = buffer.getNumSamples();
    bool isMono = getBoolean(KmeterPluginParameters::selMono);

    // length of buffer chunk in fractional seconds
    // (1024 samples / 44100 samples/s = 23.2 ms)
    processedSeconds_ = static_cast<float>(chunkSize) /
                        static_cast<float>(getSampleRate());

    // copy buffer to determine average level
    averageLevelFiltered_->copyFrom(buffer, chunkSize);

    // copy buffer to determine true peak level
    truePeakMeter_->copyFrom(buffer, chunkSize);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        if (isMono && (channel == 1))
        {
            peakLevels_.set(channel, peakLevels_[0]);
            rmsLevels_.set(channel, rmsLevels_[0]);
            averageLevelsFiltered_.set(channel, averageLevelsFiltered_[0]);
            truePeakLevels_.set(channel, truePeakLevels_[0]);

            overflowCounts_.set(channel, overflowCounts_[0]);
        }
        else
        {
            // determine peak level for chunkSize samples
            peakLevels_.set(
                channel,
                buffer.getMagnitude(channel, 0, chunkSize));

            // determine RMS level for chunkSize samples
            rmsLevels_.set(
                channel,
                buffer.getRMSLevel(channel, 0, chunkSize));

            // determine filtered average level for chunkSize samples
            // (please note that this level has already been converted
            // to decibels!)
            averageLevelsFiltered_.set(
                channel,
                averageLevelFiltered_->getLevel(channel));

            // determine true peak level for chunkSize samples
            truePeakLevels_.set(
                channel,
                truePeakMeter_->getLevel(channel));

            // determine overflows for chunkSize samples; treat all
            // samples above -0.001 dBFS as overflow
            overflowCounts_.set(
                channel,
                countOverflows(buffer, channel, chunkSize, 0.99885f));
        }

        // apply meter ballistics and store values so that the editor
        // can access them
        meterBallistics_->updateChannel(channel,
                                        processedSeconds_,
                                        peakLevels_[channel],
                                        truePeakLevels_[channel],
                                        averageLevelsFiltered_[channel],
                                        overflowCounts_[channel]);
    }

    // phase correlation is only defined for stereo signals
    if (isStereo_)
    {
        float phaseCorrelation = 1.0f;

        // check whether the stereo signal has been mixed down to mono
        if (isMono)
        {
            phaseCorrelation = 1.0f;
        }
        // otherwise, process only RMS levels at or above -80 dB
        else if ((rmsLevels_[0] >= 0.0001f) || (rmsLevels_[1] >= 0.0001f))
        {
            float sumOfProduct = 0.0f;
            float sumOfSquaresLeft = 0.0f;
            float sumOfSquaresRight = 0.0f;

            // determine correlation for chunkSize samples
            for (int sample = 0; sample < chunkSize; ++sample)
            {
                float leftChannel = buffer.getSample(0, sample);
                float rightChannel = buffer.getSample(1, sample);

                sumOfProduct += leftChannel * rightChannel;
                sumOfSquaresLeft += leftChannel * leftChannel;
                sumOfSquaresRight += rightChannel * rightChannel;
            }

            float sumsOfSquares = sumOfSquaresLeft * sumOfSquaresRight;

            // prevent division by zero and taking the square root of
            // a negative number
            if (sumsOfSquares > 0.0f)
            {
                phaseCorrelation = sumOfProduct / sqrtf(sumsOfSquares);
            }
            else
            {
                // this is mathematically incorrect, but "musically"
                // correct (i.e. signal is mono-compatible)
                phaseCorrelation = 1.0f;
            }
        }

        meterBallistics_->setPhaseCorrelation(processedSeconds_,
                                              phaseCorrelation);

        float stereoMeterValue = 0.0f;

        // do not process RMS levels below -80 dB
        if ((rmsLevels_[0] < 0.0001f) && (rmsLevels_[1] < 0.0001f))
        {
            stereoMeterValue = 0.0f;
        }
        else if (rmsLevels_[1] >= rmsLevels_[0])
        {
            stereoMeterValue = 1.0f - rmsLevels_[0] / rmsLevels_[1];
        }
        else
        {
            stereoMeterValue = rmsLevels_[1] / rmsLevels_[0] - 1.0f;
        }

        meterBallistics_->setStereoMeterValue(processedSeconds_,
                                              stereoMeterValue);
    }

    // "UM" ==> update meters
    sendActionMessage("UM");

    // To hear the audio source after average filtering, simply set
    // DEBUG_FILTER to "true".  Please remember to revert this
    // variable to "false" before committing your changes.
    if (DEBUG_FILTER)
    {
        // get average filter output
        averageLevelFiltered_->copyTo(buffer, chunkSize);

        // overwrite ring buffer contents
        return true;
    }
    else
    {
        // keep ring buffer contents
        return false;
    }
}


/// Count the number of overflows within a channel.
///
/// @param buffer audio buffer to process
///
/// @param channel audio channel to process
///
/// @param numberOfSamples number of samples to process, starting from
///        the **beginning** of the buffer
///
/// @param limitOverflow sample amplitude that does just *not* count as
///        an overflow (absolute value)
///
/// @return number of overflows
///
int KmeterAudioProcessor::countOverflows(
    const AudioBuffer<float> &buffer,
    const int channel,
    const int numberOfSamples,
    const float limitOverflow) const
{
    jassert(isPositiveAndBelow(channel, buffer.getNumChannels()));
    jassert(isPositiveAndNotGreaterThan(numberOfSamples,
                                        buffer.getNumSamples()));

    const float *audioData = buffer.getReadPointer(channel);
    int overflows = 0;

    for (int sample = 0; sample < numberOfSamples; ++sample)
    {
        // get sample value
        float amplitude = audioData[sample];

        // convert sample value to amplitude
        if (amplitude < 0.0f)
        {
            amplitude = -amplitude;
        }

        // an overflow has occurred if the amplitude lies above the
        // overflow limit
        if (amplitude > limitOverflow)
        {
            ++overflows;
        }
    }

    return overflows;
}


void KmeterAudioProcessor::silenceInput(
    bool isSilentNew)
{
    isSilent_ = isSilentNew;
}


void KmeterAudioProcessor::startValidation(
    File fileAudio,
    int nSelectedChannel,
    bool bReportCSV,
    bool bAverageMeterLevel,
    bool bPeakMeterLevel,
    bool bMaximumPeakLevel,
    bool bTruePeakMeterLevel,
    bool bMaximumTruePeakLevel,
    bool bStereoMeterValue,
    bool bPhaseCorrelation)
{
    // reset all meters before we start the validation
    meterBallistics_->reset();

    isSilent_ = false;

    int crestFactor = getRealInteger(KmeterPluginParameters::selCrestFactor);
    audioFilePlayer_ = new AudioFilePlayer(fileAudio,
                                           (int) getSampleRate(),
                                           meterBallistics_,
                                           crestFactor);

    if (audioFilePlayer_->matchingSampleRates())
    {
        audioFilePlayer_->setReporters(nSelectedChannel,
                                       bReportCSV,
                                       bAverageMeterLevel,
                                       bPeakMeterLevel,
                                       bMaximumPeakLevel,
                                       bTruePeakMeterLevel,
                                       bMaximumTruePeakLevel,
                                       bStereoMeterValue,
                                       bPhaseCorrelation);

        // refresh editor; "V+" ==> validation started
        sendActionMessage("V+");
    }
    else
    {
        stopValidation();

        AlertWindow::showMessageBoxAsync(
            AlertWindow::WarningIcon,
            "Validation error",
            "Sample rates of host and validation file do not match.");
    }
}


void KmeterAudioProcessor::stopValidation()
{
    isSilent_ = false;
    audioFilePlayer_ = nullptr;

    // reset all meters after the validation
    meterBallistics_->reset();

    // refresh editor; "V-" ==> validation stopped
    sendActionMessage("V-");
}


bool KmeterAudioProcessor::isValidating()
{
    if (audioFilePlayer_ == nullptr)
    {
        return false;
    }
    else
    {
        if (audioFilePlayer_->isPlaying())
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


MeterBallistics *KmeterAudioProcessor::getLevels()
{
    return meterBallistics_;
}


int KmeterAudioProcessor::getAverageAlgorithm()
{
    return averageAlgorithmId_;
}


void KmeterAudioProcessor::setAverageAlgorithm(
    const int averageAlgorithm)
{
    if (averageAlgorithm != averageAlgorithmId_)
    {
        if (averageLevelFiltered_ != nullptr)
        {
            averageLevelFiltered_->setAlgorithm(averageAlgorithm);
            setAverageAlgorithmFinal(averageLevelFiltered_->getAlgorithm());
        }
        else
        {
            averageAlgorithmId_ = averageAlgorithm;
        }
    }
}


void KmeterAudioProcessor::setAverageAlgorithmFinal(
    const int averageAlgorithm)
{
    averageAlgorithmId_ = averageAlgorithm;
    meterBallistics_->setAverageAlgorithm(averageAlgorithmId_);

    //  the level averaging alghorithm has been changed, so update the
    // "RMS" and "ITU-R" buttons to make sure that the correct button
    // is lit
    //
    // "AC" ==> algorithm changed
    sendActionMessage("AC");
}


AudioProcessorEditor *KmeterAudioProcessor::createEditor()
{
    return new KmeterAudioProcessorEditor(this, getMainBusNumInputChannels());
}


bool KmeterAudioProcessor::hasEditor() const
{
    return true;
}


void KmeterAudioProcessor::getStateInformation(
    MemoryBlock &destData)
{
    XmlElement xmlParameters = pluginParameters_.storeAsXml();

    DBG("[K-Meter]");
    DBG("[K-Meter] storing plug-in parameters:");
    DBG("[K-Meter]");
    DBG(String("[K-Meter]   ") + xmlParameters.createDocument("").replace(
            "\n", "\n[K-Meter]   "));

    copyXmlToBinary(xmlParameters, destData);
}


void KmeterAudioProcessor::setStateInformation(
    const void *data,
    int sizeInBytes)
{
    ScopedPointer<XmlElement> xmlParameters(
        getXmlFromBinary(data, sizeInBytes));

    DBG("[K-Meter] loading plug-in parameters:");
    DBG("[K-Meter]");
    DBG(String("[K-Meter]   ") + xmlParameters->createDocument("").replace(
            "\n", "\n[K-Meter]   "));

    pluginParameters_.loadFromXml(xmlParameters);
    updateParameters(true);
}


// This creates new instances of the plug-in.
AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new KmeterAudioProcessor();
}
