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

#include "audio_file_player.h"


AudioFilePlayer::AudioFilePlayer(const String file_name, int sample_rate, MeterBallistics* meter_ballistics)
{
    nReportChannel = -1;
    bReports = false;
    bReportPeakMeterLevel = false;
    bReportAverageMeterLevel = false;
    bReportStereoMeterValue = false;
    bReportPhaseCorrelation = false;

    pMeterBallistics = meter_ballistics;

    AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    File audioFile = File(file_name);
    AudioFormatReader* formatReader = formatManager.createReaderFor(audioFile);

    if (formatReader)
    {
        audioFileSource = new AudioFormatReaderSource(formatReader, true);
        bIsPlaying = true;

        nNumberOfSamples = audioFileSource->getTotalLength();
        // pause for three seconds after playback
        nNumberOfSamples += 3 * sample_rate;

        outputMessage(String("Audio file: \"") + file_name + T("\""));
        outputMessage(String(formatReader->numChannels) + T(" channel(s), ") + String(formatReader->sampleRate) + T(" Hz, ") + String(formatReader->bitsPerSample) + T(" bit"));

        if (formatReader->sampleRate != sample_rate)
        {
            outputMessage(String::empty);
            outputMessage(String("WARNING: sample rate mismatch (host: ") + String(sample_rate) + T(" Hz)!"));
            outputMessage(String::empty);
        }
    }
    else
    {
        audioFileSource = NULL;
        bIsPlaying = false;
    }
}


AudioFilePlayer::~AudioFilePlayer()
{
    delete audioFileSource;
    audioFileSource = NULL;
}


void AudioFilePlayer::setReporters(int nChannel, bool bPeakMeterLevel, bool bAverageMeterLevel, bool bStereoMeterValue, bool bPhaseCorrelation)
{
    nReportChannel = nChannel;
    bReportPeakMeterLevel = bPeakMeterLevel;
    bReportAverageMeterLevel = bAverageMeterLevel;
    bReportStereoMeterValue = bStereoMeterValue;
    bReportPhaseCorrelation = bPhaseCorrelation;

    bReports = bReportPeakMeterLevel || bReportAverageMeterLevel || bReportStereoMeterValue || bReportPhaseCorrelation;
}


bool AudioFilePlayer::isPlaying()
{
    if (bIsPlaying)
    {
        if (audioFileSource->getNextReadPosition() < nNumberOfSamples)
        {
            return true;
        }
        else
        {
            bIsPlaying = false;
            return false;
        }
    }
    else
    {
        return false;
    }
}


void AudioFilePlayer::fillBufferChunk(AudioSampleBuffer* buffer)
{
    // report old meter readings
    if (bReports)
    {
        if (bReportPeakMeterLevel)
        {
            if (nReportChannel < 0)
            {
                for (int nChannel = 0; nChannel < pMeterBallistics->getNumberOfChannels(); nChannel++)
                {
                    outputMessage(String("K-20 peak (ch. ") + String(nChannel) + T("):     ") + String(20.0f + pMeterBallistics->getPeakMeterLevel(nChannel), 2) + T(" dB"));
                }
            }
            else
            {
                outputMessage(String("K-20 peak (ch. ") + String(nReportChannel) + T("):     ") + String(20.0f + pMeterBallistics->getPeakMeterLevel(nReportChannel), 2) + T(" dB"));
            }
        }

        if (bReportAverageMeterLevel)
        {
            if (nReportChannel < 0)
            {
                for (int nChannel = 0; nChannel < pMeterBallistics->getNumberOfChannels(); nChannel++)
                {
                    outputMessage(String("K-20 average (ch. ") + String(nChannel) + T("):  ") + String(20.0f + pMeterBallistics->getAverageMeterLevel(nChannel), 2) + T(" dB"));
                }
            }
            else
            {
                outputMessage(String("K-20 average (ch. ") + String(nReportChannel) + T("):  ") + String(20.0f + pMeterBallistics->getAverageMeterLevel(nReportChannel), 2) + T(" dB"));
            }
        }

        if (bReportStereoMeterValue)
        {
            outputMessage(String("Stereo meter value:    ") + String(pMeterBallistics->getStereoMeterValue(), 2));
        }

        if (bReportPhaseCorrelation)
        {
            outputMessage(String("Phase correlation:     ") + String(pMeterBallistics->getPhaseCorrelation(), 2));
        }

        outputMessage(String::empty);
    }

    if (isPlaying())
    {
        AudioSourceChannelInfo channelInfo;
        channelInfo.buffer = buffer;
        channelInfo.startSample = 0;
        channelInfo.numSamples = buffer->getNumSamples();

        audioFileSource->getNextAudioBlock(channelInfo);
    }
}


void AudioFilePlayer::outputMessage(const String& strMessage)
{
    Logger::outputDebugString(String("[K-Meter validation] ") + strMessage);
}
