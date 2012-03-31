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

#include "kmeter.h"

Kmeter::Kmeter(const String& componentName, int posX, int posY, int nCrestFactor, int nNumChannels, bool bExpanded, bool bDisplayPeakMeter, int nSegmentHeight)
{
    setName(componentName);
    nInputChannels = nNumChannels;
    nStereoInputChannels = (nNumChannels + (nNumChannels % 2)) / 2;
    isExpanded = bExpanded;
    displayPeakMeter = bDisplayPeakMeter;

    if (nInputChannels <= 2)
    {
        nMeterPositionTop = 0;
    }
    else
    {
        nMeterPositionTop = 20;
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
    else
    {
        nMeterCrestFactor = 20;
    }

    int nPositionX = 0;
    LevelMeters = new MeterBar*[nInputChannels];

    for (int nChannel = 0; nChannel < nInputChannels; nChannel++)
    {
        nPositionX = 8 + nChannel * KMETER_STEREO_WIDTH_2;

        if (nChannel % 2)
        {
            nPositionX += 15;
        }

        LevelMeters[nChannel] = new MeterBar(String("Level Meter #") + String(nChannel), nPositionX, nMeterPositionTop + 48, 20, nMeterCrestFactor, bExpanded, displayPeakMeter, nMainSegmentHeight);
        addAndMakeVisible(LevelMeters[nChannel]);
    }

    OverflowMeters = new OverflowMeter*[nInputChannels];
    MaximumPeakLabels = new PeakLabel*[nInputChannels];

    for (int nChannel = 0; nChannel < nInputChannels; nChannel++)
    {
        nPositionX = 4 + nChannel * KMETER_STEREO_WIDTH_2;

        if (nChannel % 2)
        {
            nPositionX += 11;
        }

        OverflowMeters[nChannel] = new OverflowMeter(String("Overflows #") + String(nChannel));
        OverflowMeters[nChannel]->setBounds(nPositionX, nMeterPositionTop + 3, 32, 16);
        addAndMakeVisible(OverflowMeters[nChannel]);

        MaximumPeakLabels[nChannel] = new PeakLabel(String("Maximum Peak #") + String(nChannel), nCrestFactor);
        MaximumPeakLabels[nChannel]->setBounds(nPositionX, nMeterPositionTop + 23, 32, 16);
        addAndMakeVisible(MaximumPeakLabels[nChannel]);
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
    int height = 134 * nMainSegmentHeight + 74;
    int width = nStereoInputChannels * KMETER_STEREO_WIDTH + 2;
    setBounds(nPosX, nPosY, width, height);
}

void Kmeter::paint(Graphics& g)
{
    for (int nChannel = 0; nChannel < nStereoInputChannels; nChannel++)
    {
        int x = 5 + nChannel * KMETER_STEREO_WIDTH;
        int y = nMeterPositionTop + 43;
        int width = 24;
        int height = 11;
        String strMarker;

        if (nInputChannels > 2)
        {
            g.setColour(Colours::white);
            g.setFont(13.0f);

            g.drawFittedText(String("Channels ") + String(2 * nChannel + 1) + T("+") + String(2 * nChannel + 2), x - 4, 0, KMETER_STEREO_WIDTH - 5, 17, Justification::centred, 1, 1.0f);

            g.setColour(Colours::grey.withAlpha(0.3f));
            g.fillRect(x - 4, 0, KMETER_STEREO_WIDTH - 6, 17);

            g.setColour(Colours::darkgrey);
            g.drawRect(x - 4, 0, KMETER_STEREO_WIDTH - 6, 17);

            g.setColour(Colours::darkgrey.darker(0.8f));
            g.drawRect(x - 3, 1, KMETER_STEREO_WIDTH - 6, 17);

            g.setColour(Colours::darkgrey.darker(0.4f));
            g.drawRect(x - 3, 1, KMETER_STEREO_WIDTH - 7, 16);
        }

        g.setColour(Colours::grey.withAlpha(0.1f));
        g.fillRect(x - 4, nMeterPositionTop, KMETER_STEREO_WIDTH - 6, getHeight() - 21);

        g.setColour(Colours::darkgrey);
        g.drawRect(x - 4, nMeterPositionTop, KMETER_STEREO_WIDTH - 6, getHeight() - 21);

        g.setColour(Colours::darkgrey.darker(0.8f));
        g.drawRect(x - 3, nMeterPositionTop + 1, KMETER_STEREO_WIDTH - 6, getHeight() - 21);

        g.setColour(Colours::darkgrey.darker(0.4f));
        g.drawRect(x - 3, nMeterPositionTop + 1, KMETER_STEREO_WIDTH - 7, getHeight() - 22);

        g.setColour(Colours::white);
        g.setFont(12.0f);

        g.drawFittedText(T("Over"), x + 31, nMeterPositionTop + 3, 36, 16, Justification::centred, 1, 1.0f);
        g.drawFittedText(T("Peak"), x + 31, nMeterPositionTop + 23, 36, 16, Justification::centred, 1, 1.0f);

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
        else if (nMeterCrestFactor == 0)
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
        else if (nMeterCrestFactor == 12)
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
        else if (nMeterCrestFactor == 14)
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

void Kmeter::drawMarkers(Graphics& g, String& strMarker, int x, int y, int width, int height)
{
    g.setColour(Colours::white);
    g.drawFittedText(strMarker, x + 36, y, width, height, Justification::centred, 1, 1.0f);

    g.setColour(Colours::grey);

    int nMarkerY = y + 5;
    int nStart = 0;
    int nEnd = 0;
    int nWidth = 0;

    nWidth = 9;
    nStart = x + 25;
    nEnd = nStart + nWidth;

    for (int nMarkerX = nStart; nMarkerX < nEnd; nMarkerX++)
    {
        g.setPixel(nMarkerX, nMarkerY);
    }

    nStart = x + 70;
    nEnd = nStart - nWidth;

    for (int nMarkerX = nStart; nMarkerX > nEnd; nMarkerX--)
    {
        g.setPixel(nMarkerX, nMarkerY);
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
