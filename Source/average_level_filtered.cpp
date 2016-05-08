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

#include "average_level_filtered.h"


AverageLevelFiltered::AverageLevelFiltered(
    KmeterAudioProcessor *processor,
    const int channels,
    const int sampleRate,
    const int bufferSize,
    const int averageAlgorithm) :

    numberOfChannels_(channels),
    sampleRate_(sampleRate),
    bufferSize_(bufferSize),
    fftSize_(bufferSize_ * 2),
    halfFftSize_(fftSize_ / 2 + 1),
    sampleBuffer_(numberOfChannels_, bufferSize_),
    overlapAddSamples_(numberOfChannels_, bufferSize_),
    previousSamplesPreFilterInput_(
        numberOfChannels_, KMETER_MAXIMUM_FILTER_STAGES - 1),
    previousSamplesPreFilterOutput_(
        numberOfChannels_, KMETER_MAXIMUM_FILTER_STAGES - 1),
    previousSamplesWeightingFilterInput_(
        numberOfChannels_, KMETER_MAXIMUM_FILTER_STAGES - 1),
    previousSamplesWeightingFilterOutput_(
        numberOfChannels_, KMETER_MAXIMUM_FILTER_STAGES - 1),
    previousSamplesOutputTemp_(1, bufferSize_),
    dither_(24)

{
    jassert(channels > 0);

#if (defined (_WIN32) || defined (_WIN64))
    File currentExecutableFile = File::getSpecialLocation(
                                     File::currentExecutableFile);

#ifdef _WIN64
    File dynamicLibraryFftwFile = currentExecutableFile.getSiblingFile(
                                      "kmeter/fftw/libfftw3f-3_x64.dll");
#else
    File dynamicLibraryFftwFile = currentExecutableFile.getSiblingFile(
                                      "kmeter/fftw/libfftw3f-3.dll");
#endif

    String dynamicLibraryFftwPath = dynamicLibraryFftwFile.getFullPathName();

    dynamicLibraryFFTW.open(dynamicLibraryFftwPath);
    jassert(dynamicLibraryFFTW.getNativeHandle() != nullptr);

    fftwf_alloc_real = (
                           float * ( *)(size_t)) dynamicLibraryFFTW.getFunction(
                           "fftwf_alloc_real");
    fftwf_alloc_complex = (fftwf_complex * ( *)(size_t)) dynamicLibraryFFTW.getFunction(
                              "fftwf_alloc_complex");
    fftwf_free = (void ( *)(void *)) dynamicLibraryFFTW.getFunction(
                     "fftwf_free");

    fftwf_plan_dft_r2c_1d = (fftwf_plan( *)(int, float *, fftwf_complex *, unsigned)) dynamicLibraryFFTW.getFunction(
                                "fftwf_plan_dft_r2c_1d");
    fftwf_plan_dft_c2r_1d = (fftwf_plan( *)(int, fftwf_complex *, float *, unsigned)) dynamicLibraryFFTW.getFunction(
                                "fftwf_plan_dft_c2r_1d");
    fftwf_destroy_plan = (void ( *)(fftwf_plan)) dynamicLibraryFFTW.getFunction(
                             "fftwf_destroy_plan");

    fftwf_execute = (void ( *)(const fftwf_plan)) dynamicLibraryFFTW.getFunction(
                        "fftwf_execute");
#endif

    processor_ = processor;
    peakToAverageCorrection_ = 0.0f;

    filterKernel_TD_ = fftwf_alloc_real(fftSize_);
    filterKernel_FD_ = fftwf_alloc_complex(halfFftSize_);

    filterKernelPlan_DFT_ = fftwf_plan_dft_r2c_1d(
                                fftSize_, filterKernel_TD_, filterKernel_FD_,
                                FFTW_MEASURE);

    audioSamples_TD_ = fftwf_alloc_real(fftSize_);
    audioSamples_FD_ = fftwf_alloc_complex(halfFftSize_);

    audioSamplesPlan_DFT_ = fftwf_plan_dft_r2c_1d(
                                fftSize_, audioSamples_TD_, audioSamples_FD_,
                                FFTW_MEASURE);
    audioSamplesPlan_IDFT_ = fftwf_plan_dft_c2r_1d(
                                 fftSize_, audioSamples_FD_, audioSamples_TD_,
                                 FFTW_MEASURE);

    averageAlgorithm_ = -1;

    // also calculates filter kernel
    setAlgorithm(averageAlgorithm);
}


AverageLevelFiltered::~AverageLevelFiltered()
{
    fftwf_destroy_plan(filterKernelPlan_DFT_);
    fftwf_free(filterKernel_TD_);
    fftwf_free(filterKernel_FD_);

    fftwf_destroy_plan(audioSamplesPlan_DFT_);
    fftwf_destroy_plan(audioSamplesPlan_IDFT_);
    fftwf_free(audioSamples_TD_);
    fftwf_free(audioSamples_FD_);

#if (defined (_WIN32) || defined (_WIN64))
    fftwf_alloc_real = nullptr;
    fftwf_alloc_complex = nullptr;
    fftwf_free = nullptr;

    fftwf_plan_dft_r2c_1d = nullptr;
    fftwf_plan_dft_c2r_1d = nullptr;
    fftwf_destroy_plan = nullptr;

    fftwf_execute = nullptr;
#endif
}


int AverageLevelFiltered::getAlgorithm()
{
    return averageAlgorithm_;
}


void AverageLevelFiltered::setAlgorithm(
    const int averageAlgorithm)

{
    if (averageAlgorithm == averageAlgorithm_)
    {
        return;
    }

    DBG(String("[K-Meter] averaging algorithm: ") +
        (averageAlgorithm == KmeterPluginParameters::selAlgorithmItuBs1770 ?
         "ITU-R BS.1770-1" : "RMS"));

    if ((averageAlgorithm >= 0) &&
            (averageAlgorithm < KmeterPluginParameters::nNumAlgorithms))
    {
        averageAlgorithm_ = averageAlgorithm;
    }
    else
    {
        averageAlgorithm_ = KmeterPluginParameters::selAlgorithmItuBs1770;
    }

    calculateFilterKernel();
    processor_->setAverageAlgorithmFinal(averageAlgorithm_);
}


void AverageLevelFiltered::calculateFilterKernel()
{
    // reset IIR coefficients and previous samples
    preFilterInputCoefficients_.clear();
    preFilterOutputCoefficients_.clear();

    weightingFilterInputCoefficients_.clear();
    weightingFilterOutputCoefficients_.clear();

    for (int stage = 0; stage < KMETER_MAXIMUM_FILTER_STAGES; ++stage)
    {
        preFilterInputCoefficients_.add(0.0);
        preFilterOutputCoefficients_.add(0.0);

        weightingFilterInputCoefficients_.add(0.0);
        weightingFilterOutputCoefficients_.add(0.0);
    }

    previousSamplesPreFilterInput_.clear();
    previousSamplesPreFilterOutput_.clear();

    previousSamplesWeightingFilterInput_.clear();
    previousSamplesWeightingFilterOutput_.clear();

    // make sure there's no overlap yet
    sampleBuffer_.clear();
    overlapAddSamples_.clear();

    if (averageAlgorithm_ == KmeterPluginParameters::selAlgorithmItuBs1770)
    {
        calculateFilterKernel_ItuBs1770();

        // ITU-R BS.1770-1 provides its own peak-to-average gain
        // correction, so we don't need to apply any!
        setPeakToAverageCorrection(0.0f);
    }
    else
    {
        calculateFilterKernel_Rms();

        // RMS peak-to-average gain correction; this is simply the
        // level difference between the peak and RMS level of a sine
        // wave: RMS / A = sqrt(2) = +3.0103 dB
        setPeakToAverageCorrection(+3.0103f);
    }
}


/*  Set peak-to-average gain correction.

    peakToAverageCorrection: gain to add to average levels so that
	sine waves read the same on peak and average meters
*/
void AverageLevelFiltered::setPeakToAverageCorrection(
    float peakToAverageCorrection)

{
    peakToAverageCorrection_ = peakToAverageCorrection;
}


// calculate filter kernel for windowed-sinc low-pass filter (cutoff
// at 21.0 kHz)
void AverageLevelFiltered::calculateFilterKernel_Rms()
{
    float cutoffFrequency = 21000.0f;
    float relativeCutoffFrequency = cutoffFrequency / sampleRate_;

    int samples = bufferSize_ + 1;
    float samplesHalf = samples / 2.0f;

    // calculate filter kernel
    for (int i = 0; i < samples; ++i)
    {
        if (i == samplesHalf)
        {
            filterKernel_TD_[i] = float(2.0 * M_PI * relativeCutoffFrequency);
        }
        else
        {
            filterKernel_TD_[i] = float(sin(2.0 * M_PI * relativeCutoffFrequency * (i - samplesHalf)) / (i - samplesHalf) * (0.42 - 0.5 * cos(2.0 * (float) M_PI * i / samples) + 0.08 * cos(4.0 * (float) M_PI * i / samples)));
        }
    }

    // normalise filter kernel for unity gain at DC
    float nSumKernel = 0.0;

    for (int i = 0; i < samples; ++i)
    {
        nSumKernel += filterKernel_TD_[i];
    }

    for (int i = 0; i < samples; ++i)
    {
        filterKernel_TD_[i] = filterKernel_TD_[i] / nSumKernel;
    }

    // pad filter kernel with zeros
    for (int i = samples; i < fftSize_; ++i)
    {
        filterKernel_TD_[i] = 0.0f;
    }

    // calculate DFT of filter kernel
    fftwf_execute(filterKernelPlan_DFT_);
}


void AverageLevelFiltered::calculateFilterKernel_ItuBs1770()
{
    // filter specifications were taken from Raiden's nice paper
    // "ITU-R BS.1770-1 filter specifications (unofficial)" as found
    // on http://www.scribd.com/doc/49991813/ITU-R-BS-1770-1-filters
    //
    // please see here for Raiden's original forum thread:
    // http://www.hydrogenaudio.org/forums/index.php?showtopic=86116

    // initialise pre-filter (ITU-R BS.1770-1)
    double pf_vh = 1.584864701130855;
    double pf_vb = sqrt(pf_vh);
    double pf_vl = 1.0;
    double pf_q = 0.7071752369554196;
    double pf_cutoff = 1681.974450955533;
    double pf_omega = tan(M_PI * pf_cutoff / double(sampleRate_));
    double pf_omega_2 = pow(pf_omega, 2.0);
    double pf_omega_q = pf_omega / pf_q;
    double pf_div = (pf_omega_2 + pf_omega_q + 1.0);

    preFilterInputCoefficients_.set(0, (pf_vl * pf_omega_2 + pf_vb * pf_omega_q + pf_vh) / pf_div);
    preFilterInputCoefficients_.set(1, 2.0 * (pf_vl * pf_omega_2 - pf_vh) / pf_div);
    preFilterInputCoefficients_.set(2, (pf_vl * pf_omega_2 - pf_vb * pf_omega_q + pf_vh) / pf_div);

    preFilterOutputCoefficients_.set(0, -1.0);
    preFilterOutputCoefficients_.set(1, -2.0 * (pf_omega_2 - 1.0) / pf_div);
    preFilterOutputCoefficients_.set(2, -(pf_omega_2 - pf_omega_q + 1.0) / pf_div);

    // initialise RLB weighting curve (ITU-R BS.1770-1)
    double rlb_vh = 1.0;
    double rlb_vb = 0.0;
    double rlb_vl = 0.0;
    double rlb_q = 0.5003270373238773;
    double rlb_cutoff = 38.13547087602444;
    double rlb_omega = tan(M_PI * rlb_cutoff / double(sampleRate_));
    double rlb_omega_2 = pow(rlb_omega, 2.0);
    double rlb_omega_q = rlb_omega / rlb_q;
    double rlb_div_1 = (rlb_vl * rlb_omega_2 + rlb_vb * rlb_omega_q + rlb_vh);
    double rlb_div_2 = (rlb_omega_2 + rlb_omega_q + 1.0);

    weightingFilterInputCoefficients_.set(0, 1.0);
    weightingFilterInputCoefficients_.set(1, 2.0 * (rlb_vl * rlb_omega_2 - rlb_vh) / rlb_div_1);
    weightingFilterInputCoefficients_.set(2, (rlb_vl * rlb_omega_2 - rlb_vb * rlb_omega_q + rlb_vh) / rlb_div_1);

    weightingFilterOutputCoefficients_.set(0, -1.0);
    weightingFilterOutputCoefficients_.set(1, -2.0 * (rlb_omega_2 - 1.0) / rlb_div_2);
    weightingFilterOutputCoefficients_.set(2, -(rlb_omega_2 - rlb_omega_q + 1.0) / rlb_div_2);

    calculateFilterKernel_Rms();
}


// apply windowed-sinc low-pass filter (cutoff at 21.0 kHz) to samples
void AverageLevelFiltered::filterSamples_Rms(
    const int channel)

{
    jassert(channel >= 0);
    jassert(channel < numberOfChannels_);

    // copy audio data to temporary buffer as the sample buffer is not
    // optimised for MME
    memcpy(audioSamples_TD_,
           sampleBuffer_.getReadPointer(channel),
           bufferSize_ * sizeof(float));

    // pad audio data with zeros
    for (int sample = bufferSize_; sample < fftSize_; ++sample)
    {
        audioSamples_TD_[sample] = 0.0f;
    }

    // calculate DFT of audio data
    fftwf_execute(audioSamplesPlan_DFT_);

    // convolve audio data with filter kernel
    for (int i = 0; i < halfFftSize_; ++i)
    {
        // multiplication of complex numbers: index 0 contains the real
        // part, index 1 the imaginary part
        float realPart = audioSamples_FD_[i][0] * filterKernel_FD_[i][0] -
                         audioSamples_FD_[i][1] * filterKernel_FD_[i][1];
        float imagPart = audioSamples_FD_[i][1] * filterKernel_FD_[i][0] +
                         audioSamples_FD_[i][0] * filterKernel_FD_[i][1];

        audioSamples_FD_[i][0] = realPart;
        audioSamples_FD_[i][1] = imagPart;
    }

    // synthesise audio data from frequency spectrum (this destroys the
    // contents of "audioSamples_FD_"!!!)
    fftwf_execute(audioSamplesPlan_IDFT_);

    // normalise synthesised audio data
    float normaliser = float(fftSize_);

    for (int i = 0; i < fftSize_; ++i)
    {
        audioSamples_TD_[i] = audioSamples_TD_[i] / normaliser;
    }

    // copy data from temporary buffer back to sample buffer
    sampleBuffer_.copyFrom(channel, 0, audioSamples_TD_,
                           bufferSize_);

    // add old overlapping samples
    sampleBuffer_.addFrom(channel, 0, overlapAddSamples_,
                          channel, 0, bufferSize_);

    // store new overlapping samples
    overlapAddSamples_.copyFrom(channel, 0, audioSamples_TD_ + bufferSize_,
                                bufferSize_);
}


void AverageLevelFiltered::filterSamples_ItuBs1770()
{
    for (int channel = 0; channel < numberOfChannels_; ++channel)
    {
        // pre-filter
        previousSamplesOutputTemp_.clear();
        const float *samplesInput = sampleBuffer_.getReadPointer(channel);

        // temporary buffer with only one channel
        float *samplesOutput = previousSamplesOutputTemp_.getWritePointer(0);

        const float *samplesInputOld_1 = previousSamplesPreFilterInput_.getReadPointer(channel);
        const float *samplesOutputOld_1 = previousSamplesPreFilterOutput_.getReadPointer(channel);

        for (int sample = 0; sample < bufferSize_; ++sample)
        {
            double outputSum;

            if (sample < 2)
            {
                if (sample == 0)
                {
                    outputSum =
                        preFilterInputCoefficients_[0] * samplesInput[sample] +
                        preFilterInputCoefficients_[1] * samplesInputOld_1[1] +
                        preFilterInputCoefficients_[2] * samplesInputOld_1[0] +
                        preFilterOutputCoefficients_[1] * samplesOutputOld_1[1] +
                        preFilterOutputCoefficients_[2] * samplesOutputOld_1[0];
                }
                else
                {
                    outputSum =
                        preFilterInputCoefficients_[0] * samplesInput[sample] +
                        preFilterInputCoefficients_[1] * samplesInput[sample - 1] +
                        preFilterInputCoefficients_[2] * samplesInputOld_1[1] +
                        preFilterOutputCoefficients_[1] * samplesOutput[sample - 1] +
                        preFilterOutputCoefficients_[2] * samplesOutputOld_1[1];
                }
            }
            else
            {
                outputSum =
                    preFilterInputCoefficients_[0] * samplesInput[sample] +
                    preFilterInputCoefficients_[1] * samplesInput[sample - 1] +
                    preFilterInputCoefficients_[2] * samplesInput[sample - 2] +
                    preFilterOutputCoefficients_[1] * samplesOutput[sample - 1] +
                    preFilterOutputCoefficients_[2] * samplesOutput[sample - 2];
            }

            // dither output to float
            samplesOutput[sample] = dither_.dither(outputSum);

            // avoid underflows (1e-20f corresponds to -400 dBFS)
            if (fabs(samplesOutput[sample]) < 1e-20f)
            {
                samplesOutput[sample] = 0.0f;
            }
        }

        previousSamplesPreFilterInput_.copyFrom(
            channel, 0, sampleBuffer_,
            channel, bufferSize_ - 2, 2);

        previousSamplesPreFilterOutput_.copyFrom(
            channel, 0, previousSamplesOutputTemp_,
            0, bufferSize_ - 2, 2);

        sampleBuffer_.copyFrom(
            channel, 0, previousSamplesOutputTemp_,
            0, 0, bufferSize_);

        // RLB weighting filter
        previousSamplesOutputTemp_.clear();

        // clearing the buffer invalidates the pointers to its sample
        // data, so we need to update the pointers
        samplesOutput = previousSamplesOutputTemp_.getWritePointer(0);

        const float *samplesInputOld_2 = previousSamplesWeightingFilterInput_.getReadPointer(channel);
        const float *samplesOutputOld_2 = previousSamplesWeightingFilterOutput_.getReadPointer(channel);

        for (int sample = 0; sample < bufferSize_; ++sample)
        {
            double outputSum;

            if (sample < 2)
            {
                if (sample == 0)
                {
                    outputSum =
                        weightingFilterInputCoefficients_[0] * samplesInput[sample] +
                        weightingFilterInputCoefficients_[1] * samplesInputOld_2[1] +
                        weightingFilterInputCoefficients_[2] * samplesInputOld_2[0] +
                        weightingFilterOutputCoefficients_[1] * samplesOutputOld_2[1] +
                        weightingFilterOutputCoefficients_[2] * samplesOutputOld_2[0];
                }
                else
                {
                    outputSum =
                        weightingFilterInputCoefficients_[0] * samplesInput[sample] +
                        weightingFilterInputCoefficients_[1] * samplesInput[sample - 1] +
                        weightingFilterInputCoefficients_[2] * samplesInputOld_2[1] +
                        weightingFilterOutputCoefficients_[1] * samplesOutput[sample - 1] +
                        weightingFilterOutputCoefficients_[2] * samplesOutputOld_2[1];
                }
            }
            else
            {
                outputSum =
                    weightingFilterInputCoefficients_[0] * samplesInput[sample] +
                    weightingFilterInputCoefficients_[1] * samplesInput[sample - 1] +
                    weightingFilterInputCoefficients_[2] * samplesInput[sample - 2] +
                    weightingFilterOutputCoefficients_[1] * samplesOutput[sample - 1] +
                    weightingFilterOutputCoefficients_[2] * samplesOutput[sample - 2];
            }

            // dither output to float
            samplesOutput[sample] = dither_.dither(outputSum);

            // avoid underflows (1e-20f corresponds to -400 dBFS)
            if (fabs(samplesOutput[sample]) < 1e-20f)
            {
                samplesOutput[sample] = 0.0f;
            }
        }

        previousSamplesWeightingFilterInput_.copyFrom(channel, 0, sampleBuffer_, channel, bufferSize_ - 2, 2);
        previousSamplesWeightingFilterOutput_.copyFrom(channel, 0, previousSamplesOutputTemp_, 0, bufferSize_ - 2, 2);

        sampleBuffer_.copyFrom(channel, 0, previousSamplesOutputTemp_, 0, 0, bufferSize_);

        filterSamples_Rms(channel);
    }
}


float AverageLevelFiltered::getLevel(
    const int channel)

{
    jassert(channel >= 0);
    jassert(channel < numberOfChannels_);

    if (averageAlgorithm_ == KmeterPluginParameters::selAlgorithmItuBs1770)
    {
        float averageLevel = 0.0f;
        float meterMinimumDecibel = MeterBallistics::getMeterMinimumDecibel();
        float loudness = meterMinimumDecibel;

        if (channel == 0)
        {
            // filter audio data (all channels; overwrites contents of
            // sample buffer)
            filterSamples_ItuBs1770();

            for (int channel = 0; channel < numberOfChannels_; ++channel)
            {
                float averageLevelChannel = 0.0f;
                const float *sampleData = sampleBuffer_.getReadPointer(channel);

                // calculate mean square of the filtered input signal
                for (int n = 0; n < bufferSize_; ++n)
                {
                    averageLevelChannel += (sampleData[n] * sampleData[n]);
                }

                averageLevelChannel /= float(bufferSize_);

                // apply weighting factor and sum channels
                //
                // L, R, C  --> 1.00 (ignore factor)
                // LFE      --> 0.00 (skip channel)
                // LS, RS   --> 1.41
                // other    --> 0.00 (skip channel)
                if (channel < 3)
                {
                    averageLevel += averageLevelChannel;
                }
                else if (channel == 4)
                {
                    averageLevel += 1.41f * averageLevelChannel;
                }
                else if (channel == 5)
                {
                    averageLevel += 1.41f * averageLevelChannel;
                }
            }

            // calculate loudness by applying the formula from ITU-R
            // BS.1770-1; here's my guess to what the factors mean:
            //
            // -0.691 => 'K' filter frequency response at 1 kHz
            // 10.000 => factor for conversion to decibels (20.0) and
            //           square root for conversion from mean square
            //           to RMS (log10(sqrt(x)) = 0.5 * log10(x))
            loudness = -0.691f + 10.0f * log10f(averageLevel);

            if (loudness < meterMinimumDecibel)
            {
                loudness = meterMinimumDecibel;
            }
        }

        return loudness;
    }
    else
    {
        // filter audio data (overwrites contents of sample buffer)
        filterSamples_Rms(channel);

        float averageLevel = MeterBallistics::level2decibel(
                                 sampleBuffer_.getRMSLevel(
                                     channel, 0, bufferSize_));

        // apply peak-to-average gain correction so that sine waves
        // read the same on peak and average meters
        return averageLevel + peakToAverageCorrection_;
    }
}


void AverageLevelFiltered::copyFromBuffer(
    frut::audio::RingBuffer &ringBuffer,
    const unsigned int preDelay,
    const int sampleRate)

{
    // recalculate filter kernel when sample rate changes
    if (sampleRate_ != sampleRate)
    {
        sampleRate_ = sampleRate;
        calculateFilterKernel();
    }

    // copy data from ring buffer to sample buffer
    ringBuffer.copyToBuffer(sampleBuffer_, 0, bufferSize_, preDelay);
}


void AverageLevelFiltered::copyToBuffer(
    frut::audio::RingBuffer &destination,
    const unsigned int sourceStartSample,
    const unsigned int numSamples)

{
    // copy data from sample buffer to ring buffer
    destination.addSamples(sampleBuffer_, sourceStartSample, numSamples);
}


void AverageLevelFiltered::copyToBuffer(
    AudioBuffer<float> &destination,
    const int channel,
    const int destStartSample,
    const int numSamples)

{
    jassert(channel >= 0);
    jassert(channel < numberOfChannels_);
    jassert((destStartSample + numSamples) <= destination.getNumSamples());

    memcpy(destination.getWritePointer(channel, destStartSample),
           sampleBuffer_.getReadPointer(channel),
           numSamples * sizeof(float));
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
