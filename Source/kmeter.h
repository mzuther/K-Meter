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

#ifndef __KMETER_H__
#define __KMETER_H__

#include "../JuceLibraryCode/JuceHeader.h"
#include "meter_bar.h"
#include "overflow_meter.h"
#include "peak_label.h"
#include "plugin_processor.h"


//==============================================================================
/**
*/
class Kmeter : public Component
{
public:
    static const int KMETER_STEREO_WIDTH = 106;
    static const int KMETER_STEREO_WIDTH_2 = KMETER_STEREO_WIDTH / 2;

    Kmeter(const String& componentName, int PosX, int PosY, int nCrestFactor, int nNumChannels, const String& unitName, bool bIsSurround, bool bExpanded, bool bHorizontalMeter, bool bDisplayPeakMeter, int nSegmentHeight);
    ~Kmeter();

    void setLevels(MeterBallistics* pMeterBallistics);
    void paint(Graphics& g);
    void resized();
    void visibilityChanged();

private:
    JUCE_LEAK_DETECTOR(Kmeter);

    void paintMonoChannel(Graphics& g);
    void paintStereoChannel(Graphics& g, int nStereoChannel);

    int nWidth;
    int nHeight;
    int nPosX;
    int nPosY;

    int nMainSegmentHeight;
    int nMeterPositionTop;
    bool isExpanded;
    bool displayPeakMeter;
    bool isSurround;
    String strUnit;

    int nMeterCrestFactor;
    int nInputChannels;
    int nStereoInputChannels;

    MeterBar** LevelMeters;
    OverflowMeter** OverflowMeters;
    PeakLabel** MaximumPeakLabels;

    void drawMarkersMono(Graphics& g, String& strMarker, int x, int y, int width, int height);
    void drawMarkersStereo(Graphics& g, String& strMarker, int x, int y, int width, int height);
};


#endif  // __KMETER_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
