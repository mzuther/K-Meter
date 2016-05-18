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

    calculateKernelWindowedSincLPF(relativeCutoffFrequency);
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
        convolveWithKernel(channel, oversamplingRate_);

        // evaluate true peak level
        float truePeakLevel = fftSampleBuffer_.getMagnitude(
                                  channel, 0, fftBufferSize_);

        truePeakLevels_.set(channel, truePeakLevel);
    }
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
