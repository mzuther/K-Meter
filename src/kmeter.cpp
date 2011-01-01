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

#include "kmeter.h"

Kmeter::Kmeter(const String& componentName, int posX, int posY, int nHeadroom, int nNumChannels, bool bExpanded, bool bDisplayPeakMeter, int nSegmentHeight)
{
    setName(componentName);
    nChannels = nNumChannels;
    isExpanded = bExpanded;
    displayPeakMeter = bDisplayPeakMeter;

    nPosX = posX;
    nPosY = posY;
    nMainSegmentHeight = nSegmentHeight;

    if (nHeadroom == 0)
    {
        nMeterHeadroom = 0;
    }
    else if (nHeadroom == 12)
    {
        nMeterHeadroom = 12;
    }
    else if (nHeadroom == 14)
    {
        nMeterHeadroom = 14;
    }
    else
    {
        nMeterHeadroom = 20;
    }

    int nPositionX = 0;

    if (displayPeakMeter)
    {
        AverageMeters = new MeterBar*[nChannels];
        PeakMeters = new MeterBar*[nChannels];

        for (int nChannel = 0; nChannel < nChannels; nChannel++)
        {
            nPositionX = 17 + nChannel * 91 - (nChannel % 2) * 37;

            AverageMeters[nChannel] = new MeterBar(String("Average Meter #") + String(nChannel), nPositionX, 48, 18, nMeterHeadroom, bExpanded, nMainSegmentHeight, T("center"));
            addAndMakeVisible(AverageMeters[nChannel]);

            nPositionX = 3 + nChannel * 91;

            PeakMeters[nChannel] = new MeterBar(String("Peak Meter #") + String(nChannel), nPositionX, 48, 9, nMeterHeadroom, bExpanded, nMainSegmentHeight, (nChannel % 2) ? T("left") : T("right"));
            addAndMakeVisible(PeakMeters[nChannel]);
        }
    }
    else
    {
        AverageMeters = new MeterBar*[nChannels];
        PeakMeters = NULL;

        for (int nChannel = 0; nChannel < nChannels; nChannel++)
        {
            nPositionX = 7 + nChannel * 72;

            AverageMeters[nChannel] = new MeterBar(String("Average Meter #") + String(nChannel), nPositionX, 48, 20, nMeterHeadroom, bExpanded, nMainSegmentHeight, T("center"));
            addAndMakeVisible(AverageMeters[nChannel]);
        }
    }

    OverflowMeters = new OverflowMeter*[nChannels];
    MaximumPeakLabels = new PeakLabel*[nChannels];

    for (int nChannel = 0; nChannel < nChannels; nChannel++)
    {
        nPositionX = 3 + nChannel * 68;

        OverflowMeters[nChannel] = new OverflowMeter(String("Overflows #") + String(nChannel));
        OverflowMeters[nChannel]->setBounds(nPositionX, 3, 32, 16);
        addAndMakeVisible(OverflowMeters[nChannel]);

        MaximumPeakLabels[nChannel] = new PeakLabel(String("Maximum Peak #") + String(nChannel), nHeadroom);
        MaximumPeakLabels[nChannel]->setBounds(nPositionX, 23, 32, 16);
        addAndMakeVisible(MaximumPeakLabels[nChannel]);
    }
}

Kmeter::~Kmeter()
{
    for (int nChannel = 0; nChannel < nChannels; nChannel++)
    {
        delete AverageMeters[nChannel];
        AverageMeters[nChannel] = NULL;

        if (displayPeakMeter)
        {
            delete PeakMeters[nChannel];
            PeakMeters[nChannel] = NULL;
        }

        delete OverflowMeters[nChannel];
        OverflowMeters[nChannel] = NULL;

        delete MaximumPeakLabels[nChannel];
        MaximumPeakLabels[nChannel] = NULL;
    }

    delete [] AverageMeters;
    AverageMeters = NULL;

    delete [] PeakMeters;
    PeakMeters = NULL;

    delete [] OverflowMeters;
    OverflowMeters = NULL;

    delete [] MaximumPeakLabels;
    MaximumPeakLabels = NULL;

    deleteAllChildren();
}

void Kmeter::visibilityChanged()
{
    int height = 134 * nMainSegmentHeight + 52;
    setBounds(nPosX, nPosY, 106, height);
}

void Kmeter::paint(Graphics& g)
{
    g.fillAll(Colours::grey.withAlpha(0.1f));

    g.setColour(Colours::darkgrey);
    g.drawRect(0, 0, getWidth() - 1, getHeight() - 1);

    g.setColour(Colours::darkgrey.darker(0.8f));
    g.drawRect(1, 1, getWidth() - 1, getHeight() - 1);

    g.setColour(Colours::darkgrey.darker(0.4f));
    g.drawRect(1, 1, getWidth() - 2, getHeight() - 2);

    int x = 3;
    int y = 43;
    int width = 24;
    int height = 11;
    String strMarker;

    g.setColour(Colours::white);
    g.setFont(12.0f);

    g.drawFittedText("Over", 35, 3, 36, 16, Justification::centred, 1, 1.0f);
    g.drawFittedText("Peak", 35, 23, 36, 16, Justification::centred, 1, 1.0f);

    g.setFont(11.0f);

    if (isExpanded)
    {
        y -= 10 * nMainSegmentHeight;
        int nStart = 0;

        if (nMeterHeadroom < 8)
        {
            nStart = 0;
        }
        else
        {
            nStart = 8;    // zoom into important region
        }

        for (int n = 0; n >= -13; n -= 1)
        {
            if ((nStart + n) > 0)
            {
                strMarker = T("+") + String(nStart + n);
            }
            else
            {
                strMarker = String(nStart + n);
            }

            y += 10 * nMainSegmentHeight;
            drawMarkers(g, strMarker, x, y, width, height);
        }
    }
    else if (nMeterHeadroom == 0)
    {
        y -= 8 * nMainSegmentHeight;

        for (int n = 0; n >= -40; n -= 4)
        {
            if (n > 0)
            {
                strMarker = T("+") + String(n);
            }
            else
            {
                strMarker = String(n);
            }

            y += 8 * nMainSegmentHeight;
            drawMarkers(g, strMarker, x, y, width, height);
        }

        for (int n = -50; n >= -80; n -= 10)
        {
            strMarker = String(n);

            y += 10 * nMainSegmentHeight;
            drawMarkers(g, strMarker, x, y, width, height);
        }
    }
    else if (nMeterHeadroom == 12)
    {
        y -= 8 * nMainSegmentHeight;

        for (int n = 12; n >= -28; n -= 4)
        {
            if (n > 0)
            {
                strMarker = T("+") + String(n);
            }
            else
            {
                strMarker = String(n);
            }

            y += 8 * nMainSegmentHeight;
            drawMarkers(g, strMarker, x, y, width, height);
        }

        y -= 6 * nMainSegmentHeight;

        for (int n = -30; n >= -60; n -= 10)
        {
            strMarker = String(n);

            y += 10 * nMainSegmentHeight;
            drawMarkers(g, strMarker, x, y, width, height);
        }
    }
    else if (nMeterHeadroom == 14)
    {
        strMarker = String(T("+14"));
        drawMarkers(g, strMarker, x, y, width, height);
        y -= 4 * nMainSegmentHeight;

        for (int n = 12; n >= -28; n -= 4)
        {
            if (n > 0)
            {
                strMarker = T("+") + String(n);
            }
            else
            {
                strMarker = String(n);
            }

            y += 8 * nMainSegmentHeight;
            drawMarkers(g, strMarker, x, y, width, height);
        }

        y -= 6 * nMainSegmentHeight;

        for (int n = -30; n >= -60; n -= 10)
        {
            strMarker = String(n);

            y += 10 * nMainSegmentHeight;
            drawMarkers(g, strMarker, x, y, width, height);
        }
    }
    else
    {
        y -= 8 * nMainSegmentHeight;

        for (int n = 20; n >= -24; n -= 4)
        {
            if (n > 0)
            {
                strMarker = T("+") + String(n);
            }
            else
            {
                strMarker = String(n);
            }

            y += 8 * nMainSegmentHeight;
            drawMarkers(g, strMarker, x, y, width, height);
        }

        y -= 4 * nMainSegmentHeight;

        for (int n = -30; n >= -60; n -= 10)
        {
            strMarker = String(n);

            y += 10 * nMainSegmentHeight;
            drawMarkers(g, strMarker, x, y, width, height);
        }
    }
}

void Kmeter::resized()
{
}

void Kmeter::setLevels(MeterBallistics* pMeterBallistics)
{
    for (int nChannel = 0; nChannel < nChannels; nChannel++)
    {
        AverageMeters[nChannel]->setLevels(pMeterBallistics->getAverageMeterLevel(nChannel), pMeterBallistics->getAverageMeterPeakLevel(nChannel));

        if (displayPeakMeter)
        {
            PeakMeters[nChannel]->setLevels(pMeterBallistics->getPeakMeterLevel(nChannel), pMeterBallistics->getPeakMeterPeakLevel(nChannel));
        }

        MaximumPeakLabels[nChannel]->updateLevel(pMeterBallistics->getMaximumPeakLevel(nChannel));

        OverflowMeters[nChannel]->setOverflows(pMeterBallistics->getNumberOfOverflows(nChannel));
    }
}

void Kmeter::drawMarkers(Graphics& g, String& strMarker, int x, int y, int width, int height)
{
    g.setColour(Colours::white);
    g.drawFittedText(strMarker, x + 38, y, width, height, Justification::centred, 1, 1.0f);

    g.setColour(Colours::grey);

    int nMarkerY = y + 5;
    int nStart = 0;
    int nEnd = 0;
    int nWidth = 0;

    if (displayPeakMeter)
    {
        nWidth = 3;
        nStart = x + 10;
        nEnd = nStart + nWidth;
    }
    else
    {
        nWidth = 9;
        nStart = x + 25;
        nEnd = nStart + nWidth;
    }

    for (int nMarkerX = nStart; nMarkerX < nEnd; nMarkerX++)
    {
        g.setPixel(nMarkerX, nMarkerY);
    }

    if (displayPeakMeter)
    {
        nStart = x + 89;
        nEnd = nStart - nWidth;
    }
    else
    {
        nStart = x + 74;
        nEnd = nStart - nWidth;
    }

    for (int nMarkerX = nStart; nMarkerX > nEnd; nMarkerX--)
    {
        g.setPixel(nMarkerX, nMarkerY);
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
