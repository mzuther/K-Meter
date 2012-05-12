/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2012 Martin Zuther (http://www.mzuther.de/)

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

#include "juce_library_code/juce_header.h"
#include "audio_ring_buffer.h"
#include "meter_ballistics.h"
#include "plugin_processor.h"
#include "fftw3/api/fftw3.h"

//==============================================================================
/**
*/
class AverageLevelFiltered
{
public:
    static const int KMETER_MAXIMUM_IIR_FILTER_COEFFICIENTS = 3;

    AverageLevelFiltered(KmeterAudioProcessor* processor, const int channels, const int buffer_size, const int sample_rate, const int average_algorithm);
    ~AverageLevelFiltered();

    float getLevel(const int channel);
    int getAlgorithm();
    void setAlgorithm(const int average_algorithm);
    void copyFromBuffer(AudioRingBuffer& ringBuffer, const int pre_delay, const int sample_rate);
    void copyToBuffer(AudioRingBuffer& destination, const unsigned int sourceStartSample, const unsigned int numSamples);
    void copyToBuffer(AudioSampleBuffer& destination, const int channel, const int destStartSample, const int numSamples);

private:
    JUCE_LEAK_DETECTOR(AverageLevelFiltered);

    void calculateFilterKernel();
    void calculateFilterKernel_Rms();
    void calculateFilterKernel_ItuBs1770();

    void FilterSamples(const int channel);
    void FilterSamples_Rms(const int channel);
    void FilterSamples_ItuBs1770(const int channel);

    AudioSampleBuffer* pSampleBuffer;
    AudioSampleBuffer* pOverlapAddSamples;

    float* arrFilterKernel_TD;
    fftwf_complex* arrFilterKernel_FD;
    fftwf_plan planFilterKernel_DFT;

    float* arrAudioSamples_TD;
    fftwf_complex* arrAudioSamples_FD;
    fftwf_plan planAudioSamples_DFT;
    fftwf_plan planAudioSamples_IDFT;

    float** pIIRCoefficients_1;
    float** pIIRCoefficients_2;

    AudioSampleBuffer* pPreviousSamplesOutputTemp;

    AudioSampleBuffer* pPreviousSamplesInput_1;
    AudioSampleBuffer* pPreviousSamplesOutput_1;

    AudioSampleBuffer* pPreviousSamplesInput_2;
    AudioSampleBuffer* pPreviousSamplesOutput_2;

    KmeterAudioProcessor* pProcessor;
    int nChannels;
    int nAverageAlgorithm;
    int nSampleRate;
    int nBufferSize;
    int nFftSize;
    int nHalfFftSize;

    float fPeakToAverageCorrection;

#ifdef _WIN32
    void* libraryHandleFFTW;

    void *(* fftwf_malloc)(size_t);
    void (* fftwf_free)(void*);

    fftwf_plan(*fftwf_plan_dft_r2c_1d)(int, float*, fftwf_complex*, unsigned);
    fftwf_plan(*fftwf_plan_dft_c2r_1d)(int, fftwf_complex*, float*, unsigned);
    void (* fftwf_destroy_plan)(fftwf_plan);

    void (*fftwf_execute)(const fftwf_plan);
#endif
};


#endif  // __AVERAGE_LEVEL_FILTERED_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
