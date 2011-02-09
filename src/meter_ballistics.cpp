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


MeterBallistics::MeterBallistics(int nChannels, int nSampleRate, bool bPeakMeterInfiniteHold, bool bAverageMeterInfiniteHold)
/*  Constructor.

    nChannels (integer): number of audio input channels

    nSampleRate (integer): current sample rate (in Hz)

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

    // all dynamic range histograms span a time of three seconds when
    // combined; set maximum counts per single histogram accordingly
    nHistogramMaximumCounts = 3 * nSampleRate * nNumberOfChannels / (KMETER_BUFFER_SIZE * NUMBER_OF_HISTOGRAMS);

    // we will only ever use the top 20 % (1/5) of the combined
    // histograms to calculate the dynamic range value
    nHistogramTopTwentyCounts = nHistogramMaximumCounts * NUMBER_OF_HISTOGRAMS / 5;

    // allocate the histograms (average and peak levels) needed for
    // calculating the dynamic range value
    fAverageLevelHistogram = new unsigned short*[NUMBER_OF_HISTOGRAMS];
    fPeakLevelHistogram = new unsigned short*[NUMBER_OF_HISTOGRAMS];
    bHistogramIsValid = new bool[NUMBER_OF_HISTOGRAMS];

    // allocate histogram bins for calculation of the dynamic range
    // value; levels are separated into bins of 0.01 dB each and run
    // from -100.0 dBFS to 0.0 dBFS; to minimise overhead, histogram
    // indices are calculated as (-100 * level)
    for (int nHistogram = 0; nHistogram < NUMBER_OF_HISTOGRAMS; nHistogram++)
    {
        fAverageLevelHistogram[nHistogram] = new unsigned short[HISTOGRAM_BINS];
        fPeakLevelHistogram[nHistogram] = new unsigned short[HISTOGRAM_BINS];
    }

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

    // delete memory allocated for histogram bins
    for (int nHistogram = 0; nHistogram < NUMBER_OF_HISTOGRAMS; nHistogram++)
    {
        delete [] fAverageLevelHistogram[nHistogram];
        fAverageLevelHistogram[nHistogram] = NULL;

        delete [] fPeakLevelHistogram[nHistogram];
        fPeakLevelHistogram[nHistogram] = NULL;
    }

    // delete memory allocated for histograms
    delete [] fAverageLevelHistogram;
    fAverageLevelHistogram = NULL;

    delete [] fPeakLevelHistogram;
    fPeakLevelHistogram = NULL;

    delete [] bHistogramIsValid;
    bHistogramIsValid = NULL;

    delete [] nNumberOfOverflows;
    nNumberOfOverflows = NULL;
}


void MeterBallistics::reset()
/*  Reset all meter readings and dynamic range histograms.

    return value: none
*/
{
    // default phase correlation is "+1.0" (mono-compatible)
    fPhaseCorrelation = 1.0f;

    // default stereo meter value is "0" (centred)
    fStereoMeterValue = 0.0f;

    // default dynamic range value is "-1" (invalid)
    nDynamicRangeValue = -1;

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

    // reset all dynamic range histograms
    resetDynamicRangeHistogram(true);
}


void MeterBallistics::resetDynamicRangeHistogram(bool bResetAllHistograms)
/*  Reset dynamic range histogram(s) and select the one that is filled
    next.

    bResetAllHistograms (Boolean): specifies whether to reset all
    histograms and select the first one (true) or to select the next
    one and reset it accordingly (false).

    return value: none
*/
{
    // reset all histograms
    if (bResetAllHistograms)
    {
        // select the first histogram for use
        nCurrentHistogram = 0;

        // loop through all histograms
        for (int nHistogram = 0; nHistogram < NUMBER_OF_HISTOGRAMS; nHistogram++)
        {
            // loop through histogram bins
            for (int nBin = 0; nBin < HISTOGRAM_BINS; nBin++)
            {
                // reset selected histogram bin (average and peak
                // level)
                fAverageLevelHistogram[nHistogram][nBin] = 0;
                fPeakLevelHistogram[nHistogram][nBin] = 0;
            }

            // to avoid nonsense values, the dynamic range value is
            // only calculated when the peak level histogram contains
            // levels of at least -40 dBFS
            bHistogramIsValid[nHistogram] = false;
        }
    }
    // reset only the next histogram
    else
    {
        // select the next histogram for use
        nCurrentHistogram = (nCurrentHistogram + 1) % NUMBER_OF_HISTOGRAMS;

        // loop through bins of current histogram
        for (int nBin = 0; nBin < HISTOGRAM_BINS; nBin++)
        {
            // reset selected histogram bin (average and peak level)
            fAverageLevelHistogram[nCurrentHistogram][nBin] = 0;
            fPeakLevelHistogram[nCurrentHistogram][nBin] = 0;
        }

        // to avoid nonsense values, the dynamic range value is only
        // calculated when the peak level histogram contains levels of
        // at least -40 dBFS
        bHistogramIsValid[nCurrentHistogram] = false;
    }

    // reset item counter of currently selected histogram(s) which is
    // needed so we know when a histogram is ready for processing
    // (both histograms will be filled at the same time with the same
    // amount of items)
    nHistogramCurrentCounts = 0;
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


int MeterBallistics::getDynamicRangeValue()
/*  Get the dynamic range value.

    return value (integer): returns dynamic range value
*/
{
    return nDynamicRangeValue;
}


void MeterBallistics::calculateDynamicRangeValue()
/*  Calculate the dynamic range value and select the next histogram.

    return value: void
*/
{
    // loop through all histograms
    for (int nHistogram = 0; nHistogram < NUMBER_OF_HISTOGRAMS; nHistogram++)
    {
        // to avoid nonsense values, the dynamic range value is only
        // calculated when all peak level histograms contain levels of
        // at least -40 dBFS
        if (!bHistogramIsValid[nHistogram])
        {
            // select and reset next dynamic range histogram(s), ...
            resetDynamicRangeHistogram(false);

            // ..., set dynamic range to "-1" (invalid) ...
            nDynamicRangeValue = -1;

            // ... and leave
            return;
        }
    }

    // histogram bin containing the highest recorded peak level
    int nHighestPeakBin = 0;

    // histogram bin containing the next-to-highest peak level
    int nHighestPeakBin_2 = 0;

    // this variable will break nested loops if set to true
    bool bBreakLoop = false;

    // to find the highest recorded peak level, loop through all
    // histogram bins in the order of decreasing level
    for (nHighestPeakBin = 0; nHighestPeakBin < HISTOGRAM_BINS; nHighestPeakBin++)
    {
        // loop through all histograms
        for (int nHistogram = 0; nHistogram < NUMBER_OF_HISTOGRAMS; nHistogram++)
        {
            // we have found the highest recorded peak level
            if (fPeakLevelHistogram[nHistogram][nHighestPeakBin] > 0)
            {
                // break underlying loop
                bBreakLoop = true;

                // break current loop
                break;
            }
        }

        // break this loop if requested
        if (bBreakLoop)
        {
            break;
        }
    }

    // if there is not even a single filled histogram bin, we have
    // encountered a bug!
    if (nHighestPeakBin >= HISTOGRAM_BINS)
    {
        jassert(nHighestPeakBin < HISTOGRAM_BINS);
    }
    // otherwise, find the next-to-highest recorded peak level
    else
    {
        // do not break nested loops yet
        bBreakLoop = false;

        // loop through all histogram bins in the order of decreasing
        // level, starting with the highest recorded peak level
        for (nHighestPeakBin_2 = nHighestPeakBin + 1; nHighestPeakBin_2 < HISTOGRAM_BINS; nHighestPeakBin_2++)
        {
            // loop through all histograms
            for (int nHistogram = 0; nHistogram < NUMBER_OF_HISTOGRAMS; nHistogram++)
            {
                // we have found the next-to-highest recorded peak level
                if (fPeakLevelHistogram[nHistogram][nHighestPeakBin_2] > 0)
                {
                    // break underlying loop
                    bBreakLoop = true;

                    // break current loop
                    break;
                }
            }

            // break this loop if requested
            if (bBreakLoop)
            {
                break;
            }
        }

        // safeguard for the unlikely case that there is only a
        // single filled histogram bin
        if (nHighestPeakBin_2 >= HISTOGRAM_BINS)
        {
            nHighestPeakBin_2 = nHighestPeakBin;
        }
    }

    // convert histogram bin to next-to-highest peak level; bins are
    // calculated as (-100 * level); we need the *negated* value to
    // calculate the dynamic range value
    float fHighestPeakLevel_2 = nHighestPeakBin_2 / 100.0f;

    // initialise sum of squared average levels
    float fSumOfSquaredAverageLevels = 0.0f;

    // initialise total number of processed counts
    int nProcessedCounts = 0;

    // loop through all histogram bins in the order of decreasing
    // level
    for (int nBin = 0; nBin < HISTOGRAM_BINS; nBin++)
    {
        // this variable will contain the overall number of counts in
        // a single bin (found in all average level histograms)
        int nNumberOfCounts = 0;

        // loop through all histograms
        for (int nHistogram = 0; nHistogram < NUMBER_OF_HISTOGRAMS; nHistogram++)
        {
            // add the current histogram's counts to overall number of
            // counts
            nNumberOfCounts += fAverageLevelHistogram[nHistogram][nBin];
        }

        // process bin only if it contains counts
        if (nNumberOfCounts > 0)
        {
            // the number of histogram counts exceeds the needed
            // number of counts
            if ((nProcessedCounts + nNumberOfCounts) > nHistogramTopTwentyCounts)
            {
                // process exactly the needed number of histogram
                // counts
                nNumberOfCounts = nHistogramTopTwentyCounts - nProcessedCounts;
            }

            // convert histogram bin to average level and square the
            // result; bins are calculated as (-100 * level)
            float fAverageSquared = nBin * nBin / 10000.0f;

            // update the sum of squared average levels
            fSumOfSquaredAverageLevels += (nNumberOfCounts * fAverageSquared);

            // add number of counts to the total number of processed
            // counts
            nProcessedCounts += nNumberOfCounts;
        }

        // we have processed the needed number of histogram counts, so
        // break the loop
        if (nProcessedCounts >= nHistogramTopTwentyCounts)
        {
            break;
        }
    }

    // calculate the dynamic range value; I have placed the value of
    // 2.0f here rather than in the RMS calculation in order to make
    // the calculations faster (it's mathematically equivalent)
    float fDynamicRangeValue = sqrt(2.0f * fSumOfSquaredAverageLevels / nProcessedCounts) / fHighestPeakLevel_2;

    // I guess the value -20.0f found in the dynamic range meter
    // specification should be 20.0f, so let's use level2decibel()
    fDynamicRangeValue = level2decibel(fDynamicRangeValue);

    // finish calculation of the dynamic range value and store the
    // rounded result
    nDynamicRangeValue = (int)(fDynamicRangeValue + 0.5f);

    // select and reset next dynamic range histogram(s)
    resetDynamicRangeHistogram(false);
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

    // uncomment for validation of correlation meter readings:
    // DBG(String("[K-Meter] Phase correlation: ") + String(fPhaseCorrelation, 2));
}


void MeterBallistics::updateChannel(int nChannel, float fTimePassed, float fPeak, float fAverage, float fAverageFiltered, int nOverflows)
/*  Update audio levels, overflows and apply meter ballistics.

    nChannel (integer): audio input channel to update

    fTimePassed (float): time that has passed since last update (in
    fractional seconds)

    fPeak (float): current peak meter level (linear scale)

    fAverage (float): current average meter level (linear scale)

    fAverageFiltered (float): current pre-filtered average meter level
    (linear scale)

    nOverflows (integer): number of overflows in buffer chunk

    return value: none
*/
{
    jassert(nChannel >= 0);
    jassert(nChannel < nNumberOfChannels);

    // convert current peak meter level from linear scale to decibels
    fPeak = level2decibel(fPeak);

    // convert current average meter level from linear scale to
    // decibels
    fAverage = level2decibel(fAverage);

    // convert current filtered average meter level from linear scale
    // to decibels and apply peak-to-average correction so that sine
    // waves give the same read-out on peak and average meters
    fAverageFiltered = level2decibel(fAverageFiltered) + fPeakToAverageCorrection;

    // convert (unfiltered) average level to histogram bin; to
    // minimise overhead, bins are calculated as (-100 * level)
    int nAverageBin = (int)(-100.0f * fAverage);

    // limit average level histogram bins to array indices
    if (nAverageBin < 0)
    {
        nAverageBin = 0;
    }
    else if (nAverageBin >= HISTOGRAM_BINS)
    {
        nAverageBin = HISTOGRAM_BINS - 1;
    }

    // increment counts of selected bin (average level)
    fAverageLevelHistogram[nCurrentHistogram][nAverageBin]++;

    // convert peak level to histogram bin
    int nPeakBin = (int)(-100.0f * fPeak);

    // limit peak level histogram bins to array indices
    if (nPeakBin < 0)
    {
        nPeakBin = 0;
    }
    else if (nPeakBin >= HISTOGRAM_BINS)
    {
        nPeakBin = HISTOGRAM_BINS - 1;
    }

    // increment counts of selected bin (peak level)
    fPeakLevelHistogram[nCurrentHistogram][nPeakBin]++;

    // "activate" current histograms if the current peak level is at
    // least -40 dBFS (and they are not already "active")
    if (!bHistogramIsValid[nCurrentHistogram] && (fPeak >= -40.0f))
    {
        bHistogramIsValid[nCurrentHistogram] = true;
    }

    // increment item counter of currently selected histograms
    nHistogramCurrentCounts++;

    // if we have collected enough statistical data, calculate dynamic
    // range value
    if (nHistogramCurrentCounts >= nHistogramMaximumCounts)
    {
        calculateDynamicRangeValue();
    }

    // if current peak meter level exceeds overall maximum peak level,
    // store it as new overall maximum peak level
    if (fPeak > fMaximumPeakLevels[nChannel])
    {
        fMaximumPeakLevels[nChannel] = fPeak;
    }

    // apply peak meter's ballistics and store resulting level and
    // peak mark
    fPeakMeterLevels[nChannel] = PeakMeterBallistics(fTimePassed, fPeak, fPeakMeterLevels[nChannel]);
    fPeakMeterPeakLevels[nChannel] = PeakMeterPeakBallistics(fTimePassed, &fPeakMeterPeakLastChanged[nChannel], fPeak, fPeakMeterPeakLevels[nChannel]);

    // apply average meter's ballistics and store resulting level and
    // peak mark
    AverageMeterBallistics(nChannel, fTimePassed, fAverageFiltered);
    fAverageMeterPeakLevels[nChannel] = AverageMeterPeakBallistics(fTimePassed, &fAverageMeterPeakLastChanged[nChannel], fAverageMeterLevels[nChannel], fAverageMeterPeakLevels[nChannel]);

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
        return fPeakLevelOld - fReleaseCoef;
    }
}


float MeterBallistics::PeakMeterPeakBallistics(float fTimePassed, float* fLastChanged, float fPeakCurrent, float fPeakOld)
/*  Calculate ballistics for peak meter peak marks.

    fTimePassed (float): time that has passed since last update (in
    fractional seconds)

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
            *fLastChanged += fTimePassed;
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
            float fReleaseCoef = 26.0f * fTimePassed / 3.0f;

            // apply fall time
            fOutput = fPeakOld - fReleaseCoef;
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
    LogMeterBallistics(0.600f, fTimePassed, fAverageLevelCurrent, fAverageMeterLevels[nChannel]);
}


float MeterBallistics::AverageMeterPeakBallistics(float fTimePassed, float* fLastChanged, float fPeakCurrent, float fPeakOld)
/*  Calculate ballistics for average meter peak marks.

    fTimePassed (float): time that has passed since last update (in
    fractional seconds)

    fLastChanged (float pointer): time since peak mark was last
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


void MeterBallistics::LogMeterBallistics(float fMeterInertia, float fTimePassed, float fLevel, float& fReadout)
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
        // Thanks to Bram from Smartelectronix for the code snippet!
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
