/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

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

#include "meter_bar.h"


void MeterBar::create(
    int crestFactor, bool discreteMeter, bool isExpanded,
    Orientation orientation, int mainSegmentHeight,
    const Array<Colour> &segmentColours)

{
    GenericMeterBar::create();

    crestFactor *= 10;
    int numberOfBars;

    int limitTopBars;
    int limitRedBars;
    int limitAmberBars;
    int limitGreenBars;
    int limitLinearArea;

    // to prevent the inherent round-off errors of float subtraction,
    // crest factor and limits are stored as integers representing
    // 0.1 dB steps
    if (crestFactor == 0)
    {
        numberOfBars = 48;

        limitTopBars = crestFactor - 20;
        limitRedBars = -90;
        limitAmberBars = -180;
        limitGreenBars = -400;
        limitLinearArea = limitGreenBars;
    }
    else if (crestFactor == +120)
    {
        numberOfBars = 50;

        limitTopBars = crestFactor - 20;
        limitRedBars = +40;
        limitAmberBars = 0;
        limitGreenBars = -300;
        limitLinearArea = limitGreenBars;
    }
    else if (crestFactor == +140)
    {
        numberOfBars = 51;

        limitTopBars = crestFactor - 20;
        limitRedBars = +40;
        limitAmberBars = 0;
        limitGreenBars = -300;
        limitLinearArea = limitGreenBars;
    }
    else // K-20
    {
        // force crest factor of +20 dB
        crestFactor = +200;
        numberOfBars = 51;

        limitTopBars = crestFactor - 20;
        limitRedBars = +40;
        limitAmberBars = 0;
        limitGreenBars = -240;
        limitLinearArea = -300;
    }

    if (isExpanded)
    {
        numberOfBars = 52;
    }

    // bar threshold (in 0.1 dB)
    int trueLowerThreshold = 0;

    if (isExpanded)
    {
        if (crestFactor > 50)
        {
            // set base level of important region
            trueLowerThreshold = -crestFactor;
        }
        else
        {
            // set base level of important region
            trueLowerThreshold = -220;
        }

        // zoom into important region
        trueLowerThreshold += 45;
    }

    // bar K-Meter level (in 0.1 dB)
    int lowerThreshold = trueLowerThreshold + crestFactor;

    for (int n = 0; n < numberOfBars; ++n)
    {
        // bar level range (in 0.1 dB)
        int segmentRange;

        if (isExpanded)
        {
            segmentRange = 1;
        }
        else
        {
            if (lowerThreshold > limitTopBars)
            {
                segmentRange = 5;
            }
            else if (lowerThreshold > limitGreenBars)
            {
                segmentRange = 10;
            }
            else if (lowerThreshold > limitLinearArea)
            {
                segmentRange = 60;
            }
            else
            {
                segmentRange = 100;
            }
        }

        int colourId;

        if (crestFactor == 0)
        {
            if (lowerThreshold <= -280)
            {
                colourId = 3;
            }
            else if (lowerThreshold <= -220)
            {
                colourId = 1;
            }
            else if ((lowerThreshold > -160) && (lowerThreshold <= -100))
            {
                colourId = 2;
            }
            else if (lowerThreshold > limitRedBars)
            {
                colourId = 0;
            }
            else if (lowerThreshold > limitAmberBars)
            {
                colourId = 1;
            }
            else
            {
                colourId = 2;
            }
        }
        else
        {
            if (lowerThreshold > limitRedBars)
            {
                colourId = 0;
            }
            else if (lowerThreshold > limitAmberBars)
            {
                colourId = 1;
            }
            else if (lowerThreshold > limitGreenBars)
            {
                colourId = 2;
            }
            else
            {
                colourId = 3;
            }
        }

        int segmentHeight;

        if (isExpanded)
        {
            segmentHeight = 2 * mainSegmentHeight;
        }
        else if (lowerThreshold > limitTopBars)
        {
            segmentHeight = mainSegmentHeight;
        }
        else if (lowerThreshold > limitGreenBars)
        {
            segmentHeight = 2 * mainSegmentHeight;
        }
        else if (lowerThreshold > limitLinearArea)
        {
            segmentHeight = 3 * mainSegmentHeight;
        }
        else if (n == numberOfBars - 1)
        {
            if (crestFactor == +120)
            {
                segmentHeight = 5 * mainSegmentHeight;
            }
            else
            {
                segmentHeight = 4 * mainSegmentHeight;
            }
        }
        else
        {
            if (crestFactor == 0)
            {
                segmentHeight = 4 * mainSegmentHeight;
            }
            else
            {
                segmentHeight = 3 * mainSegmentHeight;
            }
        }

        trueLowerThreshold -= segmentRange;
        lowerThreshold = trueLowerThreshold + crestFactor;

        int spacingBefore = 0;
        bool hasHighestLevel;

        if (isExpanded)
        {
            hasHighestLevel = false;
        }
        else
        {
            hasHighestLevel = (n == 0) ? true : false;
        }

        if (discreteMeter)
        {
            addDiscreteSegment(
                trueLowerThreshold * 0.1f,
                segmentRange * 0.1f,
                hasHighestLevel,
                segmentHeight,
                spacingBefore,
                segmentColours[colourId],
                Colours::white);
        }
        else
        {
            addContinuousSegment(
                trueLowerThreshold * 0.1f,
                segmentRange * 0.1f,
                hasHighestLevel,
                segmentHeight,
                spacingBefore,
                segmentColours[colourId],
                Colours::white);
        }
    }

    // set orientation here to save some processing power
    setOrientation(orientation);
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
