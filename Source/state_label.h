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

#ifndef __STATE_LABEL_H__
#define __STATE_LABEL_H__

#include "JuceHeader.h"


//==============================================================================
/**
*/
class StateLabel : public Component
{
public:
    StateLabel(const String &componentName);
    ~StateLabel();

    void resized();
    void setState(bool isActivatedNew);
    void updateState();

    void setImages(Image &imageOffNew, Image &imageOnNew, int nSpacingLeftNew, int nSpacingTopNew, int nFontSize);

protected:
    Label *pLabel;
    ImageComponent *pBackgroundImage;

private:
    JUCE_LEAK_DETECTOR(StateLabel);

    int nSpacingLeft;
    int nSpacingTop;
    bool isActivated;

    Image imageOff;
    Image imageOn;
};


#endif  // __STATE_LABEL_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
