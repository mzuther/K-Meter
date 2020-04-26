/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2020 Martin Zuther (http://www.mzuther.de/)

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

#ifndef KMETER_PLUGIN_EDITOR_H
#define KMETER_PLUGIN_EDITOR_H

#include "FrutHeader.h"
#include "plugin_processor.h"
#include "kmeter.h"
#include "skin.h"
#include "window_validation_content.h"


class KmeterAudioProcessorEditor :
    public AudioProcessorEditor,
    public Button::Listener,
    public ActionListener
{
public:
    KmeterAudioProcessorEditor(KmeterAudioProcessor *ownerFilter, int nNumChannels);
    ~KmeterAudioProcessorEditor();

    void buttonClicked(Button *button) override;
    void actionListenerCallback(const String &message) override;
    void updateParameter(int nIndex);

    void windowAboutCallback(int modalResult);
    void windowSkinCallback(int modalResult);
    void windowValidationCallback(int modalResult);

    // This is just a standard Juce paint method...
    void paint(Graphics &g) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KmeterAudioProcessorEditor);

    void reloadMeters();
    void applySkin();
    void loadSkin();
    void updateAverageAlgorithm(bool reload_meters);

    bool needsMeterReload;
    bool isValidating;
    bool validationDialogOpen;
    bool isInitialising;
    bool isExpanded;
    bool usePeakMeter;

    int crestFactor;
    int numberOfInputChannels_;

    File skinDirectory;
    Skin skin;
    String currentSkinName;

    KmeterAudioProcessor *audioProcessor;
    Kmeter kmeter_;
    frut::widgets::NeedleMeter stereoMeter;
    frut::widgets::NeedleMeter phaseCorrelationMeter;

    frut::skin::LookAndFeel_Frut_V3 customLookAndFeel_;

    ImageButton ButtonK20;
    ImageButton ButtonK14;
    ImageButton ButtonK12;
    ImageButton ButtonNormal;

    ImageButton ButtonItuBs1770;
    ImageButton ButtonRms;

    ImageButton ButtonExpanded;
    ImageButton ButtonSkin;
    ImageButton ButtonDisplayPeakMeter;
    ImageButton ButtonInfinitePeakHold;
    ImageButton ButtonDiscreteMeter;

    ImageButton ButtonMono;
    ImageButton ButtonDim;
    ImageButton ButtonMute;
    ImageButton ButtonFlip;
    ImageButton ButtonReset;

    ImageButton ButtonValidation;
    ImageButton ButtonAbout;

#ifdef DEBUG
    ImageComponent LabelDebug;
#endif

    ImageComponent BackgroundImage;
};

#endif  // KMETER_PLUGIN_EDITOR_H
