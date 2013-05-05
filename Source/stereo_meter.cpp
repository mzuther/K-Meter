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

#include "stereo_meter.h"

StereoMeter::StereoMeter(const String& componentName, int PosX, int PosY, int Width, int Height)
{
    setName(componentName);

    // this component does not have any transparent areas (increases
    // performance on redrawing)
    setOpaque(true);

    fValue = 0.0f;
    nNeedlePosition = -1;

    nPosX = PosX;
    nPosY = PosY;
    nWidth = Width;
    nHeight = Height;
}

StereoMeter::~StereoMeter()
{
}

void StereoMeter::paint(Graphics& g)
{
    int width = getWidth();
    int height = getHeight();
    int middle_of_meter = width / 2;

    ColourGradient colGrad(Colours::black, 0.0f, 0.0f, Colours::black, (float) width, 0.0f, false);
    colGrad.addColour(0.5f, Colours::darkgrey);
    g.setGradientFill(colGrad);
    g.fillRect(1, 1, width - 2, height - 2);

    g.setColour(Colours::darkgrey.darker(0.3f));
    g.drawRect(0, 0, width, height, 1);

    g.setColour(Colours::white.withAlpha(0.15f));

    for (int y = 1; y < height; y++)
    {
        g.setPixel(middle_of_meter, y);
    }

    g.setColour(Colours::white);

    // upper arrow
    g.setPixel(middle_of_meter - 1, 0);
    g.setPixel(middle_of_meter + 1, 0);
    g.setPixel(middle_of_meter, 0);
    g.setPixel(middle_of_meter, 1);

    // lower arrow
    g.setPixel(middle_of_meter, height - 2);
    g.setPixel(middle_of_meter, height - 1);
    g.setPixel(middle_of_meter - 1, height - 1);
    g.setPixel(middle_of_meter + 1, height - 1);

    g.setFont(11.0f);
    g.drawFittedText("L", 0, 0, height, height, Justification::centred, 1, 1.0f);
    g.drawFittedText("R", width - height + 1, 0, height, height, Justification::centred, 1, 1.0f);

    g.setColour(Colours::red);

    for (int y = 1; y < (height - 1); y++)
    {
        g.setPixel(nNeedlePosition, y);
    }

    g.setColour(Colours::red.withAlpha(0.6f));

    for (int y = 1; y < (height - 1); y++)
    {
        g.setPixel(nNeedlePosition - 1, y);
        g.setPixel(nNeedlePosition + 1, y);
    }
}

void StereoMeter::visibilityChanged()
{
    setBounds(nPosX, nPosY, nWidth, nHeight);
}

void StereoMeter::setValue(float newValue)
{
    fValue = newValue;

    int nNeedlePositionOld = nNeedlePosition;
    nNeedlePosition = int ((1.0f + fValue) / 2.0f * (getWidth() - 4) + 2);

    if (nNeedlePosition == nNeedlePositionOld)
    {
        return;
    }

    repaint(getLocalBounds());
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
