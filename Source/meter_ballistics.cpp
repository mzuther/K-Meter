/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2016 Martin Zuther (http://www.mzuther.de/)

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


// maximum RMS peak-to-average gain correction; for more information,
// see the method AverageLevelFiltered::calculateFilterKernel()
float MeterBallistics::fPeakToAverageCorrection = +2.9881f;

// logarithmic levels have no minimum level, so let's define one:
// (70 dB meter range + 0.01 to make sure that the minimum level is
// below the meter's threshold + 20 dB maximum crest factor +
// peak-to-average gain correction)
float MeterBallistics::fMeterMinimumDecibel = -(70.01f + 20.0f + fPeakToAverageCorrection);


MeterBallistics::MeterBallistics(int nChannels, int AverageAlgorithm, bool bPeakMeterInfiniteHold, bool bAverageMeterInfiniteHold)
/*  Constructor.

    nChannels (integer): number of audio input channels

    AverageAlgorithm (integer): algorithm for calculating average
    meter levels; must be one of the "selAlgorithm..."  values defined
    in "plugin_parameters.h"

    bPeakMeterInfiniteHold (Boolean): selects "infinite peak hold"
    (true) or "falling peaks" mode (false) for peak meter

    bAverageMeterInfiniteHold (Boolean): selects "infinite peak hold"
    (true) or "falling peaks" mode (false) for average meter

    return value: none
*/
{
    // store the number of audio input channels
    nNumberOfChannels = nChannels;

    // store algorithm for average meter levels
    setAverageAlgorithm(AverageAlgorithm);

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
}


void MeterBallistics::reset()
/*  Reset all meter readings

    return value: none
*/
{
    // default phase correlation is "+1.0" (mono-compatible)
    fPhaseCorrelation = 1.0f;

    // default stereo meter value is "0" (centred)
    fStereoMeterValue = 0.0f;

    // loop through all audio channels
    for (int nChannel = 0; nChannel < nNumberOfChannels; ++nChannel)
    {
        // set peak meter's level and peak mark to meter's minimum
        arrPeakMeterLevels.set(nChannel, fMeterMinimumDecibel);
        arrPeakMeterPeakLevels.set(nChannel, fMeterMinimumDecibel);

        // set average meter's level and peak mark to meter's minimum
        arrAverageMeterLevels.set(nChannel, fMeterMinimumDecibel);
        arrAverageMeterPeakLevels.set(nChannel, fMeterMinimumDecibel);

        // set overall maximum peak level to meter's minimum
        arrMaximumPeakLevels.set(nChannel, fMeterMinimumDecibel);

        // reset number of registered overflows
        arrNumberOfOverflows.set(nChannel, 0);
    }
}


void MeterBallistics::setAverageAlgorithm(int AverageAlgorithm)
/*  Set algorithm for calculating average meter levels.

    AverageAlgorithm (integer): algorithm for calculating average
    meter levels; must be one of the "selAlgorithm..."  values defined
    in "plugin_parameters.h"

    return value: none
*/
{
    nAverageAlgorithm = AverageAlgorithm;
}


void MeterBallistics::setPeakMeterInfiniteHold(bool bInfiniteHold)
/*  Set peak meter to "infinite peak hold" or "falling peaks" mode.

    bInfiniteHold (Boolean): selects "infinite peak hold" (true) or
    "falling peaks" mode (false)

    return value: none
*/
{
    // loop through all audio channels
    for (int nChannel = 0; nChannel < nNumberOfChannels; ++nChannel)
    {
        // negative times will not be processed by meter ballistics,
        // so this effectively selects "infinite peak hold" mode
        if (bInfiniteHold)
        {
            arrPeakMeterPeakLastChanged.set(nChannel, -1.0f);
        }
        // select "falling peaks" mode by resetting time since peak
        // mark was last changed
        else
        {
            arrPeakMeterPeakLastChanged.set(nChannel, 0.0f);
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
    for (int nChannel = 0; nChannel < nNumberOfChannels; ++nChannel)
    {
        // negative times will not be processed by meter ballistics,
        // so this effectively selects "infinite peak hold" mode
        if (bInfiniteHold)
        {
            arrAverageMeterPeakLastChanged.set(nChannel, -1.0f);
        }
        // select "falling peaks" mode by resetting time since peak mark
        // was last changed
        else
        {
            arrAverageMeterPeakLastChanged.set(nChannel, 0.0f);
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

    // we only display a single meter in ITU-R BS.1770-1 mode, so
    // we'll have to evaluate the maximum level first
    if (nAverageAlgorithm == KmeterPluginParameters::selAlgorithmItuBs1770)
    {
        // initialise maximum level
        float fPeakMeterLevel = fMeterMinimumDecibel;

        // only return maximum level for the first channel
        if (nChannel == 0)
        {
            // loop through all audio channels to find maximum level
            for (int channel = 0; channel < nNumberOfChannels; ++channel)
            {
                if (arrPeakMeterLevels[channel] > fPeakMeterLevel)
                {
                    fPeakMeterLevel = arrPeakMeterLevels[channel];
                }
            }
        }

        // return maximum level
        return fPeakMeterLevel;
    }
    // otherwise, simply return the requested channel's level
    else
    {
        return arrPeakMeterLevels[nChannel];
    }
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

    // we only display a single meter in ITU-R BS.1770-1 mode, so
    // we'll have to evaluate the maximum peak level first
    if (nAverageAlgorithm == KmeterPluginParameters::selAlgorithmItuBs1770)
    {
        // initialise maximum peak level
        float fPeakMeterPeakLevel = fMeterMinimumDecibel;

        // only return maximum peak level for the first channel
        if (nChannel == 0)
        {
            // loop through all audio channels to find maximum peak
            // level
            for (int channel = 0; channel < nNumberOfChannels; ++channel)
            {
                if (arrPeakMeterPeakLevels[channel] > fPeakMeterPeakLevel)
                {
                    fPeakMeterPeakLevel = arrPeakMeterPeakLevels[channel];
                }
            }
        }

        // return maximum peak level
        return fPeakMeterPeakLevel;
    }
    // otherwise, simply return the requested channel's peak
    // level
    else
    {
        return arrPeakMeterPeakLevels[nChannel];
    }
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

    // we only display a single meter in ITU-R BS.1770-1 mode, so
    // we'll only return an average level for the first channel
    if (nAverageAlgorithm == KmeterPluginParameters::selAlgorithmItuBs1770)
    {
        if (nChannel == 0)
        {
            return arrAverageMeterLevels[nChannel];
        }
        else
        {
            return fMeterMinimumDecibel;
        }
    }
    // otherwise, simply return the requested channel's average level
    else
    {
        return arrAverageMeterLevels[nChannel];
    }
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

    // we only display a single meter in ITU-R BS.1770-1 mode, so
    // we'll only return an average peak level for the first channel
    if (nAverageAlgorithm == KmeterPluginParameters::selAlgorithmItuBs1770)
    {
        if (nChannel == 0)
        {
            return arrAverageMeterPeakLevels[nChannel];
        }
        else
        {
            return fMeterMinimumDecibel;
        }
    }
    // otherwise, simply return the requested channel's average peak
    // level
    else
    {
        return arrAverageMeterPeakLevels[nChannel];
    }
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

    // we only display a single meter in ITU-R BS.1770-1 mode, so
    // we'll have to evaluate the maximum level first
    if (nAverageAlgorithm == KmeterPluginParameters::selAlgorithmItuBs1770)
    {
        // initialise maximum peak level
        float fMaximumPeakLevel = fMeterMinimumDecibel;

        // only return maximum level for the first channel
        if (nChannel == 0)
        {
            // loop through all audio channels to find maximum level
            for (int channel = 0; channel < nNumberOfChannels; ++channel)
            {
                if (arrMaximumPeakLevels[channel] > fMaximumPeakLevel)
                {
                    fMaximumPeakLevel = arrMaximumPeakLevels[channel];
                }
            }
        }

        // return maximum level
        return fMaximumPeakLevel;
    }
    // otherwise, simply return the requested channel's maximum level
    else
    {
        return arrMaximumPeakLevels[nChannel];
    }
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

    // we only display a single meter in ITU-R BS.1770-1 mode, so
    // we'll have to add the number of overflows that occurred in each
    // channel
    if (nAverageAlgorithm == KmeterPluginParameters::selAlgorithmItuBs1770)
    {
        // initialise sum of overflows
        int nSumOfOverflows = 0;

        // only return sum of overflows for the first channel
        if (nChannel == 0)
        {
            // loop through all audio channels to add the number of
            // overflows
            for (int channel = 0; channel < nNumberOfChannels; ++channel)
            {
                nSumOfOverflows += arrNumberOfOverflows[channel];
            }
        }

        // return sum of overflows
        return nSumOfOverflows;
    }
    // otherwise, simply return the requested channel's number of
    // overflows
    else
    {
        return arrNumberOfOverflows[nChannel];
    }
}


float MeterBallistics::getStereoMeterValue()
/*  Get stereo meter value (two input channels only!).

    return value (float): returns the stereo meter value of a stereo
    audio channel pair (-1.0 to 1.0)
*/
{
    // for a stereo audio channel pair, return stereo meter value
    if (nNumberOfChannels == 2)
    {
        return fStereoMeterValue;
    }
    // otherwise, return default stereo meter value of "0" (centred)
    else
    {
        return 0.0f;
    }
}


void MeterBallistics::setStereoMeterValue(float fTimePassed, float fStereoMeterValueNew)
/*  Set stereo meter value and apply meter ballistics (two input
    channels only!).

    fTimePassed (float): time that has passed since last update (in
    fractional seconds)

    fStereoMeterValueNew (float): current stereo meter value (-1.0 to
    1.0)

    return value: none
*/
{
    // assure that we are processing a stereo audio channel pair
    jassert(nNumberOfChannels == 2);

    // apply meter ballistics
    StereoMeterBallistics(fTimePassed, fStereoMeterValueNew);
}


float MeterBallistics::getPhaseCorrelation()
/*  Get phase correlation (two input channels only!).

    return value (float): returns the phase correlation value of a
    stereo audio channel pair (-1.0 to 1.0)
*/
{
    // for a stereo audio channel pair, return phase correlation value
    if (nNumberOfChannels == 2)
    {
        return fPhaseCorrelation;
    }
    // otherwise, return default phase correlation of "+1.0"
    // (mono-compatible)
    else
    {
        return 1.0f;
    }
}


void MeterBallistics::setPhaseCorrelation(float fTimePassed, float fPhaseCorrelationNew)
/*  Set phase correlation and apply meter ballistics (two input
    channels only!).

    fTimePassed (float): time that has passed since last update (in
    fractional seconds)

    fPhaseCorrelationNew (float): current phase correlation (-1.0 to
    1.0)

    return value: none
*/
{
    // assure that we are processing a stereo audio channel pair
    jassert(nNumberOfChannels == 2);

    // apply meter ballistics
    PhaseCorrelationMeterBallistics(fTimePassed, fPhaseCorrelationNew);
}


void MeterBallistics::updateChannel(int nChannel, float fTimePassed, float fPeak, float fRms, float fAverageFiltered, int nOverflows)
/*  Update audio levels, overflows and apply meter ballistics.

    nChannel (integer): audio input channel to update

    fTimePassed (float): time that has passed since last update (in
    fractional seconds)

    fPeak (float): current peak meter level (linear scale)

    fRms (float): current RMS level (linear scale)

    fAverageFiltered (float): current pre-filtered average meter level
    (in decibels!)

    nOverflows (integer): number of overflows in buffer chunk

    return value: none
*/
{
    jassert(nChannel >= 0);
    jassert(nChannel < nNumberOfChannels);

    // convert current peak meter level from linear scale to decibels
    fPeak = level2decibel(fPeak);

    // convert current RMS level from linear scale to decibels
    fRms = level2decibel(fRms);

    // if current peak meter level exceeds overall maximum peak level,
    // store it as new overall maximum peak level
    if (fPeak > arrMaximumPeakLevels[nChannel])
    {
        arrMaximumPeakLevels.set(nChannel, fPeak);
    }

    // apply peak meter's ballistics and store resulting level and
    // peak mark
    arrPeakMeterLevels.set(nChannel, PeakMeterBallistics(fTimePassed, fPeak, arrPeakMeterLevels[nChannel]));
    arrPeakMeterPeakLevels.set(nChannel, PeakMeterPeakBallistics(fTimePassed, arrPeakMeterPeakLastChanged.getReference(nChannel), fPeak, arrPeakMeterPeakLevels[nChannel]));

    // apply average meter's ballistics and store resulting level and
    // peak mark
    AverageMeterBallistics(nChannel, fTimePassed, fAverageFiltered);
    arrAverageMeterPeakLevels.set(nChannel, AverageMeterPeakBallistics(fTimePassed, arrAverageMeterPeakLastChanged.getReference(nChannel), arrAverageMeterLevels[nChannel], arrAverageMeterPeakLevels[nChannel]));

    // update registered number of overflows
    arrNumberOfOverflows.set(nChannel, arrNumberOfOverflows[nChannel] + nOverflows);
}


float MeterBallistics::level2decibel(float fLevel)
/*  Convert level from linear scale to decibels (dB).

    fLevel (float): audio level

    return value (float): returns given level in decibels (dB) when
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
        float fDecibels = 20.0f * log10f(fLevel);

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


float MeterBallistics::decibel2level(float fDecibels)
/*  Convert level from decibels (dB) to linear scale.

    fLevel (float): audio level in decibels (dB)

    return value (float): given level in linear scale
*/
{
    // calculate audio level from decibels (a divisor of 20.0 is
    // needed to calculate *level* ratios, whereas 10.0 is needed for
    // *power* ratios!)
    float fLevel = powf(10.0f, fDecibels / 20.0f);
    return fLevel;
}


float MeterBallistics::getMeterMinimumDecibel()
{
    return fMeterMinimumDecibel;
}


float MeterBallistics::PeakMeterBallistics(float fTimePassed, float fPeakLevelCurrent, float fPeakLevelOld)
/*  Calculate ballistics for peak meter levels.

    fTimePassed (float): time that has passed since last update (in
    fractional seconds)

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
        float fReleaseCoef = 26.0f * fTimePassed / 3.0f;

        // apply fall time and return new peak meter reading
        fPeakLevelOld -= fReleaseCoef;

        // make sure that meter doesn't fall below current level
        if (fPeakLevelCurrent > fPeakLevelOld)
        {
            return fPeakLevelCurrent;
        }
        else
        {
            return fPeakLevelOld;
        }
    }
}


float MeterBallistics::PeakMeterPeakBallistics(float fTimePassed, float &fLastChanged, float fPeakCurrent, float fPeakOld)
/*  Calculate ballistics for peak meter peak marks.

    fTimePassed (float): time that has passed since last update (in
    fractional seconds)

    fLastChanged (reference to float): time since peak mark was last
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
        if (fLastChanged >= 0.0f)
        {
            fLastChanged = 0.0f;
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
        if (fLastChanged >= 0.0f)
        {
            fLastChanged += fTimePassed;
        }

        // peak meter is EITHER set to "infinite peak hold" mode
        // (negative values) OR the peak meter's hold time of 10
        // seconds has not yet been exceeded, so retain old peak level
        // mark
        if (fLastChanged < 10.0f)
        {
            fOutput = fPeakOld;
        }
        // peak meter's hold time (10 seconds) has been reached or
        // exceeded
        else
        {
            // fall time: 26 dB in 3 seconds (linear)
            float fReleaseCoef = 26.0f * fTimePassed / 3.0f;

            // apply fall time
            fOutput = fPeakOld - fReleaseCoef;

            // make sure that meter doesn't fall below current level
            if (fPeakCurrent > fOutput)
            {
                fOutput = fPeakCurrent;
            }
        }
    }

    // finally, return new peak level mark
    return fOutput;
}


void MeterBallistics::AverageMeterBallistics(int nChannel, float fTimePassed, float fAverageLevelCurrent)
/*  Calculate ballistics for average meter levels and update readout.

    fTimePassed (float): time that has passed since last update (in
    fractional seconds)

    fAverageLevelCurrent (float): current average meter level in
    decibel

    return value: none
*/
{
    // meter ballistics: 99% of final reading in 0.6 s (logarithmic)
    LogMeterBallistics(0.600f, fTimePassed, fAverageLevelCurrent, arrAverageMeterLevels.getReference(nChannel));
}


float MeterBallistics::AverageMeterPeakBallistics(float fTimePassed, float &fLastChanged, float fPeakCurrent, float fPeakOld)
/*  Calculate ballistics for average meter peak marks.

    fTimePassed (float): time that has passed since last update (in
    fractional seconds)

    fLastChanged (reference to float): time since peak mark was last
    changed in fractional seconds

    fPeakCurrent (float): current peak level mark in decibel

    fPeakOld (float): old peak level mark in decibel

    return value (float): new peak level mark in decibel
*/
{
    // the peak marks ballistics of peak meter and average meter are
    // identical, so let's reuse the peak meter code
    return PeakMeterPeakBallistics(fTimePassed, fLastChanged, fPeakCurrent, fPeakOld);
}


void MeterBallistics::StereoMeterBallistics(float fTimePassed, float fStereoMeterCurrent)
/*  Calculate ballistics for stereo meter values and update readout.

    fTimePassed (float): time that has passed since last update (in
    fractional seconds)

    fStereoMeterCurrent (float): current stereo meter value

    return value: none
*/
{
    // meter ballistics: 99% of final reading in 1.2 s (logarithmic)
    LogMeterBallistics(1.200f, fTimePassed, fStereoMeterCurrent, fStereoMeterValue);
}


void MeterBallistics::PhaseCorrelationMeterBallistics(float fTimePassed, float fPhaseCorrelationCurrent)
/*  Calculate ballistics for phase correlation meter values and update
    readout.

    fTimePassed (float): time that has passed since last update (in
    fractional seconds)

    fPhaseCorrelationCurrent (float): current phase correlation meter
    value

    return value: none
*/
{
    // meter ballistics: 99% of final reading in 1.2 s (logarithmic)
    LogMeterBallistics(1.200f, fTimePassed, fPhaseCorrelationCurrent, fPhaseCorrelation);
}


void MeterBallistics::LogMeterBallistics(float fMeterInertia, float fTimePassed, float fLevel, float &fReadout)
/*  Calculate logarithmic meter ballistics.

    fMeterInertia (float): time needed to reach 99% of the final
    readout (in fractional seconds)

    fTimePassed (float): time that has passed since last update (in
    fractional seconds)

    fLevel (float): new meter level

    fReadout (reference to float): old meter readout; this variable
    will be updated by this function

    return value: none
*/
{
    // we only have to calculate meter ballistics if meter level and
    // meter readout are not equal
    if (fLevel != fReadout)
    {
        // Thanks to Bram de Jong for the code snippet!
        // (http://www.musicdsp.org/showone.php?id=136)
        //
        // rise and fall: 99% of final reading in "fMeterInertia" seconds
        float fAttackReleaseCoef = powf(0.01f, fTimePassed / fMeterInertia);
        fReadout = fAttackReleaseCoef * (fReadout - fLevel) + fLevel;
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
