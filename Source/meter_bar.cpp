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

#include "meter_bar.h"

MeterBar::MeterBar()
{
    arrHues.add(0.00f);  // red
    arrHues.add(0.18f);  // yellow
    arrHues.add(0.30f);  // green
    arrHues.add(0.58f);  // blue
}


MeterBar::~MeterBar()
{
}


void MeterBar::create(int crestFactor, bool bExpanded, Orientation orientation, int nMainSegmentHeight)
{
    GenericMeterBar::create();

    int nCrestFactor = 10 * crestFactor;
    int nNumberOfBars;

    int nLimitTopBars;
    int nLimitRedBars;
    int nLimitAmberBars;
    int nLimitGreenBars_1;
    int nLimitGreenBars_2;

    // to prevent the inherent round-off errors of float subtraction,
    // crest factor and limits are stored as integers representing
    // 0.1 dB steps
    if (nCrestFactor == 0)
    {
        nNumberOfBars = 47;

        nLimitTopBars = nCrestFactor - 20;
        nLimitRedBars = -90;
        nLimitAmberBars = -180;
        nLimitGreenBars_1 = -400;
        nLimitGreenBars_2 = nLimitGreenBars_1;
    }
    else if (nCrestFactor == +120)
    {
        nNumberOfBars = 48;

        nLimitTopBars = nCrestFactor - 20;
        nLimitRedBars = +40;
        nLimitAmberBars = 0;
        nLimitGreenBars_1 = -300;
        nLimitGreenBars_2 = nLimitGreenBars_1;
    }
    else if (nCrestFactor == +140)
    {
        nNumberOfBars = 50;

        nLimitTopBars = nCrestFactor - 20;
        nLimitRedBars = +40;
        nLimitAmberBars = 0;
        nLimitGreenBars_1 = -300;
        nLimitGreenBars_2 = nLimitGreenBars_1;
    }
    else // K-20
    {
        // force crest factor of +20 dB
        nCrestFactor = +200;
        nNumberOfBars = 51;

        nLimitTopBars = nCrestFactor - 20;
        nLimitRedBars = +40;
        nLimitAmberBars = 0;
        nLimitGreenBars_1 = -240;
        nLimitGreenBars_2 = -300;
    }

    if (bExpanded)
    {
        nNumberOfBars = 134;
    }

    // bar threshold (in 0.1 dB)
    int nTrueLowerThreshold = 0;

    if (bExpanded && (nCrestFactor > 80))
    {
        // zoom into important region
        nTrueLowerThreshold = +80 - nCrestFactor;
    }

    // bar K-Meter level (in 0.1 dB)
    int nLowerThreshold = nTrueLowerThreshold + nCrestFactor;

    for (int n = 0; n < nNumberOfBars; n++)
    {
        // bar level range (in 0.1 dB)
        int nRange;

        if (bExpanded)
        {
            nRange = 1;
        }
        else
        {
            if (nLowerThreshold > nLimitTopBars)
            {
                nRange = 5;
            }
            else if (nLowerThreshold > nLimitGreenBars_1)
            {
                nRange = 10;
            }
            else if (nLowerThreshold > nLimitGreenBars_2)
            {
                nRange = 60;
            }
            else
            {
                nRange = 100;
            }
        }

        int nColour;

        if (nCrestFactor == 0)
        {
            if (nLowerThreshold <= -280)
            {
                nColour = 0;
            }
            else if (nLowerThreshold <= -220)
            {
                nColour = 1;
            }
            else if ((nLowerThreshold > -160) && (nLowerThreshold <= -100))
            {
                nColour = 2;
            }
            else if (nLowerThreshold > nLimitRedBars)
            {
                nColour = 0;
            }
            else if (nLowerThreshold > nLimitAmberBars)
            {
                nColour = 1;
            }
            else
            {
                nColour = 2;
            }
        }
        else
        {
            if (nLowerThreshold > nLimitRedBars)
            {
                nColour = 0;
            }
            else if (nLowerThreshold > nLimitAmberBars)
            {
                nColour = 1;
            }
            else
            {
                nColour = 2;
            }
        }

        int nSegmentHeight;

        if (bExpanded)
        {
            nSegmentHeight = nMainSegmentHeight;
        }
        else if (nLowerThreshold > nLimitTopBars)
        {
            nSegmentHeight = nMainSegmentHeight;
        }
        else if (nLowerThreshold > nLimitGreenBars_1)
        {
            nSegmentHeight = 2 * nMainSegmentHeight;
        }
        else if (nLowerThreshold > nLimitGreenBars_2)
        {
            nSegmentHeight = 6 * nMainSegmentHeight;
        }
        else if (n == nNumberOfBars - 1)
        {
            if (nCrestFactor == 0)
            {
                nSegmentHeight = 10 * nMainSegmentHeight;
            }
            else if (nCrestFactor == +120)
            {
                nSegmentHeight = 14 * nMainSegmentHeight;
            }
            else if (nCrestFactor == +140)
            {
                nSegmentHeight = 13 * nMainSegmentHeight;
            }
            else // K-20
            {
                nSegmentHeight = 10 * nMainSegmentHeight;
            }
        }
        else
        {
            if (nCrestFactor == 0)
            {
                nSegmentHeight = 11 * nMainSegmentHeight;
            }
            else if (nCrestFactor == +120)
            {
                nSegmentHeight = 12 * nMainSegmentHeight;
            }
            else if (nCrestFactor == +140)
            {
                nSegmentHeight = 11 * nMainSegmentHeight;
            }
            else // K-20
            {
                nSegmentHeight = 10 * nMainSegmentHeight;
            }
        }

        nTrueLowerThreshold -= nRange;
        nLowerThreshold = nTrueLowerThreshold + nCrestFactor;

        int nSpacingBefore = 0;
        bool bHasHighestLevel = (n == 0) ? true : false;

        addSegment(nTrueLowerThreshold * 0.1f, nRange * 0.1f, bHasHighestLevel, nSegmentHeight, nSpacingBefore, arrHues[nColour], Colours::white);
    }

    // set orientation only when *all* meter segments have been added!
    setOrientation(orientation);
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
