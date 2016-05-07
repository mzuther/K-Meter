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

#include "true_peak_meter.h"


TruePeakMeter::TruePeakMeter(const int channels, const int buffer_size) :
    nNumberOfChannels(channels),
    // 8x oversampling (maximum under-read is 0.169 dB at half the
    // sampling rate, see Annex 2 of ITU-R BS.1770-4)
    nOversamplingRate(8),
    nBufferSizeOriginal(buffer_size),
    nBufferSizeOversampled(nBufferSizeOriginal *nOversamplingRate),
    nFftSize(nBufferSizeOversampled * 2),
    nHalfFftSize(nFftSize / 2 + 1),
    sampleBufferOriginal(nNumberOfChannels, nBufferSizeOriginal),
    sampleBufferOversampled(nNumberOfChannels, nBufferSizeOversampled)
{
    jassert(channels > 0);

#if (defined (_WIN32) || defined (_WIN64))
    File fileCurrentExecutable = File::getSpecialLocation(File::currentExecutableFile);

#ifdef _WIN64
    File fileDynamicLibraryFFTW = fileCurrentExecutable.getSiblingFile("kmeter/fftw/libfftw3f-3_x64.dll");
#else
    File fileDynamicLibraryFFTW = fileCurrentExecutable.getSiblingFile("kmeter/fftw/libfftw3f-3.dll");
#endif

    String strDynamicLibraryFFTW = fileDynamicLibraryFFTW.getFullPathName();

    dynamicLibraryFFTW.open(strDynamicLibraryFFTW);
    jassert(dynamicLibraryFFTW.getNativeHandle() != nullptr);

    fftwf_alloc_real = (float * ( *)(size_t)) dynamicLibraryFFTW.getFunction("fftwf_alloc_real");
    fftwf_alloc_complex = (fftwf_complex * ( *)(size_t)) dynamicLibraryFFTW.getFunction("fftwf_alloc_complex");
    fftwf_free = (void ( *)(void *)) dynamicLibraryFFTW.getFunction("fftwf_free");

    fftwf_plan_dft_r2c_1d = (fftwf_plan( *)(int, float *, fftwf_complex *, unsigned)) dynamicLibraryFFTW.getFunction("fftwf_plan_dft_r2c_1d");
    fftwf_plan_dft_c2r_1d = (fftwf_plan( *)(int, fftwf_complex *, float *, unsigned)) dynamicLibraryFFTW.getFunction("fftwf_plan_dft_c2r_1d");
    fftwf_destroy_plan = (void ( *)(fftwf_plan)) dynamicLibraryFFTW.getFunction("fftwf_destroy_plan");

    fftwf_execute = (void ( *)(const fftwf_plan)) dynamicLibraryFFTW.getFunction("fftwf_execute");
#endif

    arrFilterKernel_TD = fftwf_alloc_real(nFftSize);
    arrFilterKernel_FD = fftwf_alloc_complex(nHalfFftSize);

    planFilterKernel_DFT = fftwf_plan_dft_r2c_1d(nFftSize, arrFilterKernel_TD, arrFilterKernel_FD, FFTW_MEASURE);

    arrAudioSamples_TD = fftwf_alloc_real(nFftSize);
    arrAudioSamples_FD = fftwf_alloc_complex(nHalfFftSize);

    planAudioSamples_DFT = fftwf_plan_dft_r2c_1d(nFftSize, arrAudioSamples_TD, arrAudioSamples_FD, FFTW_MEASURE);
    planAudioSamples_IDFT = fftwf_plan_dft_c2r_1d(nFftSize, arrAudioSamples_FD, arrAudioSamples_TD, FFTW_MEASURE);

    calculateFilterKernel();
}


TruePeakMeter::~TruePeakMeter()
{
    fftwf_destroy_plan(planFilterKernel_DFT);
    fftwf_free(arrFilterKernel_TD);
    fftwf_free(arrFilterKernel_FD);

    fftwf_destroy_plan(planAudioSamples_DFT);
    fftwf_destroy_plan(planAudioSamples_IDFT);
    fftwf_free(arrAudioSamples_TD);
    fftwf_free(arrAudioSamples_FD);

#if (defined (_WIN32) || defined (_WIN64))
    fftwf_alloc_real = nullptr;
    fftwf_alloc_complex = nullptr;
    fftwf_free = nullptr;

    fftwf_plan_dft_r2c_1d = nullptr;
    fftwf_plan_dft_c2r_1d = nullptr;
    fftwf_destroy_plan = nullptr;

    fftwf_execute = nullptr;
#endif
}


void TruePeakMeter::calculateFilterKernel()
{
    sampleBufferOriginal.clear();
    sampleBufferOversampled.clear();

    // reset levels
    arrTruePeakLevels.clear();

    for (int nChannel = 0; nChannel < nNumberOfChannels; nChannel++)
    {
        arrTruePeakLevels.add(0.0);
    }

    // interpolation filter; removes all frequencies above *original*
    // Nyquist frequency from resampled audio (the approximated filter
    // bandwidth is 21.5 Hz for a buffer size of 1024 samples and a
    // sampling rate of 44100 Hz with 8x oversampling)
    float nRelativeCutoffFrequency = 0.5f / nOversamplingRate;

    int nSamples = nBufferSizeOversampled + 1;
    float nSamplesHalf = nSamples / 2.0f;

    // calculate filter kernel
    for (int i = 0; i < nSamples; i++)
    {
        if (i == nSamplesHalf)
        {
            arrFilterKernel_TD[i] = float(2.0 * M_PI * nRelativeCutoffFrequency);
        }
        else
        {
            arrFilterKernel_TD[i] = float(sin(2.0 * M_PI * nRelativeCutoffFrequency * (i - nSamplesHalf)) / (i - nSamplesHalf) * (0.42 - 0.5 * cos(2.0 * (float) M_PI * i / nSamples) + 0.08 * cos(4.0 * (float) M_PI * i / nSamples)));
        }
    }

    // normalise filter kernel for unity gain at DC
    float nSumKernel = 0.0;

    for (int i = 0; i < nSamples; i++)
    {
        nSumKernel += arrFilterKernel_TD[i];
    }

    for (int i = 0; i < nSamples; i++)
    {
        arrFilterKernel_TD[i] = arrFilterKernel_TD[i] / nSumKernel;
    }

    // pad filter kernel with zeros
    for (int i = nSamples; i < nFftSize; i++)
    {
        arrFilterKernel_TD[i] = 0.0f;
    }

    // calculate DFT of filter kernel
    fftwf_execute(planFilterKernel_DFT);
}


void TruePeakMeter::FilterSamples(const int channel)
{
    jassert(channel >= 0);
    jassert(channel < nNumberOfChannels);

    // copy audio data to temporary buffer as the sample buffer is not
    // optimised for MME
    memcpy(arrAudioSamples_TD, sampleBufferOversampled.getReadPointer(channel), nBufferSizeOversampled * sizeof(float));

    // pad audio data with zeros
    for (int nSample = nBufferSizeOversampled; nSample < nFftSize; nSample++)
    {
        arrAudioSamples_TD[nSample] = 0.0f;
    }

    // calculate DFT of audio data
    fftwf_execute(planAudioSamples_DFT);

    // convolve audio data with filter kernel
    for (int i = 0; i < nHalfFftSize; i++)
    {
        // multiplication of complex numbers: index 0 contains the real
        // part, index 1 the imaginary part
        float real_part = arrAudioSamples_FD[i][0] * arrFilterKernel_FD[i][0] - arrAudioSamples_FD[i][1] * arrFilterKernel_FD[i][1];
        float imaginary_part = arrAudioSamples_FD[i][1] * arrFilterKernel_FD[i][0] + arrAudioSamples_FD[i][0] * arrFilterKernel_FD[i][1];

        arrAudioSamples_FD[i][0] = real_part;
        arrAudioSamples_FD[i][1] = imaginary_part;
    }

    // synthesise audio data from frequency spectrum (this destroys the
    // contents of "arrAudioSamples_FD"!!!)
    fftwf_execute(planAudioSamples_IDFT);

    // normalise synthesised audio data
    float fNorm = float(nFftSize / nOversamplingRate);

    for (int i = 0; i < nFftSize; i++)
    {
        arrAudioSamples_TD[i] = arrAudioSamples_TD[i] / fNorm;
    }

    // copy data from temporary buffer back to sample buffer
    sampleBufferOversampled.copyFrom(channel, 0, arrAudioSamples_TD, nBufferSizeOversampled);

    // evaluate true peak level
    arrTruePeakLevels.set(channel, sampleBufferOversampled.getMagnitude(channel, 0, nBufferSizeOversampled));
}


float TruePeakMeter::getLevel(const int channel)
{
    jassert(channel >= 0);
    jassert(channel < nNumberOfChannels);

    return arrTruePeakLevels[channel];
}


void TruePeakMeter::copyFromBuffer(frut::audio::RingBuffer &ringBuffer, const unsigned int pre_delay)
{
    // copy data from ring buffer to sample buffer
    ringBuffer.copyToBuffer(sampleBufferOriginal, 0, nBufferSizeOriginal, pre_delay);

    // oversample input sample buffer by clearing it and filling every
    // "nOversamplingRate" sample with the original sample values
    sampleBufferOversampled.clear();

    for (int nSample = 0; nSample < nBufferSizeOriginal; nSample++)
    {
        int nSampleOversampled = nSample * nOversamplingRate;

        for (int nChannel = 0; nChannel < nNumberOfChannels; nChannel++)
        {
            sampleBufferOversampled.copyFrom(nChannel, nSampleOversampled, sampleBufferOriginal, nChannel, nSample, 1);
        }
    }

    for (int nChannel = 0; nChannel < nNumberOfChannels; nChannel++)
    {
        // filter audio data (overwrites contents of sample buffer)
        FilterSamples(nChannel);
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
