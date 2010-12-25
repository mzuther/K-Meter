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


MeterBallistics::MeterBallistics(int nChannels, bool bPeakHold, bool bAverageHold)
{
	float fMaximumHeadroom = 20.0f; // i.e. K-20

	// the RMS of a sine wave is its amplitude divided by the square
	// root of 2, thus the difference between peak value and RMS is the
	// square root of 2 -- so let's convert this difference to dB
	fAverageCorrection = 20.0f * log10(sqrt(2.0f));

	fMeterMinimumDecibel = -(fMaximumHeadroom + fAverageCorrection + 70.0f);

	nNumberOfChannels = nChannels;

	fPeakMeter = new float[nNumberOfChannels];
	fAverageMeter = new float[nNumberOfChannels];

	fPeakMeterPeak = new float[nNumberOfChannels];
	fAverageMeterPeak = new float[nNumberOfChannels];

	fPeakMeterMaximumPeak = new float[nNumberOfChannels];
	nOverflows = new int[nNumberOfChannels];

	fPeakMeterPeakLastChanged = new float[nNumberOfChannels];
	fAverageMeterPeakLastChanged = new float[nNumberOfChannels];

	setPeakHold(bPeakHold);
	setAverageHold(bAverageHold);

	reset();
}


MeterBallistics::~MeterBallistics()
{
	delete [] fPeakMeter;
	delete [] fAverageMeter;

	delete [] fPeakMeterPeak;
	delete [] fAverageMeterPeak;

	delete [] fPeakMeterMaximumPeak;
	delete [] nOverflows;

	delete [] fPeakMeterPeakLastChanged;
	delete [] fAverageMeterPeakLastChanged;
}


void MeterBallistics::reset()
{
	fCorrelationMeterValue = 1.0f;
	fStereoMeterValue = 0.0f;

	for (int nChannel=0; nChannel < nNumberOfChannels; nChannel++)
	{
		fPeakMeter[nChannel] = fMeterMinimumDecibel;
		fAverageMeter[nChannel] = fMeterMinimumDecibel + fAverageCorrection;

		fPeakMeterPeak[nChannel] = fMeterMinimumDecibel;
		fAverageMeterPeak[nChannel] = fMeterMinimumDecibel + fAverageCorrection;

		fPeakMeterMaximumPeak[nChannel] = fMeterMinimumDecibel;
		nOverflows[nChannel] = 0;
	}
}


void MeterBallistics::setPeakHold(bool bPeakHold)
{
	for (int nChannel=0; nChannel < nNumberOfChannels; nChannel++)
	{
		if (bPeakHold)
			fPeakMeterPeakLastChanged[nChannel] = -1.0f;
		else
			fPeakMeterPeakLastChanged[nChannel] = 0.0f;
	}
}


void MeterBallistics::setAverageHold(bool bAverageHold)
{
	for (int nChannel=0; nChannel < nNumberOfChannels; nChannel++)
	{
		if (bAverageHold)
			fAverageMeterPeakLastChanged[nChannel] = -1.0f;
		else
			fAverageMeterPeakLastChanged[nChannel] = 0.0f;
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
	return fPeakMeter[0];
}


float MeterBallistics::getPeakMeterRight()
{
	return fPeakMeter[1];
}


float MeterBallistics::getAverageMeterLeft()
{
	return fAverageMeter[0];
}


float MeterBallistics::getAverageMeterRight()
{
	return fAverageMeter[1];
}


float MeterBallistics::getPeakMeterLeftPeak()
{
	return fPeakMeterPeak[0];
}


float MeterBallistics::getPeakMeterRightPeak()
{
	return fPeakMeterPeak[1];
}


float MeterBallistics::getPeakMeterLeftMaximumPeak()
{
	return fPeakMeterMaximumPeak[0];
}


float MeterBallistics::getPeakMeterRightMaximumPeak()
{
	return fPeakMeterMaximumPeak[1];
}


float MeterBallistics::getAverageMeterLeftPeak()
{
	return fAverageMeterPeak[0];
}


float MeterBallistics::getAverageMeterRightPeak()
{
	return fAverageMeterPeak[1];
}


int MeterBallistics::getOverflowsLeft()
{
	return nOverflows[0];
}


int MeterBallistics::getOverflowsRight()
{
	return nOverflows[1];
}


void MeterBallistics::updateStereoMeter(float fTimeFrame, float fAverageLeft, float fAverageRight)
{
	float fStereoMeterValueOld = fStereoMeterValue;

	// do not process levels below -80 dB
	if ((fAverageLeft < 0.0001f) && (fAverageRight < 0.0001f))
		fStereoMeterValue = 0.0f;
	else if (fAverageRight >= fAverageLeft)
		fStereoMeterValue = (fAverageRight - fAverageLeft) / fAverageRight;
	else
		fStereoMeterValue = (fAverageRight - fAverageLeft) / fAverageLeft;

	fStereoMeterValue = StereoMeterBallistics(fTimeFrame, fStereoMeterValue, fStereoMeterValueOld);
}


void MeterBallistics::updateCorrelation(float fTimeFrame, float fCorrelation)
{
	fCorrelationMeterValue = CorrelationMeterBallistics(fTimeFrame, fCorrelation, fCorrelationMeterValue);
}


void MeterBallistics::updateChannel(int nChannel, float fTimeFrame, float fPeak, float fAverage, int Overflows)
{
	// please make sure that the values of channel #0 are updated
	// before all others!
	if ((nNumberOfChannels == 1) && (nChannel > 0))
	{
	  fPeakMeter[nChannel] = fPeakMeter[0];
	  fPeakMeterPeak[nChannel] = fPeakMeterPeak[0];

	  fAverageMeter[nChannel] = fAverageMeter[0];
	  fAverageMeterPeak[nChannel] = fAverageMeterPeak[0];

	  nOverflows[nChannel] = nOverflows[0];
	}
	else
	{
		fPeak = level2decibel(fPeak);
		fAverage = level2decibel(fAverage) + fAverageCorrection;

		if (fPeak > fPeakMeterMaximumPeak[nChannel])
			fPeakMeterMaximumPeak[nChannel] = fPeak;

		fPeakMeter[nChannel] = PeakMeterBallistics(fTimeFrame, fPeak, fPeakMeter[nChannel]);
		fPeakMeterPeak[nChannel] = PeakMeterPeakBallistics(fTimeFrame, &fPeakMeterPeakLastChanged[nChannel], fPeak, fPeakMeterPeak[nChannel]);

		fAverageMeter[nChannel] = AverageMeterBallistics(fTimeFrame, fAverage, fAverageMeter[nChannel]);
		fAverageMeterPeak[nChannel] = AverageMeterPeakBallistics(fTimeFrame, &fAverageMeterPeakLastChanged[nChannel], fAverageMeter[nChannel], fAverageMeterPeak[nChannel]);

		nOverflows[nChannel] += Overflows;
	}

	// uncomment for validation of K-System meter peak readings:
	// DBG(String("[K-20 Peak]  ") + String(nChannel) + T(": ") + String(20.0f + fPeakMeter[nChannel], 2));

	// uncomment for validation of K-System meter average readings:
	// DBG(String("[K-20 Average]  ") + String(nChannel) + T(": ") + String(20.0f + fAverageMeter[nChannel], 2));
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
		float fReleaseCoef = 26.0f / (3.0f * fTimeFrame);
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
			float fReleaseCoef = 26.0f / (3.0f * fTimeFrame);
			fOutput -= fReleaseCoef;
		}
	}

	return fOutput;
}


float MeterBallistics::AverageMeterPeakBallistics(float fTimeFrame, float* fLastChanged, float fLevelCurrent, float fLevelOld)
{
	return PeakMeterPeakBallistics(fTimeFrame, fLastChanged, fLevelCurrent, fLevelOld);
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
