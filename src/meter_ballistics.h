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
    MeterBallistics(bool bPeakHold, bool bAverageHold);
    ~MeterBallistics();

	void setPeakHold(bool bPeakHold);
	void setAverageHold(bool bAverageHold);
	void reset();

	int getNumberOfChannels();

	float	getStereoMeterValue();
	float	getCorrelationMeterValue();

	float getPeakMeterLeft();
	float getPeakMeterRight();
	float getAverageMeterLeft();
	float getAverageMeterRight();

	float getPeakMeterLeftPeak();
	float getPeakMeterRightPeak();
	float getAverageMeterLeftPeak();
	float getAverageMeterRightPeak();

	float getPeakMeterLeftMaximumPeak();
	float getPeakMeterRightMaximumPeak();

	int getOverflowsLeft();
	int getOverflowsRight();

	void update(int nChannels, float fTimeFrame, float fPeakLeft, float fPeakRight, float fAverageLeft, float fAverageRight, float fCorrelation, int OverflowsLeft, int OverflowsRight);

private:
	int nNumberOfChannels;

	float fMeterMinimumDecibel;
	float fAverageCorrection;

	float fStereoMeterValue;
	float fCorrelationMeterValue;

	float fPeakMeterLeft;
	float fPeakMeterRight;
	float fAverageMeterLeft;
	float fAverageMeterRight;

	float fPeakMeterLeftPeak;
	float fPeakMeterRightPeak;
	float fAverageMeterLeftPeak;
	float fAverageMeterRightPeak;

	float fPeakMeterLeftMaximumPeak;
	float fPeakMeterRightMaximumPeak;

	int nOverflowsLeft;
	int nOverflowsRight;

	float fPeakMeterLeftPeakLastChanged;
	float fPeakMeterRightPeakLastChanged;
	float fAverageMeterLeftPeakLastChanged;
	float fAverageMeterRightPeakLastChanged;

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
