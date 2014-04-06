/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2013 Martin Zuther (http://www.mzuther.de/)

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

#ifndef __SKIN__
#define __SKIN__

class Skin;

#include "../JuceLibraryCode/JuceHeader.h"
#include "kmeter.h"
#include "plugin_parameters.h"
#include "state_label.h"


class Skin
{
public:
    Skin(String strSkinFileName, int nNumChannels, int nCrestFactor, int nAverageAlgorithm, bool bExpanded, bool bDisplayPeakMeter);
    ~Skin();

    bool loadFromXml(String strSkinFileName);
    void updateSkin(int nNumChannels, int nCrestFactor, int nAverageAlgorithm, bool bExpanded, bool bDisplayPeakMeter);
    void placeComponent(Component* component, String strXmlTag);
    void placeAndSkinButton(ImageButton* button, String strXmlTag);
    void placeAndSkinLabel(ImageComponent* label, String strXmlTag);
    void placeAndSkinStateLabel(StateLabel* label, String strXmlTag);
    void setBackgroundImage(ImageComponent* background, AudioProcessorEditor* editor);

private:
    JUCE_LEAK_DETECTOR(Skin);

    XmlElement* getComponentFromXml(String strXmlTag);

    XmlElement* xml;
    XmlElement* xmlSkinGroup;
    XmlElement* xmlSkinFallback_1;
    XmlElement* xmlSkinFallback_2;

    File* fileResourcePath;

    String strBackgroundSelector;
    String strSkinGroup;
    String strSkinFallback_1;

    int nNumberOfChannels;
};

#endif   // __SKIN__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
