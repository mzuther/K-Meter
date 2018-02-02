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

class AverageLevelFiltered;

#include "FrutHeader.h"
#include "plugin_processor.h"


class AverageLevelFiltered :
    public frut::dsp::FIRFilterBox
{
public:
    static const int KMETER_MAXIMUM_FILTER_STAGES = 3;

    AverageLevelFiltered(KmeterAudioProcessor *processor,
                         const int numberOfChannels,
                         const double sampleRate,
                         const int fftBufferSize,
                         const int averageAlgorithm);

    int getAlgorithm() const;
    void setAlgorithm(const int averageAlgorithm);

    float getLevel(const int channel);

    void getSamples(AudioBuffer<float> &destination,
                    const int sourceStartSample,
                    const int numberOfSamples);

    void getSamples(frut::audio::RingBuffer<float> &destination,
                    const int sourceStartSample,
                    const int numberOfSamples);

    void setSamples(const AudioBuffer<float> &source,
                    const double sampleRate);

    void setSamples(const frut::audio::RingBuffer<float> &source,
                    const int preDelay,
                    const double sampleRate);

    void setSamples(const frut::audio::RingBuffer<double> &source,
                    const int preDelay,
                    const double sampleRate);

private:
    JUCE_LEAK_DETECTOR(AverageLevelFiltered);

    void calculateFilterKernel();
    void calculateFilterKernel_Rms();
    void calculateFilterKernel_ItuBs1770();

    void filterSamples_Rms(const int channel);
    void filterSamples_ItuBs1770();

    double sampleRate_;

    Array<double> preFilterInputCoefficients_;
    Array<double> preFilterOutputCoefficients_;

    Array<double> weightingFilterInputCoefficients_;
    Array<double> weightingFilterOutputCoefficients_;

    AudioBuffer<float> previousSamplesPreFilterInput_;
    AudioBuffer<float> previousSamplesPreFilterOutput_;

    AudioBuffer<float> previousSamplesWeightingFilterInput_;
    AudioBuffer<float> previousSamplesWeightingFilterOutput_;

    AudioBuffer<float> previousSamplesOutputTemp_;

    frut::dsp::Dither dither_;

    KmeterAudioProcessor *processor_;
    int averageAlgorithm_;
    float peakToAverageCorrection_;
};


// Local Variables:
// ispell-local-dictionary: "british"
// End:
