/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2024 Martin Zuther (http://www.mzuther.de/)

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

#ifndef KMETER_KMETER_H
#define KMETER_KMETER_H

#include "FrutHeader.h"
#include "meter_ballistics.h"
#include "meter_bar.h"
#include "overflow_meter.h"
#include "peak_label.h"
#include "skin.h"


class Kmeter :
   public Component
{
public:
   virtual void create( int numberOfInputChannels );

   virtual void applySkin( Skin* skin,
                           int crestFactor,
                           bool discreteMeter,
                           bool isExpanded,
                           bool isHorizontal,
                           bool displayPeakMeter );

   virtual void setLevels( std::shared_ptr<MeterBallistics> meterBallistics );

   virtual void resized();

protected:
   OwnedArray<MeterBar> levelMeters_;
   OwnedArray<OverflowMeter> overflowMeters_;
   OwnedArray<PeakLabel> maximumPeakLabels_;
   OwnedArray<PeakLabel> maximumTruePeakLabels_;

   int numberOfInputChannels_;
   bool displayPeakMeter_;

private:
   JUCE_LEAK_DETECTOR( Kmeter );
};

#endif  // KMETER_KMETER_H
