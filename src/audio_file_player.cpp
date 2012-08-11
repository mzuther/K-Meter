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

    // try "300" for uncorrelated band-limited pink noise
    nSamplesMovingAverage = 50;
    nNumberOfChannels = pMeterBallistics->getNumberOfChannels();

    pAverager_AverageMeterLevels = new Averager*[nNumberOfChannels];
    pAverager_PeakMeterLevels = new Averager*[nNumberOfChannels];

    for (int nChannel = 0; nChannel < nNumberOfChannels; nChannel++)
    {
        pAverager_AverageMeterLevels[nChannel] = new Averager(nSamplesMovingAverage, fMeterMinimumDecibel);
        pAverager_PeakMeterLevels[nChannel] = new Averager(nSamplesMovingAverage, fMeterMinimumDecibel);
    }

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

        outputMessage("Audio file: \"" + audioFile.getFullPathName() + "\"");
        outputMessage(String(formatReader->numChannels) + " channel(s), " + String(formatReader->sampleRate) + " Hz, " + String(formatReader->bitsPerSample) + " bit");

        fSampleRate = (float) formatReader->sampleRate;

        if (formatReader->sampleRate != sample_rate)
        {
            outputMessage(String::empty);
            outputMessage("WARNING: sample rate mismatch (host: " + String(sample_rate) + " Hz)!");
            outputMessage(String::empty);
        }

        outputMessage(String::empty);
        outputMessage("Starting validation ...");
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
        outputMessage("Stopping validation ...");
    }

    delete audioFileSource;
    audioFileSource = NULL;

    for (int nChannel = 0; nChannel < nNumberOfChannels; nChannel++)
    {
        delete pAverager_AverageMeterLevels[nChannel];
        pAverager_AverageMeterLevels[nChannel] = NULL;

        delete pAverager_PeakMeterLevels[nChannel];
        pAverager_PeakMeterLevels[nChannel] = NULL;
    }

    delete [] pAverager_AverageMeterLevels;
    pAverager_AverageMeterLevels = NULL;

    delete [] pAverager_PeakMeterLevels;
    pAverager_PeakMeterLevels = NULL;
}


void AudioFilePlayer::setCrestFactor(int crest_factor)
{
    fCrestFactor = float(crest_factor);
    fMeterMinimumDecibel = MeterBallistics::getMeterMinimumDecibel() + fCrestFactor;

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
            outputMessage("Stopping validation ...");

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
                for (int nChannel = 0; nChannel < nNumberOfChannels; nChannel++)
                {
                    float fAverageMeterLevel = fCrestFactor + pMeterBallistics->getAverageMeterLevel(nChannel);
                    String strPrefix = strCrestFactor + " average (ch. " + String(nChannel) + "):  ";
                    String strSuffix = " dB";
                    outputValue(fAverageMeterLevel, pAverager_AverageMeterLevels[nChannel], strPrefix, strSuffix);
                }
            }
            else
            {
                float fAverageMeterLevel = fCrestFactor + pMeterBallistics->getAverageMeterLevel(nReportChannel);
                String strPrefix = strCrestFactor + " average (ch. " + String(nReportChannel) + "):  ";
                String strSuffix = " dB";
                outputValue(fAverageMeterLevel, pAverager_AverageMeterLevels[nReportChannel], strPrefix, strSuffix);
            }
        }

        if (bReportPeakMeterLevel)
        {
            if (nReportChannel < 0)
            {
                for (int nChannel = 0; nChannel < nNumberOfChannels; nChannel++)
                {
                    float fPeakMeterLevel = fCrestFactor + pMeterBallistics->getPeakMeterLevel(nChannel);
                    String strPrefix = strCrestFactor + " peak (ch. " + String(nChannel) + "):     ";
                    String strSuffix = " dB";
                    outputValue(fPeakMeterLevel, pAverager_PeakMeterLevels[nChannel], strPrefix, strSuffix);
                }
            }
            else
            {
                float fPeakMeterLevel = fCrestFactor + pMeterBallistics->getPeakMeterLevel(nReportChannel);
                String strPrefix = strCrestFactor + " peak (ch. " + String(nReportChannel) + "):     ";
                String strSuffix = " dB";
                outputValue(fPeakMeterLevel, pAverager_PeakMeterLevels[nReportChannel], strPrefix, strSuffix);
            }
        }

        if (bReportMaximumPeakLevel)
        {
            if (nReportChannel < 0)
            {
                for (int nChannel = 0; nChannel < nNumberOfChannels; nChannel++)
                {
                    float fMaximumPeakLevel = fCrestFactor + pMeterBallistics->getMaximumPeakLevel(nChannel);
                    String strPrefix = strCrestFactor + " maximum (ch. " + String(nChannel) + "):  ";
                    String strSuffix = " dB";
                    outputValue(fMaximumPeakLevel, NULL, strPrefix, strSuffix);
                }
            }
            else
            {
                float fMaximumPeakLevel = fCrestFactor + pMeterBallistics->getMaximumPeakLevel(nReportChannel);
                String strPrefix = strCrestFactor + " maximum (ch. " + String(nReportChannel) + "):  ";
                String strSuffix = " dB";
                outputValue(fMaximumPeakLevel, NULL, strPrefix, strSuffix);
            }
        }

        if (bReportStereoMeterValue)
        {
            float fStereoMeterValue = pMeterBallistics->getStereoMeterValue();
            String strPrefix = "Stereo meter value:    ";
            String strSuffix = "";
            outputValue(fStereoMeterValue, NULL, strPrefix, strSuffix);
        }

        if (bReportPhaseCorrelation)
        {
            float fPhaseCorrelation = pMeterBallistics->getPhaseCorrelation();
            String strPrefix = "Phase correlation:     ";
            String strSuffix = "";
            outputValue(fPhaseCorrelation, NULL, strPrefix, strSuffix);
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


void AudioFilePlayer::outputValue(const float fValue, Averager* pAverager, const String& strPrefix, const String& strSuffix)
{
    String strValue;

    if (fValue < 0.0f)
    {
        strValue = String(fValue, 2) + strSuffix;
    }
    else
    {
        strValue = "+" + String(fValue, 2) + strSuffix;
    }

    String strSimpleMovingAverage;

    if (pAverager)
    {
        pAverager->addSample(fValue);

        if (pAverager->isValid())
        {
            float fSimpleMovingAverage = pAverager->getSimpleMovingAverage();

            if (fSimpleMovingAverage < 0.0f)
            {
                strSimpleMovingAverage = "   SMA(" + String(nSamplesMovingAverage) + "): " + String(fSimpleMovingAverage, 2) + strSuffix;
            }
            else
            {
                strSimpleMovingAverage = "   SMA(" + String(nSamplesMovingAverage) + "): +" + String(fSimpleMovingAverage, 2) + strSuffix;
            }
        }
    }

    outputMessage(strPrefix + strValue + strSimpleMovingAverage);
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

    String strMinutes = String(nTime / 60).paddedLeft('0', 2);
    String strSeconds = String(nTime % 60).paddedLeft('0', 2);
    String strMilliSeconds = String(nMilliSeconds).paddedLeft('0', 3);

    Logger::outputDebugString("[Validation - " + strMinutes + ":" + strSeconds + "." + strMilliSeconds + "] " + strMessage);
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
