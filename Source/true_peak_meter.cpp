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


TruePeakMeter::TruePeakMeter(
    const int channels,
    const int bufferSize) :

    numberOfChannels_(channels),
    // 8x oversampling (maximum under-read is 0.169 dB at half the
    // sampling rate, see Annex 2 of ITU-R BS.1770-4)
    oversamplingRate_(8),
    bufferSizeOriginal_(bufferSize),
    bufferSizeOriginalHalf_(bufferSizeOriginal_ / 2),
    bufferSizeOversampled_(bufferSizeOriginal_ *oversamplingRate_),
    fftSize_(bufferSizeOversampled_ * 2),
    halfFftSize_(fftSize_ / 2 + 1),
    sampleBufferOriginal(numberOfChannels_, bufferSizeOriginal_),
    sampleBufferCurrent(numberOfChannels_, bufferSizeOriginalHalf_),
    sampleBufferOld(numberOfChannels_, bufferSizeOriginalHalf_),
    sampleBufferOversampled(numberOfChannels_, bufferSizeOversampled_)

{
    jassert(channels > 0);

#if (defined (_WIN32) || defined (_WIN64))
    File fileCurrentExecutable = File::getSpecialLocation(
                                     File::currentExecutableFile);

#ifdef _WIN64
    File fileDynamicLibraryFFTW = fileCurrentExecutable.getSiblingFile(
                                      "kmeter/fftw/libfftw3f-3_x64.dll");
#else
    File fileDynamicLibraryFFTW = fileCurrentExecutable.getSiblingFile(
                                      "kmeter/fftw/libfftw3f-3.dll");
#endif

    String strDynamicLibraryFFTW = fileDynamicLibraryFFTW.getFullPathName();

    dynamicLibraryFFTW.open(strDynamicLibraryFFTW);
    jassert(dynamicLibraryFFTW.getNativeHandle() != nullptr);

    fftwf_alloc_real = (float * ( *)(size_t)) dynamicLibraryFFTW.getFunction(
                           "fftwf_alloc_real");
    fftwf_alloc_complex = (fftwf_complex * ( *)(size_t)) dynamicLibraryFFTW.getFunction(
                              "fftwf_alloc_complex");
    fftwf_free = (void ( *)(void *)) dynamicLibraryFFTW.getFunction(
                     "fftwf_free");

    fftwf_plan_dft_r2c_1d = (fftwf_plan( *)(int, float *, fftwf_complex *, unsigned)) dynamicLibraryFFTW.getFunction(
                                "fftwf_plan_dft_r2c_1d");
    fftwf_plan_dft_c2r_1d = (fftwf_plan( *)(int, fftwf_complex *, float *, unsigned)) dynamicLibraryFFTW.getFunction(
                                "fftwf_plan_dft_c2r_1d");
    fftwf_destroy_plan = (void ( *)(fftwf_plan)) dynamicLibraryFFTW.getFunction(
                             "fftwf_destroy_plan");

    fftwf_execute = (void ( *)(const fftwf_plan)) dynamicLibraryFFTW.getFunction(
                        "fftwf_execute");
#endif

    filterKernel_TD = fftwf_alloc_real(fftSize_);
    filterKernel_FD = fftwf_alloc_complex(halfFftSize_);

    planFilterKernel_DFT = fftwf_plan_dft_r2c_1d(
                               fftSize_, filterKernel_TD, filterKernel_FD, FFTW_MEASURE);

    audioSamples_TD = fftwf_alloc_real(fftSize_);
    audioSamples_FD = fftwf_alloc_complex(halfFftSize_);

    planAudioSamples_DFT = fftwf_plan_dft_r2c_1d(
                               fftSize_, audioSamples_TD, audioSamples_FD, FFTW_MEASURE);
    planAudioSamples_IDFT = fftwf_plan_dft_c2r_1d(
                                fftSize_, audioSamples_FD, audioSamples_TD, FFTW_MEASURE);

    calculateFilterKernel();
}


TruePeakMeter::~TruePeakMeter()
{
    fftwf_destroy_plan(planFilterKernel_DFT);
    fftwf_free(filterKernel_TD);
    fftwf_free(filterKernel_FD);

    fftwf_destroy_plan(planAudioSamples_DFT);
    fftwf_destroy_plan(planAudioSamples_IDFT);
    fftwf_free(audioSamples_TD);
    fftwf_free(audioSamples_FD);

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
    sampleBufferCurrent.clear();
    sampleBufferOld.clear();
    sampleBufferOversampled.clear();

    // reset levels
    truePeakLevels.clear();

    for (int channel = 0; channel < numberOfChannels_; ++channel)
    {
        truePeakLevels.add(0.0);
    }

    // interpolation filter; removes all frequencies above *original*
    // Nyquist frequency from resampled audio (the approximated filter
    // bandwidth is 21.5 Hz for a buffer size of 1024 samples and a
    // sampling rate of 44100 Hz with 8x oversampling)
    float nRelativeCutoffFrequency = 0.5f / oversamplingRate_;

    int samples = bufferSizeOversampled_ + 1;
    float samplesHalf = samples / 2.0f;

    // calculate filter kernel
    for (int i = 0; i < samples; ++i)
    {
        if (i == samplesHalf)
        {
            filterKernel_TD[i] = static_cast<float>(
                                        2.0 * M_PI * nRelativeCutoffFrequency);
        }
        else
        {
            filterKernel_TD[i] = static_cast<float>(
                                        sin(2.0 * M_PI * nRelativeCutoffFrequency * (i - samplesHalf)) / (i - samplesHalf) * (0.42 - 0.5 * cos(2.0 * static_cast<float>(M_PI) * i / samples) + 0.08 * cos(4.0 * static_cast<float>(M_PI) * i / samples)));
        }
    }

    // normalise filter kernel for unity gain at DC
    float nSumKernel = 0.0;

    for (int i = 0; i < samples; ++i)
    {
        nSumKernel += filterKernel_TD[i];
    }

    for (int i = 0; i < samples; ++i)
    {
        filterKernel_TD[i] = filterKernel_TD[i] / nSumKernel;
    }

    // pad filter kernel with zeros
    for (int i = samples; i < fftSize_; ++i)
    {
        filterKernel_TD[i] = 0.0f;
    }

    // calculate DFT of filter kernel
    fftwf_execute(planFilterKernel_DFT);
}


void TruePeakMeter::filterSamples(
    const int passNumber)

{
    // oversample input sample buffer by clearing it and filling every
    // "oversamplingRate_" sample with the original sample values
    sampleBufferOversampled.clear();

    for (int channel = 0; channel < numberOfChannels_; ++channel)
    {
        int sampleOversampledOld = 0;
        int sampleOversampledCurrent = bufferSizeOriginalHalf_ *
                                       oversamplingRate_;

        for (int sample = 0; sample < bufferSizeOriginalHalf_; ++sample)
        {
            // fill the first half with old samples
            sampleBufferOversampled.copyFrom(
                channel, sampleOversampledOld, sampleBufferOld,
                channel, sample, 1);

            // fill the second half with current samples
            sampleBufferOversampled.copyFrom(
                channel, sampleOversampledCurrent, sampleBufferCurrent,
                channel, sample, 1);

            sampleOversampledOld += oversamplingRate_;
            sampleOversampledCurrent += oversamplingRate_;
        }
    }

    // filter audio data (overwrites contents of sample buffer)
    for (int channel = 0; channel < numberOfChannels_; ++channel)
    {
        filterWorker(channel);

        // evaluate true peak level
        float truePeakLevel = sampleBufferOversampled.getMagnitude(
                                  channel, 0, bufferSizeOversampled_);

        if (passNumber == 1)
        {
            truePeakLevels.set(channel, truePeakLevel);
        }
        else
        {
            if (truePeakLevel > truePeakLevels[channel])
            {
                truePeakLevels.set(channel, truePeakLevel);
            }
        }
    }
}


void TruePeakMeter::filterWorker(
    const int channel)

{
    jassert(channel >= 0);
    jassert(channel < numberOfChannels_);

    // copy audio data to temporary buffer as the sample buffer is not
    // optimised for MME
    memcpy(audioSamples_TD,
           sampleBufferOversampled.getReadPointer(channel),
           bufferSizeOversampled_ * sizeof(float));

    // pad audio data with zeros
    for (int sample = bufferSizeOversampled_; sample < fftSize_; ++sample)
    {
        audioSamples_TD[sample] = 0.0f;
    }

    // calculate DFT of audio data
    fftwf_execute(planAudioSamples_DFT);

    // convolve audio data with filter kernel
    for (int i = 0; i < halfFftSize_; ++i)
    {
        // multiplication of complex numbers: index 0 contains the real
        // part, index 1 the imaginary part
        float realPart = audioSamples_FD[i][0] * filterKernel_FD[i][0] -
                         audioSamples_FD[i][1] * filterKernel_FD[i][1];
        float imagPart = audioSamples_FD[i][1] * filterKernel_FD[i][0] +
                         audioSamples_FD[i][0] * filterKernel_FD[i][1];

        audioSamples_FD[i][0] = realPart;
        audioSamples_FD[i][1] = imagPart;
    }

    // synthesise audio data from frequency spectrum (this destroys the
    // contents of "audioSamples_FD"!!!)
    fftwf_execute(planAudioSamples_IDFT);

    // normalise synthesised audio data
    float fNorm = float(fftSize_ / oversamplingRate_);

    for (int i = 0; i < fftSize_; ++i)
    {
        audioSamples_TD[i] = audioSamples_TD[i] / fNorm;
    }

    // copy data from temporary buffer back to sample buffer
    sampleBufferOversampled.copyFrom(
        channel, 0,
        audioSamples_TD, bufferSizeOversampled_);
}


float TruePeakMeter::getLevel(
    const int channel)

{
    jassert(channel >= 0);
    jassert(channel < numberOfChannels_);

    return truePeakLevels[channel];
}


void TruePeakMeter::copyFromBuffer(
    frut::audio::RingBuffer &ringBuffer,
    const unsigned int preDelay)

{
    // copy data from ring buffer to sample buffer
    ringBuffer.copyToBuffer(sampleBufferOriginal, 0,
                            bufferSizeOriginal_, preDelay);

    // copy samples (first pass)
    for (int channel = 0; channel < numberOfChannels_; ++channel)
    {
        // copy second half of old sample buffer (sampleBufferCurrent
        // hasn't been changed yet!)
        sampleBufferOld.copyFrom(
            channel, 0, sampleBufferCurrent,
            channel, 0, bufferSizeOriginalHalf_);

        // copy first half of current sample buffer
        sampleBufferCurrent.copyFrom(
            channel, 0, sampleBufferOriginal,
            channel, 0, bufferSizeOriginalHalf_);
    }

    // filter samples (first pass)
    filterSamples(1);

    // copy samples (second pass)
    for (int channel = 0; channel < numberOfChannels_; ++channel)
    {
        // copy first half of current sample buffer
        sampleBufferOld.copyFrom(
            channel, 0, sampleBufferOriginal,
            channel, 0, bufferSizeOriginalHalf_);

        // copy second half of current sample buffer
        sampleBufferCurrent.copyFrom(
            channel, 0, sampleBufferOriginal,
            channel, bufferSizeOriginalHalf_, bufferSizeOriginalHalf_);
    }

    // filter samples (second pass)
    filterSamples(2);
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
