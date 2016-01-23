/* ----------------------------------------------------------------------------

   MZ common JUCE
   ==============
   Common classes for use with the JUCE library

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

#ifndef __GENERIC_METER_SEGMENT_DISCRETE_H__
#define __GENERIC_METER_SEGMENT_DISCRETE_H__

#include "JuceHeader.h"
#include "generic_meter_segment.h"


/// Discrete meter segment component.  This widget consists of a
/// coloured filled rectangle (meter segment) surrounded by a small
/// coloured rectangle (peak marker).  Both rectangles react to level
/// changes with a change in colour or visibility.
///
/// @see GenericMeterBar
///
class GenericMeterSegmentDiscrete :
    public GenericMeterSegment
{
public:
    GenericMeterSegmentDiscrete();

    float setThresholdAndRange(float lowerThreshold,
                               float thresholdRange,
                               bool isTopmost);

    void setColour(float segmentHue,
                   const Colour &peakMarkerColour);

    virtual void setNormalLevels(float normalLevel,
                                 float normalLevelPeak);

    virtual void setDiscreteLevels(float discreteLevel,
                                   float discreteLevelPeak);

    virtual void setLevels(float normalLevel,
                           float normalLevelPeak,
                           float discreteLevel,
                           float discreteLevelPeak);

    void paint(Graphics &g);
    void resized();
    void visibilityChanged();

protected:
    float lowerThreshold_;
    float upperThreshold_;
    float thresholdRange_;

    Colour peakMarkerColour_;

    float segmentHue_;
    float segmentBrightness_;
    float outlineBrightness_;

    bool displayPeakMarker_;
    bool isTopmost_;

private:
    JUCE_LEAK_DETECTOR(GenericMeterSegmentDiscrete);
};


#endif  // __GENERIC_METER_SEGMENT_DISCRETE_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
