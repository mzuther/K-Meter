/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010 Martin Zuther (http://www.mzuther.de/)

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

#include "juce_library_code/juce_header.h"
#include "fftw3/api/fftw3.h"

//==============================================================================
/**
*/
class AverageLevelFilteredRms
{
public:
  AverageLevelFilteredRms(AudioSampleBuffer* buffer, int buffer_size);
  ~AverageLevelFilteredRms();

  float getLevel(int channel, int sample_rate);
  float* getProcessedSamples(int channel);

private:
  void calculateFilterKernel();
  void FilterSamples(int channel);

  AudioSampleBuffer* pOriginalSampleBuffer;
  AudioSampleBuffer* pSampleBuffer;
  AudioSampleBuffer* pOverlapAddSamples;

  float* arrFilterKernel_TD;
  fftwf_complex* arrFilterKernel_FD;
  fftwf_plan planFilterKernel_DFT;

  float* arrAudioSamples_TD;
  fftwf_complex* arrAudioSamples_FD;
  fftwf_plan planAudioSamples_DFT;
  fftwf_plan planAudioSamples_IDFT;

  int nSampleRate;
  int nBufferSize;
  int nFftSize;
  int nHalfFftSize;
};


#endif  // __AVERAGE_LEVEL_FILTERED_RMS_H__
