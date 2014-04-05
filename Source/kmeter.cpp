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

Kmeter::Kmeter(const String& componentName, int posX, int posY, int nCrestFactor, int nNumChannels, bool bIsSurround, bool bExpanded, bool bHorizontalMeter, bool bDisplayPeakMeter, int nSegmentHeight)
{
    setName(componentName);

    // this component blends in with the background
    setOpaque(false);

    nInputChannels = nNumChannels;
    isSurround = bIsSurround;

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

    int nPositionX = 0;
    LevelMeters = new MeterBar*[nInputChannels];

    if (nInputChannels == 1)
    {
        nPositionX = KMETER_STEREO_WIDTH_2 - 10;

        LevelMeters[0] = new MeterBar("Level Meter #0", nPositionX - 2, nMeterPositionTop + 48, 24, nCrestFactor, bExpanded, bHorizontalMeter, bDisplayPeakMeter, nMainSegmentHeight);

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

            LevelMeters[nChannel] = new MeterBar("Level Meter #" + String(nChannel), nPositionX, nMeterPositionTop + 48, 20, nCrestFactor, bExpanded, bHorizontalMeter, bDisplayPeakMeter, nMainSegmentHeight);

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
        LevelMeters[nChannel] = nullptr;

        delete OverflowMeters[nChannel];
        OverflowMeters[nChannel] = nullptr;

        delete MaximumPeakLabels[nChannel];
        MaximumPeakLabels[nChannel] = nullptr;
    }

    delete [] LevelMeters;
    LevelMeters = nullptr;

    delete [] OverflowMeters;
    OverflowMeters = nullptr;

    delete [] MaximumPeakLabels;
    MaximumPeakLabels = nullptr;

    deleteAllChildren();
}


void Kmeter::visibilityChanged()
{
    int nStereoInputChannels = (nInputChannels + (nInputChannels % 2)) / 2;
    int nWidth = nStereoInputChannels * (KMETER_STEREO_WIDTH + 6) - 6;
    int nHeight = 134 * nMainSegmentHeight + 74;

    if (isSurround)
    {
        nHeight += 20;
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


// Local Variables:
// ispell-local-dictionary: "british"
// End:
