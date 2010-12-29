/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010 Martin Zuther (http://www.mzuther.de/)

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

#ifndef __PLUGINPROCESSOR_H_5573940C__
#define __PLUGINPROCESSOR_H_5573940C__

#define KMETER_BUFFER_SIZE 1024
#define DEBUG_FILTER 0

class KmeterAudioProcessor;

#include "juce_library_code/juce_header.h"
#include "juce_library_code/JucePluginCharacteristics.h"
#include "average_level_filtered_rms.h"
#include "audio_ring_buffer.h"
#include "meter_ballistics.h"
#include "plugin_parameters.h"

//============================================================================
class KmeterAudioProcessor  : public AudioProcessor, public ChangeBroadcaster
{
public:
    //==========================================================================

    KmeterAudioProcessor();
    ~KmeterAudioProcessor();

    void addChangeListenerParameters(ChangeListener* listener) throw();
    void removeChangeListenerParameters(ChangeListener* listener) throw();

    //==========================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void releaseResources();

    void processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    //==========================================================================
    AudioProcessorEditor* createEditor();

    //==========================================================================
    int getNumParameters();

    float getParameter(int index);
    void setParameter(int index, float newValue);

    const String getParameterName(int index);
    const String getParameterText(int index);

    void changeParameter(int index, int newValue);
    int getParameterAsInt(int index);

    void MarkParameter(int nIndex);
    void UnmarkParameter(int nIndex);
    bool isParameterMarked(int nIndex);

    //==========================================================================
    const String getName() const;

    const String getInputChannelName(const int channelIndex) const;
    const String getOutputChannelName(const int channelIndex) const;
    bool isInputChannelStereoPair(int index) const;
    bool isOutputChannelStereoPair(int index) const;

    bool acceptsMidi() const;
    bool producesMidi() const;

    MeterBallistics* getLevels();
    void processBufferChunk(AudioSampleBuffer& buffer, const unsigned int uChunkSize, const unsigned int uBufferPosition, const unsigned int uProcessedSamples);

    //==========================================================================
    int getNumPrograms();
    int getCurrentProgram();
    void setCurrentProgram(int index);
    const String getProgramName(int index);
    void changeProgramName(int index, const String& newName);

    //==========================================================================
    void getStateInformation(MemoryBlock& destData);
    void setStateInformation(const void* data, int sizeInBytes);

    //==========================================================================
    juce_UseDebuggingNewOperator

private:
    AudioRingBuffer* pRingBufferInput;
    AudioRingBuffer* pRingBufferOutput;

    AverageLevelFilteredRms* pAverageLevelFilteredRms;
    MeterBallistics* pMeterBallistics;

    KmeterPluginParameters* pPluginParameters;

    int nNumInputChannels;
    bool isStereo;

    int nSamplesInBuffer;
    float fTimeFrame;

    float fPeakLeft;
    float fPeakRight;
    float fAverageLeft;
    float fAverageRight;
    float fPhaseCorrelation;
    int nOverflowsLeft;
    int nOverflowsRight;

    bool bPreviousSampleOverLeft;
    bool bPreviousSampleOverRight;

    int countOverflows(AudioRingBuffer* ring_buffer, const unsigned int channel, const unsigned int length, const unsigned int pre_delay, bool& bPreviousSampleOver);
};

#endif  // __PLUGINPROCESSOR_H_5573940C__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
