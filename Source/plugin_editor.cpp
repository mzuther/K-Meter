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

#include "plugin_editor.h"


static void window_about_callback( int modalResult, KmeterAudioProcessorEditor* pEditor )
{
   if ( pEditor != nullptr ) {
      pEditor->windowAboutCallback( modalResult );
   }
}


static void window_skin_callback( int modalResult, KmeterAudioProcessorEditor* pEditor )
{
   if ( pEditor != nullptr ) {
      pEditor->windowSkinCallback( modalResult );
   }
}


static void window_validation_callback( int modalResult, KmeterAudioProcessorEditor* pEditor )
{
   if ( pEditor != nullptr ) {
      pEditor->windowValidationCallback( modalResult );
   }
}


KmeterAudioProcessorEditor::KmeterAudioProcessorEditor( KmeterAudioProcessor& processor, int nNumChannels ) :
   AudioProcessorEditor( &processor ),
   audioProcessor( processor ),

   ButtonK20( "K-20", DrawableButton::ButtonStyle::ImageRaw ),
   ButtonK14( "K-14", DrawableButton::ButtonStyle::ImageRaw ),
   ButtonK12( "K-12", DrawableButton::ButtonStyle::ImageRaw ),
   ButtonNormal( "Normal", DrawableButton::ButtonStyle::ImageRaw ),

   ButtonItuBs1770( "ITU BS-1770", DrawableButton::ButtonStyle::ImageRaw ),
   ButtonRms( "RMS", DrawableButton::ButtonStyle::ImageRaw ),

   ButtonExpanded( "Expanded", DrawableButton::ButtonStyle::ImageRaw ),
   ButtonSkin( "Skin", DrawableButton::ButtonStyle::ImageRaw ),
   ButtonDisplayPeakMeter( "Display Peak Meter", DrawableButton::ButtonStyle::ImageRaw ),
   ButtonInfinitePeakHold( "Infinite Peak Hold", DrawableButton::ButtonStyle::ImageRaw ),
   ButtonDiscreteMeter( "Discrete Meter", DrawableButton::ButtonStyle::ImageRaw ),

   ButtonMono( "Mono", DrawableButton::ButtonStyle::ImageRaw ),
   ButtonDim( "Dim", DrawableButton::ButtonStyle::ImageRaw ),
   ButtonMute( "Mute", DrawableButton::ButtonStyle::ImageRaw ),
   ButtonFlip( "Flip", DrawableButton::ButtonStyle::ImageRaw ),
   ButtonReset( "Reset", DrawableButton::ButtonStyle::ImageRaw ),

   ButtonValidation( "Validation", DrawableButton::ButtonStyle::ImageRaw ),
   ButtonAbout( "About", DrawableButton::ButtonStyle::ImageRaw )
{
   // load look and feel
   setLookAndFeel( &customLookAndFeel_ );

   // the editor window does not have any transparent areas
   // (increases performance on redrawing)
   setOpaque( true );

   // prevent meter reload during initialisation
   isInitialising = true;
   setNumberOfChannels( nNumChannels );

   isValidating = false;
   validationDialogOpen = false;

   crestFactor = 0;

   isExpanded = false;
   usePeakMeter = false;

   // The plug-in editor's size as well as the location of buttons
   // and labels will be set later on in this constructor.

   audioProcessor.addActionListener( this );

   ButtonK20.setRadioGroupId( 1 );
   ButtonK20.addListener( this );
   addAndMakeVisible( ButtonK20 );

   ButtonK14.setRadioGroupId( 1 );
   ButtonK14.addListener( this );
   addAndMakeVisible( ButtonK14 );

   ButtonK12.setRadioGroupId( 1 );
   ButtonK12.addListener( this );
   addAndMakeVisible( ButtonK12 );

   ButtonNormal.setRadioGroupId( 1 );
   ButtonNormal.addListener( this );
   addAndMakeVisible( ButtonNormal );

   ButtonItuBs1770.addListener( this );
   addAndMakeVisible( ButtonItuBs1770 );

   ButtonRms.addListener( this );
   addAndMakeVisible( ButtonRms );

   updateAverageAlgorithm( false );

   ButtonExpanded.addListener( this );
   addAndMakeVisible( ButtonExpanded );

   ButtonSkin.addListener( this );
   addAndMakeVisible( ButtonSkin );

   ButtonDisplayPeakMeter.addListener( this );
   addAndMakeVisible( ButtonDisplayPeakMeter );

   ButtonInfinitePeakHold.addListener( this );
   addAndMakeVisible( ButtonInfinitePeakHold );

   ButtonDiscreteMeter.addListener( this );
   addAndMakeVisible( ButtonDiscreteMeter );

   ButtonMono.addListener( this );
   addAndMakeVisible( ButtonMono );

   ButtonDim.addListener( this );
   addAndMakeVisible( ButtonDim );

   ButtonMute.addListener( this );
   addAndMakeVisible( ButtonMute );

   ButtonFlip.addListener( this );
   addAndMakeVisible( ButtonFlip );

   ButtonReset.addListener( this );
   addAndMakeVisible( ButtonReset );

   ButtonValidation.addListener( this );
   addAndMakeVisible( ButtonValidation );

   ButtonAbout.addListener( this );
   addAndMakeVisible( ButtonAbout );

   // moves K-Meter to the back of the editor's z-plane so that it
   // doesn't overlay (and thus block) any other components
   addAndMakeVisible( kmeter_, 0 );

#ifdef DEBUG
   // moves debug label to the back of the editor's z-plane to that
   // it doesn't overlay (and thus block) any other components
   addAndMakeVisible( LabelDebug, 0 );
#endif // DEBUG

   // moves background image to the back of the editor's z-plane so
   // that it doesn't overlay (and thus block) any other components
   addAndMakeVisible( DrawableBackground, 0 );

   // visibility is controlled in loadSkin()
   addChildComponent ( stereoMeter );
   addChildComponent ( phaseCorrelationMeter );

   updateParameter( KmeterPluginParameters::selCrestFactor );
   updateParameter( KmeterPluginParameters::selAverageAlgorithm );

   updateParameter( KmeterPluginParameters::selExpanded );
   updateParameter( KmeterPluginParameters::selShowPeaks );
   updateParameter( KmeterPluginParameters::selInfinitePeakHold );
   updateParameter( KmeterPluginParameters::selDiscreteMeter );

   updateParameter( KmeterPluginParameters::selMono );
   updateParameter( KmeterPluginParameters::selDim );
   updateParameter( KmeterPluginParameters::selMute );
   updateParameter( KmeterPluginParameters::selFlip );

   // force meter reload after initialisation
   isInitialising = false;

   // apply skin to plug-in editor
   loadSkin();
}


KmeterAudioProcessorEditor::~KmeterAudioProcessorEditor()
{
   audioProcessor.removeActionListener( this );

   // release look and feel
   setLookAndFeel( nullptr );
}


void KmeterAudioProcessorEditor::setNumberOfChannels( int NumberOfChannels )
{
   numberOfInputChannels_ = NumberOfChannels;
   isStereo = ( numberOfInputChannels_ == 2 );

   if ( ! isInitialising ) {
      // apply skin to plug-in editor
      loadSkin();
   }
}


void KmeterAudioProcessorEditor::loadSkin()
{
   bool loadExternalResources = true;
   skin.loadSkin( numberOfInputChannels_,
                  crestFactor,
                  audioProcessor.getAverageAlgorithm(),
                  isExpanded,
                  usePeakMeter,
                  loadExternalResources );

   // stereo and phase correlation meters and the "Mono" button only
   // make sense for stereo processing
   stereoMeter.setVisible ( isStereo );
   phaseCorrelationMeter.setVisible ( isStereo );
   ButtonMono.setEnabled( isStereo );

   // will also apply skin to plug-in editor
   needsMeterReload = true;
   reloadMeters();
}


void KmeterAudioProcessorEditor::applySkin()
{
   // prevent skin application during meter initialisation
   if ( isInitialising ) {
      return;
   }

   // update skin
   skin.updateSkin( numberOfInputChannels_,
                    crestFactor,
                    audioProcessor.getAverageAlgorithm(),
                    isExpanded,
                    usePeakMeter );

   // update UI scale
   Logger::outputDebugString(
      String( "[Skin] scaling UI to " ) +
      String( skin.getUiScale() ) + "%" );

   float scale = skin.getUiScale() / 100.0f;
   Desktop::getInstance().setGlobalScaleFactor( scale );

   // moves background image to the back of the editor's z-plane;
   // will also resize plug-in editor
   skin.setBackground( &DrawableBackground, this );

   skin.placeAndSkinButton( "button_k20",
                            &ButtonK20 );
   skin.placeAndSkinButton( "button_k14",
                            &ButtonK14 );
   skin.placeAndSkinButton( "button_k12",
                            &ButtonK12 );
   skin.placeAndSkinButton( "button_normal",
                            &ButtonNormal );

   skin.placeAndSkinButton( "button_itu",
                            &ButtonItuBs1770 );
   skin.placeAndSkinButton( "button_rms",
                            &ButtonRms );

   skin.placeAndSkinButton( "button_expand",
                            &ButtonExpanded );
   skin.placeAndSkinButton( "button_peaks",
                            &ButtonDisplayPeakMeter );
   skin.placeAndSkinButton( "button_hold",
                            &ButtonInfinitePeakHold );
   skin.placeAndSkinButton( "button_discrete",
                            &ButtonDiscreteMeter );

   skin.placeAndSkinButton( "button_mono",
                            &ButtonMono );
   skin.placeAndSkinButton( "button_dim",
                            &ButtonDim );
   skin.placeAndSkinButton( "button_mute",
                            &ButtonMute );
   skin.placeAndSkinButton( "button_flip",
                            &ButtonFlip );

   skin.placeAndSkinButton( "button_reset",
                            &ButtonReset );
   skin.placeAndSkinButton( "button_skin",
                            &ButtonSkin );

   skin.placeAndSkinButton( "button_validate",
                            &ButtonValidation );
   skin.placeAndSkinButton( "button_about",
                            &ButtonAbout );

#ifdef DEBUG
   skin.placeAndSkinLabel( "label_debug",
                           &LabelDebug );
#endif // DEBUG

   kmeter_.applySkin(
      &skin,
      crestFactor,
      ButtonDiscreteMeter.getToggleState(),
      ButtonExpanded.getToggleState(),
      false,
      ButtonDisplayPeakMeter.getToggleState() );

   if ( numberOfInputChannels_ <= 2 ) {
      skin.placeAndSkinNeedleMeter( "meter_stereo",
                                    &stereoMeter );

      skin.placeAndSkinNeedleMeter( "meter_phase_correlation",
                                    &phaseCorrelationMeter );
   }
}


void KmeterAudioProcessorEditor::windowAboutCallback( int modalResult )
{
   ignoreUnused( modalResult );

   // manually deactivate about button
   ButtonAbout.setToggleState( false, dontSendNotification );
}


void KmeterAudioProcessorEditor::windowSkinCallback( int modalResult )
{
   // manually deactivate skin button
   ButtonSkin.setToggleState( false, dontSendNotification );

   // user has selected a UI scale
   if ( modalResult > 0 ) {
      // store new UI scale
      skin.setUiScale( modalResult );

      // apply skin to plug-in editor
      loadSkin();
   }
}


void KmeterAudioProcessorEditor::windowValidationCallback( int modalResult )
{
   ignoreUnused( modalResult );

   audioProcessor.silenceInput( false );
   validationDialogOpen = false;

   // manually set button according to validation state
   ButtonValidation.setToggleState( isValidating, dontSendNotification );
}


void KmeterAudioProcessorEditor::actionListenerCallback( const String& strMessage )
{
   // "PC" ==> parameter changed, followed by a hash and the
   // parameter's ID
   if ( strMessage.startsWith( "PC#" ) ) {
      String strIndex = strMessage.substring( 3 );
      int nIndex = strIndex.getIntValue();
      jassert( nIndex >= 0 );
      jassert( nIndex < audioProcessor.getNumParameters() );

      if ( audioProcessor.hasChanged( nIndex ) ) {
         updateParameter( nIndex );
      }

      // "UM" ==> update meters
   } else if ( ! strMessage.compare( "UM" ) ) {
      std::shared_ptr<MeterBallistics> pMeterBallistics = audioProcessor.getLevels();

      if ( pMeterBallistics != nullptr ) {
         kmeter_.setLevels( pMeterBallistics );

         if ( numberOfInputChannels_ <= 2 ) {
            float fStereo = pMeterBallistics->getStereoMeterValue();
            stereoMeter.setValue( fStereo / 2.0f + 0.5f );

            float fPhase = pMeterBallistics->getPhaseCorrelation();
            phaseCorrelationMeter.setValue( fPhase / 2.0f + 0.5f );
         }
      }

      if ( isValidating && !audioProcessor.isValidating() ) {
         isValidating = false;
      }

      // "AC" ==> algorithm changed
   } else if ( ! strMessage.compare( "AC" ) ) {
      updateAverageAlgorithm( true );
      // "V+" ==> validation started
   } else if ( ( ! strMessage.compare( "V+" ) ) && audioProcessor.isValidating() ) {
      isValidating = true;
      // "V-" ==> validation stopped
   } else if ( ! strMessage.compare( "V-" ) ) {
      if ( ! validationDialogOpen ) {
         ButtonValidation.setToggleState( false, dontSendNotification );
      }

      // do nothing till you hear from me... :)
   } else {
      DBG( "[K-Meter] received unknown action strMessage \"" + strMessage + "\"." );
   }
}


void KmeterAudioProcessorEditor::updateParameter( int nIndex )
{
   int nValue = audioProcessor.getRealInteger( nIndex );

   audioProcessor.clearChangeFlag( nIndex );

   switch ( nIndex ) {
      case KmeterPluginParameters::selCrestFactor:

         if ( nValue == 0 ) {
            crestFactor = nValue;

            // will also apply skin to plug-in editor
            needsMeterReload = true;

            ButtonNormal.setToggleState( true, dontSendNotification );
         } else if ( nValue == 12 ) {
            crestFactor = nValue;

            // will also apply skin to plug-in editor
            needsMeterReload = true;

            ButtonK12.setToggleState( true, dontSendNotification );
         } else if ( nValue == 14 ) {
            crestFactor = nValue;

            // will also apply skin to plug-in editor
            needsMeterReload = true;

            ButtonK14.setToggleState( true, dontSendNotification );
         } else { // K-20
            crestFactor = 20;

            // will also apply skin to plug-in editor
            needsMeterReload = true;

            ButtonK20.setToggleState( true, dontSendNotification );
         }

         break;

      case KmeterPluginParameters::selAverageAlgorithm:

         // the "RMS" and "ITU-R" buttons will be updated from the code
         // that actually switches the level averaging alghorithm, thus
         // making sure that the correct button is lit in any given
         // situation
         //
         // we just need to make make sure that this code is actually
         // executed...
         audioProcessor.setAverageAlgorithm( nValue );

         break;

      case KmeterPluginParameters::selExpanded:
         isExpanded = ( nValue != 0 );
         ButtonExpanded.setToggleState( isExpanded, dontSendNotification );

         // will also apply skin to plug-in editor
         needsMeterReload = true;
         break;

      case KmeterPluginParameters::selShowPeaks:
         usePeakMeter = ( nValue != 0 );
         ButtonDisplayPeakMeter.setToggleState( usePeakMeter, dontSendNotification );

         // will also apply skin to plug-in editor
         needsMeterReload = true;
         break;

      case KmeterPluginParameters::selInfinitePeakHold:
         audioProcessor.setMeterInfiniteHold( nValue != 0 );
         ButtonInfinitePeakHold.setToggleState( nValue != 0, dontSendNotification );
         break;

      case KmeterPluginParameters::selDiscreteMeter:
         ButtonDiscreteMeter.setToggleState( nValue != 0, dontSendNotification );

         // will also apply skin to plug-in editor
         needsMeterReload = true;
         break;

      case KmeterPluginParameters::selMono:
         ButtonMono.setToggleState( nValue != 0, dontSendNotification );
         break;

      case KmeterPluginParameters::selDim:
         ButtonDim.setToggleState( nValue != 0, dontSendNotification );

         // will also apply skin to plug-in editor
         needsMeterReload = true;
         break;

      case KmeterPluginParameters::selMute:
         ButtonMute.setToggleState( nValue != 0, dontSendNotification );

         // will also apply skin to plug-in editor
         needsMeterReload = true;
         break;

      case KmeterPluginParameters::selFlip:
         ButtonFlip.setToggleState( nValue != 0, dontSendNotification );
         break;
   }

   // prevent meter reload during initialisation
   if ( ! isInitialising ) {
      // will also apply skin to plug-in editor
      reloadMeters();
   }
}


void KmeterAudioProcessorEditor::reloadMeters()
{
   if ( needsMeterReload ) {
      needsMeterReload = false;

      if ( audioProcessor.getAverageAlgorithm() == KmeterPluginParameters::selAlgorithmItuBs1770 ) {
         kmeter_.create( 1 );
      } else {
         kmeter_.create( numberOfInputChannels_ );
      }

      bool isAttenuated = ButtonDim.getToggleState() ||
                          ButtonMute.getToggleState();
      kmeter_.setEnabled( ! isAttenuated );

      // moves background image to the back of the editor's z-plane
      applySkin();
   }
}


void KmeterAudioProcessorEditor::paint( Graphics& g )
{
#ifdef DEBUG
   g.fillAll( Colours::green );
#else // DEBUG
   g.fillAll( Colours::black );
#endif // DEBUG
}


void KmeterAudioProcessorEditor::buttonClicked( Button* button )
{
   if ( button == &ButtonK20 ) {
      audioProcessor.changeParameter( KmeterPluginParameters::selCrestFactor, KmeterPluginParameters::selK20 / float( KmeterPluginParameters::nNumCrestFactors - 1 ) );
   } else if ( button == &ButtonK14 ) {
      audioProcessor.changeParameter( KmeterPluginParameters::selCrestFactor, KmeterPluginParameters::selK14 / float( KmeterPluginParameters::nNumCrestFactors - 1 ) );
   } else if ( button == &ButtonK12 ) {
      audioProcessor.changeParameter( KmeterPluginParameters::selCrestFactor, KmeterPluginParameters::selK12 / float( KmeterPluginParameters::nNumCrestFactors - 1 ) );
   } else if ( button == &ButtonNormal ) {
      audioProcessor.changeParameter( KmeterPluginParameters::selCrestFactor, KmeterPluginParameters::selNormal / float( KmeterPluginParameters::nNumCrestFactors - 1 ) );
   } else if ( button == &ButtonInfinitePeakHold ) {
      audioProcessor.changeParameter( KmeterPluginParameters::selInfinitePeakHold, button->getToggleState() ? 0.0f : 1.0f );
   } else if ( button == &ButtonDiscreteMeter ) {
      audioProcessor.changeParameter( KmeterPluginParameters::selDiscreteMeter, button->getToggleState() ? 0.0f : 1.0f );
   } else if ( button == &ButtonItuBs1770 ) {
      audioProcessor.changeParameter( KmeterPluginParameters::selAverageAlgorithm, KmeterPluginParameters::selAlgorithmItuBs1770 / float( KmeterPluginParameters::nNumAlgorithms - 1 ) );
   } else if ( button == &ButtonRms ) {
      audioProcessor.changeParameter( KmeterPluginParameters::selAverageAlgorithm, KmeterPluginParameters::selAlgorithmRms / float( KmeterPluginParameters::nNumAlgorithms - 1 ) );
   } else if ( button == &ButtonExpanded ) {
      audioProcessor.changeParameter( KmeterPluginParameters::selExpanded, button->getToggleState() ? 0.0f : 1.0f );
   } else if ( button == &ButtonSkin ) {
      // manually activate button (will be deactivated in dialog
      // window callback)
      button->setToggleState( true, dontSendNotification );

      // prepare and launch dialog window
      DialogWindow* windowSkin = frut::widgets::WindowSkinContent::createDialogWindow(
                                    this, skin.getUiScale() );

      // attach callback to dialog window
      ModalComponentManager::getInstance()->attachCallback( windowSkin, ModalCallbackFunction::forComponent( window_skin_callback, this ) );
   } else if ( button == &ButtonDisplayPeakMeter ) {
      audioProcessor.changeParameter( KmeterPluginParameters::selShowPeaks, button->getToggleState() ? 0.0f : 1.0f );
   } else if ( button == &ButtonReset ) {
      audioProcessor.resetMeters();

      // apply skin to plug-in editor
      loadSkin();
   } else if ( button == &ButtonMono ) {
      audioProcessor.changeParameter( KmeterPluginParameters::selMono, button->getToggleState() ? 0.0f : 1.0f );
   } else if ( button == &ButtonDim ) {
      audioProcessor.changeParameter( KmeterPluginParameters::selDim, button->getToggleState() ? 0.0f : 1.0f );
   } else if ( button == &ButtonMute ) {
      audioProcessor.changeParameter( KmeterPluginParameters::selMute, button->getToggleState() ? 0.0f : 1.0f );
   } else if ( button == &ButtonFlip ) {
      audioProcessor.changeParameter( KmeterPluginParameters::selFlip, button->getToggleState() ? 0.0f : 1.0f );
   } else if ( button == &ButtonAbout ) {
      // manually activate button (will be deactivated in dialog
      // window callback)
      button->setToggleState( true, dontSendNotification );

      StringPairArray arrChapters;

      String pluginNameAndVersion = String( ProjectInfo::projectName );
      pluginNameAndVersion += " v";
      pluginNameAndVersion += JucePlugin_VersionString;

#if JucePlugin_Build_AU
      pluginNameAndVersion += " (Audio Unit)";
#endif // JucePlugin_Build_AU

#if JucePlugin_Build_VST
      pluginNameAndVersion += " (VST2)";
#endif // JucePlugin_Build_VST

#if JucePlugin_Build_VST3
      pluginNameAndVersion += " (VST3)";
#endif // JucePlugin_Build_VST3

      pluginNameAndVersion += String( KMETER_BUILD_ID );

      arrChapters.set(
         pluginNameAndVersion,
         String( JucePlugin_Desc ) + ".\n" );

      arrChapters.set(
         "Copyright",
         "(c) 2010-2021 Martin Zuther\n" );

      arrChapters.set(
         "Contributors",
         L"Bob Katz\n"
         L"Tod Gentille (Mac version)\n"
         L"Jan Kokemüller\n"
         L"Filipe Coelho\n"
         L"Bram de Jong\n" );

      arrChapters.set(
         "Beta testing and ideas",
         L"Aldo D. Sudak\n"
         L"Rickard (Interfearing Sounds)\n"
         L"David Tkaczuk\n" );

      arrChapters.set(
         "Thanks",
         L"Thanks to Bob Katz, all contributors and beta testers "
         L"and the open source community at large!\n\n"
         L"Thank you for using free software!\n" );

      arrChapters.set(
         "Libraries",
#ifdef LINUX
         L"ALSA\n"
#endif // LINUX
         L"FFTW\n"
#ifdef LINUX
         L"FreeType\n"
         L"JACK\n"
#endif // LINUX
         L"JUCE\n"
#ifdef LINUX
         L"POSIX Threads\n"
#endif // LINUX
#if JucePlugin_Build_VST || JucePlugin_Build_VST3
         L"VST\n"
#endif // JucePlugin_Build_VST || JucePlugin_Build_VST3
#ifdef LINUX
         L"Xlib\n"
         L"Xext\n"
#endif // LINUX
      );

#if JucePlugin_Build_VST || JucePlugin_Build_VST3
      // display trademarks (but only when necessary)
      arrChapters.set(
         "Trademarks",
         L"VST is a trademark of Steinberg Media Technologies GmbH, "
         L"registered in Europe and other countries.\n" );
#endif // JucePlugin_Build_VST || JucePlugin_Build_VST3

      arrChapters.set(
         "License",
         L"This program is free software: you can redistribute it "
         L"and/or modify it under the terms of the GNU General "
         L"Public License as published by the Free Software "
         L"Foundation, either version 3 of the License, or (at "
         L"your option) any later version.\n\n"

         L"This program is distributed in the hope that it will "
         L"be useful, but WITHOUT ANY WARRANTY; without even "
         L"the implied warranty of MERCHANTABILITY or FITNESS "
         L"FOR A PARTICULAR PURPOSE.  See the GNU General Public "
         L"License for more details.\n\n"

         L"You should have received a copy of the GNU General "
         L"Public License along with this program.  If not, "
         L"see <http://www.gnu.org/licenses/>.\n\n"

         L"Thank you for using free software!" );

      // prepare and launch dialog window
      int width = 270;
      int height = 410;

      DialogWindow* windowAbout = frut::widgets::WindowAboutContent::createDialogWindow(
                                     this, width, height, arrChapters );

      // attach callback to dialog window
      ModalComponentManager::getInstance()->attachCallback( windowAbout, ModalCallbackFunction::forComponent( window_about_callback, this ) );
   } else if ( button == &ButtonValidation ) {
      // manually activate button
      button->setToggleState( true, dontSendNotification );

      validationDialogOpen = true;
      audioProcessor.stopValidation();
      audioProcessor.silenceInput( true );

      // prepare and launch dialog window
      DialogWindow* windowValidation = WindowValidationContent::createDialogWindow(
                                          *this, audioProcessor );

      // attach callback to dialog window
      ModalComponentManager::getInstance()->attachCallback( windowValidation, ModalCallbackFunction::forComponent( window_validation_callback, this ) );
   }
}


void KmeterAudioProcessorEditor::updateAverageAlgorithm( bool reload_meters )
{
   if ( audioProcessor.getAverageAlgorithm() == KmeterPluginParameters::selAlgorithmItuBs1770 ) {
      ButtonItuBs1770.setToggleState( true, dontSendNotification );
      ButtonRms.setToggleState( false, dontSendNotification );
   } else {
      ButtonItuBs1770.setToggleState( false, dontSendNotification );
      ButtonRms.setToggleState( true, dontSendNotification );
   }

   needsMeterReload = reload_meters;
   audioProcessor.resetMeters();

   if ( ! isInitialising ) {
      // will also apply skin to plug-in editor
      reloadMeters();
   }
}

void KmeterAudioProcessorEditor::resized()
{
}
