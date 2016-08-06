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

#include "rate_converter.h"


RateConverter::RateConverter(
    const int upsamplingRate,
    const int channels,
    const int bufferSize) :

    FftwRunner(channels, upsamplingRate * bufferSize),
    upsamplingRate_(upsamplingRate),
    bufferSizeOriginal_(bufferSize),
    sampleBufferOriginal_(numberOfChannels_, bufferSizeOriginal_)

{
    calculateFilterKernel();
}


void RateConverter::calculateFilterKernel()
{
    sampleBufferOriginal_.clear();
    fftSampleBuffer_.clear();
    fftOverlapAddSamples_.clear();

    // interpolation filter; removes all frequencies above *original*
    // Nyquist frequency from resampled audio; the approximated filter
    // transition is 22 Hz for a final buffer size of 8192 samples
    // (8 * 1024) and an initial sampling rate of 44100 Hz
    float relativeCutoffFrequency = 0.5f / upsamplingRate_;

    calculateKernelWindowedSincLPF(relativeCutoffFrequency);
}


void RateConverter::upsample()
{
    // upsample input sample buffer by clearing it and filling every
    // "upsamplingRate_" sample with the original sample values
    fftSampleBuffer_.clear();

    for (int channel = 0; channel < numberOfChannels_; ++channel)
    {
        int sampleUpsampled = 0;

        for (int sample = 0; sample < bufferSizeOriginal_; ++sample)
        {
            fftSampleBuffer_.copyFrom(
                channel, sampleUpsampled, sampleBufferOriginal_,
                channel, sample, 1);

            sampleUpsampled += upsamplingRate_;
        }
    }

    // filter audio data (overwrites contents of sample buffer)
    for (int channel = 0; channel < numberOfChannels_; ++channel)
    {
        convolveWithKernel(channel, upsamplingRate_);
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
