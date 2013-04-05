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

#include "meter_bar.h"

MeterBar::MeterBar(const String& componentName, int posX, int posY, int Width, int nCrestFactor, bool bExpanded, bool bDisplayPeakMeter, int nSegmentHeight)
{
    setName(componentName);
    isExpanded = bExpanded;
    displayPeakMeter = bDisplayPeakMeter;

    // this component does not have any transparent areas (increases
    // performance on redrawing)
    setOpaque(true);

    // to prevent the inherent round-off errors of float subtraction,
    // crest factor and limits are stored as integers representing
    // 0.1 dB steps
    if (nCrestFactor == 0)
    {
        nMeterCrestFactor = 0;

        nLimitTopBars = nMeterCrestFactor - 20;
        nLimitRedBars = -80;
        nLimitAmberBars = -160;
        nLimitGreenBars_1 = -400;
        nLimitGreenBars_2 = nLimitGreenBars_1;
    }
    else if (nCrestFactor == 12)
    {
        nMeterCrestFactor = +120;

        nLimitTopBars = nMeterCrestFactor - 20;
        nLimitRedBars = +40;
        nLimitAmberBars = 0;
        nLimitGreenBars_1 = -300;
        nLimitGreenBars_2 = nLimitGreenBars_1;
    }
    else if (nCrestFactor == 14)
    {
        nMeterCrestFactor = +140;

        nLimitTopBars = nMeterCrestFactor - 20;
        nLimitRedBars = +40;
        nLimitAmberBars = 0;
        nLimitGreenBars_1 = -300;
        nLimitGreenBars_2 = nLimitGreenBars_1;
    }
    else // K-20
    {
        nMeterCrestFactor = +200;

        nLimitTopBars = nMeterCrestFactor - 20;
        nLimitRedBars = +40;
        nLimitAmberBars = 0;
        nLimitGreenBars_1 = -240;
        nLimitGreenBars_2 = -300;
    }

    if (isExpanded)
    {
        nNumberOfBars = 134;
    }
    else
    {
        if (nCrestFactor == 0)
        {
            nNumberOfBars = 47;
        }
        else if (nCrestFactor == 12)
        {
            nNumberOfBars = 48;
        }
        else if (nCrestFactor == 14)
        {
            nNumberOfBars = 50;
        }
        else // K-20
        {
            nNumberOfBars = 51;
        }
    }

    nPosX = posX;
    nPosY = posY;
    nWidth = Width;
    nMainSegmentHeight = nSegmentHeight;

    fPeakLevel = 0.0f;
    fAverageLevel = 0.0f;

    fPeakLevelPeak = 0.0f;
    fAverageLevelPeak = 0.0f;

    int nThreshold = 0; // bar threshold (in 0.1 dB)

    if (isExpanded && (nMeterCrestFactor > 80))
    {
        nThreshold = +80 - nMeterCrestFactor; // zoom into important region
    }

    int nKmeterLevel = nThreshold + nMeterCrestFactor; // bar K-Meter level (in 0.1 dB)
    int nRange = 0; // bar level range (in 0.1 dB)
    int nColor = 0;

    MeterArray = new MeterSegment*[nNumberOfBars];

    for (int n = 0; n < nNumberOfBars; n++)
    {
        if (isExpanded)
        {
            nRange = 1;
        }
        else
        {
            if (nKmeterLevel > nLimitTopBars)
            {
                nRange = 5;
            }
            else if (nKmeterLevel > nLimitGreenBars_1)
            {
                nRange = 10;
            }
            else if (nKmeterLevel > nLimitGreenBars_2)
            {
                nRange = 60;
            }
            else
            {
                nRange = 100;
            }
        }

        if (nKmeterLevel > nLimitRedBars)
        {
            nColor = 0;
        }
        else if (nKmeterLevel > nLimitAmberBars)
        {
            nColor = 1;
        }
        else
        {
            nColor = 2;
        }

        nThreshold -= nRange;
        nKmeterLevel -= nRange;
        MeterArray[n] = new MeterSegment("MeterSegment #" + String(n) + " (" + componentName + ")", nThreshold * 0.1f, nRange * 0.1f, displayPeakMeter, nColor);

        addAndMakeVisible(MeterArray[n]);
    }
}

MeterBar::~MeterBar()
{
    for (int n = 0; n < nNumberOfBars; n++)
    {
        removeChildComponent(MeterArray[n]);
        delete MeterArray[n];
        MeterArray[n] = NULL;
    }

    delete [] MeterArray;
    MeterArray = NULL;

    deleteAllChildren();
}

void MeterBar::visibilityChanged()
{
    int x = 0;
    int y = 0;
    int width = nWidth;
    int height = 134 * nMainSegmentHeight + 1;
    int segment_height = nMainSegmentHeight;

    int nKmeterLevel = nMeterCrestFactor; // bar K-Meter level (in 0.1 dB)
    int nRange = 0; // bar level range (in 0.1 dB)

    setBounds(nPosX, nPosY, width, height);

    for (int n = 0; n < nNumberOfBars; n++)
    {
        if (isExpanded)
        {
            nRange = 1;
        }
        else
        {
            if (nKmeterLevel > nLimitTopBars)
            {
                nRange = 5;
            }
            else if (nKmeterLevel > nLimitGreenBars_1)
            {
                nRange = 10;
            }
            else if (nKmeterLevel > nLimitGreenBars_2)
            {
                nRange = 60;
            }
            else
            {
                nRange = 100;
            }
        }

        width = nWidth;
        x = 0;

        if (isExpanded)
        {
            segment_height = nMainSegmentHeight;
        }
        else if (nKmeterLevel > nLimitTopBars)
        {
            segment_height = nMainSegmentHeight;
        }
        else if (nKmeterLevel > nLimitGreenBars_1)
        {
            segment_height = 2 * nMainSegmentHeight;
        }
        else if (nKmeterLevel > nLimitGreenBars_2)
        {
            segment_height = 6 * nMainSegmentHeight;
        }
        else if (n == nNumberOfBars - 1)
        {
            if (nMeterCrestFactor == 0)
            {
                segment_height = 10 * nMainSegmentHeight;
            }
            else if (nMeterCrestFactor == +120)
            {
                segment_height = 14 * nMainSegmentHeight;
            }
            else if (nMeterCrestFactor == +140)
            {
                segment_height = 13 * nMainSegmentHeight;
            }
            else // K-20
            {
                segment_height = 10 * nMainSegmentHeight;
            }
        }
        else
        {
            if (nMeterCrestFactor == 0)
            {
                segment_height = 11 * nMainSegmentHeight;
            }
            else if (nMeterCrestFactor == +120)
            {
                segment_height = 12 * nMainSegmentHeight;
            }
            else if (nMeterCrestFactor == +140)
            {
                segment_height = 11 * nMainSegmentHeight;
            }
            else // K-20
            {
                segment_height = 10 * nMainSegmentHeight;
            }
        }

        MeterArray[n]->setBounds(x, y, width, segment_height + 1);
        y += segment_height;

        nKmeterLevel -= nRange;
    }
}

void MeterBar::paint(Graphics& g)
{
    g.fillAll(Colours::black);
}

void MeterBar::resized()
{
}


void MeterBar::setLevels(float peakLevel, float averageLevel, float peakLevelPeak, float averageLevelPeak)
{
    if ((peakLevel != fPeakLevel) || (averageLevel != fAverageLevel) || (peakLevelPeak != fPeakLevelPeak) || (averageLevelPeak != fAverageLevelPeak))
    {
        fPeakLevel = peakLevel;
        fAverageLevel = averageLevel;

        fPeakLevelPeak = peakLevelPeak;
        fAverageLevelPeak = averageLevelPeak;

        for (int n = 0; n < nNumberOfBars; n++)
        {
            MeterArray[n]->setLevels(fPeakLevel, fAverageLevel, fPeakLevelPeak, fAverageLevelPeak);
        }
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
