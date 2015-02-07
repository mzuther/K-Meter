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

#include "plugin_parameters.h"


// The methods of this class may be called on the audio thread, so
// they are absolutely time-critical!

KmeterPluginParameters::KmeterPluginParameters()
{
    strSettingsID = "KMETER_SETTINGS";


    WrappedParameterSwitch *ParameterCrestFactor = new WrappedParameterSwitch();
    ParameterCrestFactor->setName("Metering mode");

    ParameterCrestFactor->addConstant(0.0f,  "Normal");
    ParameterCrestFactor->addConstant(12.0f, "K-12");
    ParameterCrestFactor->addConstant(14.0f, "K-14");
    ParameterCrestFactor->addConstant(20.0f, "K-20");

    ParameterCrestFactor->setDefaultRealFloat(20.0f, true);
    add(ParameterCrestFactor, selCrestFactor);


    WrappedParameterSwitch *ParameterAverageAlgorithm = new WrappedParameterSwitch();
    ParameterAverageAlgorithm->setName("Averaging algorithm");

    ParameterAverageAlgorithm->addConstant(selAlgorithmRms, "RMS");
    ParameterAverageAlgorithm->addConstant(selAlgorithmItuBs1770,  "ITU-R BS.1770-1");

    ParameterAverageAlgorithm->setDefaultRealFloat(selAlgorithmItuBs1770, true);
    add(ParameterAverageAlgorithm, selAverageAlgorithm);


    WrappedParameterToggleSwitch *ParameterExpanded = new WrappedParameterToggleSwitch("Off", "On");
    ParameterExpanded->setName("Expand meter");
    ParameterExpanded->setDefaultBoolean(false, true);
    add(ParameterExpanded, selExpanded);


    WrappedParameterToggleSwitch *ParameterShowPeaks = new WrappedParameterToggleSwitch("Off", "On");
    ParameterShowPeaks->setName("Show peaks");
    ParameterShowPeaks->setDefaultBoolean(false, true);
    add(ParameterShowPeaks, selShowPeaks);


    WrappedParameterToggleSwitch *ParameterInfinitePeakHold = new WrappedParameterToggleSwitch("Off", "On");
    ParameterInfinitePeakHold->setName("Peak hold");
    ParameterInfinitePeakHold->setDefaultBoolean(false, true);
    add(ParameterInfinitePeakHold, selInfinitePeakHold);


    WrappedParameterToggleSwitch *ParameterMono = new WrappedParameterToggleSwitch("Off", "On");
    ParameterMono->setName("Mono input");
    ParameterMono->setDefaultBoolean(false, true);
    add(ParameterMono, selMono);


    WrappedParameterString *ParameterValidationFileName = new WrappedParameterString(String::empty);
    ParameterValidationFileName->setName("Validation: file name");
    add(ParameterValidationFileName, selValidationFileName);


    WrappedParameterSwitch *ParameterValidationSelectedChannel = new WrappedParameterSwitch();
    ParameterValidationSelectedChannel->setName("Validation: selected channel");

    // values correspond to the channel index in AudioSampleBuffer
    ParameterValidationSelectedChannel->addConstant(-1.0f, "All");
    ParameterValidationSelectedChannel->addConstant(0.0f,   "1");
    ParameterValidationSelectedChannel->addConstant(1.0f,   "2");
#ifdef KMETER_SURROUND
    ParameterValidationSelectedChannel->addConstant(2.0f,   "3");
    ParameterValidationSelectedChannel->addConstant(3.0f,   "4");
    ParameterValidationSelectedChannel->addConstant(4.0f,   "5");
    ParameterValidationSelectedChannel->addConstant(5.0f,   "6");
#endif

    ParameterValidationSelectedChannel->setDefaultRealFloat(-1.0f, true);
    add(ParameterValidationSelectedChannel, selValidationSelectedChannel);


    WrappedParameterToggleSwitch *ParameterValidationAverageMeterLevel = new WrappedParameterToggleSwitch("Off", "On");
    ParameterValidationAverageMeterLevel->setName("Validation: average meter level");
    ParameterValidationAverageMeterLevel->setDefaultBoolean(true, true);
    add(ParameterValidationAverageMeterLevel, selValidationAverageMeterLevel);


    WrappedParameterToggleSwitch *ParameterValidationPeakMeterLevel = new WrappedParameterToggleSwitch("Off", "On");
    ParameterValidationPeakMeterLevel->setName("Validation: peak meter level");
    ParameterValidationPeakMeterLevel->setDefaultBoolean(true, true);
    add(ParameterValidationPeakMeterLevel, selValidationPeakMeterLevel);


    WrappedParameterToggleSwitch *ParameterValidationMaximumPeakLevel = new WrappedParameterToggleSwitch("Off", "On");
    ParameterValidationMaximumPeakLevel->setName("Validation: maximum peak level");
    ParameterValidationMaximumPeakLevel->setDefaultBoolean(true, true);
    add(ParameterValidationMaximumPeakLevel, selValidationMaximumPeakLevel);


    WrappedParameterToggleSwitch *ParameterValidationStereoMeterValue = new WrappedParameterToggleSwitch("Off", "On");
    ParameterValidationStereoMeterValue->setName("Validation: stereo meter value");
    ParameterValidationStereoMeterValue->setDefaultBoolean(true, true);
    add(ParameterValidationStereoMeterValue, selValidationStereoMeterValue);


    WrappedParameterToggleSwitch *ParameterValidationPhaseCorrelation = new WrappedParameterToggleSwitch("Off", "On");
    ParameterValidationPhaseCorrelation->setName("Validation: phase correlation");
    ParameterValidationPhaseCorrelation->setDefaultBoolean(true, true);
    add(ParameterValidationPhaseCorrelation, selValidationPhaseCorrelation);


    WrappedParameterToggleSwitch *ParameterValidationCSVFormat = new WrappedParameterToggleSwitch("Off", "On");
    ParameterValidationCSVFormat->setName("Validation: CSV output format");
    ParameterValidationCSVFormat->setDefaultBoolean(false, true);
    add(ParameterValidationCSVFormat, selValidationCSVFormat);


    // the following may or may not work on Mac
    File fileApplicationDirectory = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory();
    File fileSkinDirectory = fileApplicationDirectory.getChildFile("./kmeter-skins/");

    // file defining the default skin's name
    File fileDefaultSkin = fileSkinDirectory.getChildFile("default_skin.ini");

    // create file if necessary
    if (!fileDefaultSkin.existsAsFile())
    {
        fileDefaultSkin.create();

        // set "Default" as default skin
        fileDefaultSkin.replaceWithText("Default", true, true);
    }

    // load name of default skin
    String strDefaultSkinName = fileDefaultSkin.loadFileAsString();

    WrappedParameterString *ParameterSkinName = new WrappedParameterString(strDefaultSkinName);
    ParameterSkinName->setName("Skin");
    add(ParameterSkinName, selSkinName);
}


KmeterPluginParameters::~KmeterPluginParameters()
{
    // parameters will be deleted in "ParameterJuggler"
}


int KmeterPluginParameters::getNumParameters(bool bIncludeHiddenParameters)
{
    if (bIncludeHiddenParameters)
    {
        return nNumParametersComplete;
    }
    else
    {
        return nNumParametersRevealed;
    }
}


File KmeterPluginParameters::getValidationFile()
{
    File fileValidation = File(getText(selValidationFileName));

    if (fileValidation.existsAsFile())
    {
        return fileValidation;
    }
    else
    {
        return File::nonexistent;
    }
}


void KmeterPluginParameters::setValidationFile(File &fileValidation)
{
    if (fileValidation.existsAsFile())
    {
        String strFilename = fileValidation.getFullPathName();
        setText(selValidationFileName, strFilename);
    }
}


String KmeterPluginParameters::getSkinName()
{
    return getText(selSkinName);
}


void KmeterPluginParameters::setSkinName(String &strSkinName)
{
    setText(selSkinName, strSkinName);
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
