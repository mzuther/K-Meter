/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2015 Martin Zuther (http://www.mzuther.de/)

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

#ifndef __KMETER_H__
#define __KMETER_H__

#include "JuceHeader.h"
#include "meter_ballistics.h"
#include "meter_bar.h"
#include "overflow_meter.h"
#include "peak_label.h"
#include "skin.h"


//==============================================================================
/**
*/
class Kmeter : public Component
{
public:
    static const int KMETER_STEREO_WIDTH = 106;
    static const int KMETER_STEREO_WIDTH_2 = KMETER_STEREO_WIDTH / 2;

    Kmeter(const String &componentName, int nCrestFactor, int nNumChannels, bool bExpanded, bool bHorizontalMeter, bool bDisplayPeakMeter, int nSegmentHeight);
    ~Kmeter();

    void setLevels(MeterBallistics *pMeterBallistics);
    void applySkin(Skin *pSkin);
    void resized();

private:
    JUCE_LEAK_DETECTOR(Kmeter);

    OwnedArray<MeterBar> p_arrLevelMeters;
    OwnedArray<OverflowMeter> p_arrOverflowMeters;
    OwnedArray<PeakLabel> p_arrMaximumPeakLabels;

    int nInputChannels;
};


#endif  // __KMETER_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
