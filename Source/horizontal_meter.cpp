/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2013 Martin Zuther (http://www.mzuther.de/)

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

#include "horizontal_meter.h"


HorizontalMeter::HorizontalMeter(const String& componentName)
{
    setName(componentName);

    // this component does not have any transparent areas (increases
    // performance on redrawing)
    setOpaque(true);

    nNeedlePosition = -1;
    nNeedleTravelPath = 1;
    bVerticalMeter = false;

    nWidth = 0;
    nHeight = 0;
    nSpacingLeft = 0;
    nSpacingTop = 0;

    imageBackground = Image();
    imageNeedle = Image();
}


HorizontalMeter::~HorizontalMeter()
{
}


void HorizontalMeter::paint(Graphics& g)
{
    g.setColour(Colours::black);
    g.drawImageAt(imageBackground, 0, 0, false);

    if (bVerticalMeter)
    {
        g.drawImageAt(imageNeedle, nSpacingLeft, nNeedlePosition, false);
    }
    else
    {
        g.drawImageAt(imageNeedle, nNeedlePosition, nSpacingTop, false);
    }
}


void HorizontalMeter::resized()
{
    initialize();
}


void HorizontalMeter::initialize()
{
    nWidth = getWidth();
    nHeight = getHeight();

    bVerticalMeter = (nHeight > nWidth);

    if (bVerticalMeter)
    {
        nNeedleTravelPath = nHeight - 2 * nSpacingTop;
        nNeedleTravelPath -= imageNeedle.getHeight();
    }
    else
    {
        nNeedleTravelPath = nWidth - 2 * nSpacingLeft;
        nNeedleTravelPath -= imageNeedle.getWidth();
    }
}


void HorizontalMeter::setValue(float newValue)
{
    int nNeedlePositionOld = nNeedlePosition;

    nNeedlePosition = int (newValue * nNeedleTravelPath + 0.5f);
    nNeedlePosition += bVerticalMeter ? nSpacingTop : nSpacingLeft;

    if (nNeedlePosition != nNeedlePositionOld)
    {
        repaint(getLocalBounds());
    }
}


void HorizontalMeter::setImages(Image& imageBackgroundNew, Image& imageNeedleNew, int nSpacingLeftNew, int nSpacingTopNew)
{
    nSpacingLeft = nSpacingLeftNew;
    nSpacingTop = nSpacingTopNew;

    imageBackground = Image(imageBackgroundNew);
    imageNeedle = Image(imageNeedleNew);

    initialize();
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
