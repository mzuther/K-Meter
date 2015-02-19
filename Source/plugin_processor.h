/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2015 Martin Zuther (http://www.mzuther.de/)

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

#ifndef __KMETER_PLUGINPROCESSOR_H__
#define __KMETER_PLUGINPROCESSOR_H__

#define DEBUG_FILTER 0

class KmeterAudioProcessor;
class MeterBallistics;

#include "JuceHeader.h"
#include "audio_file_player.h"
#include "average_level_filtered.h"
#include "meter_ballistics.h"
#include "plugin_parameters.h"
#include "common/audio/audio_ring_buffer.h"


//============================================================================
class KmeterAudioProcessor  : public AudioProcessor, public ActionBroadcaster, virtual public AudioRingBufferProcessor
{
public:
    //==========================================================================

    KmeterAudioProcessor();
    ~KmeterAudioProcessor();

    //==========================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void releaseResources();

    void processBlock(AudioSampleBuffer &buffer, MidiBuffer &midiMessages);

    void startValidation(File fileAudio, int nSelectedChannel, bool bReportCSV, bool bAverageMeterLevel, bool bPeakMeterLevel, bool bMaximumPeakLevel, bool bStereoMeterValue, bool bPhaseCorrelation);
    void stopValidation();
    bool isValidating();

    //==========================================================================
    AudioProcessorEditor *createEditor();
    bool hasEditor() const;

    //==========================================================================
    int getNumParameters();
    const String getParameterName(int nIndex);
    const String getParameterText(int nIndex);

    float getParameter(int nIndex);
    void changeParameter(int nIndex, float fValue);
    void setParameter(int nIndex, float fValue);

    void clearChangeFlag(int nIndex);
    void setChangeFlag(int nIndex);
    bool hasChanged(int nIndex);
    void updateParameters(bool bIncludeHiddenParameters);

    File getParameterValidationFile();
    void setParameterValidationFile(File &fileValidation);

    String getParameterSkinName();
    void setParameterSkinName(String &strSkinName);

    bool getBoolean(int nIndex);
    int getRealInteger(int nIndex);

    //==========================================================================
    const String getName() const;

    const String getInputChannelName(int channelIndex) const;
    const String getOutputChannelName(int channelIndex) const;

    bool isInputChannelStereoPair(int nIndex) const;
    bool isOutputChannelStereoPair(int nIndex) const;

    bool acceptsMidi() const;
    bool producesMidi() const;

    bool silenceInProducesSilenceOut() const;
    double getTailLengthSeconds() const;

    MeterBallistics *getLevels();
    virtual void processBufferChunk(AudioSampleBuffer &buffer, const unsigned int uChunkSize, const unsigned int uBufferPosition, const unsigned int uProcessedSamples);

    int getAverageAlgorithm();
    void setAverageAlgorithm(const int average_algorithm);
    void setAverageAlgorithmFinal(const int average_algorithm);

    //==========================================================================
    int getNumPrograms();
    int getNumChannels();

    int getCurrentProgram();
    void setCurrentProgram(int nIndex);

    const String getProgramName(int nIndex);
    void changeProgramName(int nIndex, const String &newName);

    //==========================================================================
    void getStateInformation(MemoryBlock &destData);
    void setStateInformation(const void *data, int sizeInBytes);

    //==========================================================================
    juce_UseDebuggingNewOperator

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KmeterAudioProcessor);

    const int nTrakmeterBufferSize;

    ScopedPointer<AudioFilePlayer> audioFilePlayer;

    ScopedPointer<AudioRingBuffer> pRingBufferInput;
    ScopedPointer<AudioRingBuffer> pRingBufferOutput;

    ScopedPointer<AverageLevelFiltered> pAverageLevelFiltered;
    ScopedPointer<MeterBallistics> pMeterBallistics;

    KmeterPluginParameters pluginParameters;

    int nNumInputChannels;
    bool isStereo;
    bool bSampleRateIsValid;

    int nAverageAlgorithm;
    int nSamplesInBuffer;
    float fProcessedSeconds;

    Array<float> arrPeakLevels;
    Array<float> arrRmsLevels;
    Array<float> arrAverageLevelsFiltered;

    Array<int> arrOverflows;

    int countOverflows(AudioRingBuffer *ring_buffer, const unsigned int channel, const unsigned int length, const unsigned int pre_delay);
};

AudioProcessor *JUCE_CALLTYPE createPluginFilter();

#endif  // __KMETER_PLUGINPROCESSOR_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
