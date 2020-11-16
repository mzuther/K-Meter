/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2020 Martin Zuther (http://www.mzuther.de/)

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

void Kmeter::create( int numberOfInputChannels )

{
   // this component blends in with the background
   setOpaque( false );

   // clear old meters and peak labels
   levelMeters_.clear();
   overflowMeters_.clear();
   maximumPeakLabels_.clear();
   maximumTruePeakLabels_.clear();

   numberOfInputChannels_ = numberOfInputChannels;
   displayPeakMeter_ = false;
}


// Creates meters and applies skin.
void Kmeter::applySkin( Skin* skin,
                        int crestFactor,
                        bool discreteMeter,
                        bool isExpanded,
                        bool isHorizontal,
                        bool displayPeakMeter )

{
   displayPeakMeter_ = displayPeakMeter;

   frut::widgets::Orientation orientation (
      frut::widgets::Orientation::bottomToTop );

   if ( isHorizontal ) {
      orientation = frut::widgets::Orientation (
                       frut::widgets::Orientation::leftToRight );
   } else {
      orientation = frut::widgets::Orientation(
                       frut::widgets::Orientation::bottomToTop );
   }

   XmlElement* xmlSetting = skin->getSetting( "kmeter_segment" );
   int segmentHeight = skin->getInteger( xmlSetting,
                                         "height",
                                         5 );

   xmlSetting = skin->getSetting( "kmeter_colour_red" );
   Colour segmentRed = skin->getColour( xmlSetting,
                                        Colour( 0.00f, 1.0f, 1.0f, 1.0f ) );

   xmlSetting = skin->getSetting( "kmeter_colour_amber" );
   Colour segmentAmber = skin->getColour( xmlSetting,
                                          Colour( 0.18f, 1.0f, 1.0f, 1.0f ) );

   xmlSetting = skin->getSetting( "kmeter_colour_green" );
   Colour segmentGreen = skin->getColour( xmlSetting,
                                          Colour( 0.30f, 1.0f, 1.0f, 1.0f ) );

   xmlSetting = skin->getSetting( "kmeter_colour_nonlinear" );
   Colour segmentNonLinear = skin->getColour( xmlSetting,
                                              Colour( 0.30f, 1.0f, 1.0f, 1.0f ) );

   Array<Colour> segmentColours;

   segmentColours.add( segmentRed );
   segmentColours.add( segmentAmber );
   segmentColours.add( segmentGreen );
   segmentColours.add( segmentNonLinear );

   levelMeters_.clear();
   overflowMeters_.clear();
   maximumPeakLabels_.clear();
   maximumTruePeakLabels_.clear();

   for ( int channel = 0; channel < numberOfInputChannels_; ++channel ) {
      MeterBar* meterBar = levelMeters_.add( new MeterBar() );

      meterBar->create( crestFactor,
                        discreteMeter,
                        isExpanded,
                        orientation,
                        segmentHeight,
                        segmentColours );

      meterBar->setEnabled( isEnabled() );

      addAndMakeVisible( meterBar );

      OverflowMeter* overflowMeter = overflowMeters_.add(
                                        new OverflowMeter() );
      addAndMakeVisible( overflowMeter );

      PeakLabel* peakLabel = maximumPeakLabels_.add(
                                new PeakLabel( crestFactor ) );

      addAndMakeVisible( peakLabel );

      PeakLabel* truePeakLabel = maximumTruePeakLabels_.add(
                                    new PeakLabel( crestFactor ) );

      addAndMakeVisible( truePeakLabel );
   }

   if ( numberOfInputChannels_ == 1 ) {
      skin->placeMeterBar( "meter_kmeter",
                           levelMeters_[0] );
      skin->placeAndSkinStateLabel( "label_over",
                                    overflowMeters_[0] );
      skin->placeAndSkinStateLabel( "label_peak",
                                    maximumPeakLabels_[0] );
      skin->placeAndSkinStateLabel( "label_true_peak",
                                    maximumTruePeakLabels_[0] );
   } else if ( numberOfInputChannels_ == 2 ) {
      skin->placeMeterBar( "meter_kmeter_left",
                           levelMeters_[0] );
      skin->placeAndSkinStateLabel( "label_over_left",
                                    overflowMeters_[0] );
      skin->placeAndSkinStateLabel( "label_peak_left",
                                    maximumPeakLabels_[0] );
      skin->placeAndSkinStateLabel( "label_true_peak_left",
                                    maximumTruePeakLabels_[0] );

      skin->placeMeterBar( "meter_kmeter_right",
                           levelMeters_[1] );
      skin->placeAndSkinStateLabel( "label_over_right",
                                    overflowMeters_[1] );
      skin->placeAndSkinStateLabel( "label_peak_right",
                                    maximumPeakLabels_[1] );
      skin->placeAndSkinStateLabel( "label_true_peak_right",
                                    maximumTruePeakLabels_[1] );
   } else if ( numberOfInputChannels_ == 6 ) {
      skin->placeMeterBar( "meter_kmeter_left",
                           levelMeters_[0] );
      skin->placeAndSkinStateLabel( "label_over_left",
                                    overflowMeters_[0] );
      skin->placeAndSkinStateLabel( "label_peak_left",
                                    maximumPeakLabels_[0] );
      skin->placeAndSkinStateLabel( "label_true_peak_left",
                                    maximumTruePeakLabels_[0] );

      skin->placeMeterBar( "meter_kmeter_right",
                           levelMeters_[1] );
      skin->placeAndSkinStateLabel( "label_over_right",
                                    overflowMeters_[1] );
      skin->placeAndSkinStateLabel( "label_peak_right",
                                    maximumPeakLabels_[1] );
      skin->placeAndSkinStateLabel( "label_true_peak_right",
                                    maximumTruePeakLabels_[1] );

      skin->placeMeterBar( "meter_kmeter_center",
                           levelMeters_[2] );
      skin->placeAndSkinStateLabel( "label_over_center",
                                    overflowMeters_[2] );
      skin->placeAndSkinStateLabel( "label_peak_center",
                                    maximumPeakLabels_[2] );
      skin->placeAndSkinStateLabel( "label_true_peak_center",
                                    maximumTruePeakLabels_[2] );

      skin->placeMeterBar( "meter_kmeter_lfe",
                           levelMeters_[3] );
      skin->placeAndSkinStateLabel( "label_over_lfe",
                                    overflowMeters_[3] );
      skin->placeAndSkinStateLabel( "label_peak_lfe",
                                    maximumPeakLabels_[3] );
      skin->placeAndSkinStateLabel( "label_true_peak_lfe",
                                    maximumTruePeakLabels_[3] );

      skin->placeMeterBar( "meter_kmeter_ls",
                           levelMeters_[4] );
      skin->placeAndSkinStateLabel( "label_over_ls",
                                    overflowMeters_[4] );
      skin->placeAndSkinStateLabel( "label_peak_ls",
                                    maximumPeakLabels_[4] );
      skin->placeAndSkinStateLabel( "label_true_peak_ls",
                                    maximumTruePeakLabels_[4] );

      skin->placeMeterBar( "meter_kmeter_rs",
                           levelMeters_[5] );
      skin->placeAndSkinStateLabel( "label_over_rs",
                                    overflowMeters_[5] );
      skin->placeAndSkinStateLabel( "label_peak_rs",
                                    maximumPeakLabels_[5] );
      skin->placeAndSkinStateLabel( "label_true_peak_rs",
                                    maximumTruePeakLabels_[5] );
   } else {
      DBG( "[K-Meter] channel configuration (" +
           String( numberOfInputChannels_ ) +
           " channels) not supported" );
   }

   Component* parent = getParentComponent();

   if ( parent != nullptr ) {
      setBounds( 0, 0, parent->getWidth(), parent->getHeight() );
   }
}


void Kmeter::resized()
{
}


void Kmeter::setLevels( std::shared_ptr<MeterBallistics> meterBallistics )
{
   for ( int channel = 0; channel < numberOfInputChannels_; ++channel ) {
      if ( displayPeakMeter_ ) {
         levelMeters_[channel]->setLevels(
            meterBallistics->getAverageMeterLevel( channel ),
            meterBallistics->getAverageMeterPeakLevel( channel ),
            meterBallistics->getPeakMeterLevel( channel ),
            meterBallistics->getPeakMeterPeakLevel( channel ) );
      } else {
         levelMeters_[channel]->setNormalLevels(
            meterBallistics->getAverageMeterLevel( channel ),
            meterBallistics->getAverageMeterPeakLevel( channel ) );
      }

      maximumPeakLabels_[channel]->updateLevel(
         meterBallistics->getMaximumPeakLevel( channel ) );

      maximumTruePeakLabels_[channel]->updateLevel(
         meterBallistics->getMaximumTruePeakLevel( channel ) );

      overflowMeters_[channel]->setOverflows(
         meterBallistics->getNumberOfOverflows( channel ) );
   }
}
