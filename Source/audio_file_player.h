/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2014 Martin Zuther (http://www.mzuther.de/)

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

#ifndef __AUDIO_FILE_PLAYER__
#define __AUDIO_FILE_PLAYER__

class AudioFilePlayer;

#include "../JuceLibraryCode/JuceHeader.h"
#include "averager.h"
#include "meter_ballistics.h"


class AudioFilePlayer
{
public:
    AudioFilePlayer(const File audioFile, int sample_rate, MeterBallistics* meter_ballistics, int crest_factor);
    ~AudioFilePlayer();

    bool isPlaying();
    void fillBufferChunk(AudioSampleBuffer* buffer);
    void setCrestFactor(int crest_factor);
    void setReporters(int nChannel, bool ReportCSV, bool bAverageMeterLevel, bool bPeakMeterLevel, bool bMaximumPeakLevel, bool bStereoMeterValue, bool bPhaseCorrelation);

private:
    JUCE_LEAK_DETECTOR(AudioFilePlayer);

    bool bIsPlaying;
    bool bHeaderIsWritten;
    int nSamplesMovingAverage;
    int64 nNumberOfSamples;
    float fSampleRate;
    float fCrestFactor;
    float fMeterMinimumDecibel;
    String strCrestFactor;

    int nNumberOfChannels;
    int nReportChannel;
    bool bReports;
    bool bReportCSV;
    bool bReportAverageMeterLevel;
    bool bReportPeakMeterLevel;
    bool bReportMaximumPeakLevel;
    bool bReportStereoMeterValue;
    bool bReportPhaseCorrelation;

    Averager** pAverager_AverageMeterLevels;
    Averager** pAverager_PeakMeterLevels;

    AudioFormatReaderSource* audioFileSource;
    MeterBallistics* pMeterBallistics;

    void outputReportPlain(void);
    void outputReportCSVHeader(void);
    void outputReportCSVLine(void);

    String formatTime(void);
    String formatValue(const float fValue);

    void outputValue(const float fValue, Averager* pAverager, const String& strPrefix, const String& strSuffix);
    void outputMessage(const String& strMessage);
};

#endif   // __AUDIO_FILE_PLAYER__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
