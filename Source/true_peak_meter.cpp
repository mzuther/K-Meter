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

    FftwRunner(channels, 8 * bufferSize),
    // 8x oversampling (maximum under-read is 0.169 dB at half the
    // sampling rate, see Annex 2 of ITU-R BS.1770-4)
    oversamplingRate_(8),
    bufferSizeOriginal_(bufferSize),
    bufferSizeOriginalHalf_(bufferSizeOriginal_ / 2),
    sampleBufferOriginal_(numberOfChannels_, bufferSizeOriginal_),
    sampleBufferCurrent_(numberOfChannels_, bufferSizeOriginalHalf_),
    sampleBufferOld_(numberOfChannels_, bufferSizeOriginalHalf_)

{
    calculateFilterKernel();
}


void TruePeakMeter::calculateFilterKernel()
{
    sampleBufferOriginal_.clear();
    sampleBufferCurrent_.clear();
    sampleBufferOld_.clear();
    fftSampleBuffer_.clear();

    // reset levels
    truePeakLevels_.clear();

    for (int channel = 0; channel < numberOfChannels_; ++channel)
    {
        truePeakLevels_.add(0.0);
    }

    // interpolation filter; removes all frequencies above *original*
    // Nyquist frequency from resampled audio (the approximated filter
    // bandwidth is 21.5 Hz for a buffer size of 1024 samples and a
    // sampling rate of 44100 Hz with 8x oversampling)
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


void TruePeakMeter::filterSamples(
    const int passNumber)

{
    // oversample input sample buffer by clearing it and filling every
    // "oversamplingRate_" sample with the original sample values
    fftSampleBuffer_.clear();

    for (int channel = 0; channel < numberOfChannels_; ++channel)
    {
        int sampleOversampledOld = 0;
        int sampleOversampledCurrent = bufferSizeOriginalHalf_ *
                                       oversamplingRate_;

        for (int sample = 0; sample < bufferSizeOriginalHalf_; ++sample)
        {
            // fill the first half with old samples
            fftSampleBuffer_.copyFrom(
                channel, sampleOversampledOld, sampleBufferOld_,
                channel, sample, 1);

            // fill the second half with current samples
            fftSampleBuffer_.copyFrom(
                channel, sampleOversampledCurrent, sampleBufferCurrent_,
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
        float truePeakLevel = fftSampleBuffer_.getMagnitude(
                                  channel, 0, fftBufferSize_);

        if (passNumber == 1)
        {
            truePeakLevels_.set(channel, truePeakLevel);
        }
        else
        {
            if (truePeakLevel > truePeakLevels_[channel])
            {
                truePeakLevels_.set(channel, truePeakLevel);
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
        channel, 0,
        audioSamples_TD_, fftBufferSize_);
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

    // copy samples (first pass)
    for (int channel = 0; channel < numberOfChannels_; ++channel)
    {
        // copy second half of old sample buffer (sampleBufferCurrent_
        // hasn't been changed yet!)
        sampleBufferOld_.copyFrom(
            channel, 0, sampleBufferCurrent_,
            channel, 0, bufferSizeOriginalHalf_);

        // copy first half of current sample buffer
        sampleBufferCurrent_.copyFrom(
            channel, 0, sampleBufferOriginal_,
            channel, 0, bufferSizeOriginalHalf_);
    }

    // filter samples (first pass)
    filterSamples(1);

    // copy samples (second pass)
    for (int channel = 0; channel < numberOfChannels_; ++channel)
    {
        // copy first half of current sample buffer
        sampleBufferOld_.copyFrom(
            channel, 0, sampleBufferOriginal_,
            channel, 0, bufferSizeOriginalHalf_);

        // copy second half of current sample buffer
        sampleBufferCurrent_.copyFrom(
            channel, 0, sampleBufferOriginal_,
            channel, bufferSizeOriginalHalf_, bufferSizeOriginalHalf_);
    }

    // filter samples (second pass)
    filterSamples(2);
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
