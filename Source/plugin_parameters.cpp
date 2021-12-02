/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2021 Martin Zuther (http://www.mzuther.de/)

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
   frut::parameters::Juggler( "KMETER_SETTINGS",
                              numberOfParametersComplete,
                              numberOfParametersRevealed )
{
   // parameters created here will be deleted in
   // "frut::parameters::Juggler"!

   auto ParameterCrestFactor = new frut::parameters::ParSwitch();
   ParameterCrestFactor->init();
   ParameterCrestFactor->setName( "Metering mode" );

   ParameterCrestFactor->addPreset( 0.0f,  "Normal" );
   ParameterCrestFactor->addPreset( 12.0f, "K-12" );
   ParameterCrestFactor->addPreset( 14.0f, "K-14" );
   ParameterCrestFactor->addPreset( 20.0f, "K-20" );

   ParameterCrestFactor->setDefaultRealFloat( 20.0f, true );
   add( ParameterCrestFactor, selCrestFactor );


   auto ParameterAverageAlgorithm = new frut::parameters::ParSwitch();
   ParameterAverageAlgorithm->init();
   ParameterAverageAlgorithm->setName( "Averaging algorithm" );

   ParameterAverageAlgorithm->addPreset( selAlgorithmRms, "RMS" );
   ParameterAverageAlgorithm->addPreset( selAlgorithmItuBs1770,  "ITU-R BS.1770-1" );

   ParameterAverageAlgorithm->setDefaultRealFloat( selAlgorithmItuBs1770, true );
   add( ParameterAverageAlgorithm, selAverageAlgorithm );


   auto ParameterExpanded = new frut::parameters::ParBoolean();
   ParameterExpanded->init( "On", "Off" );
   ParameterExpanded->setName( "Expand meter" );
   ParameterExpanded->setDefaultBoolean( false, true );
   add( ParameterExpanded, selExpanded );


   auto ParameterShowPeaks = new frut::parameters::ParBoolean();
   ParameterShowPeaks->init( "On", "Off" );
   ParameterShowPeaks->setName( "Show peaks" );
   ParameterShowPeaks->setDefaultBoolean( false, true );
   add( ParameterShowPeaks, selShowPeaks );


   auto ParameterInfinitePeakHold = new frut::parameters::ParBoolean();
   ParameterInfinitePeakHold->init( "On", "Off" );
   ParameterInfinitePeakHold->setName( "Peak hold" );
   ParameterInfinitePeakHold->setDefaultBoolean( false, true );
   add( ParameterInfinitePeakHold, selInfinitePeakHold );


   auto ParameterDiscreteMeter = new frut::parameters::ParBoolean();
   ParameterDiscreteMeter->init( "On", "Off" );
   ParameterDiscreteMeter->setName( "Discrete meter" );
   ParameterDiscreteMeter->setDefaultBoolean( false, true );
   add( ParameterDiscreteMeter, selDiscreteMeter );


   auto ParameterMono = new frut::parameters::ParBoolean();
   ParameterMono->init( "On", "Off" );
   ParameterMono->setName( "Mono input" );
   ParameterMono->setDefaultBoolean( false, true );
   add( ParameterMono, selMono );


   auto ParameterDim = new frut::parameters::ParBoolean();
   ParameterDim->init( "On", "Off" );
   ParameterDim->setName( "Dim output" );
   ParameterDim->setDefaultBoolean( false, true );
   add( ParameterDim, selDim );


   auto ParameterMute = new frut::parameters::ParBoolean();
   ParameterMute->init( "On", "Off" );
   ParameterMute->setName( "Mute output" );
   ParameterMute->setDefaultBoolean( false, true );
   add( ParameterMute, selMute );


   auto ParameterFlip = new frut::parameters::ParBoolean();
   ParameterFlip->init( "On", "Off" );
   ParameterFlip->setName( "Flip channels" );
   ParameterFlip->setDefaultBoolean( false, true );
   add( ParameterFlip, selFlip );


   auto ParameterValidationFileName = new frut::parameters::ParString();
   ParameterValidationFileName->init( "" );
   ParameterValidationFileName->setName( "Validation file" );
   add( ParameterValidationFileName, selValidationFileName );


   auto ParameterValidationSelectedChannel = new frut::parameters::ParSwitch();
   ParameterValidationSelectedChannel->init();
   ParameterValidationSelectedChannel->setName( "Validation audio channel" );

   // values correspond to the channel index in "AudioBuffer"
   ParameterValidationSelectedChannel->addPreset( -1.0f, "All" );
   ParameterValidationSelectedChannel->addPreset( 0.0f,   "1" );
   ParameterValidationSelectedChannel->addPreset( 1.0f,   "2" );
#ifndef KMETER_STEREO
   ParameterValidationSelectedChannel->addPreset( 2.0f,   "3" );
   ParameterValidationSelectedChannel->addPreset( 3.0f,   "4" );
   ParameterValidationSelectedChannel->addPreset( 4.0f,   "5" );
   ParameterValidationSelectedChannel->addPreset( 5.0f,   "6" );
#endif // KMETER_STEREO

   ParameterValidationSelectedChannel->setDefaultRealFloat( -1.0f, true );
   add( ParameterValidationSelectedChannel, selValidationSelectedChannel );


   auto ParameterValidationAverageMeterLevel = new frut::parameters::ParBoolean();
   ParameterValidationAverageMeterLevel->init( "On", "Off" );
   ParameterValidationAverageMeterLevel->setName( "Validate average meter level" );
   ParameterValidationAverageMeterLevel->setDefaultBoolean( true, true );
   add( ParameterValidationAverageMeterLevel, selValidationAverageMeterLevel );


   auto ParameterValidationPeakMeterLevel = new frut::parameters::ParBoolean();
   ParameterValidationPeakMeterLevel->init( "On", "Off" );
   ParameterValidationPeakMeterLevel->setName( "Validate peak meter level" );
   ParameterValidationPeakMeterLevel->setDefaultBoolean( true, true );
   add( ParameterValidationPeakMeterLevel, selValidationPeakMeterLevel );


   auto ParameterValidationMaximumPeakLevel = new frut::parameters::ParBoolean();
   ParameterValidationMaximumPeakLevel->init( "On", "Off" );
   ParameterValidationMaximumPeakLevel->setName( "Validate maximum peak level" );
   ParameterValidationMaximumPeakLevel->setDefaultBoolean( false, true );
   add( ParameterValidationMaximumPeakLevel, selValidationMaximumPeakLevel );


   auto ParameterValidationTruePeakMeterLevel = new frut::parameters::ParBoolean();
   ParameterValidationTruePeakMeterLevel->init( "On", "Off" );
   ParameterValidationTruePeakMeterLevel->setName( "Validate true peak meter level" );
   ParameterValidationTruePeakMeterLevel->setDefaultBoolean( false, true );
   add( ParameterValidationTruePeakMeterLevel, selValidationTruePeakMeterLevel );


   auto ParameterValidationMaximumTruePeakLevel = new frut::parameters::ParBoolean();
   ParameterValidationMaximumTruePeakLevel->init( "On", "Off" );
   ParameterValidationMaximumTruePeakLevel->setName( "Validate maximum true peak level" );
   ParameterValidationMaximumTruePeakLevel->setDefaultBoolean( false, true );
   add( ParameterValidationMaximumTruePeakLevel, selValidationMaximumTruePeakLevel );


   auto ParameterValidationStereoMeterValue = new frut::parameters::ParBoolean();
   ParameterValidationStereoMeterValue->init( "On", "Off" );
   ParameterValidationStereoMeterValue->setName( "Validate stereo meter value" );
   ParameterValidationStereoMeterValue->setDefaultBoolean( false, true );
   add( ParameterValidationStereoMeterValue, selValidationStereoMeterValue );


   auto ParameterValidationPhaseCorrelation = new frut::parameters::ParBoolean();
   ParameterValidationPhaseCorrelation->init( "On", "Off" );
   ParameterValidationPhaseCorrelation->setName( "Validate phase correlation" );
   ParameterValidationPhaseCorrelation->setDefaultBoolean( false, true );
   add( ParameterValidationPhaseCorrelation, selValidationPhaseCorrelation );


   auto ParameterValidationCSVFormat = new frut::parameters::ParBoolean();
   ParameterValidationCSVFormat->init( "CSV", "Full" );
   ParameterValidationCSVFormat->setName( "Validation output format" );
   ParameterValidationCSVFormat->setDefaultBoolean( false, true );
   add( ParameterValidationCSVFormat, selValidationCSVFormat );
}


File KmeterPluginParameters::getValidationFile()
{
   File validationFile = File( getText( selValidationFileName ) );

   if ( validationFile.existsAsFile() ) {
      return validationFile;
   } else {
      return File();
   }
}


void KmeterPluginParameters::setValidationFile( const File& validationFile )
{
   if ( validationFile.existsAsFile() ) {
      String validationFileName = validationFile.getFullPathName();
      setText( selValidationFileName, validationFileName );
   }
}


const File KmeterPluginParameters::getResourceDirectory()
{
   // On all platforms we want the resource folder to be located with
   // the executable.  On Mac, the executable is *not* the same as
   // the application folder because what looks like an application
   // is really a package (i.e. a folder) with the executable being
   // buried inside.
   //
   // When deploying on a Mac, right-click on the build target and
   // select "Show Package Contents".  Navigate through
   // Contents/MacOS and you will find the K-Meter executable.  Put
   // the "kmeter" folder here.
   //
   // Thanks to Tod Gentille!

   File applicationDirectory;

#ifdef __APPLE__
   applicationDirectory = File::getSpecialLocation(
                             File::currentExecutableFile ).getParentDirectory();
#else // __APPLE__
   applicationDirectory = File::getSpecialLocation(
                             File::currentApplicationFile ).getParentDirectory();
#endif // __APPLE__

#if JucePlugin_Build_VST3
   return applicationDirectory.getChildFile( "../Resources/" );
#else // JucePlugin_Build_VST3
   return applicationDirectory;
#endif // JucePlugin_Build_VST3
}
