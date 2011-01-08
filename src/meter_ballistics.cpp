/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2011 Martin Zuther (http://www.mzuther.de/)

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


MeterBallistics::MeterBallistics(int nChannels, bool bPeakMeterInfiniteHold, bool bAverageMeterInfiniteHold)
/*  Constructor.

    nChannels (integer): number of audio input channels

    bPeakMeterInfiniteHold (Boolean): selects "infinite peak hold"
    (true) or "falling peaks" mode (false) for peak meter

    bAverageMeterInfiniteHold (Boolean): selects "infinite peak hold"
    (true) or "falling peaks" mode (false) for average meter

    return value: none
*/
{
    // the K-20 meter has the highest maximum crest factor (20 dB) of
    // all K-System meters
    float fMaximumCrestFactor = 20.0f;

    // the RMS of a sine wave is its amplitude divided by the square
    // root of 2, thus the difference between peak value and RMS is
    // the square root of 2 -- so let's convert this difference to dB
    // and store the result (a factor of 20.0 is needed to calculate
    // *level* ratios, whereas 10.0 is needed for *power* ratios!)
    fPeakToAverageCorrection = 20.0f * log10(sqrt(2.0f));

    // logarithmic levels have no minimum level, so let's define one
    // (70 dB meter range + maximum crest factor + peak-to-average
    // correction) and store it for later use
    fMeterMinimumDecibel = -(70.0f + fMaximumCrestFactor + fPeakToAverageCorrection);

    // store the number of audio input channels
    nNumberOfChannels = nChannels;

    // allocate variables for peak meter's level and peak mark (all
    // audio input channels)
    fPeakMeterLevels = new float[nNumberOfChannels];
    fPeakMeterPeakLevels = new float[nNumberOfChannels];

    // allocate variables for average meter's level and peak mark (all
    // audio input channels)
    fAverageMeterLevels = new float[nNumberOfChannels];
    fAverageMeterPeakLevels = new float[nNumberOfChannels];

    // allocate variables for the time since the peak mark was last
    // changed (all audio input channels)
    fPeakMeterPeakLastChanged = new float[nNumberOfChannels];
    fAverageMeterPeakLastChanged = new float[nNumberOfChannels];

    // allocate variables for overall maximum peak level and number of
    // registered overflows (all audio input channels)
    fMaximumPeakLevels = new float[nNumberOfChannels];
    nNumberOfOverflows = new int[nNumberOfChannels];

    // select "infinite peak hold" or "falling peaks" mode
    setPeakMeterInfiniteHold(bPeakMeterInfiniteHold);
    setAverageMeterInfiniteHold(bAverageMeterInfiniteHold);

    // reset (i.e. initialise) all meter readings
    reset();
}


MeterBallistics::~MeterBallistics()
/*  Destructor.

    return value: none
*/
{
    // delete all allocated variables
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
/*  Reset all meter readings.

    return value: none
*/
{
    // default phase correlation is "+1.0" (mono-compatible)
    fPhaseCorrelation = 1.0f;

    // default stereo meter value is "0" (centred)
    fStereoMeterValue = 0.0f;

    // loop through all audio channels
    for (int nChannel = 0; nChannel < nNumberOfChannels; nChannel++)
    {
        // set peak meter's level and peak mark to meter's minimum
        fPeakMeterLevels[nChannel] = fMeterMinimumDecibel;
        fPeakMeterPeakLevels[nChannel] = fMeterMinimumDecibel;

        // set average meter's level and peak mark to meter's minimum
        // and apply peak-to-average correction so that sine waves
        // read the same value on peak and average meters
        fAverageMeterLevels[nChannel] = fMeterMinimumDecibel + fPeakToAverageCorrection;
        fAverageMeterPeakLevels[nChannel] = fMeterMinimumDecibel + fPeakToAverageCorrection;

        // set overall maximum peak level to meter's minimum
        fMaximumPeakLevels[nChannel] = fMeterMinimumDecibel;

        // reset number of registered overflows
        nNumberOfOverflows[nChannel] = 0;
    }
}


void MeterBallistics::setPeakMeterInfiniteHold(bool bInfiniteHold)
/*  Set peak meter to "infinite peak hold" or "falling peaks" mode.

    bInfiniteHold (Boolean): selects "infinite peak hold" (true) or
    "falling peaks" mode (false)

    return value: none
*/
{
    // loop through all audio channels
    for (int nChannel = 0; nChannel < nNumberOfChannels; nChannel++)
    {
        // negative times will not be processed by meter ballistics,
        // so this effectively selects "infinite peak hold" mode
        if (bInfiniteHold)
        {
            fPeakMeterPeakLastChanged[nChannel] = -1.0f;
        }
        // select "falling peaks" mode by resetting time since peak
        // mark was last changed
        else
        {
            fPeakMeterPeakLastChanged[nChannel] = 0.0f;
        }
    }
}


void MeterBallistics::setAverageMeterInfiniteHold(bool bInfiniteHold)
/*  Set average meter to "infinite peak hold" or "falling peaks" mode.

    bInfiniteHold (Boolean): selects "infinite peak hold" (true) or
    "falling peaks" mode (false)

    return value: none
*/
{
    // loop through all audio channels
    for (int nChannel = 0; nChannel < nNumberOfChannels; nChannel++)
    {
        // negative times will not be processed by meter ballistics,
        // so this effectively selects "infinite peak hold" mode
        if (bInfiniteHold)
        {
            fAverageMeterPeakLastChanged[nChannel] = -1.0f;
        }
        // select "falling peaks" mode by resetting time since peak mark
        // was last changed
        else
        {
            fAverageMeterPeakLastChanged[nChannel] = 0.0f;
        }
    }
}


int MeterBallistics::getNumberOfChannels()
/*  Get number of audio channels.

    return value (integer): returns the current number of processed
    audio channels
*/
{
    return nNumberOfChannels;
}


float MeterBallistics::getPeakMeterLevel(int nChannel)
/*  Get current level of an audio channel's peak level meter.

    nChannel (integer): selected audio channel

    return value (float): returns the current level in decibel of the
    given audio channel's peak level meter
*/
{
    jassert(nChannel >= 0);
    jassert(nChannel < nNumberOfChannels);

    return fPeakMeterLevels[nChannel];
}


float MeterBallistics::getPeakMeterPeakLevel(int nChannel)
/*  Get peak level of an audio channel's peak level meter.

    nChannel (integer): selected audio channel

    return value (float): returns the (changing) peak level in decibel
    of the given audio channel's peak level meter
*/
{
    jassert(nChannel >= 0);
    jassert(nChannel < nNumberOfChannels);

    return fPeakMeterPeakLevels[nChannel];
}


float MeterBallistics::getAverageMeterLevel(int nChannel)
/*  Get current level of an audio channel's average level meter.

    nChannel (integer): selected audio channel

    return value (float): returns the current level in decibel of the
    given audio channel's average level meter
*/
{
    jassert(nChannel >= 0);
    jassert(nChannel < nNumberOfChannels);

    return fAverageMeterLevels[nChannel];
}


float MeterBallistics::getAverageMeterPeakLevel(int nChannel)
/*  Get peak level of an audio channel's average level meter.

    nChannel (integer): selected audio channel

    return value (float): returns the (changing) peak level in decibel
    of the given audio channel's average level meter
*/
{
    jassert(nChannel >= 0);
    jassert(nChannel < nNumberOfChannels);

    return fAverageMeterPeakLevels[nChannel];
}


float MeterBallistics::getMaximumPeakLevel(int nChannel)
/*  Get overall maximum peak level of an audio channel.

    nChannel (integer): selected audio channel

    return value (float): returns the overall maximum peak level in
    decibel that has been registered on the given audio channel
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
/*  Get stereo meter value (two input channels only!).

    return value (float): returns the stereo meter value of a stereo
    audio channel pair (-1.0 to 1.0)
*/
{
    // assure that we are processing a stereo audio channel pair
    jassert(nNumberOfChannels == 2);

    return fStereoMeterValue;
}


void MeterBallistics::setStereoMeterValue(float fProcessedSeconds, float fStereoMeterValueNew)
/*  Set stereo meter value and apply meter ballistics (two input
    channels only!).

    fProcessedSeconds (float): length of current buffer chunk in
    fractional seconds

    fStereoMeterValueNew (float): current stereo meter value (-1.0 to
    1.0)

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
/*  Get phase correlation (two input channels only!).

    return value (float): returns the phase correlation value of a
    stereo audio channel pair (-1.0 to 1.0)
*/
{
    // assure that we are processing a stereo audio channel pair
    jassert(nNumberOfChannels == 2);

    return fPhaseCorrelation;
}


void MeterBallistics::setPhaseCorrelation(float fProcessedSeconds, float fPhaseCorrelationNew)
/*  Set phase correlation and apply meter ballistics (two input
    channels only!).

    fProcessedSeconds (float): length of current buffer chunk in
    fractional seconds

    fPhaseCorrelationNew (float): current phase correlation (-1.0 to
    1.0)

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


void MeterBallistics::updateChannel(int nChannel, float fProcessedSeconds, float fPeak, float fAverage, int nOverflows)
/*  Update audio levels, overflows and apply meter ballistics.

    nChannel (integer): audio input channel to update

    fProcessedSeconds (float): length of current buffer chunk in
    fractional seconds

    fPeak (float): current peak meter level (linear scale)

    fAverage (float): current average meter level (linear scale)

    nOverflows (integer): number of overflows in buffer chunk

    return value: none
*/
{
    jassert(nChannel >= 0);
    jassert(nChannel < nNumberOfChannels);

    // convert current peak meter level from linear scale to decibels
    fPeak = level2decibel(fPeak);

    // convert current average meter level from linear scale to
    // decibels and apply peak-to-average correction so that sine
    // waves give the same read-out on peak and average meters
    fAverage = level2decibel(fAverage) + fPeakToAverageCorrection;

    // if current peak meter level exceeds overall maximum peak level,
    // store it as new overall maximum peak level
    if (fPeak > fMaximumPeakLevels[nChannel])
    {
        fMaximumPeakLevels[nChannel] = fPeak;
    }

    // apply peak meter's ballistics and store resulting level and
    // peak mark
    fPeakMeterLevels[nChannel] = PeakMeterBallistics(fProcessedSeconds, fPeak, fPeakMeterLevels[nChannel]);
    fPeakMeterPeakLevels[nChannel] = PeakMeterPeakBallistics(fProcessedSeconds, &fPeakMeterPeakLastChanged[nChannel], fPeak, fPeakMeterPeakLevels[nChannel]);

    // apply average meter's ballistics and store resulting level and
    // peak mark
    fAverageMeterLevels[nChannel] = AverageMeterBallistics(fProcessedSeconds, fAverage, fAverageMeterLevels[nChannel]);
    fAverageMeterPeakLevels[nChannel] = AverageMeterPeakBallistics(fProcessedSeconds, &fAverageMeterPeakLastChanged[nChannel], fAverageMeterLevels[nChannel], fAverageMeterPeakLevels[nChannel]);

    // update registered number of overflows
    nNumberOfOverflows[nChannel] += nOverflows;

    // uncomment for validation of K-System meter peak readings:
    // if (nChannel == 0)
    //     DBG(String("[K-Meter] K-20 Peak (channel #") + String(nChannel) + T("): ") + String(20.0f + fPeakMeterLevels[nChannel], 2));

    // uncomment for validation of K-System meter average readings:
    // if (nChannel == 0)
    //     DBG(String("[K-Meter] K-20 Average (channel #") + String(nChannel) + T("): ") + String(20.0f + fAverageMeterLevels[nChannel], 2));
}


float MeterBallistics::level2decibel(float fLevel)
/*  Convert level from linear scale to decibels (dB).

    fLevel (float): audio level

    return value (float): return given level in decibels (dB) when
    above "fMeterMinimumDecibel", otherwise "fMeterMinimumDecibel"
*/
{
    // log(0) is not defined, so return "fMeterMinimumDecibel"
    if (fLevel == 0.0f)
    {
        return fMeterMinimumDecibel;
    }
    else
    {
        // calculate decibels from audio level (a factor of 20.0 is
        // needed to calculate *level* ratios, whereas 10.0 is needed
        // for *power* ratios!)
        float fDecibels = 20.0f * log10(fLevel);

        // to make meter ballistics look nice for low levels, do not
        // return levels below "fMeterMinimumDecibel"
        if (fDecibels < fMeterMinimumDecibel)
        {
            return fMeterMinimumDecibel;
        }
        else
        {
            return fDecibels;
        }
    }
}


float MeterBallistics::PeakMeterBallistics(float fProcessedSeconds, float fPeakLevelCurrent, float fPeakLevelOld)
/*  Calculate ballistics for peak meter levels.

    fProcessedSeconds (float): length of current buffer chunk in
    fractional seconds

    fPeakLevelCurrent (float): current peak meter level in decibel

    fPeakLevelOld (float): old peak meter reading in decibel

    return value (float): new peak meter reading in decibel
*/
{
    // apply rise time if peak level is above old level
    if (fPeakLevelCurrent >= fPeakLevelOld)
    {
        // immediate rise time, so return current peak level as new
        // peak meter reading
        return fPeakLevelCurrent;
    }
    // otherwise, apply fall time
    else
    {
        // fall time: 26 dB in 3 seconds (linear)
        float fReleaseCoef = 26.0f * fProcessedSeconds / 3.0f;

        // apply fall time and return new peak meter reading
        return fPeakLevelOld - fReleaseCoef;
    }
}


float MeterBallistics::PeakMeterPeakBallistics(float fProcessedSeconds, float* fLastChanged, float fPeakCurrent, float fPeakOld)
/*  Calculate ballistics for peak meter peak marks.

    fProcessedSeconds (float): length of current buffer chunk in
    fractional seconds

    fLastChanged (float pointer): time since peak mark was last
    changed in fractional seconds

    fPeakCurrent (float): current peak level mark in decibel

    fPeakOld (float): old peak level mark in decibel

    return value (float): new peak level mark in decibel
*/
{
    float fOutput;

    // prevent meter overshoot on overflows by limiting peak levels to
    // 0.0 dBFS
    if (fPeakCurrent >= 0.0f)
    {
        fPeakCurrent = 0.0f;
    }

    // apply rise time if peak level is above old level
    if (fPeakCurrent >= fPeakOld)
    {
        // if peak meter is set to "falling peaks" mode (non-negative
        // values), reset hold time (time that peaks are held before
        // starting to fall back down)
        if (*fLastChanged >= 0.0f)
        {
            *fLastChanged = 0.0f;
        }

        // immediate rise time, so set current peak level mark as new
        // peak level mark
        fOutput = fPeakCurrent;
    }
    // otherwise, test whether to apply fall time
    else
    {
        // if peak meter is set to "falling peaks" mode (non-negative
        // values), update hold time (time that peaks are held before
        // starting to fall back down)
        if (*fLastChanged >= 0.0f)
        {
            *fLastChanged += fProcessedSeconds;
        }

        // peak meter is EITHER set to "infinite peak hold" mode
        // (negative values) OR the peak meter's hold time of 10
        // seconds has not yet been exceeded, so retain old peak level
        // mark
        if (*fLastChanged < 10.0f)
        {
            fOutput = fPeakOld;
        }
        // peak meter's hold time (10 seconds) has been reached or
        // exceeded
        else
        {
            // fall time: 26 dB in 3 seconds (linear)
            float fReleaseCoef = 26.0f * fProcessedSeconds / 3.0f;

            // apply fall time
            fOutput = fPeakOld - fReleaseCoef;
        }
    }

    // finally, return new peak level mark
    return fOutput;
}


float MeterBallistics::AverageMeterBallistics(float fProcessedSeconds, float fAverageLevelCurrent, float fAverageLevelOld)
/*  Calculate ballistics for average meter levels.

    fProcessedSeconds (float): length of current buffer chunk in
    fractional seconds

    fAverageLevelCurrent (float): current average meter level in
    decibel

    fAverageLevelOld (float): old average meter reading in decibel

    return value (float): new average meter reading in decibel
*/
{
    // if current average level and old meter reading are equal, we
    // may skip the ballistics and simply return the current level
    if (fAverageLevelCurrent == fAverageLevelOld)
    {
        return fAverageLevelCurrent;
    }
    // otherwise, calculate meter ballistics for current level
    else
    {
        // Thanks to Bram from Smartelectronix for the code snippet!
        // (http://www.musicdsp.org/showone.php?id=136)
        //
        // rise and fall: 99% of final reading in 0.6 s (logarithmic)
        float fAttackReleaseCoef = powf(0.01f, fProcessedSeconds / 0.600f);
        float fOutput = fAttackReleaseCoef * (fAverageLevelOld - fAverageLevelCurrent) + fAverageLevelCurrent;

        return fOutput;
    }
}


float MeterBallistics::AverageMeterPeakBallistics(float fProcessedSeconds, float* fLastChanged, float fPeakCurrent, float fPeakOld)
/*  Calculate ballistics for average meter peak marks.

    fProcessedSeconds (float): length of current buffer chunk in
    fractional seconds

    fLastChanged (float pointer): time since peak mark was last
    changed in fractional seconds

    fPeakCurrent (float): current peak level mark in decibel

    fPeakOld (float): old peak level mark in decibel

    return value (float): new peak level mark in decibel
*/
{
    // the peak marks ballistics of peak meter and average meter are
    // identical, so let's reuse the peak meter code
    return PeakMeterPeakBallistics(fProcessedSeconds, fLastChanged, fPeakCurrent, fPeakOld);
}


float MeterBallistics::StereoMeterBallistics(float fProcessedSeconds, float fStereoMeterCurrent, float fStereoMeterOld)
/*  Calculate ballistics for stereo meter values.

    fProcessedSeconds (float): length of current buffer chunk in
    fractional seconds

    fStereoMeterCurrent (float): current stereo meter value

    fStereoMeterOld (float): old stereo meter value

    return value (float): new stereo meter value
*/
{
    // if current stereo meter value and old meter reading are equal,
    // we may skip the ballistics and simply return the current value
    if (fStereoMeterCurrent == fStereoMeterOld)
    {
        return fStereoMeterCurrent;
    }
    // otherwise, calculate meter ballistics for current value
    else
    {
        // Thanks to Bram from Smartelectronix for the code snippet!
        // (http://www.musicdsp.org/showone.php?id=136)
        //
        // rise and fall: 99% of final reading in 1.2 s (logarithmic)
        float fAttackReleaseCoef = powf(0.01f, fProcessedSeconds / 1.200f);
        float fOutput = fAttackReleaseCoef * (fStereoMeterOld - fStereoMeterCurrent) + fStereoMeterCurrent;

        return fOutput;
    }
}


float MeterBallistics::PhaseCorrelationMeterBallistics(float fProcessedSeconds, float fPhaseCorrelationCurrent, float fPhaseCorrelationOld)
/*  Calculate ballistics for phase correlation meter values.

    fProcessedSeconds (float): length of current buffer chunk in
    fractional seconds

    fPhaseCorrelationCurrent (float): current phase correlation meter
    value

    fPhaseCorrelationOld (float): old phase correlation meter value

    return value (float): new phase correlation meter value
*/
{
    // the ballistics of stereo meter and phase correlation meter are
    // identical, so let's reuse the stereo meter code
    return StereoMeterBallistics(fProcessedSeconds, fPhaseCorrelationCurrent, fPhaseCorrelationOld);
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
