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


AudioFilePlayer::AudioFilePlayer(const File audioFile, int sample_rate, MeterBallistics* meter_ballistics)
{
    nReportChannel = -1;
    bReports = false;
    bReportAverageMeterLevel = false;
    bReportPeakMeterLevel = false;
    bReportMaximumPeakLevel = false;
    bReportStereoMeterValue = false;
    bReportPhaseCorrelation = false;

    pMeterBallistics = meter_ballistics;

    AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    AudioFormatReader* formatReader = formatManager.createReaderFor(audioFile);

    if (formatReader)
    {
        audioFileSource = new AudioFormatReaderSource(formatReader, true);
        bIsPlaying = true;

        nNumberOfSamples = audioFileSource->getTotalLength();
        // pause for ten seconds after playback
        nNumberOfSamples += 10 * sample_rate;

        outputMessage(String("Audio file: \"") + audioFile.getFullPathName() + T("\""));
        outputMessage(String(formatReader->numChannels) + T(" channel(s), ") + String(formatReader->sampleRate) + T(" Hz, ") + String(formatReader->bitsPerSample) + T(" bit"));

        fSampleRate = (float) formatReader->sampleRate;

        if (formatReader->sampleRate != sample_rate)
        {
            outputMessage(String::empty);
            outputMessage(String("WARNING: sample rate mismatch (host: ") + String(sample_rate) + T(" Hz)!"));
            outputMessage(String::empty);
        }

        outputMessage(String::empty);
        outputMessage(T("Starting validation ..."));
        outputMessage(String::empty);
    }
    else
    {
        audioFileSource = NULL;
        bIsPlaying = false;
        bReports = false;
    }
}


AudioFilePlayer::~AudioFilePlayer()
{
    if (isPlaying())
    {
        outputMessage(T("Stopping validation ..."));
    }

    delete audioFileSource;
    audioFileSource = NULL;
}


void AudioFilePlayer::setReporters(int nChannel, bool bAverageMeterLevel, bool bPeakMeterLevel, bool bMaximumPeakLevel, bool bStereoMeterValue, bool bPhaseCorrelation)
{
    nReportChannel = nChannel;
    bReportAverageMeterLevel = bAverageMeterLevel;
    bReportPeakMeterLevel = bPeakMeterLevel;
    bReportMaximumPeakLevel = bMaximumPeakLevel;
    bReportStereoMeterValue = bStereoMeterValue;
    bReportPhaseCorrelation = bPhaseCorrelation;

    bReports = bReportAverageMeterLevel || bReportPeakMeterLevel || bReportMaximumPeakLevel || bReportStereoMeterValue || bReportPhaseCorrelation;
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
            outputMessage(T("Stopping validation ..."));

            bIsPlaying = false;
            bReports = false;
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
        if (bReportAverageMeterLevel)
        {
            if (nReportChannel < 0)
            {
                for (int nChannel = 0; nChannel < pMeterBallistics->getNumberOfChannels(); nChannel++)
                {
                    String strAverageMeterLevel = String(20.0f + pMeterBallistics->getAverageMeterLevel(nChannel), 2) + T(" dB");

                    if (!strAverageMeterLevel.startsWithChar(T('-')))
                    {
                        strAverageMeterLevel = String("+") + strAverageMeterLevel;
                    }

                    outputMessage(String("K-20 average (ch. ") + String(nChannel) + T("):  ") + strAverageMeterLevel);
                }
            }
            else
            {
                String strAverageMeterLevel = String(20.0f + pMeterBallistics->getAverageMeterLevel(nReportChannel), 2) + T(" dB");

                if (!strAverageMeterLevel.startsWithChar(T('-')))
                {
                    strAverageMeterLevel = String("+") + strAverageMeterLevel;
                }

                outputMessage(String("K-20 average (ch. ") + String(nReportChannel) + T("):  ") + strAverageMeterLevel);
            }
        }

        if (bReportPeakMeterLevel)
        {
            if (nReportChannel < 0)
            {
                for (int nChannel = 0; nChannel < pMeterBallistics->getNumberOfChannels(); nChannel++)
                {
                    String strPeakMeterLevel = String(20.0f + pMeterBallistics->getPeakMeterLevel(nChannel), 2) + T(" dB");

                    if (!strPeakMeterLevel.startsWithChar(T('-')))
                    {
                        strPeakMeterLevel = String("+") + strPeakMeterLevel;
                    }

                    outputMessage(String("K-20 peak (ch. ") + String(nChannel) + T("):     ") + strPeakMeterLevel);
                }
            }
            else
            {
                String strPeakMeterLevel = String(20.0f + pMeterBallistics->getPeakMeterLevel(nReportChannel), 2) + T(" dB");

                if (!strPeakMeterLevel.startsWithChar(T('-')))
                {
                    strPeakMeterLevel = String("+") + strPeakMeterLevel;
                }

                outputMessage(String("K-20 peak (ch. ") + String(nReportChannel) + T("):     ") + strPeakMeterLevel);
            }
        }

        if (bReportMaximumPeakLevel)
        {
            if (nReportChannel < 0)
            {
                for (int nChannel = 0; nChannel < pMeterBallistics->getNumberOfChannels(); nChannel++)
                {
                    String strMaximumPeakLevel = String(20.0f + pMeterBallistics->getMaximumPeakLevel(nChannel), 2) + T(" dB");

                    if (!strMaximumPeakLevel.startsWithChar(T('-')))
                    {
                        strMaximumPeakLevel = String("+") + strMaximumPeakLevel;
                    }

                    outputMessage(String("K-20 maximum (ch. ") + String(nChannel) + T("):  ") + strMaximumPeakLevel);
                }
            }
            else
            {
                String strMaximumPeakLevel = String(20.0f + pMeterBallistics->getMaximumPeakLevel(nReportChannel), 2) + T(" dB");

                if (!strMaximumPeakLevel.startsWithChar(T('-')))
                {
                    strMaximumPeakLevel = String("+") + strMaximumPeakLevel;
                }

                outputMessage(String("K-20 maximum (ch. ") + String(nReportChannel) + T("):  ") + strMaximumPeakLevel);
            }
        }

        if (bReportStereoMeterValue)
        {
            String strStereoMeterValue = String(pMeterBallistics->getStereoMeterValue(), 2);

            if (!strStereoMeterValue.startsWithChar(T('-')))
            {
                strStereoMeterValue = String("+") + strStereoMeterValue;
            }

            outputMessage(String("Stereo meter value:    ") + strStereoMeterValue);
        }

        if (bReportPhaseCorrelation)
        {
            String strPhaseCorrelation = String(pMeterBallistics->getPhaseCorrelation(), 2);

            if (!strPhaseCorrelation.startsWithChar(T('-')))
            {
                strPhaseCorrelation = String("+") + strPhaseCorrelation;
            }

            outputMessage(String("Phase correlation:     ") + strPhaseCorrelation);
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
    float fTime = audioFileSource->getNextReadPosition() / fSampleRate;

    // check for NaN
    if (fTime != fTime)
    {
        fTime = 0.0f;
    }

    int nTime = int(fTime);
    int nMilliSeconds = int(1000.0f * (fTime - nTime) + 0.5f);

    String strMinutes = String(nTime / 60).paddedLeft(T('0'), 2);
    String strSeconds = String(nTime % 60).paddedLeft(T('0'), 2);
    String strMilliSeconds = String(nMilliSeconds).paddedLeft(T('0'), 3);

    Logger::outputDebugString(String("[K-Meter validation - ") + strMinutes + T(":") + strSeconds + T(".") + strMilliSeconds + String("] ") + strMessage);
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
