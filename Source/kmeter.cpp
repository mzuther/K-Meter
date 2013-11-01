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

#include "kmeter.h"

Kmeter::Kmeter(const String& componentName, int posX, int posY, int nCrestFactor, int nNumChannels, const String& unitName, bool bIsSurround, bool bExpanded, bool bHorizontal, bool bDisplayPeakMeter, int nSegmentHeight)
{
    setName(componentName);

    // this component blends in with the background
    setOpaque(false);

    nInputChannels = nNumChannels;
    isSurround = bIsSurround;
    nStereoInputChannels = (nNumChannels + (nNumChannels % 2)) / 2;
    bHorizontalMeter = bHorizontal;
    isExpanded = bExpanded;
    displayPeakMeter = bDisplayPeakMeter;
    strUnit = unitName;

    if (isSurround)
    {
        nMeterPositionTop = 20;
    }
    else
    {
        nMeterPositionTop = 0;
    }

    nPosX = posX;
    nPosY = posY;
    nMainSegmentHeight = nSegmentHeight;

    if (nCrestFactor == 0)
    {
        nMeterCrestFactor = 0;
    }
    else if (nCrestFactor == 12)
    {
        nMeterCrestFactor = 12;
    }
    else if (nCrestFactor == 14)
    {
        nMeterCrestFactor = 14;
    }
    else // K-20
    {
        nMeterCrestFactor = 20;
    }

    int nPositionX = 0;
    LevelMeters = new MeterBar*[nInputChannels];

    if (nInputChannels == 1)
    {
        nPositionX = KMETER_STEREO_WIDTH_2 - 10;

        if (bHorizontalMeter)
        {
            nPositionX -= 2;

            LevelMeters[0] = new MeterBar("Level Meter #0", nMeterPositionTop + 5, nPositionX, 24, nMeterCrestFactor, isExpanded, bHorizontalMeter, displayPeakMeter, nMainSegmentHeight);
        }
        else
        {
            LevelMeters[0] = new MeterBar("Level Meter #0", nPositionX - 2, nMeterPositionTop + 48, 24, nMeterCrestFactor, isExpanded, bHorizontalMeter, displayPeakMeter, nMainSegmentHeight);
        }

        addAndMakeVisible(LevelMeters[0]);
    }
    else
    {
        for (int nChannel = 0; nChannel < nInputChannels; nChannel++)
        {
            nPositionX = 9 + nChannel * (KMETER_STEREO_WIDTH_2 + 3);

            if (nChannel % 2)
            {
                nPositionX += 12;
            }

            if (bHorizontalMeter)
            {
                LevelMeters[nChannel] = new MeterBar("Level Meter #" + String(nChannel), nMeterPositionTop + 5, nPositionX, 20, nMeterCrestFactor, isExpanded, bHorizontalMeter, displayPeakMeter, nMainSegmentHeight);
            }
            else
            {
                LevelMeters[nChannel] = new MeterBar("Level Meter #" + String(nChannel), nPositionX, nMeterPositionTop + 48, 20, nMeterCrestFactor, isExpanded, bHorizontalMeter, displayPeakMeter, nMainSegmentHeight);
            }

            addAndMakeVisible(LevelMeters[nChannel]);
        }
    }

    OverflowMeters = new OverflowMeter*[nInputChannels];
    MaximumPeakLabels = new PeakLabel*[nInputChannels];

    if (nInputChannels == 1)
    {
        OverflowMeters[0] = new OverflowMeter("Overflows #0");
        addAndMakeVisible(OverflowMeters[0]);

        MaximumPeakLabels[0] = new PeakLabel("Maximum Peak #0", nCrestFactor);
        addAndMakeVisible(MaximumPeakLabels[0]);
    }
    else
    {
        for (int nChannel = 0; nChannel < nInputChannels; nChannel++)
        {
            OverflowMeters[nChannel] = new OverflowMeter("Overflows #" + String(nChannel));
            addAndMakeVisible(OverflowMeters[nChannel]);

            MaximumPeakLabels[nChannel] = new PeakLabel("Maximum Peak #" + String(nChannel), nCrestFactor);
            addAndMakeVisible(MaximumPeakLabels[nChannel]);
        }
    }
}


Kmeter::~Kmeter()
{
    for (int nChannel = 0; nChannel < nInputChannels; nChannel++)
    {
        delete LevelMeters[nChannel];
        LevelMeters[nChannel] = NULL;

        delete OverflowMeters[nChannel];
        OverflowMeters[nChannel] = NULL;

        delete MaximumPeakLabels[nChannel];
        MaximumPeakLabels[nChannel] = NULL;
    }

    delete [] LevelMeters;
    LevelMeters = NULL;

    delete [] OverflowMeters;
    OverflowMeters = NULL;

    delete [] MaximumPeakLabels;
    MaximumPeakLabels = NULL;

    deleteAllChildren();
}


void Kmeter::visibilityChanged()
{
    if (bHorizontalMeter)
    {
        nWidth = 134 * nMainSegmentHeight + 54;
        nHeight = nStereoInputChannels * (KMETER_STEREO_WIDTH + 6) - 6;

        if (isSurround)
        {
            nWidth += 20;
        }

        if (bHorizontalMeter)
        {
            nWidth += 32;
        }

        setBounds(nPosX, nPosY, nWidth, nHeight);

        if (nInputChannels == 1)
        {
            int nPositionX = KMETER_STEREO_WIDTH_2 - 8;

            OverflowMeters[0]->setBounds(nWidth - 74, nPositionX, 32, 16);
            MaximumPeakLabels[0]->setBounds(nWidth - 38, nPositionX, 32, 16);
        }
        else
        {
            for (int nChannel = 0; nChannel < nInputChannels; nChannel++)
            {
                int nPositionX = 11 + nChannel * (KMETER_STEREO_WIDTH_2 + 3);

                if (nChannel % 2)
                {
                    nPositionX += 12;
                }

                OverflowMeters[nChannel]->setBounds(nWidth - 74, nPositionX, 32, 16);
                MaximumPeakLabels[nChannel]->setBounds(nWidth - 38, nPositionX, 32, 16);
            }
        }
    }
    else
    {
        nWidth = nStereoInputChannels * (KMETER_STEREO_WIDTH + 6) - 6;
        nHeight = 134 * nMainSegmentHeight + 54;

        if (isSurround)
        {
            nHeight += 20;
        }

        if (bHorizontalMeter)
        {
            nHeight += 32;
        }

        setBounds(nPosX, nPosY, nWidth, nHeight);

        if (nInputChannels == 1)
        {
            int nPositionX = KMETER_STEREO_WIDTH_2 - 16;

            OverflowMeters[0]->setBounds(nPositionX, nMeterPositionTop + 4, 32, 16);
            MaximumPeakLabels[0]->setBounds(nPositionX, nMeterPositionTop + 22, 32, 16);
        }
        else
        {
            for (int nChannel = 0; nChannel < nInputChannels; nChannel++)
            {
                int nPositionX = 5 + nChannel * (KMETER_STEREO_WIDTH_2 + 3);

                if (nChannel % 2)
                {
                    nPositionX += 8;
                }

                OverflowMeters[nChannel]->setBounds(nPositionX, nMeterPositionTop + 4, 32, 16);
                MaximumPeakLabels[nChannel]->setBounds(nPositionX, nMeterPositionTop + 22, 32, 16);
            }
        }
    }
}


void Kmeter::paint(Graphics& g)
{
    if (nInputChannels == 1)
    {
        paintMonoChannel(g);
    }
    else
    {
        for (int nStereoChannel = 0; nStereoChannel < nStereoInputChannels; nStereoChannel++)
        {
            paintStereoChannel(g, nStereoChannel);
        }
    }
}


void Kmeter::paintMonoChannel(Graphics& g)
{
    int x = 5;
    int y = nMeterPositionTop + 43;
    int width = 24;
    int height = 11;
    String strMarker;

    if (isSurround)
    {
        g.setColour(Colours::white);
        g.setFont(13.0f);

        g.drawFittedText("Channel sum", x - 5, 0, KMETER_STEREO_WIDTH, 17, Justification::centred, 1, 1.0f);

        g.setColour(Colours::grey.withAlpha(0.3f));
        g.fillRect(x - 5, 0, KMETER_STEREO_WIDTH, 18);

        g.setColour(Colours::darkgrey);
        g.drawRect(x - 5, 0, KMETER_STEREO_WIDTH - 1, 17);

        g.setColour(Colours::darkgrey.darker(0.7f));
        g.drawRect(x - 4, 1, KMETER_STEREO_WIDTH - 1, 17);

        g.setColour(Colours::darkgrey.darker(0.4f));
        g.drawRect(x - 4, 1, KMETER_STEREO_WIDTH - 2, 16);
    }

    if (bHorizontalMeter)
    {
        g.setColour(Colours::grey.withAlpha(0.1f));
        g.fillRect(x - 5, nMeterPositionTop, nWidth + 1, KMETER_STEREO_WIDTH);

        g.setColour(Colours::darkgrey);
        g.drawRect(x - 5, nMeterPositionTop, nWidth - 1, KMETER_STEREO_WIDTH - 1);

        g.setColour(Colours::darkgrey.darker(0.7f));
        g.drawRect(x - 4, nMeterPositionTop + 1, nWidth - 1, KMETER_STEREO_WIDTH - 1);

        g.setColour(Colours::darkgrey.darker(0.4f));
        g.drawRect(x - 4, nMeterPositionTop + 1, nWidth - 2, KMETER_STEREO_WIDTH - 2);
    }
    else
    {
        g.setColour(Colours::grey.withAlpha(0.1f));
        g.fillRect(x - 5, nMeterPositionTop, KMETER_STEREO_WIDTH, nHeight - 19);

        g.setColour(Colours::darkgrey);
        g.drawRect(x - 5, nMeterPositionTop, KMETER_STEREO_WIDTH - 1, nHeight - 21);

        g.setColour(Colours::darkgrey.darker(0.7f));
        g.drawRect(x - 4, nMeterPositionTop + 1, KMETER_STEREO_WIDTH - 1, nHeight - 21);

        g.setColour(Colours::darkgrey.darker(0.4f));
        g.drawRect(x - 4, nMeterPositionTop + 1, KMETER_STEREO_WIDTH - 2, nHeight - 22);
    }

    g.setColour(Colours::white);
    g.setFont(12.0f);

    if (bHorizontalMeter)
    {
        g.drawFittedText("Over", nWidth - 73, nMeterPositionTop + 2, 36, 16, Justification::centred, 1, 1.0f);
        g.drawFittedText("Over", nWidth - 73, nMeterPositionTop + 86, 36, 16, Justification::centred, 1, 1.0f);

        g.drawFittedText("Peak", nWidth - 41, nMeterPositionTop + 2, 36, 16, Justification::centred, 1, 1.0f);
        g.drawFittedText("Peak", nWidth - 41, nMeterPositionTop + 86, 36, 16, Justification::centred, 1, 1.0f);

        g.drawFittedText(strUnit, 4, nMeterPositionTop + 2, 36, 16, Justification::left, 1, 1.0f);
        g.drawFittedText(strUnit, 4, nMeterPositionTop + 86, 36, 16, Justification::left, 1, 1.0f);
    }
    else
    {
        g.drawFittedText("Over", x - 10, nMeterPositionTop + 4, 36, 16, Justification::right, 1, 1.0f);
        g.drawFittedText("Over", x + 70, nMeterPositionTop + 4, 36, 16, Justification::left, 1, 1.0f);

        g.drawFittedText("Peak", x - 10, nMeterPositionTop + 22, 36, 16, Justification::right, 1, 1.0f);
        g.drawFittedText("Peak", x + 70, nMeterPositionTop + 22, 36, 16, Justification::left, 1, 1.0f);

        g.drawFittedText(strUnit, x - 4, nMeterPositionTop + 571, 36, 16, Justification::centred, 1, 1.0f);
        g.drawFittedText(strUnit, x + 64, nMeterPositionTop + 571, 36, 16, Justification::centred, 1, 1.0f);
    }

    g.setFont(11.0f);

    if (isExpanded)
    {
        y -= 10 * nMainSegmentHeight;
        int nStart = 0;

        if (nMeterCrestFactor < 8)
        {
            nStart = 0;
        }
        else
        {
            nStart = 8; // zoom into important region
        }

        for (int n = 0; n >= -13; n -= 1)
        {
            if ((nStart + n) > 0)
            {
                strMarker = "+" + String(nStart + n);
            }
            else
            {
                strMarker = String(nStart + n);
            }

            y += 10 * nMainSegmentHeight;
            drawMarkersMono(g, strMarker, x, y, width, height);
        }
    }
    else if (nMeterCrestFactor == 0)
    {
        y -= 8 * nMainSegmentHeight;

        for (int n = 0; n >= -40; n -= 4)
        {
            if (n > 0)
            {
                strMarker = "+" + String(n);
            }
            else
            {
                strMarker = String(n);
            }

            y += 8 * nMainSegmentHeight;
            drawMarkersMono(g, strMarker, x, y, width, height);
        }

        for (int n = -50; n >= -80; n -= 10)
        {
            strMarker = String(n);

            y += 11 * nMainSegmentHeight;
            drawMarkersMono(g, strMarker, x, y, width, height);
        }
    }
    else if (nMeterCrestFactor == 12)
    {
        y -= 8 * nMainSegmentHeight;

        for (int n = 12; n >= -28; n -= 4)
        {
            if (n > 0)
            {
                strMarker = "+" + String(n);
            }
            else
            {
                strMarker = String(n);
            }

            y += 8 * nMainSegmentHeight;
            drawMarkersMono(g, strMarker, x, y, width, height);
        }

        y -= 8 * nMainSegmentHeight;

        for (int n = -30; n >= -60; n -= 10)
        {
            strMarker = String(n);

            y += 12 * nMainSegmentHeight;
            drawMarkersMono(g, strMarker, x, y, width, height);
        }
    }
    else if (nMeterCrestFactor == 14)
    {
        strMarker = "+14";
        drawMarkersMono(g, strMarker, x, y, width, height);
        y -= 4 * nMainSegmentHeight;

        for (int n = 12; n >= -28; n -= 4)
        {
            if (n > 0)
            {
                strMarker = "+" + String(n);
            }
            else
            {
                strMarker = String(n);
            }

            y += 8 * nMainSegmentHeight;
            drawMarkersMono(g, strMarker, x, y, width, height);
        }

        y -= 7 * nMainSegmentHeight;

        for (int n = -30; n >= -60; n -= 10)
        {
            strMarker = String(n);

            y += 11 * nMainSegmentHeight;
            drawMarkersMono(g, strMarker, x, y, width, height);
        }
    }
    else // K-20
    {
        y -= 8 * nMainSegmentHeight;

        for (int n = 20; n >= -24; n -= 4)
        {
            if (n > 0)
            {
                strMarker = "+" + String(n);
            }
            else
            {
                strMarker = String(n);
            }

            y += 8 * nMainSegmentHeight;
            drawMarkersMono(g, strMarker, x, y, width, height);
        }

        y -= 4 * nMainSegmentHeight;

        for (int n = -30; n >= -60; n -= 10)
        {
            strMarker = String(n);

            y += 10 * nMainSegmentHeight;
            drawMarkersMono(g, strMarker, x, y, width, height);
        }
    }
}


void Kmeter::paintStereoChannel(Graphics& g, int nStereoChannel)
{
    int x = 5 + nStereoChannel * (KMETER_STEREO_WIDTH + 6);
    int y = nMeterPositionTop + 43;
    int width = 24;
    int height = 11;
    String strMarker;

    if (isSurround)
    {
        g.setColour(Colours::white);
        g.setFont(13.0f);

        String strChannelLabel;

        switch (nStereoChannel)
        {
        case 0:
            strChannelLabel = "L | R";
            break;

        case 1:
            strChannelLabel = " C  | LFE";
            break;

        case 2:
            strChannelLabel = "Ls | Rs";
            break;

        default:
            strChannelLabel = "---";
            break;
        }

        g.drawFittedText(strChannelLabel, x - 4, 0, KMETER_STEREO_WIDTH - 5, 17, Justification::centred, 1, 1.0f);

        g.setColour(Colours::grey.withAlpha(0.3f));
        g.fillRect(x - 5, 0, KMETER_STEREO_WIDTH, 18);

        g.setColour(Colours::darkgrey);
        g.drawRect(x - 5, 0, KMETER_STEREO_WIDTH - 1, 17);

        g.setColour(Colours::darkgrey.darker(0.7f));
        g.drawRect(x - 4, 1, KMETER_STEREO_WIDTH - 1, 17);

        g.setColour(Colours::darkgrey.darker(0.4f));
        g.drawRect(x - 4, 1, KMETER_STEREO_WIDTH - 2, 16);
    }

    if (bHorizontalMeter)
    {
        g.setColour(Colours::grey.withAlpha(0.1f));
        g.fillRect(x - 5, nMeterPositionTop, nWidth + 1, KMETER_STEREO_WIDTH);

        g.setColour(Colours::darkgrey);
        g.drawRect(x - 5, nMeterPositionTop, nWidth - 1, KMETER_STEREO_WIDTH - 1);

        g.setColour(Colours::darkgrey.darker(0.7f));
        g.drawRect(x - 4, nMeterPositionTop + 1, nWidth - 1, KMETER_STEREO_WIDTH - 1);

        g.setColour(Colours::darkgrey.darker(0.4f));
        g.drawRect(x - 4, nMeterPositionTop + 1, nWidth - 2, KMETER_STEREO_WIDTH - 2);
    }
    else
    {
        g.setColour(Colours::grey.withAlpha(0.1f));
        g.fillRect(x - 5, nMeterPositionTop, KMETER_STEREO_WIDTH, nHeight - 19);

        g.setColour(Colours::darkgrey);
        g.drawRect(x - 5, nMeterPositionTop, KMETER_STEREO_WIDTH - 1, nHeight - 21);

        g.setColour(Colours::darkgrey.darker(0.7f));
        g.drawRect(x - 4, nMeterPositionTop + 1, KMETER_STEREO_WIDTH - 1, nHeight - 21);

        g.setColour(Colours::darkgrey.darker(0.4f));
        g.drawRect(x - 4, nMeterPositionTop + 1, KMETER_STEREO_WIDTH - 2, nHeight - 22);
    }

    g.setColour(Colours::white);
    g.setFont(12.0f);

    if (bHorizontalMeter)
    {
        g.drawFittedText("Over", nWidth - 73, x + 39, 36, 16, Justification::centred, 1, 1.0f);
        g.drawFittedText("Peak", nWidth - 41, x + 39, 36, 16, Justification::centred, 1, 1.0f);
        g.drawFittedText(strUnit, 4, x + 39, 36, 16, Justification::left, 1, 1.0f);
    }
    else
    {
        g.drawFittedText("Over", x + 30, nMeterPositionTop + 4, 36, 16, Justification::centred, 1, 1.0f);
        g.drawFittedText("Peak", x + 30, nMeterPositionTop + 22, 36, 16, Justification::centred, 1, 1.0f);
        g.drawFittedText(strUnit, x + 30, nMeterPositionTop + 571, 36, 16, Justification::centred, 1, 1.0f);
    }

    g.setFont(11.0f);

    if (isExpanded)
    {
        y -= 10 * nMainSegmentHeight;
        int nStart = 0;

        if (nMeterCrestFactor < 8)
        {
            nStart = 0;
        }
        else
        {
            nStart = 8; // zoom into important region
        }

        for (int n = 0; n >= -13; n -= 1)
        {
            if ((nStart + n) > 0)
            {
                strMarker = "+" + String(nStart + n);
            }
            else
            {
                strMarker = String(nStart + n);
            }

            y += 10 * nMainSegmentHeight;
            drawMarkersStereo(g, strMarker, x, y, width, height);
        }
    }
    else if (nMeterCrestFactor == 0)
    {
        y -= 8 * nMainSegmentHeight;

        for (int n = 0; n >= -40; n -= 4)
        {
            if (n > 0)
            {
                strMarker = "+" + String(n);
            }
            else
            {
                strMarker = String(n);
            }

            y += 8 * nMainSegmentHeight;
            drawMarkersStereo(g, strMarker, x, y, width, height);
        }

        for (int n = -50; n >= -80; n -= 10)
        {
            strMarker = String(n);

            y += 11 * nMainSegmentHeight;
            drawMarkersStereo(g, strMarker, x, y, width, height);
        }
    }
    else if (nMeterCrestFactor == 12)
    {
        y -= 8 * nMainSegmentHeight;

        for (int n = 12; n >= -28; n -= 4)
        {
            if (n > 0)
            {
                strMarker = "+" + String(n);
            }
            else
            {
                strMarker = String(n);
            }

            y += 8 * nMainSegmentHeight;
            drawMarkersStereo(g, strMarker, x, y, width, height);
        }

        y -= 8 * nMainSegmentHeight;

        for (int n = -30; n >= -60; n -= 10)
        {
            strMarker = String(n);

            y += 12 * nMainSegmentHeight;
            drawMarkersStereo(g, strMarker, x, y, width, height);
        }
    }
    else if (nMeterCrestFactor == 14)
    {
        strMarker = "+14";
        drawMarkersStereo(g, strMarker, x, y, width, height);
        y -= 4 * nMainSegmentHeight;

        for (int n = 12; n >= -28; n -= 4)
        {
            if (n > 0)
            {
                strMarker = "+" + String(n);
            }
            else
            {
                strMarker = String(n);
            }

            y += 8 * nMainSegmentHeight;
            drawMarkersStereo(g, strMarker, x, y, width, height);
        }

        y -= 7 * nMainSegmentHeight;

        for (int n = -30; n >= -60; n -= 10)
        {
            strMarker = String(n);

            y += 11 * nMainSegmentHeight;
            drawMarkersStereo(g, strMarker, x, y, width, height);
        }
    }
    else // K-20
    {
        y -= 8 * nMainSegmentHeight;

        for (int n = 20; n >= -24; n -= 4)
        {
            if (n > 0)
            {
                strMarker = "+" + String(n);
            }
            else
            {
                strMarker = String(n);
            }

            y += 8 * nMainSegmentHeight;
            drawMarkersStereo(g, strMarker, x, y, width, height);
        }

        y -= 4 * nMainSegmentHeight;

        for (int n = -30; n >= -60; n -= 10)
        {
            strMarker = String(n);

            y += 10 * nMainSegmentHeight;
            drawMarkersStereo(g, strMarker, x, y, width, height);
        }
    }
}


void Kmeter::resized()
{
}


void Kmeter::setLevels(MeterBallistics* pMeterBallistics)
{
    for (int nChannel = 0; nChannel < nInputChannels; nChannel++)
    {
        LevelMeters[nChannel]->setLevels(pMeterBallistics->getPeakMeterLevel(nChannel), pMeterBallistics->getAverageMeterLevel(nChannel), pMeterBallistics->getPeakMeterPeakLevel(nChannel), pMeterBallistics->getAverageMeterPeakLevel(nChannel));

        MaximumPeakLabels[nChannel]->updateLevel(pMeterBallistics->getMaximumPeakLevel(nChannel));

        OverflowMeters[nChannel]->setOverflows(pMeterBallistics->getNumberOfOverflows(nChannel));
    }
}


void Kmeter::drawMarkersMono(Graphics& g, String& strMarker, int x, int y, int width, int height)
{
    if (bHorizontalMeter)
    {
        drawMarkersMonoHorizontal(g, strMarker, y, x, width, height);
    }
    else
    {
        drawMarkersMonoVertical(g, strMarker, x, y, width, height);
    }
}


void Kmeter::drawMarkersMonoVertical(Graphics& g, String& strMarker, int x, int y, int width, int height)
{
    g.setColour(Colours::white);
    g.drawFittedText(strMarker, x, y, width, height, Justification::centred, 1, 1.0f);
    g.drawFittedText(strMarker, x + 72, y, width, height, Justification::centred, 1, 1.0f);

    g.setColour(Colours::grey);

    int nMarkerY = y + 5;
    int nMarkerWidth = 9;
    int nStart = x + 25;
    int nEnd = nStart + nMarkerWidth;

    for (int nMarkerX = nStart; nMarkerX < nEnd; nMarkerX++)
    {
        g.setPixel(nMarkerX, nMarkerY);
    }

    nStart = x + 70;
    nEnd = nStart - nMarkerWidth;

    for (int nMarkerX = nStart; nMarkerX > nEnd; nMarkerX--)
    {
        g.setPixel(nMarkerX, nMarkerY);
    }
}


void Kmeter::drawMarkersMonoHorizontal(Graphics& g, String& strMarker, int x, int y, int width, int height)
{
    int nMeterWidth = LevelMeters[0]->getWidth() - x + 35;
    g.setColour(Colours::white);

    // make sure numbers don't overlap
    if ((nMeterCrestFactor == 14) && (strMarker == "+12"))
    {
        g.drawFittedText(strMarker, nMeterWidth - 6, y, width, height, Justification::centred, 1, 1.0f);
        g.drawFittedText(strMarker, nMeterWidth - 6, y + 84, width, height, Justification::centred, 1, 1.0f);
    }
    else
    {
        g.drawFittedText(strMarker, nMeterWidth, y, width, height, Justification::centred, 1, 1.0f);
        g.drawFittedText(strMarker, nMeterWidth, y + 84, width, height, Justification::centred, 1, 1.0f);
    }

    g.setColour(Colours::grey);

    int nMarkerX = nMeterWidth + 12;
    int nMarkerHeight = 16;
    int nStart = y + 17;
    int nEnd = nStart + nMarkerHeight;

    for (int nMarkerY = nStart; nMarkerY < nEnd; nMarkerY++)
    {
        g.setPixel(nMarkerX, nMarkerY);
    }

    nStart = y + 78;
    nEnd = nStart - nMarkerHeight;

    for (int nMarkerY = nStart; nMarkerY > nEnd; nMarkerY--)
    {
        g.setPixel(nMarkerX, nMarkerY);
    }
}


void Kmeter::drawMarkersStereo(Graphics& g, String& strMarker, int x, int y, int width, int height)
{
    if (bHorizontalMeter)
    {
        drawMarkersStereoHorizontal(g, strMarker, y, x, width, height);
    }
    else
    {
        drawMarkersStereoVertical(g, strMarker, x, y, width, height);
    }
}


void Kmeter::drawMarkersStereoVertical(Graphics& g, String& strMarker, int x, int y, int width, int height)
{
    g.setColour(Colours::white);
    g.drawFittedText(strMarker, x + 36, y, width, height, Justification::centred, 1, 1.0f);

    g.setColour(Colours::grey);

    int nMarkerY = y + 5;
    int nMarkerWidth = 9;
    int nStart = x + 25;
    int nEnd = nStart + nMarkerWidth;

    for (int nMarkerX = nStart; nMarkerX < nEnd; nMarkerX++)
    {
        g.setPixel(nMarkerX, nMarkerY);
    }

    nStart = x + 70;
    nEnd = nStart - nMarkerWidth;

    for (int nMarkerX = nStart; nMarkerX > nEnd; nMarkerX--)
    {
        g.setPixel(nMarkerX, nMarkerY);
    }
}


void Kmeter::drawMarkersStereoHorizontal(Graphics& g, String& strMarker, int x, int y, int width, int height)
{
    int nMeterWidth = LevelMeters[0]->getWidth() - x + 35;
    g.setColour(Colours::white);

    // make sure numbers don't overlap
    if ((nMeterCrestFactor == 14) && (strMarker == "+12"))
    {
        g.drawFittedText(strMarker, nMeterWidth - 6, y + 42, width, height, Justification::centred, 1, 1.0f);
    }
    else
    {
        g.drawFittedText(strMarker, nMeterWidth, y + 42, width, height, Justification::centred, 1, 1.0f);
    }

    g.setColour(Colours::grey);

    int nMarkerX = nMeterWidth + 12;
    int nMarkerHeight = 12;
    int nStart = y + 25;
    int nEnd = nStart + nMarkerHeight;

    for (int nMarkerY = nStart; nMarkerY < nEnd; nMarkerY++)
    {
        g.setPixel(nMarkerX, nMarkerY);
    }

    nStart = y + 70;
    nEnd = nStart - nMarkerHeight;

    for (int nMarkerY = nStart; nMarkerY > nEnd; nMarkerY--)
    {
        g.setPixel(nMarkerX, nMarkerY);
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
