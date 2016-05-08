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

#include "plugin_parameters.h"


// The methods of this class may be called on the audio thread, so
// they are absolutely time-critical!

KmeterPluginParameters::KmeterPluginParameters() :
    frut::parameter::Juggler("KMETER_SETTINGS", numberOfParametersComplete,
                             numberOfParametersRevealed)
{
    // parameters created here will be deleted in
    // "frut::parameter::Juggler"!

    frut::parameter::ParSwitch *ParameterCrestFactor =
        new frut::parameter::ParSwitch();
    ParameterCrestFactor->setName("Metering mode");

    ParameterCrestFactor->addPreset(0.0f,  "Normal");
    ParameterCrestFactor->addPreset(12.0f, "K-12");
    ParameterCrestFactor->addPreset(14.0f, "K-14");
    ParameterCrestFactor->addPreset(20.0f, "K-20");

    ParameterCrestFactor->setDefaultRealFloat(20.0f, true);
    add(ParameterCrestFactor, selCrestFactor);


    frut::parameter::ParSwitch *ParameterAverageAlgorithm =
        new frut::parameter::ParSwitch();
    ParameterAverageAlgorithm->setName("Averaging algorithm");

    ParameterAverageAlgorithm->addPreset(selAlgorithmRms, "RMS");
    ParameterAverageAlgorithm->addPreset(selAlgorithmItuBs1770,  "ITU-R BS.1770-1");

    ParameterAverageAlgorithm->setDefaultRealFloat(selAlgorithmItuBs1770, true);
    add(ParameterAverageAlgorithm, selAverageAlgorithm);


    frut::parameter::ParBoolean *ParameterExpanded =
        new frut::parameter::ParBoolean("On", "Off");
    ParameterExpanded->setName("Expand meter");
    ParameterExpanded->setDefaultBoolean(false, true);
    add(ParameterExpanded, selExpanded);


    frut::parameter::ParBoolean *ParameterShowPeaks =
        new frut::parameter::ParBoolean("On", "Off");
    ParameterShowPeaks->setName("Show peaks");
    ParameterShowPeaks->setDefaultBoolean(false, true);
    add(ParameterShowPeaks, selShowPeaks);


    frut::parameter::ParBoolean *ParameterInfinitePeakHold =
        new frut::parameter::ParBoolean("On", "Off");
    ParameterInfinitePeakHold->setName("Peak hold");
    ParameterInfinitePeakHold->setDefaultBoolean(false, true);
    add(ParameterInfinitePeakHold, selInfinitePeakHold);


    frut::parameter::ParBoolean *ParameterMono =
        new frut::parameter::ParBoolean("On", "Off");
    ParameterMono->setName("Mono input");
    ParameterMono->setDefaultBoolean(false, true);
    add(ParameterMono, selMono);


    frut::parameter::ParBoolean *ParameterDiscreteMeter =
        new frut::parameter::ParBoolean("On", "Off");
    ParameterDiscreteMeter->setName("Discrete meter");
    ParameterDiscreteMeter->setDefaultBoolean(false, true);
    add(ParameterDiscreteMeter, selDiscreteMeter);


    frut::parameter::ParString *ParameterValidationFileName =
        new frut::parameter::ParString(String::empty);
    ParameterValidationFileName->setName("Validation file");
    add(ParameterValidationFileName, selValidationFileName);


    frut::parameter::ParSwitch *ParameterValidationSelectedChannel =
        new frut::parameter::ParSwitch();
    ParameterValidationSelectedChannel->setName("Validation audio channel");

    // values correspond to the channel index in "AudioBuffer"
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


    frut::parameter::ParBoolean *ParameterValidationAverageMeterLevel =
        new frut::parameter::ParBoolean("On", "Off");
    ParameterValidationAverageMeterLevel->setName("Validate average meter level");
    ParameterValidationAverageMeterLevel->setDefaultBoolean(true, true);
    add(ParameterValidationAverageMeterLevel, selValidationAverageMeterLevel);


    frut::parameter::ParBoolean *ParameterValidationPeakMeterLevel =
        new frut::parameter::ParBoolean("On", "Off");
    ParameterValidationPeakMeterLevel->setName("Validate peak meter level");
    ParameterValidationPeakMeterLevel->setDefaultBoolean(true, true);
    add(ParameterValidationPeakMeterLevel, selValidationPeakMeterLevel);


    frut::parameter::ParBoolean *ParameterValidationMaximumPeakLevel =
        new frut::parameter::ParBoolean("On", "Off");
    ParameterValidationMaximumPeakLevel->setName("Validate maximum peak level");
    ParameterValidationMaximumPeakLevel->setDefaultBoolean(false, true);
    add(ParameterValidationMaximumPeakLevel, selValidationMaximumPeakLevel);


    frut::parameter::ParBoolean *ParameterValidationTruePeakMeterLevel =
        new frut::parameter::ParBoolean("On", "Off");
    ParameterValidationTruePeakMeterLevel->setName("Validate true peak meter level");
    ParameterValidationTruePeakMeterLevel->setDefaultBoolean(false, true);
    add(ParameterValidationTruePeakMeterLevel, selValidationTruePeakMeterLevel);


    frut::parameter::ParBoolean *ParameterValidationMaximumTruePeakLevel =
        new frut::parameter::ParBoolean("On", "Off");
    ParameterValidationMaximumTruePeakLevel->setName("Validate maximum true peak level");
    ParameterValidationMaximumTruePeakLevel->setDefaultBoolean(false, true);
    add(ParameterValidationMaximumTruePeakLevel, selValidationMaximumTruePeakLevel);


    frut::parameter::ParBoolean *ParameterValidationStereoMeterValue =
        new frut::parameter::ParBoolean("On", "Off");
    ParameterValidationStereoMeterValue->setName("Validate stereo meter value");
    ParameterValidationStereoMeterValue->setDefaultBoolean(false, true);
    add(ParameterValidationStereoMeterValue, selValidationStereoMeterValue);


    frut::parameter::ParBoolean *ParameterValidationPhaseCorrelation =
        new frut::parameter::ParBoolean("On", "Off");
    ParameterValidationPhaseCorrelation->setName("Validate phase correlation");
    ParameterValidationPhaseCorrelation->setDefaultBoolean(false, true);
    add(ParameterValidationPhaseCorrelation, selValidationPhaseCorrelation);


    frut::parameter::ParBoolean *ParameterValidationCSVFormat =
        new frut::parameter::ParBoolean("CSV", "Full");
    ParameterValidationCSVFormat->setName("Validation output format");
    ParameterValidationCSVFormat->setDefaultBoolean(false, true);
    add(ParameterValidationCSVFormat, selValidationCSVFormat);


    // locate directory containing the skins
    File skinDirectory = getSkinDirectory();

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

    frut::parameter::ParString *ParameterSkinName =
        new frut::parameter::ParString(defaultSkinName);
    ParameterSkinName->setName("Skin");
    add(ParameterSkinName, selSkinName);
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


void KmeterPluginParameters::setValidationFile(const File &validationFile)
{
    if (validationFile.existsAsFile())
    {
        String validationFileName = validationFile.getFullPathName();
        setText(selValidationFileName, validationFileName);
    }
}


const File KmeterPluginParameters::getSkinDirectory()
{
    // On all platforms we want the skins folder to be located with
    // the executable.  On Mac, the executable is *not* the same as
    // the application folder because what looks like an application
    // is really a package (i.e. a folder) with the executable being
    // buried inside.
    //
    // When deploying on a Mac, right-click on the build target and
    // select "Show Package Contents".  Navigate through
    // Contents/MacOS and you will find the K-Meter executable.  Put
    // the kmeter folder here.
    //
    // Thanks to Tod Gentille!

    File applicationDirectory;

#ifdef __APPLE__
    applicationDirectory = File::getSpecialLocation(
                               File::currentExecutableFile).getParentDirectory();
#else
    applicationDirectory = File::getSpecialLocation(
                               File::currentApplicationFile).getParentDirectory();
#endif

    return applicationDirectory.getChildFile("./kmeter/skins/");
}


String KmeterPluginParameters::getSkinName()
{
    return getText(selSkinName);
}


void KmeterPluginParameters::setSkinName(const String &strSkinName)
{
    setText(selSkinName, strSkinName);
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
