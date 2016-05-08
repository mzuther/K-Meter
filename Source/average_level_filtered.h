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

//==============================================================================
/**
*/
class AverageLevelFiltered
{
public:
    static const int KMETER_MAXIMUM_FILTER_STAGES = 3;

    AverageLevelFiltered(KmeterAudioProcessor *processor, const int channels, const int sample_rate, const int buffer_size, const int average_algorithm);
    ~AverageLevelFiltered();

    float getLevel(const int channel);
    int getAlgorithm();
    void setAlgorithm(const int average_algorithm);
    void copyFromBuffer(frut::audio::RingBuffer &ringBuffer, const unsigned int pre_delay, const int sample_rate);
    void copyToBuffer(frut::audio::RingBuffer &destination, const unsigned int sourceStartSample, const unsigned int numSamples);
    void copyToBuffer(AudioBuffer<float> &destination, const int channel, const int destStartSample, const int numSamples);

private:
    JUCE_LEAK_DETECTOR(AverageLevelFiltered);

    void calculateFilterKernel();
    void calculateFilterKernel_Rms();
    void calculateFilterKernel_ItuBs1770();

    void FilterSamples_Rms(const int channel);
    void FilterSamples_ItuBs1770();

    void setPeakToAverageCorrection(float peak_to_average_correction);

    float *arrFilterKernel_TD;
    fftwf_complex *arrFilterKernel_FD;
    fftwf_plan planFilterKernel_DFT;

    float *arrAudioSamples_TD;
    fftwf_complex *arrAudioSamples_FD;
    fftwf_plan planAudioSamples_DFT;
    fftwf_plan planAudioSamples_IDFT;

    int nNumberOfChannels;
    int nSampleRate;
    int nBufferSize;

    Array<double> arrPreFilterInputCoefficients;
    Array<double> arrPreFilterOutputCoefficients;

    Array<double> arrWeightingFilterInputCoefficients;
    Array<double> arrWeightingFilterOutputCoefficients;

    AudioBuffer<float> sampleBuffer;
    AudioBuffer<float> overlapAddSamples;

    AudioBuffer<float> previousSamplesPreFilterInput;
    AudioBuffer<float> previousSamplesPreFilterOutput;

    AudioBuffer<float> previousSamplesWeightingFilterInput;
    AudioBuffer<float> previousSamplesWeightingFilterOutput;

    AudioBuffer<float> previousSamplesOutputTemp;

    frut::audio::Dither dither;

    KmeterAudioProcessor *pProcessor;
    int nAverageAlgorithm;
    int nFftSize;
    int nHalfFftSize;

    float fAverageLevelItuBs1770;
    float fPeakToAverageCorrection;

#if (defined (_WIN32) || defined (_WIN64))
    DynamicLibrary dynamicLibraryFFTW;

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
