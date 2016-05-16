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
    const int oversamplingRate,
    const int channels,
    const int bufferSize) :

    FftwRunner(channels, oversamplingRate *bufferSize),
    oversamplingRate_(oversamplingRate),
    bufferSizeOriginal_(bufferSize),
    sampleBufferOriginal_(numberOfChannels_, bufferSizeOriginal_)

{
    calculateFilterKernel();
}


void TruePeakMeter::calculateFilterKernel()
{
    sampleBufferOriginal_.clear();
    fftSampleBuffer_.clear();
    fftOverlapAddSamples_.clear();

    // reset levels
    truePeakLevels_.clear();

    for (int channel = 0; channel < numberOfChannels_; ++channel)
    {
        truePeakLevels_.add(0.0);
    }

    // interpolation filter; removes all frequencies above *original*
    // Nyquist frequency from resampled audio; the approximated filter
    // transition is 22 Hz for a final buffer size of 8192 samples
    // (8 * 1024) and an initial sampling rate of 44100 Hz
    float relativeCutoffFrequency = 0.5f / oversamplingRate_;

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


void TruePeakMeter::filterSamples()
{
    // oversample input sample buffer by clearing it and filling every
    // "oversamplingRate_" sample with the original sample values
    fftSampleBuffer_.clear();

    for (int channel = 0; channel < numberOfChannels_; ++channel)
    {
        int sampleOversampled = 0;

        for (int sample = 0; sample < bufferSizeOriginal_; ++sample)
        {
            fftSampleBuffer_.copyFrom(
                channel, sampleOversampled, sampleBufferOriginal_,
                channel, sample, 1);

            sampleOversampled += oversamplingRate_;
        }
    }

    // filter audio data (overwrites contents of sample buffer)
    for (int channel = 0; channel < numberOfChannels_; ++channel)
    {
        filterWorker(channel);

        // evaluate true peak level
        float truePeakLevel = fftSampleBuffer_.getMagnitude(
                                  channel, 0, fftBufferSize_);

        truePeakLevels_.set(channel, truePeakLevel);
    }
}


void TruePeakMeter::filterWorker(
    const int channel)

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
    float normaliser = float(fftSize_ / oversamplingRate_);

    for (int i = 0; i < fftSize_; ++i)
    {
        audioSamples_TD_[i] = audioSamples_TD_[i] / normaliser;
    }

    // copy data from temporary buffer back to sample buffer
    fftSampleBuffer_.copyFrom(
        channel, 0, audioSamples_TD_,
        fftBufferSize_);

    // add old overlapping samples
    fftSampleBuffer_.addFrom(channel, 0, fftOverlapAddSamples_,
                             channel, 0, fftBufferSize_);

    // store new overlapping samples
    fftOverlapAddSamples_.copyFrom(channel, 0, audioSamples_TD_ + fftBufferSize_,
                                   fftBufferSize_);
}


float TruePeakMeter::getLevel(
    const int channel)

{
    jassert(channel >= 0);
    jassert(channel < numberOfChannels_);

    return truePeakLevels_[channel];
}


void TruePeakMeter::copyFromBuffer(
    frut::audio::RingBuffer &ringBuffer,
    const unsigned int preDelay)

{
    // copy data from ring buffer to sample buffer
    ringBuffer.copyToBuffer(sampleBufferOriginal_, 0,
                            bufferSizeOriginal_, preDelay);

    // filter samples
    filterSamples();
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
