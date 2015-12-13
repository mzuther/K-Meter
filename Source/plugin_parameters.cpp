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

KmeterPluginParameters::KmeterPluginParameters() :
    ParameterJuggler("KMETER_SETTINGS", numberOfParametersComplete,
                     numberOfParametersRevealed)
{
    PluginParameterSwitch *ParameterCrestFactor = new PluginParameterSwitch();
    ParameterCrestFactor->setName("Metering mode");

    ParameterCrestFactor->addPreset(0.0f,  "Normal");
    ParameterCrestFactor->addPreset(12.0f, "K-12");
    ParameterCrestFactor->addPreset(14.0f, "K-14");
    ParameterCrestFactor->addPreset(20.0f, "K-20");

    ParameterCrestFactor->setDefaultRealFloat(20.0f, true);
    add(ParameterCrestFactor, selCrestFactor);


    PluginParameterSwitch *ParameterAverageAlgorithm = new PluginParameterSwitch();
    ParameterAverageAlgorithm->setName("Averaging algorithm");

    ParameterAverageAlgorithm->addPreset(selAlgorithmRms, "RMS");
    ParameterAverageAlgorithm->addPreset(selAlgorithmItuBs1770,  "ITU-R BS.1770-1");

    ParameterAverageAlgorithm->setDefaultRealFloat(selAlgorithmItuBs1770, true);
    add(ParameterAverageAlgorithm, selAverageAlgorithm);


    PluginParameterBoolean *ParameterExpanded = new PluginParameterBoolean("On", "Off");
    ParameterExpanded->setName("Expand meter");
    ParameterExpanded->setDefaultBoolean(false, true);
    add(ParameterExpanded, selExpanded);


    PluginParameterBoolean *ParameterShowPeaks = new PluginParameterBoolean("On", "Off");
    ParameterShowPeaks->setName("Show peaks");
    ParameterShowPeaks->setDefaultBoolean(false, true);
    add(ParameterShowPeaks, selShowPeaks);


    PluginParameterBoolean *ParameterInfinitePeakHold = new PluginParameterBoolean("On", "Off");
    ParameterInfinitePeakHold->setName("Peak hold");
    ParameterInfinitePeakHold->setDefaultBoolean(false, true);
    add(ParameterInfinitePeakHold, selInfinitePeakHold);


    PluginParameterBoolean *ParameterMono = new PluginParameterBoolean("On", "Off");
    ParameterMono->setName("Mono input");
    ParameterMono->setDefaultBoolean(false, true);
    add(ParameterMono, selMono);


    PluginParameterString *ParameterValidationFileName = new PluginParameterString(String::empty);
    ParameterValidationFileName->setName("Validation file");
    add(ParameterValidationFileName, selValidationFileName);


    PluginParameterSwitch *ParameterValidationSelectedChannel = new PluginParameterSwitch();
    ParameterValidationSelectedChannel->setName("Validation audio channel");

    // values correspond to the channel index in AudioSampleBuffer
    ParameterValidationSelectedChannel->addPreset(-1.0f, "All");
    ParameterValidationSelectedChannel->addPreset(0.0f,   "1");
    ParameterValidationSelectedChannel->addPreset(1.0f,   "2");
#ifdef KMETER_SURROUND
    ParameterValidationSelectedChannel->addPreset(2.0f,   "3");
    ParameterValidationSelectedChannel->addPreset(3.0f,   "4");
    ParameterValidationSelectedChannel->addPreset(4.0f,   "5");
    ParameterValidationSelectedChannel->addPreset(5.0f,   "6");
#endif

    ParameterValidationSelectedChannel->setDefaultRealFloat(-1.0f, true);
    add(ParameterValidationSelectedChannel, selValidationSelectedChannel);


    PluginParameterBoolean *ParameterValidationAverageMeterLevel = new PluginParameterBoolean("On", "Off");
    ParameterValidationAverageMeterLevel->setName("Validate average meter level");
    ParameterValidationAverageMeterLevel->setDefaultBoolean(true, true);
    add(ParameterValidationAverageMeterLevel, selValidationAverageMeterLevel);


    PluginParameterBoolean *ParameterValidationPeakMeterLevel = new PluginParameterBoolean("On", "Off");
    ParameterValidationPeakMeterLevel->setName("Validate peak meter level");
    ParameterValidationPeakMeterLevel->setDefaultBoolean(true, true);
    add(ParameterValidationPeakMeterLevel, selValidationPeakMeterLevel);


    PluginParameterBoolean *ParameterValidationMaximumPeakLevel = new PluginParameterBoolean("On", "Off");
    ParameterValidationMaximumPeakLevel->setName("Validate maximum peak level");
    ParameterValidationMaximumPeakLevel->setDefaultBoolean(true, true);
    add(ParameterValidationMaximumPeakLevel, selValidationMaximumPeakLevel);


    PluginParameterBoolean *ParameterValidationStereoMeterValue = new PluginParameterBoolean("On", "Off");
    ParameterValidationStereoMeterValue->setName("Validate stereo meter value");
    ParameterValidationStereoMeterValue->setDefaultBoolean(true, true);
    add(ParameterValidationStereoMeterValue, selValidationStereoMeterValue);


    PluginParameterBoolean *ParameterValidationPhaseCorrelation = new PluginParameterBoolean("On", "Off");
    ParameterValidationPhaseCorrelation->setName("Validate phase correlation");
    ParameterValidationPhaseCorrelation->setDefaultBoolean(true, true);
    add(ParameterValidationPhaseCorrelation, selValidationPhaseCorrelation);


    PluginParameterBoolean *ParameterValidationCSVFormat = new PluginParameterBoolean("CSV", "Full");
    ParameterValidationCSVFormat->setName("Validation output format");
    ParameterValidationCSVFormat->setDefaultBoolean(false, true);
    add(ParameterValidationCSVFormat, selValidationCSVFormat);


    // the following may or may not work on Mac
    File applicationDirectory = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory();
    File skinDirectory = applicationDirectory.getChildFile("./kmeter/skins/");

    // locate file containing the default skin's name
    File defaultSkinFile = skinDirectory.getChildFile("default_skin.ini");

    // make sure the file exists
    if (!defaultSkinFile.existsAsFile())
    {
        // create file
        defaultSkinFile.create();

        // set "Default" as default skin (using Unicode encoding)
        defaultSkinFile.replaceWithText("Default", true, true);
    }

    // load name of default skin from file
    String defaultSkinName = defaultSkinFile.loadFileAsString();

    PluginParameterString *ParameterSkinName = new PluginParameterString(defaultSkinName);
    ParameterSkinName->setName("Skin");
    add(ParameterSkinName, selSkinName);
}


KmeterPluginParameters::~KmeterPluginParameters()
{
    // parameters will be deleted in "ParameterJuggler"
}


File KmeterPluginParameters::getValidationFile()
{
    File validationFile = File(getText(selValidationFileName));

    if (validationFile.existsAsFile())
    {
        return validationFile;
    }
    else
    {
        return File::nonexistent;
    }
}


void KmeterPluginParameters::setValidationFile(File &validationFile)
{
    if (validationFile.existsAsFile())
    {
        String validationFileName = validationFile.getFullPathName();
        setText(selValidationFileName, validationFileName);
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
