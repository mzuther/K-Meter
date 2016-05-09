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

#include "FrutHeader.h"
#include "fftw_runner.h"
#include "meter_ballistics.h"

class TruePeakMeter :
    public FftwRunner
{
public:
    TruePeakMeter(const int channels, const int bufferSize);

    float getLevel(const int channel);
    void copyFromBuffer(frut::audio::RingBuffer &ringBuffer,
                        const unsigned int preDelay);

private:
    JUCE_LEAK_DETECTOR(TruePeakMeter);

    void calculateFilterKernel();
    void filterSamples(const int passNumber);
    void filterWorker(const int channel);

    int oversamplingRate_;

    int bufferSizeOriginal_;
    int bufferSizeOriginalHalf_;
    int bufferSizeOversampled_;

    Array<double> truePeakLevels_;

    AudioBuffer<float> sampleBufferOriginal_;
    AudioBuffer<float> sampleBufferCurrent_;
    AudioBuffer<float> sampleBufferOld_;
};


#endif  // __TRUE_PEAK_METER_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
