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

#ifndef __TRUE_PEAK_METER_H__
#define __TRUE_PEAK_METER_H__

class TruePeakMeter;

#include "FrutHeader.h"
#include "meter_ballistics.h"
#include "fftw3/api/fftw3.h"

//==============================================================================
/**
*/
class TruePeakMeter
{
public:
    TruePeakMeter(const int channels, const int buffer_size);
    ~TruePeakMeter();

    float getLevel(const int channel);
    void copyFromBuffer(frut::audio::RingBuffer &ringBuffer, const unsigned int pre_delay);

private:
    JUCE_LEAK_DETECTOR(TruePeakMeter);

    void calculateFilterKernel();
    void FilterSamples(const int channel);

    Array<double> arrTruePeakLevels;

    float *arrFilterKernel_TD;
    fftwf_complex *arrFilterKernel_FD;
    fftwf_plan planFilterKernel_DFT;

    float *arrAudioSamples_TD;
    fftwf_complex *arrAudioSamples_FD;
    fftwf_plan planAudioSamples_DFT;
    fftwf_plan planAudioSamples_IDFT;

    int nNumberOfChannels;
    int nOversamplingRate;

    int nBufferSizeOriginal;
    int nBufferSizeOversampled;
    int nFftSize;
    int nHalfFftSize;

    AudioSampleBuffer sampleBufferOriginal;
    AudioSampleBuffer sampleBufferOversampled;

#if (defined (_WIN32) || defined (_WIN64))
    DynamicLibrary *pDynamicLibraryFFTW;

    float *(*fftwf_alloc_real)(size_t);
    fftwf_complex *(*fftwf_alloc_complex)(size_t);
    void (*fftwf_free)(void *);

    fftwf_plan(*fftwf_plan_dft_r2c_1d)(int, float *, fftwf_complex *, unsigned);
    fftwf_plan(*fftwf_plan_dft_c2r_1d)(int, fftwf_complex *, float *, unsigned);
    void (*fftwf_destroy_plan)(fftwf_plan);

    void (*fftwf_execute)(const fftwf_plan);
#endif
};


#endif  // __TRUE_PEAK_METER_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
