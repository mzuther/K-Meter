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

#include "phase_correlation_meter.h"

PhaseCorrelationMeter::PhaseCorrelationMeter(const String& componentName)
{
    setName(componentName);

    // this component does not have any transparent areas (increases
    // performance on redrawing)
    setOpaque(true);

    fValue = 0.0f;
    nNeedlePosition = -1;

    nPosX = -1;
    nPosY = -1;
    nWidth = 1;
    nHeight = 1;

    bVerticalMeter = false;
}


PhaseCorrelationMeter::~PhaseCorrelationMeter()
{
}


void PhaseCorrelationMeter::paint(Graphics& g)
{
    int width = getWidth();
    int height = getHeight();

    if (bVerticalMeter)
    {
        int middle_of_meter = height / 2;
        int width_of_meter = width;

        ColourGradient colGrad(Colours::red.darker(1.0f), 0.0f, 0.0f, Colours::green.darker(1.0f), 0.0f, (float) height, false);
        colGrad.addColour(0.2f, Colours::red.darker(1.0f));
        colGrad.addColour(0.6f, Colours::yellow.darker(1.0f));
        g.setGradientFill(colGrad);
        g.fillRect(1, 1, width - 2, height - 2);

        g.setColour(Colours::darkgrey.darker(0.3f));
        g.drawRect(0, 0, width, height, 1);

        g.setColour(Colours::white);

        g.setFont(11.0f);
        g.drawFittedText("-1", -1, 0, width_of_meter + 2, width_of_meter, Justification::centred, 1, 1.0f);
        g.drawFittedText("0", 0, middle_of_meter - width_of_meter / 2, width_of_meter, width_of_meter, Justification::centred, 1, 1.0f);
        g.drawFittedText("+1", -1, height - width, width_of_meter + 2, width_of_meter, Justification::centred, 1, 1.0f);

        g.setColour(Colours::red);

        for (int x = 1; x < (width_of_meter - 1); x++)
        {
            g.setPixel(x, nNeedlePosition);
        }

        g.setColour(Colours::red.withAlpha(0.6f));

        for (int x = 1; x < (width_of_meter - 1); x++)
        {
            g.setPixel(x, nNeedlePosition - 1);
            g.setPixel(x, nNeedlePosition + 1);
        }
    }
    else
    {
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
        g.drawFittedText("-1", 1, 0, height_of_meter + 2, height_of_meter, Justification::centred, 1, 1.0f);
        g.drawFittedText("0", middle_of_meter - height_of_meter / 2, 0, height_of_meter, height_of_meter, Justification::centred, 1, 1.0f);
        g.drawFittedText("+1", width - height - 4, 0, height_of_meter + 2, height_of_meter, Justification::centred, 1, 1.0f);

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
}


void PhaseCorrelationMeter::resized()
{
    Rectangle<int> rect = getBounds();

    nPosX = rect.getX();
    nPosY = rect.getY();
    nWidth = rect.getWidth();
    nHeight = rect.getHeight();

    bVerticalMeter = (nHeight > nWidth);
}


void PhaseCorrelationMeter::setValue(float newValue)
{
    fValue = newValue;

    int nNeedlePositionOld = nNeedlePosition;

    if (bVerticalMeter)
    {
        nNeedlePosition = int ((1.0f + fValue) / 2.0f * (getHeight() - 4) + 2);
    }
    else
    {
        nNeedlePosition = int ((1.0f + fValue) / 2.0f * (getWidth() - 4) + 2);
    }

    if (nNeedlePosition == nNeedlePositionOld)
    {
        return;
    }

    repaint(getLocalBounds());
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
