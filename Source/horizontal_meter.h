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

#ifndef __HORIZONTAL_METER_H__
#define __HORIZONTAL_METER_H__

#include "../JuceLibraryCode/JuceHeader.h"


//==============================================================================
/**
*/
class HorizontalMeter : public Component
{
public:
    HorizontalMeter(const String &componentName);
    ~HorizontalMeter();

    void setImages(Image &imageBackgroundNew, Image &imageNeedleNew, int nSpacingLeftNew, int nSpacingTopNew);
    void setValue(float newValue);
    void paint(Graphics &g);
    void initialize();
    void resized();

private:
    JUCE_LEAK_DETECTOR(HorizontalMeter);

    int nNeedlePosition;
    int nNeedleTravelPath;
    bool bVerticalMeter;

    int nWidth;
    int nHeight;
    int nSpacingLeft;
    int nSpacingTop;

    Image imageBackground;
    Image imageNeedle;
};


#endif  // __HORIZONTAL_METER_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
