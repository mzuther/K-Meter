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

#include "audio_file_player.h"


AudioFilePlayer::AudioFilePlayer( const File audioFile,
                                  int sample_rate,
                                  std::shared_ptr<MeterBallistics> meter_ballistics,
                                  int crest_factor ) :
   nullAverager( 3, 0.0f )
{
   nReportChannel = -1;
   bReports = false;
   bReportCSV = false;
   bReportAverageMeterLevel = false;
   bReportPeakMeterLevel = false;
   bReportMaximumPeakLevel = false;
   bReportTruePeakMeterLevel = false;
   bReportMaximumTruePeakLevel = false;
   bReportStereoMeterValue = false;
   bReportPhaseCorrelation = false;

   bSampleRatesMatch = true;
   bHeaderIsWritten = false;
   setCrestFactor( crest_factor );

   pMeterBallistics = meter_ballistics;

   // try "300" for uncorrelated band-limited pink noise
   nSamplesMovingAverage = 50;
   nNumberOfChannels = pMeterBallistics->getNumberOfChannels();

   for ( int nChannel = 0; nChannel < nNumberOfChannels; ++nChannel ) {
      arrAverager_AverageMeterLevels.add( frut::math::Averager( nSamplesMovingAverage, fMeterMinimumDecibel ) );
      arrAverager_PeakMeterLevels.add( frut::math::Averager( nSamplesMovingAverage, fMeterMinimumDecibel ) );
      arrAverager_TruePeakMeterLevels.add( frut::math::Averager( nSamplesMovingAverage, fMeterMinimumDecibel ) );
   }

   AudioFormatManager formatManager;
   formatManager.registerBasicFormats();

   // will be deleted by "audioFileSource"
   AudioFormatReader* formatReader = formatManager.createReaderFor( audioFile );

   if ( formatReader ) {
      audioFileSource = std::make_unique<AudioFormatReaderSource>( formatReader, true );
      bIsPlaying = true;

      nNumberOfSamples = audioFileSource->getTotalLength();
      // pause for twenty seconds after playback
      nNumberOfSamples += 20 * sample_rate;

      outputMessage( "Audio file: \"" + audioFile.getFullPathName() + "\"" );
      outputMessage( String( formatReader->numChannels ) + " channel(s), " + String( formatReader->sampleRate ) + " Hz, " + String( formatReader->bitsPerSample ) + " bit" );

      fSampleRate = ( float ) formatReader->sampleRate;

      if ( formatReader->sampleRate != sample_rate ) {
         bSampleRatesMatch = false;

         outputMessage( "" );
         outputMessage( "WARNING: sample rate mismatch (host: " + String( sample_rate ) + " Hz)!" );
         outputMessage( "" );
      }

      outputMessage( "" );
      outputMessage( "Starting validation ..." );
      outputMessage( "" );
   } else {
      bIsPlaying = false;
      bReports = false;
   }
}


AudioFilePlayer::~AudioFilePlayer()
{
   if ( isPlaying() ) {
      outputMessage( "Stopping validation ..." );
   }
}


void AudioFilePlayer::setCrestFactor( int crest_factor )
{
   fCrestFactor = float( crest_factor );
   fMeterMinimumDecibel = MeterBallistics::getMeterMinimumDecibel() + fCrestFactor;

   if ( crest_factor == 20 ) {
      strCrestFactor = "K-20";
   } else if ( crest_factor == 14 ) {
      strCrestFactor = "K-14";
   } else if ( crest_factor == 12 ) {
      strCrestFactor = "K-12";
   } else {
      strCrestFactor = "NORM";
   }
}


void AudioFilePlayer::setReporters( int nChannel,
                                    bool ReportCSV,
                                    bool bAverageMeterLevel,
                                    bool bPeakMeterLevel,
                                    bool bMaximumPeakLevel,
                                    bool bTruePeakMeterLevel,
                                    bool bMaximumTruePeakLevel,
                                    bool bStereoMeterValue,
                                    bool bPhaseCorrelation )
{
   bReportCSV = ReportCSV;

   nReportChannel = nChannel;
   bReportAverageMeterLevel = bAverageMeterLevel;
   bReportPeakMeterLevel = bPeakMeterLevel;
   bReportMaximumPeakLevel = bMaximumPeakLevel;
   bReportTruePeakMeterLevel = bTruePeakMeterLevel;
   bReportMaximumTruePeakLevel = bMaximumTruePeakLevel;
   bReportStereoMeterValue = bStereoMeterValue;
   bReportPhaseCorrelation = bPhaseCorrelation;

   bReports = bReportAverageMeterLevel || bReportPeakMeterLevel || bReportMaximumPeakLevel || bReportTruePeakMeterLevel || bReportMaximumTruePeakLevel || bReportStereoMeterValue || bReportPhaseCorrelation;
}


bool AudioFilePlayer::isPlaying()
{
   if ( bIsPlaying ) {
      if ( audioFileSource->getNextReadPosition() < nNumberOfSamples ) {
         return true;
      } else {
         outputMessage( "Stopping validation ..." );

         bIsPlaying = false;
         bReports = false;
         return false;
      }
   } else {
      return false;
   }
}


bool AudioFilePlayer::matchingSampleRates()
{
   return bSampleRatesMatch;
}


void AudioFilePlayer::copyTo( AudioBuffer<float>& buffer )
{
   // report old meter readings
   if ( bReports ) {
      if ( bReportCSV ) {
         outputReportCSVLine();
      } else {
         outputReportPlain();
      }
   }

   if ( isPlaying() ) {
      AudioSourceChannelInfo channelInfo;
      channelInfo.buffer = &buffer;
      channelInfo.startSample = 0;
      channelInfo.numSamples = buffer.getNumSamples();

      channelInfo.clearActiveBufferRegion();
      audioFileSource->getNextAudioBlock( channelInfo );
   }
}


void AudioFilePlayer::outputReportPlain()
{
   if ( bReportAverageMeterLevel ) {
      if ( nReportChannel < 0 ) {
         for ( int nChannel = 0; nChannel < nNumberOfChannels; ++nChannel ) {
            float fAverageMeterLevel = fCrestFactor + pMeterBallistics->getAverageMeterLevel( nChannel );
            String strPrefix = strCrestFactor + " average (ch. " + String( nChannel + 1 ) + "):    ";
            String strSuffix = " dB";
            outputValue( fAverageMeterLevel, arrAverager_AverageMeterLevels.getReference( nChannel ), strPrefix, strSuffix );
         }
      } else {
         float fAverageMeterLevel = fCrestFactor + pMeterBallistics->getAverageMeterLevel( nReportChannel );
         String strPrefix = strCrestFactor + " average (ch. " + String( nReportChannel + 1 ) + "):    ";
         String strSuffix = " dB";
         outputValue( fAverageMeterLevel, arrAverager_AverageMeterLevels.getReference( nReportChannel ), strPrefix, strSuffix );
      }
   }

   if ( bReportPeakMeterLevel ) {
      if ( nReportChannel < 0 ) {
         for ( int nChannel = 0; nChannel < nNumberOfChannels; ++nChannel ) {
            float fPeakMeterLevel = fCrestFactor + pMeterBallistics->getPeakMeterLevel( nChannel );
            String strPrefix = strCrestFactor + " peak (ch. " + String( nChannel + 1 ) + "):       ";
            String strSuffix = " dB";
            outputValue( fPeakMeterLevel, arrAverager_PeakMeterLevels.getReference( nChannel ), strPrefix, strSuffix );
         }
      } else {
         float fPeakMeterLevel = fCrestFactor + pMeterBallistics->getPeakMeterLevel( nReportChannel );
         String strPrefix = strCrestFactor + " peak (ch. " + String( nReportChannel + 1 ) + "):       ";
         String strSuffix = " dB";
         outputValue( fPeakMeterLevel, arrAverager_PeakMeterLevels.getReference( nReportChannel ), strPrefix, strSuffix );
      }
   }

   if ( bReportTruePeakMeterLevel ) {
      if ( nReportChannel < 0 ) {
         for ( int nChannel = 0; nChannel < nNumberOfChannels; ++nChannel ) {
            float fTruePeakMeterLevel = fCrestFactor + pMeterBallistics->getTruePeakMeterLevel( nChannel );
            String strPrefix = strCrestFactor + " true peak (ch. " + String( nChannel + 1 ) + "):  ";
            String strSuffix = " dB";
            outputValue( fTruePeakMeterLevel, arrAverager_TruePeakMeterLevels.getReference( nChannel ), strPrefix, strSuffix );
         }
      } else {
         float fTruePeakMeterLevel = fCrestFactor + pMeterBallistics->getTruePeakMeterLevel( nReportChannel );
         String strPrefix = strCrestFactor + " true peak (ch. " + String( nReportChannel + 1 ) + "):  ";
         String strSuffix = " dB";
         outputValue( fTruePeakMeterLevel, arrAverager_TruePeakMeterLevels.getReference( nReportChannel ), strPrefix, strSuffix );
      }
   }

   if ( bReportMaximumPeakLevel ) {
      if ( nReportChannel < 0 ) {
         for ( int nChannel = 0; nChannel < nNumberOfChannels; ++nChannel ) {
            float fMaximumPeakLevel = fCrestFactor + pMeterBallistics->getMaximumPeakLevel( nChannel );
            String strPrefix = strCrestFactor + " maximum (ch. " + String( nChannel + 1 ) + "):    ";
            String strSuffix = " dB";
            outputValue( fMaximumPeakLevel, nullAverager, strPrefix, strSuffix );
         }
      } else {
         float fMaximumPeakLevel = fCrestFactor + pMeterBallistics->getMaximumPeakLevel( nReportChannel );
         String strPrefix = strCrestFactor + " maximum (ch. " + String( nReportChannel + 1 ) + "):    ";
         String strSuffix = " dB";
         outputValue( fMaximumPeakLevel, nullAverager, strPrefix, strSuffix );
      }
   }

   if ( bReportMaximumTruePeakLevel ) {
      if ( nReportChannel < 0 ) {
         for ( int nChannel = 0; nChannel < nNumberOfChannels; ++nChannel ) {
            float fMaximumTruePeakLevel = fCrestFactor + pMeterBallistics->getMaximumTruePeakLevel( nChannel );
            String strPrefix = strCrestFactor + " true max. (ch. " + String( nChannel + 1 ) + "):  ";
            String strSuffix = " dB";
            outputValue( fMaximumTruePeakLevel, nullAverager, strPrefix, strSuffix );
         }
      } else {
         float fMaximumTruePeakLevel = fCrestFactor + pMeterBallistics->getMaximumTruePeakLevel( nReportChannel );
         String strPrefix = strCrestFactor + " true max. (ch. " + String( nReportChannel + 1 ) + "):  ";
         String strSuffix = " dB";
         outputValue( fMaximumTruePeakLevel, nullAverager, strPrefix, strSuffix );
      }
   }

   if ( bReportStereoMeterValue ) {
      float fStereoMeterValue = pMeterBallistics->getStereoMeterValue();
      String strPrefix = "Stereo meter value:      ";
      outputValue( fStereoMeterValue, nullAverager, strPrefix, "" );
   }

   if ( bReportPhaseCorrelation ) {
      float fPhaseCorrelation = pMeterBallistics->getPhaseCorrelation();
      String strPrefix = "Phase correlation:       ";
      outputValue( fPhaseCorrelation, nullAverager, strPrefix, "" );
   }

   outputMessage( "" );
}


void AudioFilePlayer::outputReportCSVHeader()
{
   bHeaderIsWritten = true;
   String strOutput = "\"timecode\"\t";

   if ( bReportAverageMeterLevel ) {
      if ( nReportChannel < 0 ) {
         for ( int nChannel = 0; nChannel < nNumberOfChannels; ++nChannel ) {
            strOutput += "\"avg_" + String( nChannel + 1 ) + "\"\t";
         }
      } else {
         strOutput += "\"avg_" + String( nReportChannel + 1 ) + "\"\t";
      }
   }

   if ( bReportPeakMeterLevel ) {
      if ( nReportChannel < 0 ) {
         for ( int nChannel = 0; nChannel < nNumberOfChannels; ++nChannel ) {
            strOutput += "\"pk_" + String( nChannel + 1 ) + "\"\t";
         }
      } else {
         strOutput += "\"pk_" + String( nReportChannel + 1 ) + "\"\t";
      }
   }

   if ( bReportTruePeakMeterLevel ) {
      if ( nReportChannel < 0 ) {
         for ( int nChannel = 0; nChannel < nNumberOfChannels; ++nChannel ) {
            strOutput += "\"tru_" + String( nChannel + 1 ) + "\"\t";
         }
      } else {
         strOutput += "\"tru_" + String( nReportChannel + 1 ) + "\"\t";
      }
   }

   if ( bReportMaximumPeakLevel ) {
      if ( nReportChannel < 0 ) {
         for ( int nChannel = 0; nChannel < nNumberOfChannels; ++nChannel ) {
            strOutput += "\"max_" + String( nChannel + 1 ) + "\"\t";
         }
      } else {
         strOutput += "\"max_" + String( nReportChannel + 1 ) + "\"\t";
      }
   }

   if ( bReportMaximumTruePeakLevel ) {
      if ( nReportChannel < 0 ) {
         for ( int nChannel = 0; nChannel < nNumberOfChannels; ++nChannel ) {
            strOutput += "\"mxt_" + String( nChannel + 1 ) + "\"\t";
         }
      } else {
         strOutput += "\"mxt_" + String( nReportChannel + 1 ) + "\"\t";
      }
   }

   if ( bReportStereoMeterValue ) {
      strOutput += "\"stereo\"\t";
   }

   if ( bReportPhaseCorrelation ) {
      strOutput += "\"corr\"\t";
   }

   Logger::outputDebugString( strOutput );
}


void AudioFilePlayer::outputReportCSVLine()
{
   String strOutput;

   if ( ! bHeaderIsWritten ) {
      outputReportCSVHeader();
   }

   if ( bReportAverageMeterLevel ) {
      if ( nReportChannel < 0 ) {
         for ( int nChannel = 0; nChannel < nNumberOfChannels; ++nChannel ) {
            float fAverageMeterLevel = fCrestFactor + pMeterBallistics->getAverageMeterLevel( nChannel );
            strOutput += formatValue( fAverageMeterLevel );
         }
      } else {
         float fAverageMeterLevel = fCrestFactor + pMeterBallistics->getAverageMeterLevel( nReportChannel );
         strOutput += formatValue( fAverageMeterLevel );
      }
   }

   if ( bReportPeakMeterLevel ) {
      if ( nReportChannel < 0 ) {
         for ( int nChannel = 0; nChannel < nNumberOfChannels; ++nChannel ) {
            float fPeakMeterLevel = fCrestFactor + pMeterBallistics->getPeakMeterLevel( nChannel );
            strOutput += formatValue( fPeakMeterLevel );
         }
      } else {
         float fPeakMeterLevel = fCrestFactor + pMeterBallistics->getPeakMeterLevel( nReportChannel );
         strOutput += formatValue( fPeakMeterLevel );
      }
   }

   if ( bReportTruePeakMeterLevel ) {
      if ( nReportChannel < 0 ) {
         for ( int nChannel = 0; nChannel < nNumberOfChannels; ++nChannel ) {
            float fTruePeakMeterLevel = fCrestFactor + pMeterBallistics->getTruePeakMeterLevel( nChannel );
            strOutput += formatValue( fTruePeakMeterLevel );
         }
      } else {
         float fTruePeakMeterLevel = fCrestFactor + pMeterBallistics->getTruePeakMeterLevel( nReportChannel );
         strOutput += formatValue( fTruePeakMeterLevel );
      }
   }

   if ( bReportMaximumPeakLevel ) {
      if ( nReportChannel < 0 ) {
         for ( int nChannel = 0; nChannel < nNumberOfChannels; ++nChannel ) {
            float fMaximumPeakLevel = fCrestFactor + pMeterBallistics->getMaximumPeakLevel( nChannel );
            strOutput += formatValue( fMaximumPeakLevel );
         }
      } else {
         float fMaximumPeakLevel = fCrestFactor + pMeterBallistics->getMaximumPeakLevel( nReportChannel );
         strOutput += formatValue( fMaximumPeakLevel );
      }
   }

   if ( bReportMaximumTruePeakLevel ) {
      if ( nReportChannel < 0 ) {
         for ( int nChannel = 0; nChannel < nNumberOfChannels; ++nChannel ) {
            float fMaximumTruePeakLevel = fCrestFactor + pMeterBallistics->getMaximumTruePeakLevel( nChannel );
            strOutput += formatValue( fMaximumTruePeakLevel );
         }
      } else {
         float fMaximumTruePeakLevel = fCrestFactor + pMeterBallistics->getMaximumTruePeakLevel( nReportChannel );
         strOutput += formatValue( fMaximumTruePeakLevel );
      }
   }

   if ( bReportStereoMeterValue ) {
      float fStereoMeterValue = pMeterBallistics->getStereoMeterValue();
      strOutput += formatValue( fStereoMeterValue );
   }

   if ( bReportPhaseCorrelation ) {
      float fPhaseCorrelation = pMeterBallistics->getPhaseCorrelation();
      strOutput += formatValue( fPhaseCorrelation );
   }

   Logger::outputDebugString( "\"" + formatTime() + "\"\t" + strOutput );
}


String AudioFilePlayer::formatTime()
{
   float fTime = audioFileSource->getNextReadPosition() / fSampleRate;

   // check for NaN
   if ( fTime != fTime ) {
      fTime = 0.0f;
   }

   int nTime = int( fTime );
   int nMilliSeconds = int( 1000.0f * ( fTime - nTime ) + 0.5f );

   String strMinutes = String( nTime / 60 ).paddedLeft( '0', 2 );
   String strSeconds = String( nTime % 60 ).paddedLeft( '0', 2 );
   String strMilliSeconds = String( nMilliSeconds ).paddedLeft( '0', 3 );

   return strMinutes + ":" + strSeconds + "." + strMilliSeconds;
}


String AudioFilePlayer::formatValue( const float fValue )
{
   String strValue;

   if ( fValue < 0.0f ) {
      strValue = String( fValue, 2 );
   } else {
      strValue = "+" + String( fValue, 2 );
   }

   return ( strValue + "\t" );
}


void AudioFilePlayer::outputValue( const float fValue,
                                   frut::math::Averager& averager,
                                   const String& strPrefix,
                                   const String& strSuffix )
{
   String strValue;

   if ( fValue < 0.0f ) {
      strValue = String( fValue, 2 ) + strSuffix;
   } else {
      strValue = "+" + String( fValue, 2 ) + strSuffix;
   }

   String strSimpleMovingAverage;

   averager.addSample( fValue );

   if ( averager.isValid() ) {
      float fSimpleMovingAverage = averager.getSimpleMovingAverage();

      if ( fSimpleMovingAverage < 0.0f ) {
         strSimpleMovingAverage = "   SMA(" + String( nSamplesMovingAverage ) + "): " + String( fSimpleMovingAverage, 2 ) + strSuffix;
      } else {
         strSimpleMovingAverage = "   SMA(" + String( nSamplesMovingAverage ) + "): +" + String( fSimpleMovingAverage, 2 ) + strSuffix;
      }
   }

   outputMessage( strPrefix + strValue + strSimpleMovingAverage );
}


void AudioFilePlayer::outputMessage( const String& strMessage )
{

   Logger::outputDebugString( "[Validation - " + formatTime() + "] " + strMessage );
}
