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
    enum Parameters  // public namespace!
    {
        ButtonNormal = 0,
        ButtonK12,
        ButtonK14,
        ButtonK20,

        ButtonItuBs1770,
        ButtonRms,

        ButtonExpanded,
        ButtonHorizontal,
        ButtonDisplayPeakMeter,
        ButtonInfiniteHold,
        ButtonReset,

        ButtonMono,
        ButtonValidation,
        ButtonAbout,

        LabelDebug,
    };

    Skin(int number_of_channels, int crest_factor, int average_algorithm, bool horizontal_layout);
    ~Skin();

    void updateSkin(int number_of_channels, int crest_factor, int average_algorithm, bool horizontal_layout);
    void placeButton(int nButtonID, Component* pButton);

private:
    JUCE_LEAK_DETECTOR(Skin);

    void setBoundsButtonColumn(Component* component, int x, int y, int width, int height);

    int nNumberOfChannels;
    int nStereoInputChannels;
    int nCrestFactor;
    int nAverageAlgorithm;
    bool bHorizontalLayout;

    int nWidth;
    int nHeight;
    int nButtonColumnLeft;
    int nButtonColumnTop;
};

#endif   // __SKIN__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
