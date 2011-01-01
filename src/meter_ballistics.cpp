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

    fPeakMeterLevels = new float[nNumberOfChannels];
    fPeakMeterPeakLevels = new float[nNumberOfChannels];

    fAverageMeterLevels = new float[nNumberOfChannels];
    fAverageMeterPeakLevels = new float[nNumberOfChannels];

    fPeakMeterPeakLastChanged = new float[nNumberOfChannels];
    fAverageMeterPeakLastChanged = new float[nNumberOfChannels];

    fMaximumPeakLevels = new float[nNumberOfChannels];
    nNumberOfOverflows = new int[nNumberOfChannels];

    setPeakHold(bPeakHold);
    setAverageHold(bAverageHold);

    reset();
}


MeterBallistics::~MeterBallistics()
{
    delete [] fPeakMeterLevels;
    fPeakMeterLevels = NULL;

    delete [] fPeakMeterPeakLevels;
    fPeakMeterPeakLevels = NULL;

    delete [] fAverageMeterLevels;
    fAverageMeterLevels = NULL;

    delete [] fAverageMeterPeakLevels;
    fAverageMeterPeakLevels = NULL;

    delete [] fPeakMeterPeakLastChanged;
    fPeakMeterPeakLastChanged = NULL;

    delete [] fAverageMeterPeakLastChanged;
    fAverageMeterPeakLastChanged = NULL;

    delete [] fMaximumPeakLevels;
    fMaximumPeakLevels = NULL;

    delete [] nNumberOfOverflows;
    nNumberOfOverflows = NULL;
}


void MeterBallistics::reset()
{
    fPhaseCorrelation = 1.0f;
    fStereoMeterValue = 0.0f;

    for (int nChannel = 0; nChannel < nNumberOfChannels; nChannel++)
    {
        fPeakMeterLevels[nChannel] = fMeterMinimumDecibel;
        fPeakMeterPeakLevels[nChannel] = fMeterMinimumDecibel;

        fAverageMeterLevels[nChannel] = fMeterMinimumDecibel + fAverageCorrection;
        fAverageMeterPeakLevels[nChannel] = fMeterMinimumDecibel + fAverageCorrection;

        fMaximumPeakLevels[nChannel] = fMeterMinimumDecibel;
        nNumberOfOverflows[nChannel] = 0;
    }
}


void MeterBallistics::setPeakHold(bool bPeakHold)
{
    for (int nChannel = 0; nChannel < nNumberOfChannels; nChannel++)
    {
        if (bPeakHold)
        {
            fPeakMeterPeakLastChanged[nChannel] = -1.0f;
        }
        else
        {
            fPeakMeterPeakLastChanged[nChannel] = 0.0f;
        }
    }
}


void MeterBallistics::setAverageHold(bool bAverageHold)
{
    for (int nChannel = 0; nChannel < nNumberOfChannels; nChannel++)
    {
        if (bAverageHold)
        {
            fAverageMeterPeakLastChanged[nChannel] = -1.0f;
        }
        else
        {
            fAverageMeterPeakLastChanged[nChannel] = 0.0f;
        }
    }
}


int MeterBallistics::getNumberOfChannels()
/*  Get number of audio channels.

    return value (float): returns the current number of processed
    audio channels
*/
{
    return nNumberOfChannels;
}


float MeterBallistics::getPeakMeterLevel(int nChannel)
/*  Get current level of an audio channel's peak level meter.

    nChannel (integer): selected audio channel

    return value (float): returns the current level of the given audio
    channel's peak level meter
*/
{
    jassert(nChannel >= 0);
    jassert(nChannel < nNumberOfChannels);

    return fPeakMeterLevels[nChannel];
}


float MeterBallistics::getPeakMeterPeakLevel(int nChannel)
/*  Get peak level of an audio channel's peak level meter.

    nChannel (integer): selected audio channel

    return value (float): returns the (changing) peak level of the
    given audio channel's peak level meter
*/
{
    jassert(nChannel >= 0);
    jassert(nChannel < nNumberOfChannels);

    return fPeakMeterPeakLevels[nChannel];
}


float MeterBallistics::getAverageMeterLevel(int nChannel)
/*  Get current level of an audio channel's average level meter.

    nChannel (integer): selected audio channel

    return value (float): returns the current level of the given audio
    channel's average level meter
*/
{
    jassert(nChannel >= 0);
    jassert(nChannel < nNumberOfChannels);

    return fAverageMeterLevels[nChannel];
}


float MeterBallistics::getAverageMeterPeakLevel(int nChannel)
/*  Get peak level of an audio channel's average level meter.

    nChannel (integer): selected audio channel

    return value (float): returns the (changing) peak level of the
    given audio channel's average level meter
*/
{
    jassert(nChannel >= 0);
    jassert(nChannel < nNumberOfChannels);

    return fAverageMeterPeakLevels[nChannel];
}


float MeterBallistics::getMaximumPeakLevel(int nChannel)
/*  Get overall maximum peak level of an audio channel.

    nChannel (integer): selected audio channel

    return value (float): returns the overall maximum peak level that
    has been registered on the given audio channel
*/
{
    jassert(nChannel >= 0);
    jassert(nChannel < nNumberOfChannels);

    return fMaximumPeakLevels[nChannel];
}


int MeterBallistics::getNumberOfOverflows(int nChannel)
/*  Get number of overflows of an audio channel.

    nChannel (integer): selected audio channel

    return value (integer): returns the number of overflows that has
    been registered on the given audio channel
*/
{
    jassert(nChannel >= 0);
    jassert(nChannel < nNumberOfChannels);

    return nNumberOfOverflows[nChannel];
}


float MeterBallistics::getStereoMeterValue()
/*  Get stereo meter value.

    return value (float): returns the stereo meter value of a stereo
    audio channel pair
*/
{
    // assure that we are processing a stereo audio channel pair
    jassert(nNumberOfChannels == 2);

    return fStereoMeterValue;
}


void MeterBallistics::setStereoMeterValue(float fProcessedSeconds, float fStereoMeterValueNew)
/*  Set stereo meter value and apply meter ballistics.

    fProcessedSeconds (float): length of current buffer chunk in
    fractional seconds

    fStereoMeterValueNew (float): current stereo meter value

    return value: none
*/
{
    // assure that we are processing a stereo audio channel pair
    jassert(nNumberOfChannels == 2);

    // apply meter ballistics
    fStereoMeterValue = StereoMeterBallistics(fProcessedSeconds, fStereoMeterValueNew, fStereoMeterValue);

    // uncomment for validation of stereo meter readings:
    // DBG(String("[K-Meter] Stereo meter value: ") + String(fStereoMeterValue, 2));
}


float MeterBallistics::getPhaseCorrelation()
/*  Get phase correlation.

    return value (float): returns the phase correlation value of a
    stereo audio channel pair
*/
{
    // assure that we are processing a stereo audio channel pair
    jassert(nNumberOfChannels == 2);

    return fPhaseCorrelation;
}


void MeterBallistics::setPhaseCorrelation(float fProcessedSeconds, float fPhaseCorrelationNew)
/*  Set phase correlation and apply meter ballistics.

    fProcessedSeconds (float): length of current buffer chunk in
    fractional seconds

    fPhaseCorrelationNew (float): current phase correlation

    return value: none
*/
{
    // assure that we are processing a stereo audio channel pair
    jassert(nNumberOfChannels == 2);

    // apply meter ballistics
    fPhaseCorrelation = PhaseCorrelationMeterBallistics(fProcessedSeconds, fPhaseCorrelationNew, fPhaseCorrelation);

    // uncomment for validation of correlation meter readings:
    // DBG(String("[K-Meter] Phase correlation: ") + String(fPhaseCorrelation, 2));
}


void MeterBallistics::updateChannel(int nChannel, float fProcessedSeconds, float fPeak, float fAverage, int Overflows)
{
    jassert(nChannel >= 0);
    jassert(nChannel < nNumberOfChannels);

    // please make sure that the values of channel #0 are updated
    // before all others!
    if ((nNumberOfChannels == 1) && (nChannel > 0))
    {
        fPeakMeterLevels[nChannel] = fPeakMeterLevels[0];
        fPeakMeterPeakLevels[nChannel] = fPeakMeterPeakLevels[0];

        fAverageMeterLevels[nChannel] = fAverageMeterLevels[0];
        fAverageMeterPeakLevels[nChannel] = fAverageMeterPeakLevels[0];

        nNumberOfOverflows[nChannel] = nNumberOfOverflows[0];
    }
    else
    {
        fPeak = level2decibel(fPeak);
        fAverage = level2decibel(fAverage) + fAverageCorrection;

        if (fPeak > fMaximumPeakLevels[nChannel])
        {
            fMaximumPeakLevels[nChannel] = fPeak;
        }

        fPeakMeterLevels[nChannel] = PeakMeterBallistics(fProcessedSeconds, fPeak, fPeakMeterLevels[nChannel]);
        fPeakMeterPeakLevels[nChannel] = PeakMeterPeakBallistics(fProcessedSeconds, &fPeakMeterPeakLastChanged[nChannel], fPeak, fPeakMeterPeakLevels[nChannel]);

        fAverageMeterLevels[nChannel] = AverageMeterBallistics(fProcessedSeconds, fAverage, fAverageMeterLevels[nChannel]);
        fAverageMeterPeakLevels[nChannel] = AverageMeterPeakBallistics(fProcessedSeconds, &fAverageMeterPeakLastChanged[nChannel], fAverageMeterLevels[nChannel], fAverageMeterPeakLevels[nChannel]);

        nNumberOfOverflows[nChannel] += Overflows;
    }

    // uncomment for validation of K-System meter peak readings:
    // DBG(String("[K-Meter] K-20 Peak (channel #") + String(nChannel) + T("): ") + String(20.0f + fPeakMeterLevels[nChannel], 2));

    // uncomment for validation of K-System meter average readings:
    // DBG(String("[K-Meter] K-20 Average (channel #") + String(nChannel) + T("): ") + String(20.0f + fAverageMeterLevels[nChannel], 2));
}


float MeterBallistics::level2decibel(float level)
{
    if (level == 0.0f)
    {
        return fMeterMinimumDecibel;
    }
    else
    {
        float output = 20.0f * log10(level);

        if (output < fMeterMinimumDecibel)
        {
            return fMeterMinimumDecibel;
        }
        else
        {
            return output;
        }
    }
}


float MeterBallistics::PeakMeterBallistics(float fProcessedSeconds, float fLevelCurrent, float fLevelOld)
{
    if (fLevelCurrent >= fLevelOld)
    {
        return fLevelCurrent;
    }
    else
    {
        float fReleaseCoef = 26.0f / (3.0f * fProcessedSeconds);
        return fLevelOld - fReleaseCoef;
    }
}


float MeterBallistics::PeakMeterPeakBallistics(float fProcessedSeconds, float* fLastChanged, float fLevelCurrent, float fLevelOld)
{
    float fHoldTime = 10.0f;
    float fOutput = fMeterMinimumDecibel;

    // prevent the meter from overshooting
    if (fLevelCurrent >= 0.0f)
    {
        fLevelCurrent = 0.0f;
    }

    if (fLevelCurrent >= fLevelOld)
    {
        // reset hold time
        if (*fLastChanged >= 0.0f)
        {
            *fLastChanged = 0.0f;
        }

        fOutput = fLevelCurrent;
    }
    else
    {
        // update hold time
        if (*fLastChanged >= 0.0f)
        {
            *fLastChanged += (1.0f / fProcessedSeconds);
        }

        fOutput = fLevelOld;

        if (*fLastChanged > fHoldTime)
        {
            float fReleaseCoef = 26.0f / (3.0f * fProcessedSeconds);
            fOutput -= fReleaseCoef;
        }
    }

    return fOutput;
}


float MeterBallistics::AverageMeterBallistics(float fProcessedSeconds, float fLevelCurrent, float fLevelOld)
{
    // Thanks to Bram from Smartelectronix for the code snippet!
    // (http://www.musicdsp.org/showone.php?id=136)
    float fOutput = fLevelOld / fMeterMinimumDecibel;
    float fTemp = fLevelCurrent / fMeterMinimumDecibel;

    // level has changed
    if (fTemp != fOutput)
    {
        float fAttackReleaseCoef = powf(0.01f, 1.0f / (0.600f * fProcessedSeconds));
        fOutput = fAttackReleaseCoef * (fOutput - fTemp) + fTemp;
    }

    return fOutput * fMeterMinimumDecibel;
}


float MeterBallistics::AverageMeterPeakBallistics(float fProcessedSeconds, float* fLastChanged, float fLevelCurrent, float fLevelOld)
{
    return PeakMeterPeakBallistics(fProcessedSeconds, fLastChanged, fLevelCurrent, fLevelOld);
}


float MeterBallistics::StereoMeterBallistics(float fProcessedSeconds, float fStereoMeterCurrent, float fStereoMeterOld)
{
    // Thanks to Bram from Smartelectronix for the code snippet!
    // (http://www.musicdsp.org/showone.php?id=136)
    float fOutput = fStereoMeterOld;
    float fTemp = fStereoMeterCurrent;

    // level has changed
    if (fTemp != fOutput)
    {
        float fAttackReleaseCoef = powf(0.01f, 1.0f / (1.200f * fProcessedSeconds));
        fOutput = fAttackReleaseCoef * (fOutput - fTemp) + fTemp;
    }

    return fOutput;
}


float MeterBallistics::PhaseCorrelationMeterBallistics(float fProcessedSeconds, float fPhaseCorrelationCurrent, float fPhaseCorrelationOld)
{
    return StereoMeterBallistics(fProcessedSeconds, fPhaseCorrelationCurrent, fPhaseCorrelationOld);
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
