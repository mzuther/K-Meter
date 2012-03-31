/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2012 Martin Zuther (http://www.mzuther.de/)

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

    This is the header file that your files should include in order to get all
    the Juce library headers. You should NOT include juce.h or
    juce_amalgamated.h directly in your own source files, because that wouldn't
    pick up the correct Juce configuration options for your app.

---------------------------------------------------------------------------- */

#ifndef __APPHEADERFILE_D32E6EED__
#define __APPHEADERFILE_D32E6EED__

#include "app_config.h"
#include "JucePluginCharacteristics.h"
#include "juce/juce_amalgamated.h"

namespace ProjectInfo
{
    const char* const  projectName    = JucePlugin_Name;
    const char* const  versionString  = JucePlugin_VersionString;
    const int          versionNumber  = JucePlugin_VersionCode;
}

#endif   // __APPHEADERFILE_D32E6EED__
