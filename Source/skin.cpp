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

#include "skin.h"


Skin::Skin(File &fileSkin, int nNumChannels, int nCrestFactor, int nAverageAlgorithm, bool bExpanded, bool bDisplayPeakMeter)
    : GenericSkin(fileSkin, "kmeter-skin")
{
    updateSkin(nNumChannels, nCrestFactor, nAverageAlgorithm, bExpanded, bDisplayPeakMeter);
    loadFromXml(fileSkin);
}


Skin::~Skin()
{
}


void Skin::updateSkin(int nNumChannels, int nCrestFactor, int nAverageAlgorithm, bool bExpanded, bool bDisplayPeakMeter)
{
    jassert(nNumChannels > 0);
    nNumberOfChannels = nNumChannels;

    if (bExpanded)
    {
        strBackgroundSelector = "image_expanded";
    }
    else
    {
        strBackgroundSelector = "image";
    }

    if (bDisplayPeakMeter)
    {
        strBackgroundSelector += "_peaks";
    }
    else
    {
        strBackgroundSelector += "_no_peaks";
    }

    if (nNumberOfChannels <= 2)
    {
        strSkinFallback_1 = "stereo";
    }
    else
    {
        strSkinFallback_1 = "surround";
    }

    if (nAverageAlgorithm == KmeterPluginParameters::selAlgorithmItuBs1770)
    {
        strSkinFallback_1 += "_itu";
    }
    else
    {
        strSkinFallback_1 += "_rms";
    }

    switch (nCrestFactor)
    {
    case 20:
        strSkinGroup = strSkinFallback_1 + "_k20";
        break;

    case 14:
        strSkinGroup = strSkinFallback_1 + "_k14";
        break;

    case 12:
        strSkinGroup = strSkinFallback_1 + "_k12";
        break;

    default:
        strSkinGroup = strSkinFallback_1 + "_normal";
        break;
    }

    if (xml != nullptr)
    {
        xmlSkinGroup = xml->getChildByName(strSkinGroup);
        xmlSkinFallback_1 = xml->getChildByName(strSkinFallback_1);
        xmlSkinFallback_2 = xml->getChildByName("default");
    }
    else
    {
        xmlSkinGroup = nullptr;
        xmlSkinFallback_1 = nullptr;
        xmlSkinFallback_2 = nullptr;
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
