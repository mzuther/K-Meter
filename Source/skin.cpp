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

#include "skin.h"


void Skin::loadSkin(File &skinFile,
                    int numberOfChannels,
                    int crestFactor,
                    int averageAlgorithm,
                    bool isExpanded,
                    bool displayPeakMeter)

{
    updateSkin(numberOfChannels,
               crestFactor,
               averageAlgorithm,
               isExpanded,
               displayPeakMeter);

    loadFromXml(skinFile, "kmeter-skin");
}


void Skin::updateSkin(int numberOfChannels,
                      int crestFactor,
                      int averageAlgorithm,
                      bool isExpanded,
                      bool displayPeakMeter)

{
    jassert(numberOfChannels > 0);

    if (isExpanded)
    {
        currentBackgroundName_ = "image_expanded";
    }
    else
    {
        currentBackgroundName_ = "image";
    }

    if (displayPeakMeter)
    {
        currentBackgroundName_ += "_peaks";
    }
    else
    {
        currentBackgroundName_ += "_no_peaks";
    }

    if (numberOfChannels <= 2)
    {
        currentFallbackName_ = "stereo";
    }
    else
    {
        currentFallbackName_ = "surround";
    }

    if (averageAlgorithm == KmeterPluginParameters::selAlgorithmItuBs1770)
    {
        currentFallbackName_ += "_itu";
    }
    else
    {
        currentFallbackName_ += "_rms";
    }

    switch (crestFactor)
    {
    case 20:
        currentGroupName_ = currentFallbackName_ + "_k20";
        break;

    case 14:
        currentGroupName_ = currentFallbackName_ + "_k14";
        break;

    case 12:
        currentGroupName_ = currentFallbackName_ + "_k12";
        break;

    default:
        currentGroupName_ = currentFallbackName_ + "_normal";
        break;
    }

    if (document_ != nullptr)
    {
        skinGroup_ = document_->getChildByName(currentGroupName_);
        skinFallback_1_ = document_->getChildByName(currentFallbackName_);
        skinFallback_2_ = document_->getChildByName("default");
    }
    else
    {
        skinGroup_ = nullptr;
        skinFallback_1_ = nullptr;
        skinFallback_2_ = nullptr;
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
