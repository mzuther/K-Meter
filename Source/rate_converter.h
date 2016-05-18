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

#ifndef __RATE_CONVERTER_H__
#define __RATE_CONVERTER_H__

#include "FrutHeader.h"
#include "fftw_runner.h"

class RateConverter :
    public FftwRunner
{
public:
    RateConverter(const int upsamplingRate,
                  const int channels,
                  const int bufferSize);

protected:
    void calculateFilterKernel();
    void upsample();

    int upsamplingRate_;
    int bufferSizeOriginal_;

    AudioBuffer<float> sampleBufferOriginal_;

private:
    JUCE_LEAK_DETECTOR(RateConverter);
};


#endif  // __RATE_CONVERTER_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
