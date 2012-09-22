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

#ifndef __METER_BALLISTICS_H__
#define __METER_BALLISTICS_H__

#include "juce_library_code/juce_header.h"
#include "plugin_processor.h"


//==============================================================================
/**
*/
class MeterBallistics
{
public:
    MeterBallistics(int nChannels, int AverageAlgorithm, bool bPeakMeterInfiniteHold, bool bAverageMeterInfiniteHold);
    ~MeterBallistics();

    void setAverageAlgorithm(int AverageAlgorithm);
    void setPeakMeterInfiniteHold(bool bInfiniteHold);
    void setAverageMeterInfiniteHold(bool bInfiniteHold);
    void reset();

    int getNumberOfChannels();

    float getPeakMeterLevel(int nChannel);
    float getPeakMeterPeakLevel(int nChannel);

    float getAverageMeterLevel(int nChannel);
    float getAverageMeterPeakLevel(int nChannel);

    float getMaximumPeakLevel(int nChannel);
    int getNumberOfOverflows(int nChannel);

    float getStereoMeterValue();
    void setStereoMeterValue(float fTimePassed, float fStereoMeterValueNew);

    float getPhaseCorrelation();
    void setPhaseCorrelation(float fTimePassed, float fPhaseCorrelationNew);

    void updateChannel(int nChannel, float fTimePassed, float fPeak, float fRms, float fAverageFiltered, int nOverflows);

    static float level2decibel(float fLevel);
    static float decibel2level(float fDecibels);
    static float getMeterMinimumDecibel();
    static void setPeakToAverageCorrection(float peak_to_average_correction);
private:
    JUCE_LEAK_DETECTOR(MeterBallistics);

    int nNumberOfChannels;
    int nAverageAlgorithm;

    static float fMeterMinimumDecibel;
    static float fPeakToAverageCorrection;

    float* fPeakMeterLevels;
    float* fPeakMeterPeakLevels;

    float* fAverageMeterLevels;
    float* fAverageMeterPeakLevels;

    float* fMaximumPeakLevels;
    int* nNumberOfOverflows;

    float* fPeakMeterPeakLastChanged;
    float* fAverageMeterPeakLastChanged;

    float fStereoMeterValue;
    float fPhaseCorrelation;

    float PeakMeterBallistics(float fTimePassed, float fPeakLevelCurrent, float fPeakLevelOld);
    float PeakMeterPeakBallistics(float fTimePassed, float* fLastChanged, float fPeakLevelCurrent, float fPeakLevelOld);

    void AverageMeterBallistics(int nChannel, float fTimePassed, float fAverageLevelCurrent);
    float AverageMeterPeakBallistics(float fTimePassed, float* fLastChanged, float fAverageLevelCurrent, float fAverageLevelOld);

    void StereoMeterBallistics(float fTimePassed, float fStereoMeterCurrent);

    void PhaseCorrelationMeterBallistics(float fTimePassed, float fPhaseCorrelationCurrent);

    void LogMeterBallistics(float fMeterInertia, float fTimePassed, float fLevel, float& fReadout);
};


#endif  // __METER_BALLISTICS_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
