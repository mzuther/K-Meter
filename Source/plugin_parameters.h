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

#ifndef __PLUGIN_PARAMETERS_KMETER_H__
#define __PLUGIN_PARAMETERS_KMETER_H__

class KmeterPluginParameters;

#include "JuceHeader.h"
#include "parameter_juggler/parameter_juggler.h"


//============================================================================
class KmeterPluginParameters  : public ParameterJuggler
{
public:
    //==========================================================================

    KmeterPluginParameters();
    ~KmeterPluginParameters();

    int getNumParameters(bool bIncludeHiddenParameters);

    File getValidationFile();
    void setValidationFile(File &fileValidation);

    String getSkinName();
    void setSkinName(String &strSkinName);

    enum Parameters  // public namespace!
    {
        selCrestFactor = 0,
        selAverageAlgorithm,
        selExpanded,
        selShowPeaks,
        selInfinitePeakHold,
        selMono,

        nNumParametersRevealed,

        selValidationFileName = nNumParametersRevealed,
        selValidationSelectedChannel,
        selValidationAverageMeterLevel,
        selValidationPeakMeterLevel,
        selValidationMaximumPeakLevel,
        selValidationStereoMeterValue,
        selValidationPhaseCorrelation,
        selValidationCSVFormat,
        selSkinName,

        nNumParametersComplete,

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
    JUCE_LEAK_DETECTOR(KmeterPluginParameters);

    WrappedParameterSwitch        *ParameterCrestFactor;
    WrappedParameterSwitch        *ParameterAverageAlgorithm;
    WrappedParameterToggleSwitch  *ParameterExpanded;
    WrappedParameterToggleSwitch  *ParameterShowPeaks;
    WrappedParameterToggleSwitch  *ParameterInfinitePeakHold;
    WrappedParameterToggleSwitch  *ParameterMono;

    WrappedParameterString        *ParameterValidationFileName;
    WrappedParameterSwitch        *ParameterValidationSelectedChannel;
    WrappedParameterToggleSwitch  *ParameterValidationAverageMeterLevel;
    WrappedParameterToggleSwitch  *ParameterValidationPeakMeterLevel;
    WrappedParameterToggleSwitch  *ParameterValidationMaximumPeakLevel;
    WrappedParameterToggleSwitch  *ParameterValidationStereoMeterValue;
    WrappedParameterToggleSwitch  *ParameterValidationPhaseCorrelation;
    WrappedParameterToggleSwitch  *ParameterValidationCSVFormat;

    WrappedParameterString        *ParameterSkinName;
};

#endif  // __PLUGIN_PARAMETERS_KMETER_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
