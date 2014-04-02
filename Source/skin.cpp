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

#include "skin.h"


Skin::Skin(int number_of_channels, int crest_factor, int average_algorithm)
{
    xml = nullptr;

    updateSkin(number_of_channels, crest_factor, average_algorithm);
    loadFromXml("./kmeter-skins/default.xml");
}


Skin::~Skin()
{
    if (xml != nullptr)
    {
        delete xml;
        xml = nullptr;
    }
}


bool Skin::loadFromXml(String strFileName)
{
    // may not work on Mac OS
    File fileApplicationDirectory = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory();
    File fileSkin = fileApplicationDirectory.getChildFile(strFileName);

    if (xml != nullptr)
    {
        delete xml;
        xml = nullptr;
    }

    xml = XmlDocument::parse(fileSkin);

    if (xml == nullptr)
    {
        DBG(String("[K-Meter] skin file \"") + fileSkin.getFullPathName() + "\" not found");
        return false;
    }
    else if ((!xml->hasTagName("kmeter-skin")) || (xml->getChildByName("default") == nullptr))
    {
        DBG("[K-Meter] skin file not valid");
        return false;
    }
    else
    {
        xmlSkinGroup = xml->getChildByName(strSkinGroup);
        xmlSkinFallback_1 = xml->getChildByName(strSkinFallback_1);
        xmlSkinFallback_2 = xml->getChildByName("default");

        return true;
    }
}


void Skin::updateSkin(int number_of_channels, int crest_factor, int average_algorithm)
{
    jassert(number_of_channels > 0);

    nNumberOfChannels = number_of_channels;
    nStereoInputChannels = (nNumberOfChannels + 1) / 2;
    nCrestFactor = crest_factor;
    nAverageAlgorithm = average_algorithm;

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
}


XmlElement* Skin::getComponentFromXml(String strXmlTag)
{
    XmlElement* xmlComponent;

    if ((xmlSkinGroup != nullptr) && (xmlSkinGroup->getChildByName(strXmlTag) != nullptr))
    {
        xmlComponent = xmlSkinGroup->getChildByName(strXmlTag);
    }
    else if ((xmlSkinFallback_1 != nullptr) && (xmlSkinFallback_1->getChildByName(strXmlTag) != nullptr))
    {
        xmlComponent = xmlSkinFallback_1->getChildByName(strXmlTag);
    }
    else
    {
        xmlComponent = xmlSkinFallback_2->getChildByName(strXmlTag);
    }

    if (xmlComponent == nullptr)
    {
        DBG(String("[K-Meter] skin element \"") + strXmlTag + "\" not found");
        jassert(false);
    }

    return xmlComponent;
}


void Skin::placeComponent(Component* component, String strXmlTag)
{
    jassert(component != nullptr);

    XmlElement* xmlComponent = getComponentFromXml(strXmlTag);

    if (xmlComponent != nullptr)
    {
        int x = xmlComponent->getIntAttribute("x", -1);
        int y = xmlComponent->getIntAttribute("y", -1);
        int width = xmlComponent->getIntAttribute("width", -1);
        int height = xmlComponent->getIntAttribute("height", -1);

        component->setBounds(x, y, width, height);
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
