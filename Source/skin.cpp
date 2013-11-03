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


Skin::Skin(int number_of_channels, int crest_factor, int average_algorithm, bool horizontal_layout)
{
    updateSkin(number_of_channels, crest_factor, average_algorithm, horizontal_layout);
}


Skin::~Skin()
{
}


void Skin::updateSkin(int number_of_channels, int crest_factor, int average_algorithm, bool horizontal_layout)
{
    jassert(number_of_channels > 0);

    nNumberOfChannels = number_of_channels;
    nStereoInputChannels = (nNumberOfChannels + 1) / 2;
    nCrestFactor = crest_factor;
    nAverageAlgorithm = average_algorithm;
    bHorizontalLayout = horizontal_layout;

    if (bHorizontalLayout)
    {
        if (nNumberOfChannels <= 2)
        {
            nWidth = 680;
            nButtonColumnTop = nStereoInputChannels * Kmeter::KMETER_STEREO_WIDTH + 24;
        }
        else
        {
            nWidth = 662;

            if (nAverageAlgorithm == KmeterPluginParameters::selAlgorithmItuBs1770)
            {
                nButtonColumnTop = Kmeter::KMETER_STEREO_WIDTH + 24;
            }
            else
            {
                nButtonColumnTop = nStereoInputChannels * (Kmeter::KMETER_STEREO_WIDTH + 6) + 18;
            }
        }

        nButtonColumnLeft = 10;
        nHeight = nButtonColumnTop + 56;
    }
    else
    {
        if (nNumberOfChannels <= 2)
        {
            nHeight = 648;
            nButtonColumnLeft = nStereoInputChannels * Kmeter::KMETER_STEREO_WIDTH + 24;
        }
        else
        {
            nHeight = 630;

            if (nAverageAlgorithm == KmeterPluginParameters::selAlgorithmItuBs1770)
            {
                nButtonColumnLeft = Kmeter::KMETER_STEREO_WIDTH + 24;
            }
            else
            {
                nButtonColumnLeft = nStereoInputChannels * (Kmeter::KMETER_STEREO_WIDTH + 6) + 18;
            }
        }

        nButtonColumnTop = 10;
        nWidth = nButtonColumnLeft + 70;
    }
}


void Skin::placeButton(int nButtonID, Component* pButton)
{
    jassert(pButton != NULL);

    if (bHorizontalLayout)
    {
        switch (nButtonID)
        {
        case ButtonK20:
            setBoundsButtonColumn(pButton, 0, 0, 60, 20);
            break;

        case ButtonK14:
            setBoundsButtonColumn(pButton, 66, 0, 60, 20);
            break;

        case ButtonK12:
            setBoundsButtonColumn(pButton, 132, 0, 60, 20);
            break;

        case ButtonNormal:
            setBoundsButtonColumn(pButton, 198, 0, 60, 20);
            break;

        case ButtonItuBs1770:
            setBoundsButtonColumn(pButton, 0, 25, 60, 20);
            break;

        case ButtonRms:
            setBoundsButtonColumn(pButton, 66, 25, 60, 20);
            break;

        case ButtonInfiniteHold:
            setBoundsButtonColumn(pButton, 300, 0, 60, 20);
            break;

        case ButtonDisplayPeakMeter:
            setBoundsButtonColumn(pButton, 366, 0, 60, 20);
            break;

        case ButtonExpanded:
            setBoundsButtonColumn(pButton, 432, 0, 60, 20);
            break;

        case ButtonMono:
            setBoundsButtonColumn(pButton, 300, 25, 60, 20);
            break;

        case ButtonReset:
            setBoundsButtonColumn(pButton, 366, 25, 60, 20);
            break;

        case ButtonHorizontal:
            setBoundsButtonColumn(pButton, 432, 25, 60, 20);
            break;

        case ButtonValidation:
            setBoundsButtonColumn(pButton, nWidth - 80, 0, 60, 20);
            break;

        case ButtonAbout:
            setBoundsButtonColumn(pButton, nWidth - 80, 25, 60, 20);
            break;

        case LabelDebug:
            setBoundsButtonColumn(pButton, 198, 25, 60, 16);
            break;
        }
    }
    else
    {
        switch (nButtonID)
        {
        case ButtonK20:
            setBoundsButtonColumn(pButton, 0, 0, 60, 20);
            break;

        case ButtonK14:
            setBoundsButtonColumn(pButton, 0, 25, 60, 20);
            break;

        case ButtonK12:
            setBoundsButtonColumn(pButton, 0, 50, 60, 20);
            break;

        case ButtonNormal:
            setBoundsButtonColumn(pButton, 0, 75, 60, 20);
            break;

        case ButtonItuBs1770:
            setBoundsButtonColumn(pButton, 0, 115, 60, 20);
            break;

        case ButtonRms:
            setBoundsButtonColumn(pButton, 0, 140, 60, 20);
            break;

        case ButtonInfiniteHold:
            setBoundsButtonColumn(pButton, 0, 180, 60, 20);
            break;

        case ButtonDisplayPeakMeter:
            setBoundsButtonColumn(pButton, 0, 205, 60, 20);
            break;

        case ButtonExpanded:
            setBoundsButtonColumn(pButton, 0, 230, 60, 20);
            break;

        case ButtonHorizontal:
            setBoundsButtonColumn(pButton, 0, 255, 60, 20);
            break;

        case ButtonMono:
            setBoundsButtonColumn(pButton, 0, 295, 60, 20);
            break;

        case ButtonReset:
            setBoundsButtonColumn(pButton, 0, 320, 60, 20);
            break;

        case ButtonValidation:
            setBoundsButtonColumn(pButton, 0, nHeight - 66, 60, 20);
            break;

        case ButtonAbout:
            setBoundsButtonColumn(pButton, 0, nHeight - 41, 60, 20);
            break;

        case LabelDebug:
            setBoundsButtonColumn(pButton, 0, nHeight - 102, 60, 16);
            break;
        }
    }
}


void Skin::setBoundsButtonColumn(Component* component, int x, int y, int width, int height)
{
    component->setBounds(nButtonColumnLeft + x, nButtonColumnTop + y, width, height);
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
