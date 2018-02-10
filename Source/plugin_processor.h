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

#pragma once

class KmeterAudioProcessor;
class MeterBallistics;

#include "FrutHeader.h"
#include "audio_file_player.h"
#include "average_level_filtered.h"
#include "meter_ballistics.h"
#include "plugin_parameters.h"


class KmeterAudioProcessor :
    public AudioProcessor,
    public ActionBroadcaster,
    virtual public frut::audio::RingBufferProcessor<double>
{
public:
    KmeterAudioProcessor();
    ~KmeterAudioProcessor();

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;

    void processBlock(AudioBuffer<float> &buffer,
                      MidiBuffer &midiMessages) override;
    void processBlock(AudioBuffer<double> &buffer,
                      MidiBuffer &midiMessages) override;
    void process(AudioBuffer<double> &buffer);

    void silenceInput(bool isSilentNew);

    void startValidation(File fileAudio, int nSelectedChannel,
                         bool bReportCSV, bool bAverageMeterLevel,
                         bool bPeakMeterLevel, bool bMaximumPeakLevel,
                         bool bTruePeakMeterLevel, bool bMaximumTruePeakLevel,
                         bool bStereoMeterValue, bool bPhaseCorrelation);

    void stopValidation();
    bool isValidating();

    AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    int getNumParameters() override;
    const String getParameterName(int nIndex) override;
    const String getParameterText(int nIndex) override;

    float getParameter(int nIndex) override;
    void changeParameter(int nIndex, float fValue);
    void setParameter(int nIndex, float fValue) override;

    void clearChangeFlag(int nIndex);
    bool hasChanged(int nIndex);
    void updateParameters(bool bIncludeHiddenParameters);

    File getParameterValidationFile();
    void setParameterValidationFile(const File &fileValidation);

    String getParameterSkinName();
    void setParameterSkinName(const String &strSkinName);

    bool getBoolean(int nIndex);
    int getRealInteger(int nIndex);

    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;

    double getTailLengthSeconds() const override;

    MeterBallistics *getLevels();
    virtual void processBufferChunk(frut::audio::RingBuffer<double> *ringBuffer,
                                    const int chunkSize) override;

    int getAverageAlgorithm();
    void setAverageAlgorithm(const int averageAlgorithm);
    void setAverageAlgorithmFinal(const int averageAlgorithm);

    int getNumPrograms() override;

    int getCurrentProgram() override;
    void setCurrentProgram(int nIndex) override;

    const String getProgramName(int nIndex) override;

    void changeProgramName(int nIndex, const String &newName) override;

    void getStateInformation(MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KmeterAudioProcessor);

    static BusesProperties getBusesProperties();

    int countOverflows(frut::audio::RingBuffer<double> *ring_buffer,
                       const int channel,
                       const int length);

    ScopedPointer<AudioFilePlayer> audioFilePlayer_;

    ScopedPointer<frut::audio::RingBuffer<double>> ringBufferInput_;
    ScopedPointer<frut::audio::RingBuffer<double>> ringBufferOutput_;

    ScopedPointer<AverageLevelFiltered> averageLevelFiltered_;
    ScopedPointer<frut::dsp::TruePeakMeter> truePeakMeter_;
    ScopedPointer<MeterBallistics> meterBallistics_;

    KmeterPluginParameters pluginParameters_;

    const int kmeterBufferSize_;

    bool isStereo_;
    bool sampleRateIsValid_;
    bool isSilent_;

    int averageAlgorithmId_;
    int samplesInBuffer_;
    float processedSeconds_;
    double attenuationLevel_;

    Array<float> peakLevels_;
    Array<float> rmsLevels_;
    Array<float> averageLevelsFiltered_;
    Array<float> truePeakLevels_;

    Array<int> overflowCounts_;

    frut::dsp::Dither dither_;
};

AudioProcessor *JUCE_CALLTYPE createPluginFilter();


// Local Variables:
// ispell-local-dictionary: "british"
// End:
