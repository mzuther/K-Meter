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


class Skin
{
public:
    Skin(int number_of_channels, int crest_factor, int average_algorithm);
    ~Skin();

    bool loadFromXml(String strFileName);
    void updateSkin(int number_of_channels, int crest_factor, int average_algorithm);
    void placeComponent(Component* component, String strXmlTag);

private:
    JUCE_LEAK_DETECTOR(Skin);

    XmlElement* getComponentFromXml(String strXmlTag);

    XmlElement* xml;
    XmlElement* xmlSkinGroup;
    XmlElement* xmlSkinFallback_1;
    XmlElement* xmlSkinFallback_2;

    String strSkinGroup;
    String strSkinFallback_1;

    int nNumberOfChannels;
    int nStereoInputChannels;
    int nCrestFactor;
    int nAverageAlgorithm;

    int nWidth;
    int nHeight;
};

#endif   // __SKIN__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
