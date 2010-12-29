/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010 Martin Zuther (http://www.mzuther.de/)

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

#include "phase_correlation_meter.h"

PhaseCorrelationMeter::PhaseCorrelationMeter(const String& componentName, int PosX, int PosY, int Width, int Height)
{
    setName(componentName);

    fValue = 0.0f;
    nNeedlePosition = -1;

    nPosX = PosX;
    nPosY = PosY;
    nWidth = Width;
    nHeight = Height;
}

PhaseCorrelationMeter::~PhaseCorrelationMeter()
{
}

void PhaseCorrelationMeter::paint(Graphics& g)
{
    int width = getWidth();
    int height = getHeight();
    int middle_of_meter = width / 2;
    int height_of_meter = height;

    ColourGradient colGrad(Colours::red.darker(1.0f), 0.0f, 0.0f, Colours::green.darker(1.0f), (float) width, 0.0f, false);
    colGrad.addColour(0.2f, Colours::red.darker(1.0f));
    colGrad.addColour(0.6f, Colours::yellow.darker(1.0f));
    g.setGradientFill(colGrad);
    g.fillRect(1, 1, width - 2, height - 2);

    g.setColour(Colours::darkgrey.darker(0.3f));
    g.drawRect(0, 0, width, height, 1);

    g.setColour(Colours::white);

    g.setFont(11.0f);
    g.drawFittedText(T("-1"), 1, 0, height_of_meter + 2, height_of_meter, Justification::centred, 1, 1.0f);
    g.drawFittedText(T("0"), middle_of_meter - height_of_meter / 2, 0, height_of_meter, height_of_meter, Justification::centred, 1, 1.0f);
    g.drawFittedText(T("+1"), width - height - 4, 0, height_of_meter + 2, height_of_meter, Justification::centred, 1, 1.0f);

    g.setColour(Colours::red);

    for (int y = 1; y < (height_of_meter - 1); y++)
    {
        g.setPixel(nNeedlePosition, y);
    }

    g.setColour(Colours::red.withAlpha(0.6f));

    for (int y = 1; y < (height_of_meter - 1); y++)
    {
        g.setPixel(nNeedlePosition - 1, y);
        g.setPixel(nNeedlePosition + 1, y);
    }
}

void PhaseCorrelationMeter::visibilityChanged()
{
    setBounds(nPosX, nPosY, nWidth, nHeight);
}

void PhaseCorrelationMeter::setValue(float newValue)
{
    fValue = newValue;

    int nNeedlePositionOld = nNeedlePosition;
    nNeedlePosition = int ((1.0f + fValue) / 2.0f * (getWidth() - 4) + 2);

    if (nNeedlePosition == nNeedlePositionOld)
    {
        return;
    }

    repaint();
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
