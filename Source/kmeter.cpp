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

#include "kmeter.h"

Kmeter::Kmeter(int nCrestFactor, int nNumChannels, bool bExpanded, bool bHorizontalMeter, bool bDisplayPeakMeter, int nSegmentHeight)
{
    // this component blends in with the background
    setOpaque(false);

    nInputChannels = nNumChannels;
    displayPeakMeter = bDisplayPeakMeter;

    MeterBar::Orientation orientation;

    if (bHorizontalMeter)
    {
        orientation = MeterBar::orientationHorizontal;
    }
    else
    {
        orientation = MeterBar::orientationVertical;
    }

    for (int nChannel = 0; nChannel < nInputChannels; ++nChannel)
    {
        MeterBar *pMeterBar = p_arrLevelMeters.add(new MeterBar());
        pMeterBar->create(nCrestFactor, bExpanded, orientation, nSegmentHeight);

        addAndMakeVisible(pMeterBar);

        OverflowMeter *pOverflowMeter = p_arrOverflowMeters.add(new OverflowMeter("Overflows #" + String(nChannel)));
        addAndMakeVisible(pOverflowMeter);

        PeakLabel *pPeakLabel = p_arrMaximumPeakLabels.add(new PeakLabel("Maximum Peak #" + String(nChannel), nCrestFactor));
        addAndMakeVisible(pPeakLabel);
    }
}


void Kmeter::applySkin(Skin *pSkin)
{
    if (nInputChannels == 1)
    {
        pSkin->placeMeterBar(p_arrLevelMeters[0], "meter_kmeter");
        pSkin->placeAndSkinStateLabel(p_arrOverflowMeters[0], "label_over");
        pSkin->placeAndSkinStateLabel(p_arrMaximumPeakLabels[0], "label_peak");
    }
    else if (nInputChannels == 2)
    {
        pSkin->placeMeterBar(p_arrLevelMeters[0], "meter_kmeter_left");
        pSkin->placeAndSkinStateLabel(p_arrOverflowMeters[0], "label_over_left");
        pSkin->placeAndSkinStateLabel(p_arrMaximumPeakLabels[0], "label_peak_left");

        pSkin->placeMeterBar(p_arrLevelMeters[1], "meter_kmeter_right");
        pSkin->placeAndSkinStateLabel(p_arrOverflowMeters[1], "label_over_right");
        pSkin->placeAndSkinStateLabel(p_arrMaximumPeakLabels[1], "label_peak_right");
    }
    else if (nInputChannels == 6)
    {
        pSkin->placeMeterBar(p_arrLevelMeters[0], "meter_kmeter_left");
        pSkin->placeAndSkinStateLabel(p_arrOverflowMeters[0], "label_over_left");
        pSkin->placeAndSkinStateLabel(p_arrMaximumPeakLabels[0], "label_peak_left");

        pSkin->placeMeterBar(p_arrLevelMeters[1], "meter_kmeter_right");
        pSkin->placeAndSkinStateLabel(p_arrOverflowMeters[1], "label_over_right");
        pSkin->placeAndSkinStateLabel(p_arrMaximumPeakLabels[1], "label_peak_right");

        pSkin->placeMeterBar(p_arrLevelMeters[2], "meter_kmeter_center");
        pSkin->placeAndSkinStateLabel(p_arrOverflowMeters[2], "label_over_center");
        pSkin->placeAndSkinStateLabel(p_arrMaximumPeakLabels[2], "label_peak_center");

        pSkin->placeMeterBar(p_arrLevelMeters[3], "meter_kmeter_lfe");
        pSkin->placeAndSkinStateLabel(p_arrOverflowMeters[3], "label_over_lfe");
        pSkin->placeAndSkinStateLabel(p_arrMaximumPeakLabels[3], "label_peak_lfe");

        pSkin->placeMeterBar(p_arrLevelMeters[4], "meter_kmeter_ls");
        pSkin->placeAndSkinStateLabel(p_arrOverflowMeters[4], "label_over_ls");
        pSkin->placeAndSkinStateLabel(p_arrMaximumPeakLabels[4], "label_peak_ls");

        pSkin->placeMeterBar(p_arrLevelMeters[5], "meter_kmeter_rs");
        pSkin->placeAndSkinStateLabel(p_arrOverflowMeters[5], "label_over_rs");
        pSkin->placeAndSkinStateLabel(p_arrMaximumPeakLabels[5], "label_peak_rs");
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
    for (int nChannel = 0; nChannel < nInputChannels; ++nChannel)
    {
        if (displayPeakMeter)
        {
            p_arrLevelMeters[nChannel]->setLevels(pMeterBallistics->getAverageMeterLevel(nChannel), pMeterBallistics->getPeakMeterLevel(nChannel), pMeterBallistics->getAverageMeterPeakLevel(nChannel), pMeterBallistics->getPeakMeterPeakLevel(nChannel));
        }
        else
        {
            p_arrLevelMeters[nChannel]->setNormalLevels(pMeterBallistics->getAverageMeterLevel(nChannel), pMeterBallistics->getAverageMeterPeakLevel(nChannel));
        }

        p_arrMaximumPeakLabels[nChannel]->updateLevel(pMeterBallistics->getMaximumPeakLevel(nChannel));

        p_arrOverflowMeters[nChannel]->setOverflows(pMeterBallistics->getNumberOfOverflows(nChannel));
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
