/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2011 Martin Zuther (http://www.mzuther.de/)

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

#ifndef __AVERAGE_LEVEL_FILTERED_RMS_H__
#define __AVERAGE_LEVEL_FILTERED_RMS_H__

class AverageLevelFilteredRms;

#include "juce_library_code/juce_header.h"
#include "audio_ring_buffer.h"
#include "fftw3/api/fftw3.h"

//==============================================================================
/**
*/
class AverageLevelFilteredRms
{
public:
    AverageLevelFilteredRms(const int channels, const int buffer_size);
    ~AverageLevelFilteredRms();

    float getLevel(const int channel);
    void copyFromBuffer(AudioRingBuffer& ringBuffer, const int pre_delay, const int sample_rate);
    void copyToBuffer(AudioRingBuffer& destination, const unsigned int sourceStartSample, const unsigned int numSamples);
    void copyToBuffer(AudioSampleBuffer& destination, const int channel, const int destStartSample, const int numSamples);

private:
    void calculateFilterKernel();
    void FilterSamples(const int channel);

    AudioSampleBuffer* pSampleBuffer;
    AudioSampleBuffer* pOverlapAddSamples;

    float* arrFilterKernel_TD;
    fftwf_complex* arrFilterKernel_FD;
    fftwf_plan planFilterKernel_DFT;

    float* arrAudioSamples_TD;
    fftwf_complex* arrAudioSamples_FD;
    fftwf_plan planAudioSamples_DFT;
    fftwf_plan planAudioSamples_IDFT;

    int nChannels;
    int nSampleRate;
    int nBufferSize;
    int nFftSize;
    int nHalfFftSize;
};


#endif  // __AVERAGE_LEVEL_FILTERED_RMS_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
