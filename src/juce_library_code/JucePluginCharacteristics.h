/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2011 Martin Zuther (http://www.mzuther.de/)

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

--------------------------------------------------------------------------------

    This header file contains configuration options for the plug-in.

---------------------------------------------------------------------------- */

#ifndef __PLUGINCHARACTERISTICS_A4D510D9__
#define __PLUGINCHARACTERISTICS_A4D510D9__

#define JucePlugin_Build_VST    1
#define JucePlugin_Build_AU     0
#define JucePlugin_Build_RTAS   0

#ifdef KMETER_SURROUND
#ifdef DEBUG
#define JucePlugin_Name                 "K-Meter Surround (Debug)"
#else
#define JucePlugin_Name                 "K-Meter Surround"
#endif
#else
#ifdef DEBUG
#define JucePlugin_Name                 "K-Meter Stereo (Debug)"
#else
#define JucePlugin_Name                 "K-Meter Stereo"
#endif
#endif
#define JucePlugin_Desc                 "Implementation of a K-System meter according to Bob Katz' specifications"
#define JucePlugin_Manufacturer         "mzuther"
#define JucePlugin_ManufacturerCode     'mz'
#define JucePlugin_PluginCode           'kmet'
#ifdef KMETER_SURROUND
#define JucePlugin_MaxNumInputChannels  8
#define JucePlugin_MaxNumOutputChannels 8
#define JucePlugin_PreferredChannelConfigurations   {6, 6}, {8, 8}
#else
#define JucePlugin_MaxNumInputChannels  2
#define JucePlugin_MaxNumOutputChannels 2
#define JucePlugin_PreferredChannelConfigurations   {1, 1}, {2, 2}
#endif
#define JucePlugin_IsSynth              0
#define JucePlugin_WantsMidiInput       0
#define JucePlugin_ProducesMidiOutput   0
#define JucePlugin_SilenceInProducesSilenceOut  1
#define JucePlugin_TailLengthSeconds    0
#define JucePlugin_EditorRequiresKeyboardFocus  0
#define JucePlugin_VersionCode          0x11600
#define JucePlugin_VersionString        "1.22.0"
#define JucePlugin_VSTUniqueID          JucePlugin_PluginCode
#define JucePlugin_VSTCategory          kPlugCategEffect
#define JucePlugin_AUMainType           kAudioUnitType_Effect
#define JucePlugin_AUSubType            JucePlugin_PluginCode
#define JucePlugin_AUExportPrefix       JuceProjectAU
#define JucePlugin_AUExportPrefixQuoted "JuceProjectAU"
#define JucePlugin_AUManufacturerCode   JucePlugin_ManufacturerCode
#define JucePlugin_CFBundleIdentifier   de.mzuther.KMeter
#define JucePlugin_AUCocoaViewClassName JuceProjectAU_V1
#define JucePlugin_RTASCategory         ePlugInCategory_None
#define JucePlugin_RTASManufacturerCode JucePlugin_ManufacturerCode
#define JucePlugin_RTASProductId        JucePlugin_PluginCode

#endif   // __PLUGINCHARACTERISTICS_A4D510D9__
