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

#include "meter_ballistics.h"

MeterBallistics::MeterBallistics(bool bPeakHold, bool bAverageHold)
{
	float fMaximumHeadroom = 20.0f; // i.e. K-20
	fAverageCorrection = 3.0f;
	fMeterMinimumDecibel = -(fMaximumHeadroom + fAverageCorrection + 70.0f);
	
	setPeakHold(bPeakHold);
	setAverageHold(bAverageHold);

	reset();
}

MeterBallistics::~MeterBallistics()
{
}

void MeterBallistics::reset()
{
	nNumberOfChannels = 0;

	fStereoMeterValue = 0.0f;
	fCorrelationMeterValue = 1.0f;

	fPeakMeterLeft = fMeterMinimumDecibel;
	fPeakMeterRight = fMeterMinimumDecibel;
	fAverageMeterLeft = fMeterMinimumDecibel;
	fAverageMeterRight = fMeterMinimumDecibel;

	fPeakMeterLeftPeak = fMeterMinimumDecibel;
	fPeakMeterRightPeak = fMeterMinimumDecibel;
	fAverageMeterLeftPeak = fMeterMinimumDecibel;
	fAverageMeterRightPeak = fMeterMinimumDecibel;

	nOverflowsLeft = 0;
	nOverflowsRight = 0;
}

void MeterBallistics::setPeakHold(bool bPeakHold)
{
	if (bPeakHold)
	{
		fPeakMeterLeftPeakLastChanged = -1.0f;
		fPeakMeterRightPeakLastChanged = -1.0f;
	}
	else
	{
		fPeakMeterLeftPeakLastChanged = 0.0f;
		fPeakMeterRightPeakLastChanged = 0.0f;
	}
}

void MeterBallistics::setAverageHold(bool bAverageHold)
{
	if (bAverageHold)
	{
		fAverageMeterLeftPeakLastChanged = -1.0f;
		fAverageMeterRightPeakLastChanged = -1.0f;
	}
	else
	{
		fAverageMeterLeftPeakLastChanged = 0.0f;
		fAverageMeterRightPeakLastChanged = 0.0f;
	}
}

int MeterBallistics::getNumberOfChannels()
{
	return nNumberOfChannels;
}

float	MeterBallistics::getStereoMeterValue()
{
	return fStereoMeterValue;
}

float	MeterBallistics::getCorrelationMeterValue()
{
	return fCorrelationMeterValue;
}

float MeterBallistics::getPeakMeterLeft()
{
	return fPeakMeterLeft;
}

float MeterBallistics::getPeakMeterRight()
{
	return fPeakMeterRight;
}

float MeterBallistics::getAverageMeterLeft()
{
	return fAverageMeterLeft;
}

float MeterBallistics::getAverageMeterRight()
{
	return fAverageMeterRight;
}

float MeterBallistics::getPeakMeterLeftPeak()
{
	return fPeakMeterLeftPeak;
}

float MeterBallistics::getPeakMeterRightPeak()
{
	return fPeakMeterRightPeak;
}

float MeterBallistics::getAverageMeterLeftPeak()
{
	return fAverageMeterLeftPeak;
}

float MeterBallistics::getAverageMeterRightPeak()
{
	return fAverageMeterRightPeak;
}

int MeterBallistics::getOverflowsLeft()
{
	return nOverflowsLeft;
}

int MeterBallistics::getOverflowsRight()
{
	return nOverflowsRight;
}

void MeterBallistics::update(int nChannels, float fTimeFrame, float fPeakLeft, float fPeakRight, float fAverageLeft, float fAverageRight, float fCorrelation, int OverflowsLeft, int OverflowsRight)
{
	nNumberOfChannels = nChannels;

	float fStereoMeterLeft = fAverageLeft;
	float fStereoMeterRight = fAverageRight;
	float fStereoMeterValueOld = fStereoMeterValue;

	// do not process levels below -80 dB
	if ((fAverageLeft < 0.0001f) && (fAverageRight < 0.0001f))
		fStereoMeterValue = 0.0f;
	else if (fStereoMeterRight >= fStereoMeterLeft)
		fStereoMeterValue = (fStereoMeterRight - fStereoMeterLeft) / fStereoMeterRight;
	else
		fStereoMeterValue = (fStereoMeterRight - fStereoMeterLeft) / fStereoMeterLeft;

	fStereoMeterValue = StereoMeterBallistics(fTimeFrame, fStereoMeterValue, fStereoMeterValueOld);

	fCorrelationMeterValue = CorrelationMeterBallistics(fTimeFrame, fCorrelation, fCorrelationMeterValue);

	fPeakLeft = level2decibel(fPeakLeft);
	fAverageLeft = level2decibel(fAverageLeft) + fAverageCorrection;

	if (nNumberOfChannels == 1)
	{
	  fPeakRight = fPeakLeft;
	  fAverageRight = fAverageLeft;
	}
	else
	{
	  fPeakRight = level2decibel(fPeakRight);
	  fAverageRight = level2decibel(fAverageRight) + fAverageCorrection;
	}

	fPeakMeterLeft = PeakMeterBallistics(fTimeFrame, fPeakLeft, fPeakMeterLeft);
	fPeakMeterLeftPeak = PeakMeterPeakBallistics(fTimeFrame, &fPeakMeterLeftPeakLastChanged, fPeakLeft, fPeakMeterLeftPeak);

	fAverageMeterLeft = AverageMeterBallistics(fTimeFrame, fAverageLeft, fAverageMeterLeft);
	fAverageMeterLeftPeak = AverageMeterPeakBallistics(fTimeFrame, &fAverageMeterLeftPeakLastChanged, fAverageLeft, fAverageMeterLeftPeak);

	nOverflowsLeft += OverflowsLeft;

	if (nNumberOfChannels == 1)
	{
	  fPeakMeterRight = fPeakMeterLeft;
	  fPeakMeterRightPeak = fPeakMeterLeftPeak;

	  fAverageMeterRight = fAverageMeterLeft;
	  fAverageMeterRightPeak = fAverageMeterLeftPeak;

	  nOverflowsRight = nOverflowsLeft;
	}
	else
	{
	  fPeakMeterRight = PeakMeterBallistics(fTimeFrame, fPeakRight, fPeakMeterRight);
	  fPeakMeterRightPeak = PeakMeterPeakBallistics(fTimeFrame, &fPeakMeterRightPeakLastChanged, fPeakRight, fPeakMeterRightPeak);

	  fAverageMeterRight = AverageMeterBallistics(fTimeFrame, fAverageRight, fAverageMeterRight);
	  fAverageMeterRightPeak = AverageMeterPeakBallistics(fTimeFrame, &fAverageMeterRightPeakLastChanged, fAverageRight, fAverageMeterRightPeak);

	  nOverflowsRight += OverflowsRight;
	}
}

float MeterBallistics::level2decibel(float level)
{
	if (level == 0.0f)
		return fMeterMinimumDecibel;
	else
	{
		float output = 20.0f * log10(level);
		if (output < fMeterMinimumDecibel)
			return fMeterMinimumDecibel;
		else
			return output;
	}
}

float MeterBallistics::PeakMeterBallistics(float fTimeFrame, float fLevelCurrent, float fLevelOld)
{
	if (fLevelCurrent >= fLevelOld)
		return fLevelCurrent;
	else
	{
		float fReleaseCoef = 9.0f / fTimeFrame;
		return fLevelOld - fReleaseCoef;
	}
}

float MeterBallistics::AverageMeterBallistics(float fTimeFrame, float fLevelCurrent, float fLevelOld)
{
	// Thanks to Bram from Smartelectronix (http://www.musicdsp.org/showone.php?id=136) for the code snippet!
	float fOutput = fLevelOld / fMeterMinimumDecibel;
	float fTemp = fLevelCurrent / fMeterMinimumDecibel;

	// level has changed
	if (fTemp != fOutput)
	{
		float fAttackReleaseCoef = powf(0.01f, 1.0f / (0.600f * fTimeFrame));
	    fOutput = fAttackReleaseCoef * (fOutput - fTemp) + fTemp;
	}

	return fOutput * fMeterMinimumDecibel;
}

float MeterBallistics::StereoMeterBallistics(float fTimeFrame, float fLevelCurrent, float fLevelOld)
{
	// Thanks to Bram from Smartelectronix (http://www.musicdsp.org/showone.php?id=136) for the code snippet!
	float fOutput = fLevelOld;
	float fTemp = fLevelCurrent;

	// level has changed
	if (fTemp != fOutput)
	{
		float fAttackReleaseCoef = powf(0.01f, 1.0f / (1.200f * fTimeFrame));
	    fOutput = fAttackReleaseCoef * (fOutput - fTemp) + fTemp;
	}

	return fOutput;
}

float MeterBallistics::CorrelationMeterBallistics(float fTimeFrame, float fLevelCurrent, float fLevelOld)
{
	return StereoMeterBallistics(fTimeFrame, fLevelCurrent, fLevelOld);
}

float MeterBallistics::PeakMeterPeakBallistics(float fTimeFrame, float* fLastChanged, float fLevelCurrent, float fLevelOld)
{
	float fHoldTime = 10.0f;
	float fOutput = fMeterMinimumDecibel;

	// prevent the meter from overshooting
	if (fLevelCurrent >= 0.0f)
		fLevelCurrent = 0.0f;

	if (fLevelCurrent >= fLevelOld)
	{
		// reset hold time
		if (*fLastChanged >= 0.0f)
			*fLastChanged = 0.0f;

		fOutput = fLevelCurrent;
	}
	else
	{
		// update hold time
		if (*fLastChanged >= 0.0f)
			*fLastChanged += (1.0f / fTimeFrame);

		fOutput = fLevelOld;

		if (*fLastChanged > fHoldTime)
		{
			float fReleaseCoef = 9.0f / fTimeFrame;
			fOutput -= fReleaseCoef;
		}
	}

	return fOutput;
}

float MeterBallistics::AverageMeterPeakBallistics(float fTimeFrame, float* fLastChanged, float fLevelCurrent, float fLevelOld)
{
	return PeakMeterPeakBallistics(fTimeFrame, fLastChanged, fLevelCurrent, fLevelOld);
}
