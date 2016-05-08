/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2016 Martin Zuther (http://www.mzuther.de/)

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

#ifndef __AVERAGE_LEVEL_FILTERED_H__
#define __AVERAGE_LEVEL_FILTERED_H__

class AverageLevelFiltered;

#include "FrutHeader.h"
#include "meter_ballistics.h"
#include "plugin_processor.h"
#include "fftw3/api/fftw3.h"


class AverageLevelFiltered
{
public:
    static const int KMETER_MAXIMUM_FILTER_STAGES = 3;

    AverageLevelFiltered(KmeterAudioProcessor *processor,
                         const int channels,
                         const int sampleRate,
                         const int bufferSize,
                         const int averageAlgorithm);
    ~AverageLevelFiltered();

    float getLevel(const int channel);
    int getAlgorithm();
    void setAlgorithm(const int averageAlgorithm);

    void copyFromBuffer(frut::audio::RingBuffer &ringBuffer,
                        const unsigned int preDelay,
                        const int sampleRate);
    void copyToBuffer(frut::audio::RingBuffer &destination,
                      const unsigned int sourceStartSample,
                      const unsigned int numSamples);
    void copyToBuffer(AudioBuffer<float> &destination,
                      const int channel,
                      const int destStartSample,
                      const int numSamples);

private:
    JUCE_LEAK_DETECTOR(AverageLevelFiltered);

    void calculateFilterKernel();
    void calculateFilterKernel_Rms();
    void calculateFilterKernel_ItuBs1770();

    void filterSamples_Rms(const int channel);
    void filterSamples_ItuBs1770();

    void setPeakToAverageCorrection(float peakToAverageCorrection);

    float *filterKernel_TD_;
    fftwf_complex *filterKernel_FD_;
    fftwf_plan filterKernelPlan_DFT_;

    float *audioSamples_TD_;
    fftwf_complex *audioSamples_FD_;
    fftwf_plan audioSamplesPlan_DFT_;
    fftwf_plan audioSamplesPlan_IDFT_;

    int numberOfChannels_;
    int sampleRate_;
    int bufferSize_;

    int fftSize_;
    int halfFftSize_;

    Array<double> preFilterInputCoefficients_;
    Array<double> preFilterOutputCoefficients_;

    Array<double> weightingFilterInputCoefficients_;
    Array<double> weightingFilterOutputCoefficients_;

    AudioBuffer<float> sampleBuffer_;
    AudioBuffer<float> overlapAddSamples_;

    AudioBuffer<float> previousSamplesPreFilterInput_;
    AudioBuffer<float> previousSamplesPreFilterOutput_;

    AudioBuffer<float> previousSamplesWeightingFilterInput_;
    AudioBuffer<float> previousSamplesWeightingFilterOutput_;

    AudioBuffer<float> previousSamplesOutputTemp_;

    frut::audio::Dither dither_;

    KmeterAudioProcessor *processor_;
    int averageAlgorithm_;
    float peakToAverageCorrection_;

#if (defined (_WIN32) || defined (_WIN64))
    float *(*fftwf_alloc_real)(size_t);
    fftwf_complex *(*fftwf_alloc_complex)(size_t);
    void (*fftwf_free)(void *);

    fftwf_plan(*fftwf_plan_dft_r2c_1d)(int, float *, fftwf_complex *, unsigned);
    fftwf_plan(*fftwf_plan_dft_c2r_1d)(int, fftwf_complex *, float *, unsigned);
    void (*fftwf_destroy_plan)(fftwf_plan);

    void (*fftwf_execute)(const fftwf_plan);
#endif
};


#endif  // __AVERAGE_LEVEL_FILTERED_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
