/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010 Martin Zuther (http://www.mzuther.de/)

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


//==============================================================================
/**
*/
class MeterBallistics
{
public:
    MeterBallistics(int nChannels, bool bPeakHold, bool bAverageHold);
    ~MeterBallistics();

	void setPeakHold(bool bPeakHold);
	void setAverageHold(bool bAverageHold);
	void reset();

	int getNumberOfChannels();

	float getStereoMeterValue();
	float getCorrelationMeterValue();

	float getPeakMeter(int nChannel);
	float getAverageMeter(int nChannel);

	float getPeakMeterPeak(int nChannel);
	float getAverageMeterPeak(int nChannel);

	float getMaximumPeak(int nChannel);
	int getOverflows(int nChannel);

	void updateChannel(int nChannel, float fTimeFrame, float fPeak, float fAverage, int Overflows);
	void updateCorrelation(float fTimeFrame, float fCorrelation);
	void updateStereoMeter(float fTimeFrame, float fAverageLeft, float fAverageRight);

private:
	int nNumberOfChannels;

	float fMeterMinimumDecibel;
	float fAverageCorrection;

	float fStereoMeterValue;
	float fCorrelationMeterValue;

	float *fPeakMeter;
	float *fAverageMeter;

	float *fPeakMeterPeak;
	float *fAverageMeterPeak;

	float *fMaximumPeak;

	int *nOverflows;

	float *fPeakMeterPeakLastChanged;
	float *fAverageMeterPeakLastChanged;

	float level2decibel(float level);

	float StereoMeterBallistics(float fTimeFrame, float fLevelCurrent, float fLevelOld);
	float CorrelationMeterBallistics(float fTimeFrame, float fLevelCurrent, float fLevelOld);

	float PeakMeterBallistics(float fTimeFrame, float fLevelCurrent, float fLevelOld);
	float PeakMeterPeakBallistics(float fTimeFrame, float* fLastChanged, float fLevelCurrent, float fLevelOld);

	float AverageMeterBallistics(float fTimeFrame, float fLevelCurrent, float fLevelOld);
	float AverageMeterPeakBallistics(float fTimeFrame, float* fLastChanged, float fLevelCurrent, float fLevelOld);
};


#endif  // __METER_BALLISTICS_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
