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

#ifndef KMETER_PLUGIN_PARAMETERS_H
#define KMETER_PLUGIN_PARAMETERS_H

#include "FrutHeader.h"


class KmeterPluginParameters :
   public frut::parameters::Juggler
{
public:
   KmeterPluginParameters();

   File getValidationFile();
   void setValidationFile( const File& fileValidation );

   static const File getResourceDirectory();

   enum Parameters { // public namespace!
      selCrestFactor = 0,
      selAverageAlgorithm,
      selExpanded,
      selShowPeaks,
      selInfinitePeakHold,
      selDiscreteMeter,
      selMono,
      selDim,
      selMute,
      selFlip,

      numberOfParametersRevealed,

      selValidationFileName = numberOfParametersRevealed,
      selValidationSelectedChannel,
      selValidationAverageMeterLevel,
      selValidationPeakMeterLevel,
      selValidationMaximumPeakLevel,
      selValidationTruePeakMeterLevel,
      selValidationMaximumTruePeakLevel,
      selValidationStereoMeterValue,
      selValidationPhaseCorrelation,
      selValidationCSVFormat,

      numberOfParametersComplete,

      selNormal = 0,
      selK12,
      selK14,
      selK20,

      nNumCrestFactors,

      selAlgorithmRms = 0,
      selAlgorithmItuBs1770,

      nNumAlgorithms,
   };

private:
   JUCE_LEAK_DETECTOR( KmeterPluginParameters );
};

#endif  // KMETER_PLUGIN_PARAMETERS_H
