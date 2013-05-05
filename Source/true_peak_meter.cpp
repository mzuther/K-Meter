/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2013 Martin Zuther (http://www.mzuther.de/)

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

#include "true_peak_meter.h"

TruePeakMeter::TruePeakMeter(const int channels, const int buffer_size)
{
    jassert(channels > 0);

#ifdef _WIN32
    pDynamicLibraryFFTW = new DynamicLibrary();
    pDynamicLibraryFFTW->open("libfftw3f-3.dll");

    fftwf_alloc_real = (float * (*)(size_t)) pDynamicLibraryFFTW->getFunction("fftwf_alloc_real");
    fftwf_alloc_complex = (fftwf_complex * (*)(size_t)) pDynamicLibraryFFTW->getFunction("fftwf_alloc_complex");
    fftwf_free = (void (*)(void*)) pDynamicLibraryFFTW->getFunction("fftwf_free");

    fftwf_plan_dft_r2c_1d = (fftwf_plan(*)(int, float*, fftwf_complex*, unsigned)) pDynamicLibraryFFTW->getFunction("fftwf_plan_dft_r2c_1d");
    fftwf_plan_dft_c2r_1d = (fftwf_plan(*)(int, fftwf_complex*, float*, unsigned)) pDynamicLibraryFFTW->getFunction("fftwf_plan_dft_c2r_1d");
    fftwf_destroy_plan = (void (*)(fftwf_plan)) pDynamicLibraryFFTW->getFunction("fftwf_destroy_plan");

    fftwf_execute = (void (*)(const fftwf_plan)) pDynamicLibraryFFTW->getFunction("fftwf_execute");
#endif

    nNumberOfChannels = channels;
    nBufferSizeOriginal = buffer_size;

    // 8x oversampling (maximum under-read is 0.169 dB at half the
    // sampling rate, see Annex 2 of ITU-R BS.1770-3)
    nOversamplingRate = 8;
    nBufferSizeOversampled = nBufferSizeOriginal * nOversamplingRate;

    nFftSize = nBufferSizeOversampled * 2;
    nHalfFftSize = nFftSize / 2 + 1;

    pSampleBufferOriginal = new AudioSampleBuffer(nNumberOfChannels, nBufferSizeOriginal);
    pSampleBufferOversampled = new AudioSampleBuffer(nNumberOfChannels, nBufferSizeOversampled);

    pSampleBufferOriginal->clear();
    pSampleBufferOversampled->clear();

    fTruePeakLevels = new float[nNumberOfChannels];
    nNumberOfOverflows = new int[nNumberOfChannels];

    arrFilterKernel_TD = fftwf_alloc_real(nFftSize);
    arrFilterKernel_FD = fftwf_alloc_complex(nHalfFftSize);

    planFilterKernel_DFT = fftwf_plan_dft_r2c_1d(nFftSize, arrFilterKernel_TD, arrFilterKernel_FD, FFTW_MEASURE);

    arrAudioSamples_TD = fftwf_alloc_real(nFftSize);
    arrAudioSamples_FD = fftwf_alloc_complex(nHalfFftSize);

    planAudioSamples_DFT = fftwf_plan_dft_r2c_1d(nFftSize, arrAudioSamples_TD, arrAudioSamples_FD, FFTW_MEASURE);
    planAudioSamples_IDFT = fftwf_plan_dft_c2r_1d(nFftSize, arrAudioSamples_FD, arrAudioSamples_TD, FFTW_MEASURE);

    calculateFilterKernel();
}


TruePeakMeter::~TruePeakMeter()
{
    delete pSampleBufferOriginal;
    pSampleBufferOriginal = NULL;

    delete pSampleBufferOversampled;
    pSampleBufferOversampled = NULL;

    delete [] fTruePeakLevels;
    fTruePeakLevels = NULL;

    delete [] nNumberOfOverflows;
    nNumberOfOverflows = NULL;

    fftwf_destroy_plan(planFilterKernel_DFT);
    fftwf_free(arrFilterKernel_TD);
    fftwf_free(arrFilterKernel_FD);

    fftwf_destroy_plan(planAudioSamples_DFT);
    fftwf_destroy_plan(planAudioSamples_IDFT);
    fftwf_free(arrAudioSamples_TD);
    fftwf_free(arrAudioSamples_FD);

#ifdef _WIN32
    fftwf_alloc_real = NULL;
    fftwf_alloc_complex = NULL;
    fftwf_free = NULL;

    fftwf_plan_dft_r2c_1d = NULL;
    fftwf_plan_dft_c2r_1d = NULL;
    fftwf_destroy_plan = NULL;

    fftwf_execute = NULL;

    pDynamicLibraryFFTW->close();
    delete pDynamicLibraryFFTW;
    pDynamicLibraryFFTW = NULL;
#endif
}


void TruePeakMeter::calculateFilterKernel()
{
    // reset levels and overflows
    for (int nChannel = 0; nChannel < nNumberOfChannels; nChannel++)
    {
        fTruePeakLevels[nChannel] = 0.0f;
        nNumberOfOverflows[nChannel] = 0;
    }

    // interpolation filter; removes all frequencies above *original*
    // Nyquist frequency from resampled audio (the approximated filter
    // bandwidth is 21.5 Hz for a buffer size of 1024 samples and a
    // sampling rate of 44100 Hz with 8x oversampling)
    float nRelativeCutoffFrequency = 0.5f / nOversamplingRate;

    int nSamples = nBufferSizeOversampled + 1;
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

    // normalise filter kernel for unity gain at DC
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


void TruePeakMeter::FilterSamples(const int channel)
{
    jassert(channel >= 0);
    jassert(channel < nNumberOfChannels);

    // copy audio data to temporary buffer as the sample buffer is not
    // optimised for MME
    memcpy(arrAudioSamples_TD, pSampleBufferOversampled->getSampleData(channel), nBufferSizeOversampled * sizeof(float));

    // pad audio data with zeros
    for (int nSample = nBufferSizeOversampled; nSample < nFftSize; nSample++)
    {
        arrAudioSamples_TD[nSample] = 0.0f;
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
    float fNorm = float(nFftSize / nOversamplingRate);

    for (int i = 0; i < nFftSize; i++)
    {
        arrAudioSamples_TD[i] = arrAudioSamples_TD[i] / fNorm;
    }

    // copy data from temporary buffer back to sample buffer
    pSampleBufferOversampled->copyFrom(channel, 0, arrAudioSamples_TD, nBufferSizeOversampled);

    // evaluate true peak level
    fTruePeakLevels[channel] = pSampleBufferOversampled->getMagnitude(channel, 0, nBufferSizeOversampled);

    // finally, count number of overflows
    countOverflows(channel);
}


void TruePeakMeter::countOverflows(const int channel)
{
    jassert(channel >= 0);
    jassert(channel < nNumberOfChannels);

    // initialise number of overflows in this channel
    int nOverflows = 0;

    // get pointer to sample values
    float* pSampleValues = pSampleBufferOversampled->getSampleData(channel);

    // loop through samples of buffer
    for (int nSample = 0; nSample < nBufferSizeOversampled; nSample++)
    {
        // get current sample value
        float fSampleValue = pSampleValues[nSample];

        // 8x oversampling means a maximum under-read of 0.169 dB at
        // half the sampling rate, so we'll treat absolute levels of
        // -0.169 dBFS and above as overflows; this corresponds to a
        // linear level of 0.9807.
        if ((fSampleValue < -0.9807f) || (fSampleValue > 0.9807f))
        {
            nOverflows++;
        }
    }

    // set number of overflows in this channel
    nNumberOfOverflows[channel] = nOverflows;
}


float TruePeakMeter::getLevel(const int channel)
{
    jassert(channel >= 0);
    jassert(channel < nNumberOfChannels);

    return fTruePeakLevels[channel];
}


int TruePeakMeter::getNumberOfOverflows(const int channel)
{
    jassert(channel >= 0);
    jassert(channel < nNumberOfChannels);

    return nNumberOfOverflows[channel];
}


void TruePeakMeter::copyFromBuffer(AudioRingBuffer& ringBuffer, const unsigned int pre_delay)
{
    // copy data from ring buffer to sample buffer
    ringBuffer.copyToBuffer(*pSampleBufferOriginal, 0, nBufferSizeOriginal, pre_delay);

    // oversample input sample buffer by clearing it and filling every
    // "nOversamplingRate" sample with the original sample values
    pSampleBufferOversampled->clear();

    for (int nSample = 0; nSample < nBufferSizeOriginal; nSample++)
    {
        int nSampleOversampled = nSample * nOversamplingRate;

        for (int nChannel = 0; nChannel < nNumberOfChannels; nChannel++)
        {
            pSampleBufferOversampled->copyFrom(nChannel, nSampleOversampled, *pSampleBufferOriginal, nChannel, nSample, 1);
        }
    }

    for (int nChannel = 0; nChannel < nNumberOfChannels; nChannel++)
    {
        // filter audio data (overwrites contents of sample buffer)
        FilterSamples(nChannel);
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
