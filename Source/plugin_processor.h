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
#include "true_peak_meter.h"


class KmeterAudioProcessor :
    public AudioProcessor,
    public ActionBroadcaster,
    virtual public frut::audio::RingBufferProcessor<float>
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

    float getParameter(int nIndex);
    void changeParameter(int nIndex, float fValue);
    void setParameter(int nIndex, float fValue);

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
    virtual void processBufferChunk(AudioBuffer<float> &buffer,
                                    const unsigned int uChunkSize,
                                    const unsigned int uBufferPosition,
                                    const unsigned int uProcessedSamples);

    int getAverageAlgorithm();
    void setAverageAlgorithm(const int average_algorithm);
    void setAverageAlgorithmFinal(const int average_algorithm);

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

    frut::dsp::Dither dither_;

    ScopedPointer<AudioFilePlayer> audioFilePlayer;

    ScopedPointer<frut::audio::RingBuffer<float>> ringBufferInput_;
    ScopedPointer<frut::audio::RingBuffer<float>> ringBufferOutput_;

    ScopedPointer<AverageLevelFiltered> pAverageLevelFiltered;
    ScopedPointer<TruePeakMeter> pTruePeakMeter;
    ScopedPointer<MeterBallistics> pMeterBallistics;

    KmeterPluginParameters pluginParameters;

    const int nTrakmeterBufferSize;

    bool isStereo;
    bool bSampleRateIsValid;
    bool isSilent;

    int nAverageAlgorithm;
    int nSamplesInBuffer;
    float fProcessedSeconds;
    float attenuationLevel_;

    Array<float> arrPeakLevels;
    Array<float> arrRmsLevels;
    Array<float> arrAverageLevelsFiltered;
    Array<float> arrTruePeakLevels;

    Array<int> arrOverflows;

    int countOverflows(frut::audio::RingBuffer<float> *ring_buffer,
                       const unsigned int channel,
                       const unsigned int length,
                       const unsigned int pre_delay);
};

AudioProcessor *JUCE_CALLTYPE createPluginFilter();


// Local Variables:
// ispell-local-dictionary: "british"
// End:
