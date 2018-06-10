/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2018 Martin Zuther (http://www.mzuther.de/)

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

#pragma once

#include "FrutHeader.h"
#include "meter_ballistics.h"


class AudioFilePlayer
{
public:
    AudioFilePlayer(const File audioFile,
                    int sample_rate,
                    MeterBallistics *meter_ballistics,
                    int crest_factor);

    ~AudioFilePlayer();

    bool isPlaying();
    bool matchingSampleRates();

    void copyTo(AudioBuffer<float> &buffer);

    void setCrestFactor(int crest_factor);
    void setReporters(int nChannel, bool ReportCSV,
                      bool bAverageMeterLevel, bool bPeakMeterLevel,
                      bool bMaximumPeakLevel, bool bTruePeakMeterLevel,
                      bool bMaximumTruePeakLevel, bool bStereoMeterValue,
                      bool bPhaseCorrelation);

private:
    JUCE_LEAK_DETECTOR(AudioFilePlayer);

    bool bIsPlaying;
    bool bSampleRatesMatch;
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
    bool bReportTruePeakMeterLevel;
    bool bReportMaximumTruePeakLevel;
    bool bReportStereoMeterValue;
    bool bReportPhaseCorrelation;

    frut::math::Averager nullAverager;
    Array<frut::math::Averager> arrAverager_AverageMeterLevels;
    Array<frut::math::Averager> arrAverager_PeakMeterLevels;
    Array<frut::math::Averager> arrAverager_TruePeakMeterLevels;

    frut::dsp::Dither dither_;

    ScopedPointer<AudioFormatReaderSource> audioFileSource;
    MeterBallistics *pMeterBallistics;

    void outputReportPlain(void);
    void outputReportCSVHeader(void);
    void outputReportCSVLine(void);

    String formatTime(void);
    String formatValue(const float fValue);

    void outputValue(const float fValue,
                     frut::math::Averager &averager,
                     const String &strPrefix,
                     const String &strSuffix);

    void outputMessage(const String &strMessage);
};
