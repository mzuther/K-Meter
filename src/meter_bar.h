/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2012 Martin Zuther (http://www.mzuther.de/)

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

#ifndef __METER_BAR_H__
#define __METER_BAR_H__

#include "juce_library_code/juce_header.h"
#include "meter_segment.h"


//==============================================================================
/**
*/
class MeterBar : public Component
{
public:
    MeterBar(const String& componentName, int PosX, int PosY, int Width, int nCrestFactor, bool bExpanded, bool bDisplayPeakMeter, int nSegmentHeight, String justify);
    ~MeterBar();

    void setLevels(float peakLevel, float averageLevel, float peakLevelPeak, float averageLevelPeak);
    void paint(Graphics& g);
    void resized();
    void visibilityChanged();

private:
    // JUCE_LEAK_DETECTOR(MeterBar);

    float fPeakLevel;
    float fAverageLevel;

    float fPeakLevelPeak;
    float fAverageLevelPeak;

    int nMeterCrestFactor;
    bool isExpanded;
    bool displayPeakMeter;

    int nPosX;
    int nPosY;
    int nWidth;
    int nMainSegmentHeight;

    int nNumberOfBars;
    int nLimitTopBars;
    int nLimitRedBars;
    int nLimitAmberBars;
    int nLimitGreenBars_1;
    int nLimitGreenBars_2;

    String justifyMeter;

    MeterSegment** MeterArray;
};


#endif  // __METER_BAR_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
