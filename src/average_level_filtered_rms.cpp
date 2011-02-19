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

#include "average_level_filtered_rms.h"

AverageLevelFilteredRms::AverageLevelFilteredRms(const int channels, const int buffer_size)
{
    jassert(channels > 0);

    nChannels = channels;
    nSampleRate = -1;
    nBufferSize = buffer_size;

    nFftSize = nBufferSize * 2;
    nHalfFftSize = nFftSize / 2 + 1;

    pSampleBuffer = new AudioSampleBuffer(nChannels, nBufferSize);
    pOverlapAddSamples = new AudioSampleBuffer(nChannels, nBufferSize);

    // make sure there's no overlap yet
    pSampleBuffer->clear();
    pOverlapAddSamples->clear();

    arrFilterKernel_TD = (float*) fftwf_malloc(sizeof(float) * nFftSize);
    arrFilterKernel_FD = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * nHalfFftSize);

    planFilterKernel_DFT = fftwf_plan_dft_r2c_1d(nFftSize, arrFilterKernel_TD, arrFilterKernel_FD, FFTW_MEASURE);

    arrAudioSamples_TD = (float*) fftwf_malloc(sizeof(float) * nFftSize);
    arrAudioSamples_FD = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * nHalfFftSize);

    planAudioSamples_DFT = fftwf_plan_dft_r2c_1d(nFftSize, arrAudioSamples_TD, arrAudioSamples_FD, FFTW_MEASURE);
    planAudioSamples_IDFT = fftwf_plan_dft_c2r_1d(nFftSize, arrAudioSamples_FD, arrAudioSamples_TD, FFTW_MEASURE);
}


AverageLevelFilteredRms::~AverageLevelFilteredRms()
{
    delete pSampleBuffer;
    pSampleBuffer = NULL;

    delete pOverlapAddSamples;
    pOverlapAddSamples = NULL;

    fftwf_destroy_plan(planFilterKernel_DFT);
    fftwf_free(arrFilterKernel_TD);
    fftwf_free(arrFilterKernel_FD);

    fftwf_destroy_plan(planAudioSamples_DFT);
    fftwf_destroy_plan(planAudioSamples_IDFT);
    fftwf_free(arrAudioSamples_TD);
    fftwf_free(arrAudioSamples_FD);
}


void AverageLevelFilteredRms::calculateFilterKernel()
{
    float nCutoffFrequency = 21000.0f;
    float nRelativeCutoffFrequency = nCutoffFrequency / nSampleRate;

    int nSamples = nBufferSize + 1;
    float nSamplesHalf = nSamples / 2.0f;

    // calculate filter kernel
    for (int i = 0; i < nSamples; i++)
    {
        if (i == nSamplesHalf)
        {
            arrFilterKernel_TD[i] = float(2.0 * M_PI * nRelativeCutoffFrequency);
        }
        else
        {
            arrFilterKernel_TD[i] = float(sin(2.0 * M_PI * nRelativeCutoffFrequency * (i - nSamplesHalf)) / (i - nSamplesHalf) * (0.42 - 0.5 * cos(2.0 * (float) M_PI * i / nSamples) + 0.08 * cos(4.0 * (float) M_PI * i / nSamples)));
        }
    }

    // normalise filter kernel
    float nSumKernel = 0.0;

    for (int i = 0; i < nSamples; i++)
    {
        nSumKernel += arrFilterKernel_TD[i];
    }

    for (int i = 0; i < nSamples; i++)
    {
        arrFilterKernel_TD[i] = arrFilterKernel_TD[i] / nSumKernel;
    }

    // pad filter kernel with zeros
    for (int i = nSamples; i < nFftSize; i++)
    {
        arrFilterKernel_TD[i] = 0.0f;
    }

    // calculate DFT of filter kernel
    fftwf_execute(planFilterKernel_DFT);
}


void AverageLevelFilteredRms::FilterSamples(const int channel)
{
    jassert(channel >= 0);
    jassert(channel < nChannels);

    // copy audio data to temporary buffer as the sample buffer is not
    // optimised for MME
    memcpy(arrAudioSamples_TD, pSampleBuffer->getSampleData(channel), nBufferSize * sizeof(float));

    // pad audio data with zeros
    for (int i = nBufferSize; i < nFftSize; i++)
    {
        arrAudioSamples_TD[i] = 0.0f;
    }

    // calculate DFT of audio data
    fftwf_execute(planAudioSamples_DFT);

    // convolve audio data with filter kernel
    for (int i = 0; i < nHalfFftSize; i++)
    {
        // multiplication of complex numbers: index 0 contains the real
        // part, index 1 the imaginary part
        float real_part = arrAudioSamples_FD[i][0] * arrFilterKernel_FD[i][0] - arrAudioSamples_FD[i][1] * arrFilterKernel_FD[i][1];
        float imaginary_part = arrAudioSamples_FD[i][1] * arrFilterKernel_FD[i][0] + arrAudioSamples_FD[i][0] * arrFilterKernel_FD[i][1];

        arrAudioSamples_FD[i][0] = real_part;
        arrAudioSamples_FD[i][1] = imaginary_part;
    }

    // synthesise audio data from frequency spectrum (this destroys the
    // contents of "arrAudioSamples_FD"!!!)
    fftwf_execute(planAudioSamples_IDFT);

    // normalise synthesised audio data
    for (int i = 0; i < nFftSize; i++)
    {
        arrAudioSamples_TD[i] = arrAudioSamples_TD[i] / float(nFftSize);
    }

    // copy data from temporary buffer back to sample buffer
    pSampleBuffer->copyFrom(channel, 0, arrAudioSamples_TD, nBufferSize);

    // add old overlapping samples
    pSampleBuffer->addFrom(channel, 0, *pOverlapAddSamples, channel, 0, nBufferSize);

    // store new overlapping samples
    pOverlapAddSamples->copyFrom(channel, 0, arrAudioSamples_TD + nBufferSize, nBufferSize);
}


float AverageLevelFilteredRms::getLevel(const int channel)
{
    jassert(channel >= 0);
    jassert(channel < nChannels);

    // filter audio data (overwrites contents of sample buffer)
    FilterSamples(channel);

    return pSampleBuffer->getRMSLevel(channel, 0, nBufferSize);
}


void AverageLevelFilteredRms::copyFromBuffer(AudioRingBuffer& ringBuffer, const int pre_delay, const int sample_rate)
{
    // recalculate filter kernel when sample rate changes
    if (nSampleRate != sample_rate)
    {
        nSampleRate = sample_rate;
        calculateFilterKernel();
    }

    // copy data from ring buffer to sample buffer
    ringBuffer.copyToBuffer(*pSampleBuffer, 0, nBufferSize, pre_delay);
}


void AverageLevelFilteredRms::copyToBuffer(AudioRingBuffer& destination, const unsigned int sourceStartSample, const unsigned int numSamples)
{
    // copy data from sample buffer to ring buffer
    destination.addSamples(*pSampleBuffer, sourceStartSample, numSamples);
}


void AverageLevelFilteredRms::copyToBuffer(AudioSampleBuffer& destination, const int channel, const int destStartSample, const int numSamples)
{
    jassert(channel >= 0);
    jassert(channel < nChannels);
    jassert((destStartSample + numSamples) <= destination.getNumSamples());

    memcpy(destination.getSampleData(channel, destStartSample), pSampleBuffer->getSampleData(channel), numSamples * sizeof(float));
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
