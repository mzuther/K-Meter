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

#include "average_level_filtered.h"

AverageLevelFiltered::AverageLevelFiltered(KmeterAudioProcessor* processor, const int channels, const int buffer_size, const int sample_rate, const int average_algorithm)
{
    jassert(channels > 0);

#ifdef _WIN32
    File libraryFFTW = File::getSpecialLocation(File::currentExecutableFile).getSiblingFile(T("libfftw3f-3.dll"));
    libraryHandleFFTW = PlatformUtilities::loadDynamicLibrary(libraryFFTW.getFullPathName());

    fftwf_malloc = (void * (*)(size_t)) PlatformUtilities::getProcedureEntryPoint(libraryHandleFFTW, "fftwf_malloc");
    fftwf_free = (void (*)(void*)) PlatformUtilities::getProcedureEntryPoint(libraryHandleFFTW, "fftwf_free");

    fftwf_plan_dft_r2c_1d = (fftwf_plan(*)(int, float*, fftwf_complex*, unsigned)) PlatformUtilities::getProcedureEntryPoint(libraryHandleFFTW, "fftwf_plan_dft_r2c_1d");
    fftwf_plan_dft_c2r_1d = (fftwf_plan(*)(int, fftwf_complex*, float*, unsigned)) PlatformUtilities::getProcedureEntryPoint(libraryHandleFFTW, "fftwf_plan_dft_c2r_1d");
    fftwf_destroy_plan = (void (*)(fftwf_plan)) PlatformUtilities::getProcedureEntryPoint(libraryHandleFFTW, "fftwf_destroy_plan");

    fftwf_execute = (void (*)(const fftwf_plan)) PlatformUtilities::getProcedureEntryPoint(libraryHandleFFTW, "fftwf_execute");
#endif

    pProcessor = processor;
    nChannels = channels;
    nSampleRate = sample_rate;
    nBufferSize = buffer_size;
    fPeakToAverageCorrection = 0.0f;

    nFftSize = nBufferSize * 2;
    nHalfFftSize = nFftSize / 2 + 1;

    pSampleBuffer = new AudioSampleBuffer(nChannels, nBufferSize);
    pOverlapAddSamples = new AudioSampleBuffer(nChannels, nBufferSize);

    // IIR coefficients: 0 represents input, 1 represents output
    pIIRCoefficients_1 = new float*[2];
    pIIRCoefficients_1[0] = new float[KMETER_MAXIMUM_IIR_FILTER_COEFFICIENTS];
    pIIRCoefficients_1[1] = new float[KMETER_MAXIMUM_IIR_FILTER_COEFFICIENTS];

    pIIRCoefficients_2 = new float*[2];
    pIIRCoefficients_2[0] = new float[KMETER_MAXIMUM_IIR_FILTER_COEFFICIENTS];
    pIIRCoefficients_2[1] = new float[KMETER_MAXIMUM_IIR_FILTER_COEFFICIENTS];

    // previous samples
    pPreviousSamplesInput_1 = new AudioSampleBuffer(nChannels, KMETER_MAXIMUM_IIR_FILTER_COEFFICIENTS - 1);
    pPreviousSamplesOutput_1 = new AudioSampleBuffer(nChannels, KMETER_MAXIMUM_IIR_FILTER_COEFFICIENTS - 1);

    pPreviousSamplesInput_2 = new AudioSampleBuffer(nChannels, KMETER_MAXIMUM_IIR_FILTER_COEFFICIENTS - 1);
    pPreviousSamplesOutput_2 = new AudioSampleBuffer(nChannels, KMETER_MAXIMUM_IIR_FILTER_COEFFICIENTS - 1);

    pPreviousSamplesOutputTemp = new AudioSampleBuffer(1, nBufferSize);

    // reset IIR coefficients and previous samples
    for (int nSource = 0; nSource <= 1; nSource++)
    {
        for (int nSample = 0; nSample < KMETER_MAXIMUM_IIR_FILTER_COEFFICIENTS; nSample++)
        {
            pIIRCoefficients_1[nSource][nSample] = 0.0f;
            pIIRCoefficients_2[nSource][nSample] = 0.0f;
        }
    }

    pPreviousSamplesInput_1->clear();
    pPreviousSamplesOutput_1->clear();

    pPreviousSamplesInput_2->clear();
    pPreviousSamplesOutput_2->clear();

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

    setAlgorithm(average_algorithm);
}


AverageLevelFiltered::~AverageLevelFiltered()
{
    delete pSampleBuffer;
    pSampleBuffer = NULL;

    delete pOverlapAddSamples;
    pOverlapAddSamples = NULL;

    for (int nSource = 0; nSource <= 1; nSource++)
    {
        delete [] pIIRCoefficients_1[nSource];
        pIIRCoefficients_1[nSource] = NULL;

        delete [] pIIRCoefficients_2[nSource];
        pIIRCoefficients_2[nSource] = NULL;
    }

    delete [] pIIRCoefficients_1;
    pIIRCoefficients_1 = NULL;

    delete [] pIIRCoefficients_2;
    pIIRCoefficients_2 = NULL;

    delete pPreviousSamplesInput_1;
    pPreviousSamplesInput_1 = NULL;

    delete pPreviousSamplesOutput_1;
    pPreviousSamplesOutput_1 = NULL;

    delete pPreviousSamplesInput_2;
    pPreviousSamplesInput_2 = NULL;

    delete pPreviousSamplesOutput_2;
    pPreviousSamplesOutput_2 = NULL;

    delete pPreviousSamplesOutputTemp;
    pPreviousSamplesOutputTemp = NULL;

    fftwf_destroy_plan(planFilterKernel_DFT);
    fftwf_free(arrFilterKernel_TD);
    fftwf_free(arrFilterKernel_FD);

    fftwf_destroy_plan(planAudioSamples_DFT);
    fftwf_destroy_plan(planAudioSamples_IDFT);
    fftwf_free(arrAudioSamples_TD);
    fftwf_free(arrAudioSamples_FD);

#ifdef _WIN32
    fftwf_malloc = NULL;
    fftwf_free = NULL;

    fftwf_plan_dft_r2c_1d = NULL;
    fftwf_plan_dft_c2r_1d = NULL;
    fftwf_destroy_plan = NULL;

    fftwf_execute = NULL;

    PlatformUtilities::freeDynamicLibrary(libraryHandleFFTW);
    libraryHandleFFTW = NULL;
#endif
}


int AverageLevelFiltered::getAlgorithm()
{
    return nAverageAlgorithm;
}


void AverageLevelFiltered::setAlgorithm(const int average_algorithm)
{
    if (average_algorithm == nAverageAlgorithm)
    {
        return;
    }

    if ((average_algorithm >= 0) && (average_algorithm < KmeterPluginParameters::nNumAlgorithms))
    {
        nAverageAlgorithm = average_algorithm;
    }
    else
    {
        nAverageAlgorithm = KmeterPluginParameters::selAlgorithmItuBs1770;
    }

    calculateFilterKernel();
    pProcessor->setAverageAlgorithmFinal(nAverageAlgorithm);
}


void AverageLevelFiltered::calculateFilterKernel()
{
    // reset IIR coefficients and previous samples
    for (int nSource = 0; nSource <= 1; nSource++)
    {
        for (int nSample = 0; nSample < KMETER_MAXIMUM_IIR_FILTER_COEFFICIENTS; nSample++)
        {
            pIIRCoefficients_1[nSource][nSample] = 0.0f;
            pIIRCoefficients_2[nSource][nSample] = 0.0f;
        }
    }

    pPreviousSamplesInput_1->clear();
    pPreviousSamplesOutput_1->clear();

    pPreviousSamplesInput_2->clear();
    pPreviousSamplesOutput_2->clear();

    // make sure there's no overlap yet
    pSampleBuffer->clear();
    pOverlapAddSamples->clear();

    if (nAverageAlgorithm == KmeterPluginParameters::selAlgorithmItuBs1770)
    {
        calculateFilterKernel_ItuBs1770();

        // "Regardless of the frequency response or methodology of the
        // loudness method, reference 0 dB of all meters is calibrated
        // such that 20-20 kHz pink noise at 0 dB reads 83 dB SPL, C
        // weighted, slow." (Bob Katz on his website)
        //
        // Thus, I have simply taken the mean average level of both
        // stereo channels on validation with "pink_noise.flac".
        fPeakToAverageCorrection = MeterBallistics::level2decibel(sqrt(2.0f));
    }
    else
    {
        calculateFilterKernel_Rms();

        // the RMS of a sine wave is its amplitude divided by the
        // square root of 2, thus the difference between peak value
        // and RMS is the square root of 2 -- so let's convert this
        // difference to dB and store the result
        fPeakToAverageCorrection = MeterBallistics::level2decibel(sqrt(2.0f));
    }
}


void AverageLevelFiltered::calculateFilterKernel_Rms()
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


void AverageLevelFiltered::calculateFilterKernel_ItuBs1770()
{
    // filter specifications were taken from Raiden's wonderful paper
    // "ITU-R BS.1770-1 filter specifications (unofficial)" as found
    // on http://www.scribd.com/doc/49991813/ITU-R-BS-1770-1-filters
    //
    // please see here for Raiden's original forum thread:
    // http://www.hydrogenaudio.org/forums/index.php?showtopic=86116

    // initialise pre-filter (ITU-r BS.1770-2)
    float pf_vh = 1.584864701130855f;
    float pf_vb = sqrt(pf_vh);
    float pf_vl = 1.0f;
    float pf_q = 0.7071752369554196f;
    float pf_cutoff = 1681.974450955533f;
    float pf_omega = (float) tan(M_PI * pf_cutoff / nSampleRate);
    float pf_omega_2 = powf(pf_omega, 2.0f);
    float pf_omega_q = pf_omega / pf_q;
    float pf_div = (pf_omega_2 + pf_omega_q + 1.0f);

    pIIRCoefficients_1[0][0] = (pf_vl * pf_omega_2 + pf_vb * pf_omega_q + pf_vh) / pf_div;
    pIIRCoefficients_1[0][1] = 2.0f * (pf_vl * pf_omega_2 - pf_vh) / pf_div;
    pIIRCoefficients_1[0][2] = (pf_vl * pf_omega_2 - pf_vb * pf_omega_q + pf_vh) / pf_div;

    pIIRCoefficients_1[1][0] = -1.0f;
    pIIRCoefficients_1[1][1] = -2.0f * (pf_omega_2 - 1.0f) / pf_div;
    pIIRCoefficients_1[1][2] = -(pf_omega_2 - pf_omega_q + 1.0f) / pf_div;

    // initialise RLB weighting curve (ITU-r BS.1770-2)
    float rlb_vh = 1.0f;
    float rlb_vb = 0.0f;
    float rlb_vl = 0.0f;
    float rlb_q = 0.5003270373238773f;
    float rlb_cutoff = 38.13547087602444f;
    float rlb_omega = (float) tan(M_PI * rlb_cutoff / nSampleRate);
    float rlb_omega_2 = powf(rlb_omega, 2.0f);
    float rlb_omega_q = rlb_omega / rlb_q;
    float rlb_div_1 = (rlb_vl * rlb_omega_2 + rlb_vb * rlb_omega_q + rlb_vh);
    float rlb_div_2 = (rlb_omega_2 + rlb_omega_q + 1.0f);

    pIIRCoefficients_2[0][0] = 1.0f;
    pIIRCoefficients_2[0][1] = 2.0f * (rlb_vl * rlb_omega_2 - rlb_vh) / rlb_div_1;
    pIIRCoefficients_2[0][2] = (rlb_vl * rlb_omega_2 - rlb_vb * rlb_omega_q + rlb_vh) / rlb_div_1;

    pIIRCoefficients_2[1][0] = -1.0f;
    pIIRCoefficients_2[1][1] = -2.0f * (rlb_omega_2 - 1) / rlb_div_2;
    pIIRCoefficients_2[1][2] = -(rlb_omega_2 - rlb_omega_q + 1) / rlb_div_2;

    calculateFilterKernel_Rms();
}


void AverageLevelFiltered::FilterSamples(const int channel)
{
    if (nAverageAlgorithm == KmeterPluginParameters::selAlgorithmItuBs1770)
    {
        FilterSamples_ItuBs1770(channel);
    }
    else
    {
        FilterSamples_Rms(channel);
    }
}


void AverageLevelFiltered::FilterSamples_Rms(const int channel)
{
    jassert(channel >= 0);
    jassert(channel < nChannels);

    // copy audio data to temporary buffer as the sample buffer is not
    // optimised for MME
    memcpy(arrAudioSamples_TD, pSampleBuffer->getSampleData(channel), nBufferSize * sizeof(float));

    // pad audio data with zeros
    for (int nSample = nBufferSize; nSample < nFftSize; nSample++)
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


void AverageLevelFiltered::FilterSamples_ItuBs1770(const int channel)
{
    jassert(channel >= 0);
    jassert(channel < nChannels);

    // pre-filter
    pPreviousSamplesOutputTemp->clear();
    float* pSamplesInput = pSampleBuffer->getSampleData(channel);
    float* pSamplesOutput = pPreviousSamplesOutputTemp->getSampleData(0);

    float* pSamplesInputOld_1 = pPreviousSamplesInput_1->getSampleData(channel);
    float* pSamplesOutputOld_1 = pPreviousSamplesOutput_1->getSampleData(channel);

    for (int nSample = 0; nSample < nBufferSize; nSample++)
    {
        if (nSample < 2)
        {
            if (nSample == 0)
            {
                pSamplesOutput[nSample] =
                    pIIRCoefficients_1[0][0] * pSamplesInput[nSample] +
                    pIIRCoefficients_1[0][1] * pSamplesInputOld_1[1] +
                    pIIRCoefficients_1[0][2] * pSamplesInputOld_1[0] +
                    pIIRCoefficients_1[1][1] * pSamplesOutputOld_1[1] +
                    pIIRCoefficients_1[1][2] * pSamplesOutputOld_1[0];
            }
            else
            {
                pSamplesOutput[nSample] =
                    pIIRCoefficients_1[0][0] * pSamplesInput[nSample] +
                    pIIRCoefficients_1[0][1] * pSamplesInput[nSample - 1] +
                    pIIRCoefficients_1[0][2] * pSamplesInputOld_1[1] +
                    pIIRCoefficients_1[1][1] * pSamplesOutput[nSample - 1] +
                    pIIRCoefficients_1[1][2] * pSamplesOutputOld_1[1];
            }

            // avoid underflows (1e-20f corresponds to -400 dBFS)
            if (fabs(pSamplesOutput[nSample]) < 1e-20f)
            {
                pSamplesOutput[nSample] = 0.0f;
            }
        }
        else
        {
            pSamplesOutput[nSample] =
                pIIRCoefficients_1[0][0] * pSamplesInput[nSample] +
                pIIRCoefficients_1[0][1] * pSamplesInput[nSample - 1] +
                pIIRCoefficients_1[0][2] * pSamplesInput[nSample - 2] +
                pIIRCoefficients_1[1][1] * pSamplesOutput[nSample - 1] +
                pIIRCoefficients_1[1][2] * pSamplesOutput[nSample - 2];
        }
    }

    pPreviousSamplesInput_1->copyFrom(channel, 0, *pSampleBuffer, channel, nBufferSize - 2, 2);
    pPreviousSamplesOutput_1->copyFrom(channel, 0, *pPreviousSamplesOutputTemp, 0, nBufferSize - 2, 2);

    pSampleBuffer->copyFrom(channel, 0, *pPreviousSamplesOutputTemp, 0, 0, nBufferSize);

    // RLB weighting filter
    pPreviousSamplesOutputTemp->clear();

    float* pSamplesInputOld_2 = pPreviousSamplesInput_2->getSampleData(channel);
    float* pSamplesOutputOld_2 = pPreviousSamplesOutput_2->getSampleData(channel);

    for (int nSample = 0; nSample < nBufferSize; nSample++)
    {
        if (nSample < 2)
        {
            if (nSample == 0)
            {
                pSamplesOutput[nSample] =
                    pIIRCoefficients_2[0][0] * pSamplesInput[nSample] +
                    pIIRCoefficients_2[0][1] * pSamplesInputOld_2[1] +
                    pIIRCoefficients_2[0][2] * pSamplesInputOld_2[0] +
                    pIIRCoefficients_2[1][1] * pSamplesOutputOld_2[1] +
                    pIIRCoefficients_2[1][2] * pSamplesOutputOld_2[0];
            }
            else
            {
                pSamplesOutput[nSample] =
                    pIIRCoefficients_2[0][0] * pSamplesInput[nSample] +
                    pIIRCoefficients_2[0][1] * pSamplesInput[nSample - 1] +
                    pIIRCoefficients_2[0][2] * pSamplesInputOld_2[1] +
                    pIIRCoefficients_2[1][1] * pSamplesOutput[nSample - 1] +
                    pIIRCoefficients_2[1][2] * pSamplesOutputOld_2[1];
            }

            // avoid underflows (1e-20f corresponds to -400 dBFS)
            if (fabs(pSamplesOutput[nSample]) < 1e-20f)
            {
                pSamplesOutput[nSample] = 0.0f;
            }
        }
        else
        {
            pSamplesOutput[nSample] =
                pIIRCoefficients_2[0][0] * pSamplesInput[nSample] +
                pIIRCoefficients_2[0][1] * pSamplesInput[nSample - 1] +
                pIIRCoefficients_2[0][2] * pSamplesInput[nSample - 2] +
                pIIRCoefficients_2[1][1] * pSamplesOutput[nSample - 1] +
                pIIRCoefficients_2[1][2] * pSamplesOutput[nSample - 2];
        }
    }

    pPreviousSamplesInput_2->copyFrom(channel, 0, *pSampleBuffer, channel, nBufferSize - 2, 2);
    pPreviousSamplesOutput_2->copyFrom(channel, 0, *pPreviousSamplesOutputTemp, 0, nBufferSize - 2, 2);

    pSampleBuffer->copyFrom(channel, 0, *pPreviousSamplesOutputTemp, 0, 0, nBufferSize);

    FilterSamples_Rms(channel);
}


float AverageLevelFiltered::getLevel(const int channel)
{
    jassert(channel >= 0);
    jassert(channel < nChannels);

    // filter audio data (overwrites contents of sample buffer)
    FilterSamples(channel);

    if (nAverageAlgorithm == KmeterPluginParameters::selAlgorithmItuBs1770)
    {
        // weighting factor for L, C, R (incorrect for surround
        // channels and LFE!)
        float fWeightingFactor = 1.0f;

        float fAverageLevel = pSampleBuffer->getRMSLevel(channel, 0, nBufferSize);
        float fMeterMinimumDecibel = MeterBallistics::getMeterMinimumDecibel() - fPeakToAverageCorrection;

        // the square root of this level has already been taken, so
        // revert this to fit the formula (of course, the formula
        // could easily be modified to accept RMS levels, but this
        // would introduce a bug as soon as some channels were summed)
        fAverageLevel = powf(fWeightingFactor * fAverageLevel, 2.0f);

        if (fAverageLevel == 0.0f)
        {
            fAverageLevel = fMeterMinimumDecibel;
        }
        else
        {
            // apply the formula from ITU-R BS.1770-2
            fAverageLevel = -0.691f + 10.0f * log10(fAverageLevel);

            if (fAverageLevel < fMeterMinimumDecibel)
            {
                fAverageLevel = fMeterMinimumDecibel;
            }
        }

        // apply peak-to-average correction so that sine waves give
        // the same read-out on peak and average meters
        return fAverageLevel + fPeakToAverageCorrection;
    }
    else
    {
        float fAverageLevel = MeterBallistics::level2decibel(pSampleBuffer->getRMSLevel(channel, 0, nBufferSize));

        // apply peak-to-average correction so that sine waves give
        // the same read-out on peak and average meters
        return fAverageLevel + fPeakToAverageCorrection;
    }
}


void AverageLevelFiltered::copyFromBuffer(AudioRingBuffer& ringBuffer, const int pre_delay, const int sample_rate)
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


void AverageLevelFiltered::copyToBuffer(AudioRingBuffer& destination, const unsigned int sourceStartSample, const unsigned int numSamples)
{
    // copy data from sample buffer to ring buffer
    destination.addSamples(*pSampleBuffer, sourceStartSample, numSamples);
}


void AverageLevelFiltered::copyToBuffer(AudioSampleBuffer& destination, const int channel, const int destStartSample, const int numSamples)
{
    jassert(channel >= 0);
    jassert(channel < nChannels);
    jassert((destStartSample + numSamples) <= destination.getNumSamples());

    memcpy(destination.getSampleData(channel, destStartSample), pSampleBuffer->getSampleData(channel), numSamples * sizeof(float));
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
