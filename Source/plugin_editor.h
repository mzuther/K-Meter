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

#ifndef __PLUGINEDITOR_KMETER_H__
#define __PLUGINEDITOR_KMETER_H__

#include "../JuceLibraryCode/JuceHeader.h"
#include "plugin_processor.h"
#include "kmeter.h"
#include "stereo_meter.h"
#include "phase_correlation_meter.h"
#include "window_about.h"
#include "window_validation.h"


//==============================================================================
/**
*/
class KmeterAudioProcessorEditor : public AudioProcessorEditor, public ButtonListener, public ActionListener
{
public:
    KmeterAudioProcessorEditor(KmeterAudioProcessor* ownerFilter, int nNumChannels);
    ~KmeterAudioProcessorEditor();

    void buttonClicked(Button* button);
    void actionListenerCallback(const String& message);
    void changeParameter(int nIndex);
    void changeParameter(int nIndex, int nValue);

    //==============================================================================
    // This is just a standard Juce paint method...
    void paint(Graphics& g);
    void resized();

private:
    JUCE_LEAK_DETECTOR(KmeterAudioProcessorEditor);

    void setBoundsButtonColumn(Component* component, int x, int y, int width, int height);

    void reloadMeters();
    void resizeEditor();
    void updateAverageAlgorithm(bool reload_meters);

    bool bReloadMeters;
    bool bIsValidating;

    bool bRotateMeters;
    float fTinyScale;

    int nCrestFactor;
    int nInputChannels;
    int nStereoInputChannels;
    int nButtonColumnLeft;
    int nButtonColumnTop;
    int nHeight;
    int nWidth;

    KmeterAudioProcessor* pProcessor;
    Kmeter* kmeter;
    StereoMeter* stereoMeter;
    PhaseCorrelationMeter* phaseCorrelationMeter;

    TextButton* ButtonNormal;
    TextButton* ButtonK12;
    TextButton* ButtonK14;
    TextButton* ButtonK20;

    TextButton* ButtonItuBs1770;
    TextButton* ButtonRms;

    TextButton* ButtonExpanded;
    TextButton* ButtonDisplayPeakMeter;
    TextButton* ButtonInfiniteHold;
    TextButton* ButtonReset;

    TextButton* ButtonMono;
    TextButton* ButtonValidation;
    TextButton* ButtonAbout;

    Label* LabelDebug;
};


#endif  // __PLUGINEDITOR_KMETER_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
