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

#include "channel_slider.h"

ChannelSlider::ChannelSlider(const String& componentName, int nNumChannels) : Slider(componentName)
{
    setRange(-1.0f, nNumChannels, 1.0f);
    setSliderStyle(Slider::IncDecButtons);
    setTextBoxStyle(Slider::TextBoxLeft, true, 30, 20);
    setIncDecButtonsMode(Slider::incDecButtonsNotDraggable);

    setValue(-1, false, false);
}


ChannelSlider::~ChannelSlider()
{
}


double ChannelSlider::getValueFromText(const String& strText)
{
    if (strText == "All")
    {
        return -1.0f;
    }
    else
    {
        return strText.getFloatValue() - 1.0f;
    }
}


const String ChannelSlider::getTextFromValue(double fValue)
{
    if (fValue < 0)
    {
        return String("All");
    }
    else
    {
        return String(int(fValue + 0.5f) + 1);
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
