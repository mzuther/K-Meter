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

#ifndef __SKIN_H__
#define __SKIN_H__

class Skin;

#include "JuceHeader.h"
#include "plugin_parameters.h"
#include "common/skin/generic_skin.h"


class Skin : public GenericSkin
{
public:
    Skin();

    void loadSkin(File &fileSkin, int nNumChannels, int nCrestFactor, int nAverageAlgorithm, bool bExpanded, bool bDisplayPeakMeter);
    void updateSkin(int nNumChannels, int nCrestFactor, int nAverageAlgorithm, bool bExpanded, bool bDisplayPeakMeter);

protected:
    int nNumberOfChannels;

private:
    JUCE_LEAK_DETECTOR(Skin);
};

#endif   // __SKIN_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
