/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2011 Martin Zuther (http://www.mzuther.de/)

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

#ifndef __WINDOW_VALIDATION_H__
#define __WINDOW_VALIDATION_H__

#include "juce_library_code/juce_header.h"
#include "channel_slider.h"
#include "plugin_processor.h"
#include "prohibiting_bounds_constrainer.h"


class WindowValidation : public ResizableWindow, ButtonListener
{
public:
    WindowValidation(int nWidth, int nHeight, KmeterAudioProcessor* processor);
    ~WindowValidation();

    void paint(Graphics& g);
    void buttonClicked(Button* button);

private:
    // JUCE_LEAK_DETECTOR(WindowValidation);

    KmeterAudioProcessor* pProcessor;
    File fileValidation;

    Component* contentComponent;
    ProhibitingBoundsConstrainer* pConstrainer;

    Label* LabelFileSelection;
    Label* LabelSampleRate;
    Label* LabelSampleRateValue;

    TextButton* ButtonFileSelection;
    TextButton* ButtonValidation;
    TextButton* ButtonCancel;

    Label* LabelDumpSelectedChannel;
    ChannelSlider* SliderDumpSelectedChannel;
    ToggleButton* ButtonDumpAverageMeterLevel;
    ToggleButton* ButtonDumpPeakMeterLevel;
    ToggleButton* ButtonDumpMaximumPeakLevel;
    ToggleButton* ButtonDumpStereoMeterValue;
    ToggleButton* ButtonDumpPhaseCorrelation;
};


#endif  // __WINDOW_VALIDATION_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
