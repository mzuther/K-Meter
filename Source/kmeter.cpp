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

void Kmeter::create(
    int crestFactor, int numberOfInputChannels, bool discreteMeter,
    bool isExpanded, bool isHorizontal, bool displayPeakMeter,
    int segmentHeight)

{
    // this component blends in with the background
    setOpaque(false);

    // clear old meters and peak labels
    levelMeters_.clear();
    overflowMeters_.clear();
    maximumPeakLabels_.clear();

    numberOfInputChannels_ = numberOfInputChannels;
    displayPeakMeter_ = displayPeakMeter;

    MeterBar::Orientation orientation;

    // force discrete segments for expanded meters
    if (isExpanded)
    {
        discreteMeter = true;
    }

    if (isHorizontal)
    {
        orientation = MeterBar::orientationHorizontal;
    }
    else
    {
        orientation = MeterBar::orientationVertical;
    }

    for (int channel = 0; channel < numberOfInputChannels_; ++channel)
    {
        MeterBar *meterBar = levelMeters_.add(new MeterBar());

        meterBar->create(crestFactor,
                         discreteMeter,
                         isExpanded,
                         orientation,
                         segmentHeight);

        addAndMakeVisible(meterBar);

        OverflowMeter *overflowMeter = overflowMeters_.add(new OverflowMeter(
                                           "Overflows #" + String(channel)));
        addAndMakeVisible(overflowMeter);

        PeakLabel *peakLabel = maximumPeakLabels_.add(new PeakLabel(
                                   "Maximum Peak #" + String(channel),
                                   crestFactor));
        addAndMakeVisible(peakLabel);
    }
}


void Kmeter::applySkin(
    Skin *skin)

{
    if (numberOfInputChannels_ == 1)
    {
        skin->placeMeterBar(levelMeters_[0],
                            "meter_kmeter");
        skin->placeAndSkinStateLabel(overflowMeters_[0],
                                     "label_over");
        skin->placeAndSkinStateLabel(maximumPeakLabels_[0],
                                     "label_peak");
    }
    else if (numberOfInputChannels_ == 2)
    {
        skin->placeMeterBar(levelMeters_[0],
                            "meter_kmeter_left");
        skin->placeAndSkinStateLabel(overflowMeters_[0],
                                     "label_over_left");
        skin->placeAndSkinStateLabel(maximumPeakLabels_[0],
                                     "label_peak_left");

        skin->placeMeterBar(levelMeters_[1],
                            "meter_kmeter_right");
        skin->placeAndSkinStateLabel(overflowMeters_[1],
                                     "label_over_right");
        skin->placeAndSkinStateLabel(maximumPeakLabels_[1],
                                     "label_peak_right");
    }
    else if (numberOfInputChannels_ == 6)
    {
        skin->placeMeterBar(levelMeters_[0],
                            "meter_kmeter_left");
        skin->placeAndSkinStateLabel(overflowMeters_[0],
                                     "label_over_left");
        skin->placeAndSkinStateLabel(maximumPeakLabels_[0],
                                     "label_peak_left");

        skin->placeMeterBar(levelMeters_[1],
                            "meter_kmeter_right");
        skin->placeAndSkinStateLabel(overflowMeters_[1],
                                     "label_over_right");
        skin->placeAndSkinStateLabel(maximumPeakLabels_[1],
                                     "label_peak_right");

        skin->placeMeterBar(levelMeters_[2],
                            "meter_kmeter_center");
        skin->placeAndSkinStateLabel(overflowMeters_[2],
                                     "label_over_center");
        skin->placeAndSkinStateLabel(maximumPeakLabels_[2],
                                     "label_peak_center");

        skin->placeMeterBar(levelMeters_[3],
                            "meter_kmeter_lfe");
        skin->placeAndSkinStateLabel(overflowMeters_[3],
                                     "label_over_lfe");
        skin->placeAndSkinStateLabel(maximumPeakLabels_[3],
                                     "label_peak_lfe");

        skin->placeMeterBar(levelMeters_[4],
                            "meter_kmeter_ls");
        skin->placeAndSkinStateLabel(overflowMeters_[4],
                                     "label_over_ls");
        skin->placeAndSkinStateLabel(maximumPeakLabels_[4],
                                     "label_peak_ls");

        skin->placeMeterBar(levelMeters_[5],
                            "meter_kmeter_rs");
        skin->placeAndSkinStateLabel(overflowMeters_[5],
                                     "label_over_rs");
        skin->placeAndSkinStateLabel(maximumPeakLabels_[5],
                                     "label_peak_rs");
    }
    else
    {
        DBG("[K-Meter] channel configuration (" +
            String(numberOfInputChannels_) +
            " channels) not supported");
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


void Kmeter::setLevels(
    MeterBallistics *meterBallistics)

{
    for (int channel = 0; channel < numberOfInputChannels_; ++channel)
    {
        if (displayPeakMeter_)
        {
            levelMeters_[channel]->setLevels(
                meterBallistics->getAverageMeterLevel(channel),
                meterBallistics->getAverageMeterPeakLevel(channel),
                meterBallistics->getPeakMeterLevel(channel),
                meterBallistics->getPeakMeterPeakLevel(channel));
        }
        else
        {
            levelMeters_[channel]->setNormalLevels(
                meterBallistics->getAverageMeterLevel(channel),
                meterBallistics->getAverageMeterPeakLevel(channel));
        }

        maximumPeakLabels_[channel]->updateLevel(
            meterBallistics->getMaximumPeakLevel(channel));

        overflowMeters_[channel]->setOverflows(
            meterBallistics->getNumberOfOverflows(channel));
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
