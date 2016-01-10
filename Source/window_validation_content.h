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

#ifndef __WINDOW_VALIDATION_CONTENT_H__
#define __WINDOW_VALIDATION_CONTENT_H__

#include "JuceHeader.h"
#include "plugin_processor.h"
#include "common/widgets/generic_window_validation_content.h"


/// Customized dialog window for validation settings.
///
class WindowValidationContent : public GenericWindowValidationContent
{
public:
    WindowValidationContent(KmeterAudioProcessor *processor);

    static DialogWindow *createDialogWindow(AudioProcessorEditor *pluginEditor, KmeterAudioProcessor *audioProcessor);

    virtual void buttonClicked(Button *button);
    virtual void applySkin();

    virtual void initialise(int width, int height, int numberOfInputChannels,
                            int sampleRate, int selectedChannel,
                            const File &validationFileNew);
    virtual void selectValidationFile(const File &validationFileNew);

private:
    JUCE_LEAK_DETECTOR(WindowValidationContent);

    KmeterAudioProcessor *audioProcessor;

    ToggleButton ButtonDumpCSV;
    ToggleButton ButtonDumpAverageLevel;
    ToggleButton ButtonDumpPeakLevel;
    ToggleButton ButtonDumpMaximumPeakLevel;
    ToggleButton ButtonDumpStereoMeter;
    ToggleButton ButtonDumpPhaseCorrelation;
};


#endif  // __WINDOW_VALIDATION_CONTENT_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
