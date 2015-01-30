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

#include "kmeter.h"

Kmeter::Kmeter(const String &componentName, int nCrestFactor, int nNumChannels, bool bExpanded, bool bHorizontalMeter, bool bDisplayPeakMeter, int nSegmentHeight)
{
    setName(componentName);

    // this component blends in with the background
    setOpaque(false);

    nInputChannels = nNumChannels;
    LevelMeters = new MeterBar*[nInputChannels];

    for (int nChannel = 0; nChannel < nInputChannels; nChannel++)
    {
        LevelMeters[nChannel] = new MeterBar("Level Meter #" + String(nChannel), nCrestFactor, bExpanded, bHorizontalMeter, bDisplayPeakMeter, nSegmentHeight);

        addAndMakeVisible(LevelMeters[nChannel]);
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


void Kmeter::applySkin(Skin *pSkin)
{
    if (nInputChannels == 1)
    {
        pSkin->placeComponent(LevelMeters[0], "meter_kmeter");
        pSkin->placeAndSkinStateLabel(OverflowMeters[0], "label_over");
        pSkin->placeAndSkinStateLabel(MaximumPeakLabels[0], "label_peak");
    }
    else if (nInputChannels == 2)
    {
        pSkin->placeComponent(LevelMeters[0], "meter_kmeter_left");
        pSkin->placeAndSkinStateLabel(OverflowMeters[0], "label_over_left");
        pSkin->placeAndSkinStateLabel(MaximumPeakLabels[0], "label_peak_left");

        pSkin->placeComponent(LevelMeters[1], "meter_kmeter_right");
        pSkin->placeAndSkinStateLabel(OverflowMeters[1], "label_over_right");
        pSkin->placeAndSkinStateLabel(MaximumPeakLabels[1], "label_peak_right");
    }
    else if (nInputChannels == 6)
    {
        pSkin->placeComponent(LevelMeters[0], "meter_kmeter_left");
        pSkin->placeAndSkinStateLabel(OverflowMeters[0], "label_over_left");
        pSkin->placeAndSkinStateLabel(MaximumPeakLabels[0], "label_peak_left");

        pSkin->placeComponent(LevelMeters[1], "meter_kmeter_right");
        pSkin->placeAndSkinStateLabel(OverflowMeters[1], "label_over_right");
        pSkin->placeAndSkinStateLabel(MaximumPeakLabels[1], "label_peak_right");

        pSkin->placeComponent(LevelMeters[2], "meter_kmeter_center");
        pSkin->placeAndSkinStateLabel(OverflowMeters[2], "label_over_center");
        pSkin->placeAndSkinStateLabel(MaximumPeakLabels[2], "label_peak_center");

        pSkin->placeComponent(LevelMeters[3], "meter_kmeter_lfe");
        pSkin->placeAndSkinStateLabel(OverflowMeters[3], "label_over_lfe");
        pSkin->placeAndSkinStateLabel(MaximumPeakLabels[3], "label_peak_lfe");

        pSkin->placeComponent(LevelMeters[4], "meter_kmeter_ls");
        pSkin->placeAndSkinStateLabel(OverflowMeters[4], "label_over_ls");
        pSkin->placeAndSkinStateLabel(MaximumPeakLabels[4], "label_peak_ls");

        pSkin->placeComponent(LevelMeters[5], "meter_kmeter_rs");
        pSkin->placeAndSkinStateLabel(OverflowMeters[5], "label_over_rs");
        pSkin->placeAndSkinStateLabel(MaximumPeakLabels[5], "label_peak_rs");
    }
    else
    {
        DBG("[K-Meter] channel configuration (" + String(nInputChannels) + " channels) not supported");
    }

    Component *parent = getParentComponent();

    if (parent != nullptr)
    {
        setBounds(0, 0, parent->getWidth(), parent->getHeight());
    }
}


void Kmeter::resized()
{
}


void Kmeter::setLevels(MeterBallistics *pMeterBallistics)
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
