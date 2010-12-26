/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010 Martin Zuther (http://www.mzuther.de/)

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

#ifndef __PLUGINEDITOR_H_E5792227__
#define __PLUGINEDITOR_H_E5792227__

#include "juce_library_code/juce_header.h"
#include "juce_library_code/JucePluginCharacteristics.h"
#include "plugin_processor.h"
#include "kmeter.h"
#include "stereo_meter.h"
#include "correlation_meter.h"
#include "about_window.h"


//==============================================================================
/**
*/
class KmeterAudioProcessorEditor : public AudioProcessorEditor, public ButtonListener, public ChangeListener
{
public:
    KmeterAudioProcessorEditor(KmeterAudioProcessor* ownerFilter);
    ~KmeterAudioProcessorEditor();

    void buttonClicked(Button* button);
    void changeListenerCallback(void* objectThatHasChanged);
    void changeParameter(int nIndex);
    void changeParameter(int nIndex, int nValue);

    //==============================================================================
    // This is just a standard Juce paint method...
    void paint(Graphics& g);
    void resized();

private:
    int nHeadroom;

    KmeterAudioProcessor* pProcessor;
    Kmeter* kmeter;
    StereoMeter* stereoMeter;
    CorrelationMeter* correlationMeter;

    TextButton* ButtonNormal;
    TextButton* ButtonK12;
    TextButton* ButtonK14;
    TextButton* ButtonK20;

    TextButton* ButtonExpanded;
    TextButton* ButtonDisplayPeakMeter;
    TextButton* ButtonHold;
    TextButton* ButtonReset;

    TextButton* ButtonMono;
    TextButton* ButtonAbout;
};


#endif  // __PLUGINEDITOR_H_E5792227__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
