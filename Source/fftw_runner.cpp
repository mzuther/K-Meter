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

#include "fftw_runner.h"


FftwRunner::FftwRunner(
    const int channels,
    const int bufferSize) :

    numberOfChannels_(channels),
    fftBufferSize_(bufferSize),
    fftSize_(fftBufferSize_ * 2),
    halfFftSize_(fftSize_ / 2 + 1),
    fftSampleBuffer_(numberOfChannels_, fftBufferSize_),
    fftOverlapAddSamples_(numberOfChannels_, fftBufferSize_)

{
    jassert(channels > 0);

#if (defined (_WIN32) || defined (_WIN64))
    File currentExecutableFile = File::getSpecialLocation(
                                     File::currentExecutableFile);

#ifdef _WIN64
    File dynamicLibraryFftwFile = currentExecutableFile.getSiblingFile(
                                      "kmeter/fftw/libfftw3f-3_x64.dll");
#else
    File dynamicLibraryFftwFile = currentExecutableFile.getSiblingFile(
                                      "kmeter/fftw/libfftw3f-3.dll");
#endif

    String dynamicLibraryFftwPath = dynamicLibraryFftwFile.getFullPathName();

    dynamicLibraryFFTW.open(dynamicLibraryFftwPath);
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

    filterKernel_TD_ = fftwf_alloc_real(fftSize_);
    filterKernel_FD_ = fftwf_alloc_complex(halfFftSize_);

    filterKernelPlan_DFT_ = fftwf_plan_dft_r2c_1d(
                                fftSize_, filterKernel_TD_, filterKernel_FD_,
                                FFTW_MEASURE);

    audioSamples_TD_ = fftwf_alloc_real(fftSize_);
    audioSamples_FD_ = fftwf_alloc_complex(halfFftSize_);

    audioSamplesPlan_DFT_ = fftwf_plan_dft_r2c_1d(
                                fftSize_, audioSamples_TD_, audioSamples_FD_,
                                FFTW_MEASURE);
    audioSamplesPlan_IDFT_ = fftwf_plan_dft_c2r_1d(
                                 fftSize_, audioSamples_FD_, audioSamples_TD_,
                                 FFTW_MEASURE);
}


FftwRunner::~FftwRunner()
{
    fftwf_destroy_plan(filterKernelPlan_DFT_);
    fftwf_free(filterKernel_TD_);
    fftwf_free(filterKernel_FD_);

    fftwf_destroy_plan(audioSamplesPlan_DFT_);
    fftwf_destroy_plan(audioSamplesPlan_IDFT_);
    fftwf_free(audioSamples_TD_);
    fftwf_free(audioSamples_FD_);

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


// calculate filter kernel for windowed-sinc low-pass filter
void FftwRunner::calculateKernelWindowedSincLPF(
    const float relativeCutoffFrequency)

{
    int samples = fftBufferSize_ + 1;
    float samplesHalf = samples / 2.0f;

    // calculate filter kernel
    for (int i = 0; i < samples; ++i)
    {
        if (i == samplesHalf)
        {
            filterKernel_TD_[i] = static_cast<float>(
                                      2.0 * M_PI * relativeCutoffFrequency);
        }
        else
        {
            filterKernel_TD_[i] = static_cast<float>(
                                      sin(2.0 * M_PI * relativeCutoffFrequency * (i - samplesHalf)) / (i - samplesHalf) * (0.42 - 0.5 * cos(2.0 * static_cast<float>(M_PI) * i / samples) + 0.08 * cos(4.0 * static_cast<float>(M_PI) * i / samples)));
        }
    }

    // normalise filter kernel for unity gain at DC
    float kernelSum = 0.0;

    for (int i = 0; i < samples; ++i)
    {
        kernelSum += filterKernel_TD_[i];
    }

    for (int i = 0; i < samples; ++i)
    {
        filterKernel_TD_[i] = filterKernel_TD_[i] / kernelSum;
    }

    // pad filter kernel with zeros
    for (int i = samples; i < fftSize_; ++i)
    {
        filterKernel_TD_[i] = 0.0f;
    }

    // calculate DFT of filter kernel
    fftwf_execute(filterKernelPlan_DFT_);
}


// "oversamplingRate" is needed for normalising the synthesised audio
// data during oversampling only and should be left alone in any other
// case
void FftwRunner::convolveWithKernel(
    const int channel,
    const float oversamplingRate)

{
    jassert(channel >= 0);
    jassert(channel < numberOfChannels_);

    // copy audio data to temporary buffer as the sample buffer is not
    // optimised for MME
    memcpy(audioSamples_TD_,
           fftSampleBuffer_.getReadPointer(channel),
           fftBufferSize_ * sizeof(float));

    // pad audio data with zeros
    for (int sample = fftBufferSize_; sample < fftSize_; ++sample)
    {
        audioSamples_TD_[sample] = 0.0f;
    }

    // calculate DFT of audio data
    fftwf_execute(audioSamplesPlan_DFT_);

    // convolve audio data with filter kernel
    for (int i = 0; i < halfFftSize_; ++i)
    {
        // multiplication of complex numbers: index 0 contains the real
        // part, index 1 the imaginary part
        float realPart = audioSamples_FD_[i][0] * filterKernel_FD_[i][0] -
                         audioSamples_FD_[i][1] * filterKernel_FD_[i][1];
        float imagPart = audioSamples_FD_[i][1] * filterKernel_FD_[i][0] +
                         audioSamples_FD_[i][0] * filterKernel_FD_[i][1];

        audioSamples_FD_[i][0] = realPart;
        audioSamples_FD_[i][1] = imagPart;
    }

    // synthesise audio data from frequency spectrum (this destroys the
    // contents of "audioSamples_FD_"!!!)
    fftwf_execute(audioSamplesPlan_IDFT_);

    // normalise synthesised audio data
    float normaliser = float(fftSize_ / oversamplingRate);

    for (int i = 0; i < fftSize_; ++i)
    {
        audioSamples_TD_[i] = audioSamples_TD_[i] / normaliser;
    }

    // copy data from temporary buffer back to sample buffer
    fftSampleBuffer_.copyFrom(channel, 0, audioSamples_TD_,
                              fftBufferSize_);

    // add old overlapping samples
    fftSampleBuffer_.addFrom(channel, 0, fftOverlapAddSamples_,
                             channel, 0, fftBufferSize_);

    // store new overlapping samples
    fftOverlapAddSamples_.copyFrom(channel, 0, audioSamples_TD_ + fftBufferSize_,
                                   fftBufferSize_);
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
