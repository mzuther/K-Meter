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

#include "peak_label.h"


PeakLabel::PeakLabel( int nCrestFactor )
{
   nMeterCrestFactor = nCrestFactor;
   setLabelColour( Colours::white );

   resetLevel();
}


void PeakLabel::resetLevel()
{
   float fMaximumCrestFactor = 20.0f; // i.e. K-20
   float fMeterMinimumDecibel = -( fMaximumCrestFactor + 90.0f );

   // reset level
   fMaximumLevel = fMeterMinimumDecibel;

   // ensure peak label update on next call of updateLevel()
   fMaximumLevel -= 0.1f;
}


void PeakLabel::updateLevel( float newLevel )
{
   if ( newLevel == fMaximumLevel ) {
      return;
   }

   fMaximumLevel = newLevel;
   float fCorrectedLevel = fMaximumLevel + nMeterCrestFactor;

   if ( fCorrectedLevel < 0.0f ) {
      setLabelText( String( fCorrectedLevel, 1 ) );
   } else {
      setLabelText( "+" + String( fCorrectedLevel, 1 ) );
   }

   if ( fMaximumLevel < -0.20f ) {
      setState( false );
   } else {
      setState( true );
   }
}
