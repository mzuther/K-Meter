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

#ifndef KMETER_SKIN_H
#define KMETER_SKIN_H

#include "FrutHeader.h"
#include "plugin_parameters.h"


class Skin :
   public frut::skin::Skin
{
public:
   bool loadSkin( int numberOfChannels,
                  int crestFactor,
                  int averageAlgorithm,
                  bool isExpanded,
                  bool displayPeakMeter );

   void updateSkin( int numberOfChannels,
                    int crestFactor,
                    int averageAlgorithm,
                    bool isExpanded,
                    bool displayPeakMeter );

   virtual File getSkinDirectory() override;
   virtual File getSettingsFile() override;

private:
   JUCE_LEAK_DETECTOR( Skin );
};

#endif  // KMETER_SKIN_H
