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

#include "dynamic_range_label.h"

DynamicRangeLabel::DynamicRangeLabel(const String& componentName) : Label(componentName, T("0"))
{
    resetValue();

    setFont(13.0f);
    setText(T("none"), false);
    setJustificationType(Justification::centred);
    setColour(Label::backgroundColourId, Colours::grey.darker(0.7f));
    setColour(Label::textColourId, Colours::white);
    setColour(Label::outlineColourId, Colours::grey.darker(0.2f));
}

DynamicRangeLabel::~DynamicRangeLabel()
{
}

void DynamicRangeLabel::resetValue()
{
    nValue = -1;
    setValue(nValue);
}

void DynamicRangeLabel::setValue(int newValue)
{
    if (newValue == nValue)
    {
        return;
    }

    nValue = newValue;

    if (nValue < 0)
    {
        setText("none", false);
    }
    else if (nValue < 10)
    {
        setText(String("DR 0") + String(nValue), false);
    }
    else
    {
        setText(String("DR ") + String(nValue), false);
    }

    if (nValue < 0)
    {
        setColour(Label::backgroundColourId, Colours::grey.darker(0.7f));
        setColour(Label::textColourId, Colours::white);
    }
    else if (nValue > 12)
    {
        setColour(Label::backgroundColourId, Colour(0.26f, 1.0f, 0.8f, 1.0f));
        setColour(Label::textColourId, Colours::black);
    }
    else if (nValue > 7)
    {
        float fHue = (nValue - 7.0f) / 23.0f;
        setColour(Label::backgroundColourId, Colour(fHue, 1.0f, 0.8f, 1.0f));
        setColour(Label::textColourId, Colours::black);
    }
    else
    {
        setColour(Label::backgroundColourId, Colour(0.00f, 1.0f, 0.8f, 1.0f));
        setColour(Label::textColourId, Colours::black);
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
