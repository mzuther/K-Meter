/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2021 Martin Zuther (http://www.mzuther.de/)

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

#include "build_id.h"

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
   KmeterAudioProcessorEditor( KmeterAudioProcessor& processor, int nNumChannels );
   ~KmeterAudioProcessorEditor();

   void setNumberOfChannels( int NumberOfChannels );

   void buttonClicked( Button* button ) override;
   void actionListenerCallback( const String& message ) override;
   void updateParameter( int nIndex );

   void windowAboutCallback( int modalResult );
   void windowSkinCallback( int modalResult );
   void windowValidationCallback( int modalResult );

   // This is just a standard Juce paint method...
   void paint( Graphics& g ) override;
   void resized() override;

private:
   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( KmeterAudioProcessorEditor );

   void reloadMeters();
   void applySkin();
   void loadSkin();
   void updateAverageAlgorithm( bool reload_meters );

   bool needsMeterReload;
   bool isValidating;
   bool validationDialogOpen;
   bool isInitialising;
   bool isExpanded;
   bool usePeakMeter;
   bool isStereo;

   int crestFactor;
   int numberOfInputChannels_;

   Skin skin;

   KmeterAudioProcessor& audioProcessor;
   Kmeter kmeter_;
   frut::widgets::NeedleMeter stereoMeter;
   frut::widgets::NeedleMeter phaseCorrelationMeter;

   frut::skin::LookAndFeel_Frut_V3 customLookAndFeel_;

   DrawableButton ButtonK20;
   DrawableButton ButtonK14;
   DrawableButton ButtonK12;
   DrawableButton ButtonNormal;

   DrawableButton ButtonItuBs1770;
   DrawableButton ButtonRms;

   DrawableButton ButtonExpanded;
   DrawableButton ButtonSkin;
   DrawableButton ButtonDisplayPeakMeter;
   DrawableButton ButtonInfinitePeakHold;
   DrawableButton ButtonDiscreteMeter;

   DrawableButton ButtonMono;
   DrawableButton ButtonDim;
   DrawableButton ButtonMute;
   DrawableButton ButtonFlip;
   DrawableButton ButtonReset;

   DrawableButton ButtonValidation;
   DrawableButton ButtonAbout;

#ifdef DEBUG
   DrawableComposite LabelDebug;
#endif // DEBUG

   DrawableComposite DrawableBackground;
};

#endif  // KMETER_PLUGIN_EDITOR_H
