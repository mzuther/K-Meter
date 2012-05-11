/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2012 Martin Zuther (http://www.mzuther.de/)

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


AudioFilePlayer::AudioFilePlayer(const File audioFile, int sample_rate, MeterBallistics* meter_ballistics, int crest_factor)
{
    nReportChannel = -1;
    bReports = false;
    bReportAverageMeterLevel = false;
    bReportPeakMeterLevel = false;
    bReportMaximumPeakLevel = false;
    bReportStereoMeterValue = false;
    bReportPhaseCorrelation = false;

    setCrestFactor(crest_factor);

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


void AudioFilePlayer::setCrestFactor(int crest_factor)
{
    fCrestFactor = crest_factor;

    if (crest_factor == 20)
    {
        strCrestFactor = "K-20";
    }
    else if (crest_factor == 14)
    {
        strCrestFactor = "K-14";
    }
    else if (crest_factor == 12)
    {
        strCrestFactor = "K-12";
    }
    else
    {
        strCrestFactor = "NORM";
    }
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
                    float fAverageMeterLevel = fCrestFactor + pMeterBallistics->getAverageMeterLevel(nChannel);
                    String strPrefix = strCrestFactor + T(" average (ch. ") + String(nChannel) + T("):  ");
                    String strSuffix = T(" dB");
                    outputValue(fAverageMeterLevel, strPrefix, strSuffix);
                }
            }
            else
            {
                float fAverageMeterLevel = fCrestFactor + pMeterBallistics->getAverageMeterLevel(nReportChannel);
                String strPrefix = strCrestFactor + T(" average (ch. ") + String(nReportChannel) + T("):  ");
                String strSuffix = T(" dB");
                outputValue(fAverageMeterLevel, strPrefix, strSuffix);
            }
        }

        if (bReportPeakMeterLevel)
        {
            if (nReportChannel < 0)
            {
                for (int nChannel = 0; nChannel < pMeterBallistics->getNumberOfChannels(); nChannel++)
                {
                    float fPeakMeterLevel = fCrestFactor + pMeterBallistics->getPeakMeterLevel(nChannel);
                    String strPrefix = strCrestFactor + T(" peak (ch. ") + String(nChannel) + T("):     ");
                    String strSuffix = T(" dB");
                    outputValue(fPeakMeterLevel, strPrefix, strSuffix);
                }
            }
            else
            {
                float fPeakMeterLevel = fCrestFactor + pMeterBallistics->getPeakMeterLevel(nReportChannel);
                String strPrefix = strCrestFactor + T(" peak (ch. ") + String(nReportChannel) + T("):     ");
                String strSuffix = T(" dB");
                outputValue(fPeakMeterLevel, strPrefix, strSuffix);
            }
        }

        if (bReportMaximumPeakLevel)
        {
            if (nReportChannel < 0)
            {
                for (int nChannel = 0; nChannel < pMeterBallistics->getNumberOfChannels(); nChannel++)
                {
                    float fMaximumPeakLevel = fCrestFactor + pMeterBallistics->getMaximumPeakLevel(nChannel);
                    String strPrefix = strCrestFactor + T(" maximum (ch. ") + String(nChannel) + T("):  ");
                    String strSuffix = T(" dB");
                    outputValue(fMaximumPeakLevel, strPrefix, strSuffix);
                }
            }
            else
            {
                float fMaximumPeakLevel = fCrestFactor + pMeterBallistics->getMaximumPeakLevel(nReportChannel);
                String strPrefix = strCrestFactor + T(" maximum (ch. ") + String(nReportChannel) + T("):  ");
                String strSuffix = T(" dB");
                outputValue(fMaximumPeakLevel, strPrefix, strSuffix);
            }
        }

        if (bReportStereoMeterValue)
        {
            float fStereoMeterValue = pMeterBallistics->getStereoMeterValue();
            String strPrefix = T("Stereo meter value:    ");
            String strSuffix = T("");
            outputValue(fStereoMeterValue, strPrefix, strSuffix);
        }

        if (bReportPhaseCorrelation)
        {
            float fPhaseCorrelation = pMeterBallistics->getPhaseCorrelation();
            String strPrefix = T("Phase correlation:     ");
            String strSuffix = T("");
            outputValue(fPhaseCorrelation, strPrefix, strSuffix);
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


void AudioFilePlayer::outputValue(const float fValue, const String& strPrefix, const String& strSuffix)
{
    String strValue;

    if (fValue < 0.0f)
    {
        strValue = String(fValue, 2);
    }
    else
    {
        strValue = T("+") + String(fValue, 2);
    }

    outputMessage(strPrefix + strValue + strSuffix);
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
