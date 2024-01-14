/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2024 Martin Zuther (http://www.mzuther.de/)

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

#include "plugin_processor.h"
#include "plugin_editor.h"


// To hear the audio source after average filtering, simply set
// DEBUG_FILTER to "true".  Please remember to revert this variable to
// "false" before committing your changes.
const bool DEBUG_FILTER = false;

/*==============================================================================

Flow of parameter processing:

  Editor:      buttonClicked(button) / sliderValueChanged(slider)
  or
  Processor:   updateParameters(bIncludeHiddenParameters)

  Processor:   changeParameter(nIndex, fValue)
  Processor:   setParameter(nIndex, fValue)
  Parameters:  setFloat(nIndex, fValue)
  Editor:      actionListenerCallback(strMessage)
  Editor:      updateParameter(nIndex)

==============================================================================*/

KmeterAudioProcessor::KmeterAudioProcessor() :
#ifndef JucePlugin_PreferredChannelConfigurations
   AudioProcessor( getBusesProperties() ),
#endif // JucePlugin_PreferredChannelConfigurations
   kmeterBufferSize_( 1024 )
{
   frut::Frut::printVersionNumbers();

   // cppcheck-suppress knownConditionTrueFalse
   if ( DEBUG_FILTER ) {
      Logger::outputDebugString( "" );
      Logger::outputDebugString( "********************************************************************************" );
      Logger::outputDebugString( "*                                                                              *" );
      Logger::outputDebugString( "*  Debugging average filtering.  Please reset DEBUG_FILTER before committing!  *" );
      Logger::outputDebugString( "*                                                                              *" );
      Logger::outputDebugString( "********************************************************************************" );
      Logger::outputDebugString( "" );
   }

#ifdef KMETER_STEREO
   numberOfChannels_ = 2;
#else // KMETER_STEREO
   numberOfChannels_ = 6;
#endif // KMETER_STEREO

   meterBallistics_ = nullptr;
   averageLevelFiltered_ = nullptr;
   truePeakMeter_ = nullptr;

   ringBuffer_ = nullptr;
   ringBufferDouble_ = nullptr;

   sampleRateIsValid_ = false;
   isStereo_ = true;
   isSilent_ = false;
   hasStopped_ = true;
   reloadEditor_ = false;

   attenuationDecibel_ = 0.0;
   currentAttenuationDecibel_ = attenuationDecibel_;

   outputGain_ = 1.0;
   outputFadeRate_ = 0.0;

   // compensate for pre-delay
   setLatencySamples( kmeterBufferSize_ );

   // depends on "KmeterPluginParameters"!
   averageAlgorithmId_ = getRealInteger(
                            KmeterPluginParameters::selAverageAlgorithm );

   processedSeconds_ = 0.0f;
}


KmeterAudioProcessor::~KmeterAudioProcessor()
{
   removeAllActionListeners();
}


#ifndef JucePlugin_PreferredChannelConfigurations

AudioProcessor::BusesProperties KmeterAudioProcessor::getBusesProperties()
{
#ifdef KMETER_STEREO

   return BusesProperties()
          .withInput( "Main",
                      AudioChannelSet::stereo() )
          .withOutput( "Main",
                       AudioChannelSet::stereo() );

#else // KMETER_STEREO

   return BusesProperties()
          .withInput( "Main",
                      AudioChannelSet::create5point1() )
          .withOutput( "Main",
                       AudioChannelSet::create5point1() );

#endif // KMETER_STEREO
}

#endif // JucePlugin_PreferredChannelConfigurations


#ifndef JucePlugin_PreferredChannelConfigurations

bool KmeterAudioProcessor::isBusesLayoutSupported( const BusesLayout& layouts ) const
{
#ifdef KMETER_STEREO

   // main output must be stereo
   if ( layouts.getMainOutputChannelSet() != AudioChannelSet::stereo() ) {
      return false;
   }

#else // KMETER_STEREO

#ifdef KMETER_SURROUND

   // main output must be 5.1 surround
   if ( layouts.getMainOutputChannelSet() != AudioChannelSet::create5point1() ) {
      return false;
   }

#else // KMETER_SURROUND

   // main output can be either stereo or 5.1 surround
   if ( ( layouts.getMainOutputChannelSet() != AudioChannelSet::stereo() ) &&
        ( layouts.getMainOutputChannelSet() != AudioChannelSet::create5point1() ) ) {
      return false;
   }

#endif // KMETER_SURROUND

#endif // KMETER_STEREO

   // main input must be enabled
   if ( layouts.getMainInputChannelSet().isDisabled() ) {
      return false;
   }

   // main input and main output must be identical
   if ( layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet() ) {
      return false;
   }

   // valid channel layout
   return true;
}

#endif // JucePlugin_PreferredChannelConfigurations


#ifndef JucePlugin_PreferredChannelConfigurations

void KmeterAudioProcessor::processorLayoutsChanged()
{
   reloadEditor_ = true;
   numberOfChannels_ = getMainBusNumOutputChannels();
}

#endif // JucePlugin_PreferredChannelConfigurations


const String KmeterAudioProcessor::getName() const
{
   return JucePlugin_Name;
}


int KmeterAudioProcessor::getNumParameters()
{
   return pluginParameters_.getNumParameters( false );
}


const String KmeterAudioProcessor::getParameterName( int nIndex )
{
   return pluginParameters_.getName( nIndex );
}


const String KmeterAudioProcessor::getParameterText( int nIndex )
{
   return pluginParameters_.getText( nIndex );
}


float KmeterAudioProcessor::getParameter( int nIndex )
{
   // This method will be called by the host, probably on the audio
   // thread, so it's absolutely time-critical. Don't use critical
   // sections or anything GUI-related, or anything at all that may
   // block in any way!

   return pluginParameters_.getFloat( nIndex );
}


void KmeterAudioProcessor::changeParameter( int nIndex,
                                            float fValue )
{
   // This method will be called by the host, probably on the audio
   // thread, so it's absolutely time-critical. Don't use critical
   // sections or anything GUI-related, or anything at all that may
   // block in any way!

   if ( nIndex == KmeterPluginParameters::selMono ) {
      // automatically enable "Mono" button for mono channels
      if ( getMainBusNumInputChannels() == 1 ) {
         fValue = 1.0f;
         // automatically disable "Mono" button for multi-channel audio
      } else if ( getMainBusNumInputChannels() > 2 ) {
         fValue = 0.0f;
      }
   }

   // notify host of parameter change (this will automatically call
   // "setParameter"!)
   beginParameterChangeGesture( nIndex );
   setParameterNotifyingHost( nIndex, fValue );
   endParameterChangeGesture( nIndex );

   if ( ( nIndex == KmeterPluginParameters::selMute ) ||
        ( nIndex == KmeterPluginParameters::selDim ) ) {
      if ( getBoolean( KmeterPluginParameters::selMute ) ) {
         attenuationDecibel_ = -60.0;
      } else if ( getBoolean( KmeterPluginParameters::selDim ) ) {
         attenuationDecibel_ = -20.0;
      } else {
         attenuationDecibel_ = 0.0;
      }
   }
}


void KmeterAudioProcessor::setParameter( int nIndex,
                                         float fValue )
{
   // This method will be called by the host, probably on the audio
   // thread, so it's absolutely time-critical. Don't use critical
   // sections or anything GUI-related, or anything at all that may
   // block in any way!

   // Please only call this method directly for non-automatable
   // values!

   pluginParameters_.setFloat( nIndex, fValue );

   // notify plug-in editor of parameter change
   if ( pluginParameters_.hasChanged( nIndex ) ) {
      // for visible parameters, notify the editor of changes (this
      // will also clear the change flag)
      if ( nIndex < pluginParameters_.getNumParameters( false ) ) {
         if ( nIndex == KmeterPluginParameters::selCrestFactor ) {
            if ( audioFilePlayer_ ) {
               audioFilePlayer_->setCrestFactor( getRealInteger( nIndex ) );
            }
         } else if ( nIndex == KmeterPluginParameters::selAverageAlgorithm ) {
            setAverageAlgorithm( getRealInteger( nIndex ) );
         }

         // "PC" ==> parameter changed, followed by a hash and the
         // parameter's ID
         sendActionMessage( "PC#" + String( nIndex ) );
         // for hidden parameters, we only have to clear the change
         // flag
      } else {
         pluginParameters_.clearChangeFlag( nIndex );
      }
   }
}


void KmeterAudioProcessor::clearChangeFlag( int nIndex )
{
   pluginParameters_.clearChangeFlag( nIndex );
}


bool KmeterAudioProcessor::hasChanged( int nIndex )
{
   return pluginParameters_.hasChanged( nIndex );
}


void KmeterAudioProcessor::updateParameters( bool bIncludeHiddenParameters )
{
   int nNumParameters = pluginParameters_.getNumParameters( false );

   for ( int nIndex = 0; nIndex < nNumParameters; ++nIndex ) {
      if ( pluginParameters_.hasChanged( nIndex ) ) {
         float fValue = pluginParameters_.getFloat( nIndex );
         changeParameter( nIndex, fValue );
      }
   }

   if ( bIncludeHiddenParameters ) {
      // handle hidden parameters here!

      // the following parameters need no updating:
      //
      // * selValidationFileName
      // * selValidationSelectedChannel
      // * selValidationAverageMeterLevel
      // * selValidationPeakMeterLevel
      // * selValidationMaximumPeakLevel
      // * selValidationStereoMeterValue
      // * selValidationPhaseCorrelation
      // * selValidationCSVFormat
   }
}


bool KmeterAudioProcessor::getBoolean( int nIndex )
{
   // This method will be called by the host, probably on the audio
   // thread, so it's absolutely time-critical. Don't use critical
   // sections or anything GUI-related, or anything at all that may
   // block in any way!

   return pluginParameters_.getBoolean( nIndex );
}


int KmeterAudioProcessor::getRealInteger( int nIndex )
{
   // This method will be called by the host, probably on the audio
   // thread, so it's absolutely time-critical. Don't use critical
   // sections or anything GUI-related, or anything at all that may
   // block in any way!

   return pluginParameters_.getRealInteger( nIndex );
}


File KmeterAudioProcessor::getParameterValidationFile()
{
   // This method will be called by the host, probably on the audio
   // thread, so it's absolutely time-critical. Don't use critical
   // sections or anything GUI-related, or anything at all that may
   // block in any way!

   return pluginParameters_.getValidationFile();
}


void KmeterAudioProcessor::setParameterValidationFile( const File& fileValidation )
{
   // This method will be called by the host, probably on the audio
   // thread, so it's absolutely time-critical. Don't use critical
   // sections or anything GUI-related, or anything at all that may
   // block in any way!

   pluginParameters_.setValidationFile( fileValidation );
}


bool KmeterAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
   return true;
#else // JucePlugin_WantsMidiInput
   return false;
#endif // JucePlugin_WantsMidiInput
}


bool KmeterAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
   return true;
#else // JucePlugin_ProducesMidiOutput
   return false;
#endif // JucePlugin_ProducesMidiOutput
}


bool KmeterAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
   return true;
#else // JucePlugin_IsMidiEffect
   return false;
#endif // JucePlugin_IsMidiEffect
}


double KmeterAudioProcessor::getTailLengthSeconds() const
{
   return 0.0;
}


StringArray KmeterAudioProcessor::getAlternateDisplayNames() const
{
   StringArray displayNames( JucePlugin_Name );
   displayNames.add( "K-Meter" );
   displayNames.add( "kmet" );

   return displayNames;
}


int KmeterAudioProcessor::getNumPrograms()
{
   // some hosts don't cope very well if you tell them there are no
   // programs, so this should be at least 1, even if you're not
   // really implementing programs.
   return 1;
}


int KmeterAudioProcessor::getCurrentProgram()
{
   return 0;
}


void KmeterAudioProcessor::setCurrentProgram( int nIndex )
{
   ignoreUnused( nIndex );
}


const String KmeterAudioProcessor::getProgramName( int nIndex )
{
   ignoreUnused( nIndex );

   return String();
}


void KmeterAudioProcessor::changeProgramName( int nIndex,
                                              const String& newName )
{
   ignoreUnused( nIndex, newName );
}


void KmeterAudioProcessor::prepareToPlay( double sampleRate,
                                          int samplesPerBlock )
{
   // Use this method as the place to do any pre-playback
   // initialisation that you need..

   Logger::outputDebugString( "[K-Meter] preparing to play" );

   if ( ( sampleRate < 44100 ) || ( sampleRate > 192000 ) ) {
      Logger::outputDebugString( "[K-Meter] WARNING: sample rate of " +
                                 String( sampleRate ) + " Hz not supported" );
      sampleRateIsValid_ = false;
      return;
   } else {
      sampleRateIsValid_ = true;
   }

   isSilent_ = false;
   hasStopped_ = true;

   Logger::outputDebugString( "[K-Meter] number of main/aux input channels:  " +
                              String( getMainBusNumInputChannels() ) + "/" +
                              String( getTotalNumInputChannels() - getMainBusNumInputChannels() ) );
   Logger::outputDebugString( "[K-Meter] number of main/aux output channels: " +
                              String( getMainBusNumOutputChannels() ) + "/" +
                              String( getTotalNumOutputChannels() - getMainBusNumOutputChannels() ) );

   if ( reloadEditor_ ) {
      reloadEditor_ = false;
      auto editor = dynamic_cast<KmeterAudioProcessorEditor*>( getActiveEditor() );

      if ( editor != nullptr ) {
         editor->setNumberOfChannels( numberOfChannels_ );
      }
   }

   // force initialization of "outputGain_" in "processBlock()"
   currentAttenuationDecibel_ = attenuationDecibel_ + 1e-12;

   // output fade rate: 60 dB/s
   outputFadeRate_ = 60.0 / sampleRate;

   dither_.initialise( jmax( getMainBusNumInputChannels(),
                             getMainBusNumOutputChannels() ),
                       24 );

   isStereo_ = ( numberOfChannels_ == 2 );

   meterBallistics_ = std::make_shared<MeterBallistics>(
                         numberOfChannels_,
                         averageAlgorithmId_,
                         false,
                         false );

   peakLevels_.clear();
   rmsLevels_.clear();
   averageLevelsFiltered_.clear();
   truePeakLevels_.clear();

   overflowCounts_.clear();

   for ( int channel = 0; channel < numberOfChannels_; ++channel ) {
      peakLevels_.add( 0.0f );
      rmsLevels_.add( 0.0f );
      averageLevelsFiltered_.add( MeterBallistics::getMeterMinimumDecibel() );
      truePeakLevels_.add( 0.0f );

      overflowCounts_.add( 0 );
   }

   averageLevelFiltered_ = std::make_unique<AverageLevelFiltered>(
                              numberOfChannels_,
                              ( int ) sampleRate,
                              kmeterBufferSize_,
                              averageAlgorithmId_ );

   // maximum under-read of true peak measurement is 0.169 dB (see
   // Annex 2 of ITU-R BS.1770-4)
   int oversamplingFactor = 8;

   if ( sampleRate >= 176400 ) {
      oversamplingFactor /= 4;
   } else if ( sampleRate >= 88200 ) {
      oversamplingFactor /= 2;
   }

   truePeakMeter_ = std::make_unique<frut::dsp::TruePeakMeter>(
                       KmeterPluginParameters::getResourceDirectory(),
                       numberOfChannels_,
                       kmeterBufferSize_,
                       oversamplingFactor );

   // make sure that ring buffer can hold at least kmeterBufferSize_
   // samples and is large enough to receive a full block of audio
   int ringBufferSize = jmax( samplesPerBlock, kmeterBufferSize_ );

   int preDelay = kmeterBufferSize_;
   int chunkSize = kmeterBufferSize_;

   ringBuffer_ = std::make_unique<frut::audio::RingBuffer<float>>(
                    getMainBusNumOutputChannels(),
                    ringBufferSize,
                    preDelay,
                    chunkSize );

   ringBuffer_->setCallbackClass( this );

   ringBufferDouble_ = std::make_unique<frut::audio::RingBuffer<double>>(
                          getMainBusNumOutputChannels(),
                          ringBufferSize,
                          preDelay,
                          chunkSize );
}


void KmeterAudioProcessor::releaseResources()
{
   // When playback stops, you can use this as an opportunity to free
   // up any spare memory, etc.

   Logger::outputDebugString( "[K-Meter] releasing resources" );
   Logger::outputDebugString( "" );

   hasStopped_ = true;

   meterBallistics_ = nullptr;
   averageLevelFiltered_ = nullptr;
   truePeakMeter_ = nullptr;

   ringBuffer_ = nullptr;
   ringBufferDouble_ = nullptr;
}


void KmeterAudioProcessor::reset()
{
   // Use this method as the place to clear any delay lines, buffers,
   // etc, as it means there's been a break in the audio's
   // continuity.

   hasStopped_ = true;
   processedSeconds_ = 0.0f;

   ringBuffer_->clear();

   averageLevelFiltered_->reset();
   truePeakMeter_->reset();
}


void KmeterAudioProcessor::processBlock( AudioBuffer<float>& buffer,
                                         MidiBuffer& midiMessages )
{
   ignoreUnused( midiMessages );
   jassert( ! isUsingDoublePrecision() );

   // temporarily disable denormals
   ScopedNoDenormals noDenormals;

   // mute output if sample rate is invalid
   if ( ! sampleRateIsValid_ ) {
      buffer.clear();
      return;
   }

   // in case we have more main outputs than inputs, we'll clear any
   // output channels that didn't contain input data, because these
   // aren't guaranteed to be empty -- they may contain garbage.
   for ( int channel = getMainBusNumInputChannels(); channel < getMainBusNumOutputChannels(); ++channel ) {
      buffer.clear( channel, 0, buffer.getNumSamples() );
   }

   // return if there is no input
   if ( getMainBusNumInputChannels() < 1 ) {
      Logger::outputDebugString( "[K-Meter] no input channels!" );
      return;
   }

   // reset meters if playback has started
   resetOnPlay();

   // overwrite buffer with output of audio file player
   if ( audioFilePlayer_ ) {
      audioFilePlayer_->copyTo( buffer );
      // mute buffer if validation window is open
   } else if ( isSilent_ ) {
      buffer.clear();
   }

   // process two channels only
   if ( isStereo_ ) {
      float* leftChannel = buffer.getWritePointer( 0 );
      float* rightChannel = buffer.getWritePointer( 1 );

      // "Mono" button has been pressed
      if ( getBoolean( KmeterPluginParameters::selMono ) ) {
         for ( int sample = 0; sample < buffer.getNumSamples(); ++sample ) {
            // create mono mix
            double monoSample = 0.5 *
                                ( static_cast<double>( leftChannel[sample] ) +
                                  static_cast<double>( rightChannel[sample] ) );

            // dither mono mix and store in buffer
            leftChannel[sample] = dither_.ditherSample( 0, monoSample );
            rightChannel[sample] = leftChannel[sample];
         }

         // "Flip" button has been pressed
      } else if ( getBoolean( KmeterPluginParameters::selFlip ) ) {
         for ( int sample = 0; sample < buffer.getNumSamples(); ++sample ) {
            // flip stereo channels
            float oldleftChannel = leftChannel[sample];
            leftChannel[sample] = rightChannel[sample];
            rightChannel[sample] = oldleftChannel;
         }
      }
   }

   // copy buffer to ring buffer (applies pre-delay)
   //
   // calls "processBufferChunk" each time chunkSize samples have
   // been added!
   ringBuffer_->addFrom( buffer, 0, buffer.getNumSamples() );

   // copy ring buffer back to buffer
   ringBuffer_->removeTo( buffer, 0, buffer.getNumSamples() );

   float** bufferSample = buffer.getArrayOfWritePointers();

   // fade to mute / dim
   for ( int sample = 0; sample < buffer.getNumSamples(); ++sample ) {
      // fade in
      if ( currentAttenuationDecibel_ < attenuationDecibel_ ) {
         currentAttenuationDecibel_ += outputFadeRate_;

         if ( currentAttenuationDecibel_ > attenuationDecibel_ ) {
            currentAttenuationDecibel_ = attenuationDecibel_;
         }

         if ( currentAttenuationDecibel_ < 0.0 ) {
            outputGain_ = MeterBallistics::decibel2level_double(
                             currentAttenuationDecibel_ );
         } else {
            outputGain_ = 1.0;
         }

         // fade out
      } else if ( currentAttenuationDecibel_ > attenuationDecibel_ ) {
         currentAttenuationDecibel_ -= outputFadeRate_;

         if ( currentAttenuationDecibel_ < attenuationDecibel_ ) {
            currentAttenuationDecibel_ = attenuationDecibel_;
         }

         if ( currentAttenuationDecibel_ > -60.0 ) {
            outputGain_ = MeterBallistics::decibel2level_double(
                             currentAttenuationDecibel_ );
         } else {
            outputGain_ = 0.0;
         }
      }

      if ( outputGain_ != 1.0 ) {
         for ( int channel = 0; channel < buffer.getNumChannels(); ++channel ) {
            // apply fade to sample
            double fadedSample = outputGain_ * static_cast<double>(
                                    bufferSample[channel][sample] );

            // dither faded sample and store in buffer
            bufferSample[channel][sample] = dither_.ditherSample(
                                               channel, fadedSample );
         }
      }
   }
}


void KmeterAudioProcessor::processBlock( AudioBuffer<double>& buffer,
                                         MidiBuffer& midiMessages )
{
   ignoreUnused( midiMessages );
   jassert( isUsingDoublePrecision() );

   DBG( "[K-Meter] using double precision" );

   // temporarily disable denormals
   ScopedNoDenormals noDenormals;

   // mute output if sample rate is invalid
   if ( ! sampleRateIsValid_ ) {
      buffer.clear();
      return;
   }

   // in case we have more main outputs than inputs, we'll clear any
   // output channels that didn't contain input data, because these
   // aren't guaranteed to be empty -- they may contain garbage.
   for ( int channel = getMainBusNumInputChannels(); channel < getMainBusNumOutputChannels(); ++channel ) {
      buffer.clear( channel, 0, buffer.getNumSamples() );
   }

   // return if there is no input
   if ( getMainBusNumInputChannels() < 1 ) {
      Logger::outputDebugString( "[K-Meter] no input channels!" );
      return;
   }

   // reset meters if playback has started
   resetOnPlay();

   // overwrite buffer with output of audio file player
   if ( audioFilePlayer_ ) {
      AudioBuffer<float> processBuffer( buffer.getNumChannels(), buffer.getNumSamples() );

      // copy output of audio file player and convert to double
      audioFilePlayer_->copyTo( processBuffer );
      dither_.convertToDouble( processBuffer, buffer );

      // mute buffer if validation window is open
   } else if ( isSilent_ ) {
      buffer.clear();
   }

   // process two channels only
   if ( isStereo_ ) {
      double* leftChannel = buffer.getWritePointer( 0 );
      double* rightChannel = buffer.getWritePointer( 1 );

      // "Mono" button has been pressed
      if ( getBoolean( KmeterPluginParameters::selMono ) ) {
         for ( int sample = 0; sample < buffer.getNumSamples(); ++sample ) {
            // create mono mix
            double monoSample = 0.5 * ( leftChannel[sample] +
                                        rightChannel[sample] );

            // dither mono mix and store in buffer
            leftChannel[sample] = monoSample;
            rightChannel[sample] = leftChannel[sample];
         }

         // "Flip" button has been pressed
      } else if ( getBoolean( KmeterPluginParameters::selFlip ) ) {
         for ( int sample = 0; sample < buffer.getNumSamples(); ++sample ) {
            // flip stereo channels
            double oldleftChannel = leftChannel[sample];
            leftChannel[sample] = rightChannel[sample];
            rightChannel[sample] = oldleftChannel;
         }
      }
   }

   // create temporary buffer
   AudioBuffer<float> processBuffer( buffer.getNumChannels(), buffer.getNumSamples() );

   // dither input samples and store in temporary buffer
   dither_.ditherToFloat( buffer, processBuffer );

   // copy temporary buffer to ring buffer (applies pre-delay)
   //
   // calls "processBufferChunk" each time chunkSize samples have
   // been added!
   ringBuffer_->addFrom( processBuffer, 0, buffer.getNumSamples() );

   // to allow debugging of the average level filter, we'll have to
   // overwrite the input buffer from the ring buffer
   //
   // cppcheck-suppress knownConditionTrueFalse
   if ( DEBUG_FILTER ) {
      // copy ring buffer back to temporary buffer
      ringBuffer_->removeTo( processBuffer, 0, buffer.getNumSamples() );

      // convert temporary buffer to double and store in output
      // buffer
      dither_.convertToDouble( processBuffer, buffer );
      // otherwise, do not reduce the bit depth and stay in the double
      // domain
   } else {
      // apply pre-delay by using a dedicated ring buffer
      ringBufferDouble_->addFrom( buffer, 0, buffer.getNumSamples() );
      ringBufferDouble_->removeTo( buffer, 0, buffer.getNumSamples() );

      // the processed buffer data is not needed, so simulate
      // reading the ring buffer (move read pointer to prevent the
      // "overwriting unread data" debug message from appearing)
      ringBuffer_->removeToNull( buffer.getNumSamples() );
   }

   double** bufferSample = buffer.getArrayOfWritePointers();

   // fade to mute / dim
   for ( int sample = 0; sample < buffer.getNumSamples(); ++sample ) {
      // fade in
      if ( currentAttenuationDecibel_ < attenuationDecibel_ ) {
         currentAttenuationDecibel_ += outputFadeRate_;

         if ( currentAttenuationDecibel_ > attenuationDecibel_ ) {
            currentAttenuationDecibel_ = attenuationDecibel_;
         }

         if ( currentAttenuationDecibel_ < 0.0 ) {
            outputGain_ = MeterBallistics::decibel2level_double(
                             currentAttenuationDecibel_ );
         } else {
            outputGain_ = 1.0;
         }

         // fade out
      } else if ( currentAttenuationDecibel_ > attenuationDecibel_ ) {
         currentAttenuationDecibel_ -= outputFadeRate_;

         if ( currentAttenuationDecibel_ < attenuationDecibel_ ) {
            currentAttenuationDecibel_ = attenuationDecibel_;
         }

         if ( currentAttenuationDecibel_ > -60.0 ) {
            outputGain_ = MeterBallistics::decibel2level_double(
                             currentAttenuationDecibel_ );
         } else {
            outputGain_ = 0.0;
         }
      }

      if ( outputGain_ != 1.0 ) {
         for ( int channel = 0; channel < buffer.getNumChannels(); ++channel ) {
            // apply fade to sample
            double fadedSample = outputGain_ *
                                 bufferSample[channel][sample];

            // store faded sample in buffer
            bufferSample[channel][sample] = fadedSample;
         }
      }
   }
}


/// Called every time a certain number of samples have been added to a
/// RingBuffer.
///
/// @param buffer audio buffer with filled "chunk"
///
/// @return determines whether the audio buffer's contents should be
///         copied back to the original RingBuffer.
///
bool KmeterAudioProcessor::processBufferChunk( AudioBuffer<float>& buffer )
{
   bool isMono = getBoolean( KmeterPluginParameters::selMono );

   // length of buffer chunk in fractional seconds
   // (1024 samples / 44100 samples/s = 23.2 ms)
   processedSeconds_ = static_cast<float>( buffer.getNumSamples() ) /
                       static_cast<float>( getSampleRate() );

   // copy buffer to determine average level
   averageLevelFiltered_->copyFrom( buffer, buffer.getNumSamples() );

   // copy buffer to determine true peak level
   truePeakMeter_->copyFrom( buffer, buffer.getNumSamples() );

   for ( int channel = 0; channel < buffer.getNumChannels(); ++channel ) {
      if ( isMono && ( channel == 1 ) ) {
         peakLevels_.set( channel, peakLevels_[0] );
         rmsLevels_.set( channel, rmsLevels_[0] );
         averageLevelsFiltered_.set( channel, averageLevelsFiltered_[0] );
         truePeakLevels_.set( channel, truePeakLevels_[0] );

         overflowCounts_.set( channel, overflowCounts_[0] );
      } else {
         // determine peak level
         peakLevels_.set( channel,
                          buffer.getMagnitude( channel, 0, buffer.getNumSamples() ) );

         // determine RMS level
         rmsLevels_.set( channel,
                         buffer.getRMSLevel( channel, 0, buffer.getNumSamples() ) );

         // determine filtered average level (please note that this
         // level has already been converted to decibels!)
         averageLevelsFiltered_.set( channel,
                                     averageLevelFiltered_->getLevel( channel ) );

         // determine true peak level
         truePeakLevels_.set( channel,
                              truePeakMeter_->getLevel( channel ) );

         // determine overflows; treat all samples above -0.001 dBFS
         // as overflow
         //
         // in the 16-bit domain, full scale corresponds to an
         // absolute integer value of 32'767 or 32'768, so we'll
         // treat absolute levels of 32'767 and above as overflows;
         // this corresponds to a floating-point level of 32'767 /
         // 32'768 = 0.9999694 (approx. -0.001 dBFS).
         overflowCounts_.set( channel,
                              countOverflows( buffer, channel, buffer.getNumSamples(), 0.9999f ) );
      }

      // apply meter ballistics and store values so that the editor
      // can access them
      meterBallistics_->updateChannel( channel,
                                       processedSeconds_,
                                       peakLevels_[channel],
                                       truePeakLevels_[channel],
                                       averageLevelsFiltered_[channel],
                                       overflowCounts_[channel] );
   }

   // phase correlation is only defined for stereo signals
   if ( isStereo_ ) {
      float phaseCorrelation = 1.0f;

      // check whether the stereo signal has been mixed down to mono
      if ( isMono ) {
         phaseCorrelation = 1.0f;
         // otherwise, process only RMS levels at or above -80 dB
      } else if ( ( rmsLevels_[0] >= 0.0001f ) || ( rmsLevels_[1] >= 0.0001f ) ) {
         float sumOfProduct = 0.0f;
         float sumOfSquaresLeft = 0.0f;
         float sumOfSquaresRight = 0.0f;

         // determine phase correlation
         for ( int sample = 0; sample < buffer.getNumSamples(); ++sample ) {
            float leftChannel = buffer.getSample( 0, sample );
            float rightChannel = buffer.getSample( 1, sample );

            sumOfProduct += leftChannel * rightChannel;
            sumOfSquaresLeft += leftChannel * leftChannel;
            sumOfSquaresRight += rightChannel * rightChannel;
         }

         float sumsOfSquares = sumOfSquaresLeft * sumOfSquaresRight;

         // prevent division by zero and taking the square root of
         // a negative number
         if ( sumsOfSquares > 0.0f ) {
            phaseCorrelation = sumOfProduct / sqrtf( sumsOfSquares );
         } else {
            // this is mathematically incorrect, but "musically"
            // correct (i.e. signal is mono-compatible)
            phaseCorrelation = 1.0f;
         }
      }

      meterBallistics_->setPhaseCorrelation( processedSeconds_,
                                             phaseCorrelation );

      float stereoMeterValue = 0.0f;

      // do not process RMS levels below -80 dB
      if ( ( rmsLevels_[0] < 0.0001f ) && ( rmsLevels_[1] < 0.0001f ) ) {
         stereoMeterValue = 0.0f;
      } else if ( rmsLevels_[1] >= rmsLevels_[0] ) {
         stereoMeterValue = 1.0f - rmsLevels_[0] / rmsLevels_[1];
      } else {
         stereoMeterValue = rmsLevels_[1] / rmsLevels_[0] - 1.0f;
      }

      meterBallistics_->setStereoMeterValue( processedSeconds_,
                                             stereoMeterValue );
   }

   // "UM" ==> update meters
   sendActionMessage( "UM" );

   // To hear the audio source after average filtering, simply set
   // DEBUG_FILTER to "true".  Please remember to revert this
   // variable to "false" before committing your changes.
   //
   // cppcheck-suppress knownConditionTrueFalse
   if ( DEBUG_FILTER ) {
      // get average filter output
      averageLevelFiltered_->copyTo( buffer, buffer.getNumSamples() );

      // overwrite ring buffer contents
      return true;
   } else {
      // keep ring buffer contents
      return false;
   }
}


/// Count the number of overflows within a channel.
///
/// @param buffer audio buffer to process
///
/// @param channel audio channel to process
///
/// @param numberOfSamples number of samples to process, starting from
///        the **beginning** of the buffer
///
/// @param limitOverflow sample amplitude that does just *not* count as
///        an overflow (absolute value)
///
/// @return number of overflows
///
int KmeterAudioProcessor::countOverflows( const AudioBuffer<float>& buffer,
                                          const int channel,
                                          const int numberOfSamples,
                                          const float limitOverflow ) const
{
   jassert( isPositiveAndBelow( channel, buffer.getNumChannels() ) );
   jassert( isPositiveAndNotGreaterThan( numberOfSamples,
                                         buffer.getNumSamples() ) );

   const float* audioData = buffer.getReadPointer( channel );
   int overflows = 0;

   for ( int sample = 0; sample < numberOfSamples; ++sample ) {
      // get sample value
      float amplitude = audioData[sample];

      // convert sample value to amplitude
      if ( amplitude < 0.0f ) {
         amplitude = -amplitude;
      }

      // an overflow has occurred if the amplitude lies above the
      // overflow limit
      if ( amplitude > limitOverflow ) {
         ++overflows;
      }
   }

   return overflows;
}


void KmeterAudioProcessor::resetOnPlay()
{
   // get play head
   AudioPlayHead* playHead = AudioProcessor::getPlayHead();

   // check success
   if ( playHead != nullptr ) {
      AudioPlayHead::CurrentPositionInfo currentPosition;

      // get current position of play head (and check success)
      if ( playHead->getCurrentPosition( currentPosition ) ) {
         // check whether sequencer is currently playing
         bool isPlayingAgain = currentPosition.isPlaying;

         // check whether playback has just started
         if ( hasStopped_ && isPlayingAgain ) {
            // clear meters
            if ( meterBallistics_ != nullptr ) {
               meterBallistics_->reset();
            }
         }

         // update play state
         hasStopped_ = !isPlayingAgain;
      }
   }
}


void KmeterAudioProcessor::silenceInput( bool isSilentNew )
{
   isSilent_ = isSilentNew;
}


void KmeterAudioProcessor::startValidation( File fileAudio,
                                            int nSelectedChannel,
                                            bool bReportCSV,
                                            bool bAverageMeterLevel,
                                            bool bPeakMeterLevel,
                                            bool bMaximumPeakLevel,
                                            bool bTruePeakMeterLevel,
                                            bool bMaximumTruePeakLevel,
                                            bool bStereoMeterValue,
                                            bool bPhaseCorrelation )
{
   // reset all meters before we start the validation
   meterBallistics_->reset();

   isSilent_ = false;

   int crestFactor = getRealInteger( KmeterPluginParameters::selCrestFactor );
   audioFilePlayer_ = std::make_unique<AudioFilePlayer>(
                         fileAudio,
                         ( int ) getSampleRate(),
                         meterBallistics_,
                         crestFactor );

   if ( audioFilePlayer_->matchingSampleRates() ) {
      audioFilePlayer_->setReporters( nSelectedChannel,
                                      bReportCSV,
                                      bAverageMeterLevel,
                                      bPeakMeterLevel,
                                      bMaximumPeakLevel,
                                      bTruePeakMeterLevel,
                                      bMaximumTruePeakLevel,
                                      bStereoMeterValue,
                                      bPhaseCorrelation );

      // refresh editor; "V+" ==> validation started
      sendActionMessage( "V+" );
   } else {
      stopValidation();

      AlertWindow::showMessageBoxAsync(
         AlertWindow::WarningIcon,
         "Validation error",
         "Sample rates of host and validation file do not match." );
   }
}


void KmeterAudioProcessor::stopValidation()
{
   isSilent_ = false;
   audioFilePlayer_ = nullptr;

   // reset all meters after the validation
   meterBallistics_->reset();

   // refresh editor; "V-" ==> validation stopped
   sendActionMessage( "V-" );
}


bool KmeterAudioProcessor::isValidating()
{
   if ( audioFilePlayer_ == nullptr ) {
      return false;
   } else {
      if ( audioFilePlayer_->isPlaying() ) {
         return true;
      } else {
         stopValidation();
         return false;
      }
   }
}


std::shared_ptr<MeterBallistics> KmeterAudioProcessor::getLevels()
{
   return meterBallistics_;
}


void KmeterAudioProcessor::setMeterInfiniteHold( bool infiniteHold )
{
   if ( meterBallistics_ ) {
      meterBallistics_->setPeakMeterInfiniteHold( infiniteHold );
      meterBallistics_->setAverageMeterInfiniteHold( infiniteHold );
   }
}


void KmeterAudioProcessor::resetMeters()
{
   if ( meterBallistics_ ) {
      meterBallistics_->reset();
   }
}


int KmeterAudioProcessor::getAverageAlgorithm()
{
   return averageAlgorithmId_;
}


void KmeterAudioProcessor::setAverageAlgorithm(
   const int averageAlgorithm )
{
   if ( averageAlgorithm != averageAlgorithmId_ ) {
      if ( averageLevelFiltered_ != nullptr ) {
         averageLevelFiltered_->setAlgorithm( averageAlgorithm );
         setAverageAlgorithmFinal( averageLevelFiltered_->getAlgorithm() );
      } else {
         averageAlgorithmId_ = averageAlgorithm;
      }
   }
}


void KmeterAudioProcessor::setAverageAlgorithmFinal( const int averageAlgorithm )
{
   averageAlgorithmId_ = averageAlgorithm;
   meterBallistics_->setAverageAlgorithm( averageAlgorithmId_ );

   //  the level averaging alghorithm has been changed, so update the
   // "RMS" and "ITU-R" buttons to make sure that the correct button
   // is lit
   //
   // "AC" ==> algorithm changed
   sendActionMessage( "AC" );
}


AudioProcessorEditor* KmeterAudioProcessor::createEditor()
{
   return new KmeterAudioProcessorEditor( *this, numberOfChannels_ );
}


bool KmeterAudioProcessor::hasEditor() const
{
   return true;
}


void KmeterAudioProcessor::getStateInformation( MemoryBlock& destData )
{
   XmlElement xmlParameters = pluginParameters_.storeAsXml();

   DBG( "[K-Meter]" );
   DBG( "[K-Meter] storing plug-in parameters:" );
   DBG( "[K-Meter]" );
   DBG( String( "[K-Meter]   " ) + xmlParameters.toString().replace(
           "\n", "\n[K-Meter]   " ) );

   copyXmlToBinary( xmlParameters, destData );
}


void KmeterAudioProcessor::setStateInformation( const void* data,
                                                int sizeInBytes )
{
   std::unique_ptr<XmlElement> xmlParameters( getXmlFromBinary( data, sizeInBytes ) );

   DBG( "[K-Meter] loading plug-in parameters:" );
   DBG( "[K-Meter]" );
   DBG( String( "[K-Meter]   " ) + xmlParameters->toString().replace(
           "\n", "\n[K-Meter]   " ) );

   pluginParameters_.loadFromXml( xmlParameters.get() );
   updateParameters( true );
}


// This creates new instances of the plug-in.
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
   return new KmeterAudioProcessor();
}
