/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2020 Martin Zuther (http://www.mzuther.de/)

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


AverageLevelFiltered::AverageLevelFiltered( const int numberOfChannels,
                                            const double sampleRate,
                                            const int fftBufferSize,
                                            const int averageAlgorithm ) :

   frut::dsp::FIRFilterBox(
      KmeterPluginParameters::getResourceDirectory(),
      numberOfChannels,
      fftBufferSize ),
   sampleRate_( sampleRate ),
   previousSamplesPreFilterInput_(
      numberOfChannels_, KMETER_MAXIMUM_FILTER_STAGES - 1 ),
   previousSamplesPreFilterOutput_(
      numberOfChannels_, KMETER_MAXIMUM_FILTER_STAGES - 1 ),
   previousSamplesWeightingFilterInput_(
      numberOfChannels_, KMETER_MAXIMUM_FILTER_STAGES - 1 ),
   previousSamplesWeightingFilterOutput_(
      numberOfChannels_, KMETER_MAXIMUM_FILTER_STAGES - 1 ),
   previousSamplesOutputTemp_( 1, fftBufferSize_ )
{
   dither_.initialise( numberOfChannels_, 24 );

   peakToAverageCorrection_ = 0.0f;
   averageAlgorithm_ = -1;

   float meterMinimumDecibel = MeterBallistics::getMeterMinimumDecibel();

   for ( int channel = 0; channel < numberOfChannels_; ++channel ) {
      loudnessValues_.add( meterMinimumDecibel );
   }

   // also calculates filter kernel
   setAlgorithm( averageAlgorithm );
}


AverageLevelFiltered::~AverageLevelFiltered()
{
}


void AverageLevelFiltered::reset()
{
   FIRFilterBox::reset();

   float meterMinimumDecibel = MeterBallistics::getMeterMinimumDecibel();
   loudnessValues_.fill( meterMinimumDecibel );

   previousSamplesPreFilterInput_.clear();
   previousSamplesPreFilterOutput_.clear();

   previousSamplesWeightingFilterInput_.clear();
   previousSamplesWeightingFilterOutput_.clear();

   previousSamplesOutputTemp_.clear();
}


int AverageLevelFiltered::getAlgorithm() const
{
   return averageAlgorithm_;
}


void AverageLevelFiltered::setAlgorithm( const int averageAlgorithm )
{
   if ( averageAlgorithm == averageAlgorithm_ ) {
      return;
   }

   DBG( String( "[K-Meter] averaging algorithm: " ) +
        ( averageAlgorithm == KmeterPluginParameters::selAlgorithmItuBs1770 ?
          "ITU-R BS.1770-1" : "RMS" ) );

   if ( ( averageAlgorithm >= 0 ) &&
        ( averageAlgorithm < KmeterPluginParameters::nNumAlgorithms ) ) {
      averageAlgorithm_ = averageAlgorithm;
   } else {
      averageAlgorithm_ = KmeterPluginParameters::selAlgorithmItuBs1770;
   }

   calculateFilterKernel();
}


void AverageLevelFiltered::calculateFilterKernel()
{
   // reset IIR coefficients and previous samples
   preFilterInputCoefficients_.clear();
   preFilterOutputCoefficients_.clear();

   weightingFilterInputCoefficients_.clear();
   weightingFilterOutputCoefficients_.clear();

   for ( int stage = 0; stage < KMETER_MAXIMUM_FILTER_STAGES; ++stage ) {
      preFilterInputCoefficients_.add( 0.0 );
      preFilterOutputCoefficients_.add( 0.0 );

      weightingFilterInputCoefficients_.add( 0.0 );
      weightingFilterOutputCoefficients_.add( 0.0 );
   }

   previousSamplesPreFilterInput_.clear();
   previousSamplesPreFilterOutput_.clear();

   previousSamplesWeightingFilterInput_.clear();
   previousSamplesWeightingFilterOutput_.clear();

   // make sure there's no overlap yet
   fftSampleBuffer_.clear();
   fftOverlapAddSamples_.clear();

   // set peak-to-average gain correction, the gain to add to average
   // levels so that sine waves read the same on peak and average
   // meters
   if ( averageAlgorithm_ == KmeterPluginParameters::selAlgorithmItuBs1770 ) {
      calculateFilterKernel_ItuBs1770();

      // ITU-R BS.1770-1 provides its own peak-to-average gain
      // correction, so we don't need to apply any!
      peakToAverageCorrection_ = 0.0f;
   } else {
      calculateFilterKernel_Rms();

      // RMS peak-to-average gain correction; this is simply the
      // level difference between the peak and RMS level of a sine
      // wave: RMS / A = sqrt(2) = +3.0103 dB
      peakToAverageCorrection_ = +3.0103f;
   }
}


// calculate filter kernel for windowed-sinc low-pass filter (cutoff
// at 21.0 kHz)
void AverageLevelFiltered::calculateFilterKernel_Rms()
{
   double cutoffFrequency = 21000.0;
   double relativeCutoffFrequency = cutoffFrequency / sampleRate_;

   calculateKernelWindowedSincLPF( relativeCutoffFrequency );
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
   double pf_vb = sqrt( pf_vh );
   double pf_vl = 1.0;
   double pf_q = 0.7071752369554196;
   double pf_cutoff = 1681.974450955533;
   double pf_omega = tan( M_PI * pf_cutoff / sampleRate_ );
   double pf_omega_2 = pow( pf_omega, 2.0 );
   double pf_omega_q = pf_omega / pf_q;
   double pf_div = ( pf_omega_2 + pf_omega_q + 1.0 );

   preFilterInputCoefficients_.set( 0, ( pf_vl * pf_omega_2 + pf_vb * pf_omega_q + pf_vh ) / pf_div );
   preFilterInputCoefficients_.set( 1, 2.0 * ( pf_vl * pf_omega_2 - pf_vh ) / pf_div );
   preFilterInputCoefficients_.set( 2, ( pf_vl * pf_omega_2 - pf_vb * pf_omega_q + pf_vh ) / pf_div );

   preFilterOutputCoefficients_.set( 0, -1.0 );
   preFilterOutputCoefficients_.set( 1, -2.0 * ( pf_omega_2 - 1.0 ) / pf_div );
   preFilterOutputCoefficients_.set( 2, -( pf_omega_2 - pf_omega_q + 1.0 ) / pf_div );

   // initialise RLB weighting curve (ITU-R BS.1770-1)
   double rlb_vh = 1.0;
   double rlb_vb = 0.0;
   double rlb_vl = 0.0;
   double rlb_q = 0.5003270373238773;
   double rlb_cutoff = 38.13547087602444;
   double rlb_omega = tan( M_PI * rlb_cutoff / sampleRate_ );
   double rlb_omega_2 = pow( rlb_omega, 2.0 );
   double rlb_omega_q = rlb_omega / rlb_q;
   double rlb_div_1 = ( rlb_vl * rlb_omega_2 + rlb_vb * rlb_omega_q + rlb_vh );
   double rlb_div_2 = ( rlb_omega_2 + rlb_omega_q + 1.0 );

   weightingFilterInputCoefficients_.set( 0, 1.0 );
   weightingFilterInputCoefficients_.set( 1, 2.0 * ( rlb_vl * rlb_omega_2 - rlb_vh ) / rlb_div_1 );
   weightingFilterInputCoefficients_.set( 2, ( rlb_vl * rlb_omega_2 - rlb_vb * rlb_omega_q + rlb_vh ) / rlb_div_1 );

   weightingFilterOutputCoefficients_.set( 0, -1.0 );
   weightingFilterOutputCoefficients_.set( 1, -2.0 * ( rlb_omega_2 - 1.0 ) / rlb_div_2 );
   weightingFilterOutputCoefficients_.set( 2, -( rlb_omega_2 - rlb_omega_q + 1.0 ) / rlb_div_2 );

   calculateFilterKernel_Rms();
}


// apply windowed-sinc low-pass filter (cutoff at 21.0 kHz) to samples
void AverageLevelFiltered::filterSamples_Rms( const int channel )
{
   convolveWithKernel( channel );
}


void AverageLevelFiltered::filterSamples_ItuBs1770()
{
   for ( int channel = 0; channel < numberOfChannels_; ++channel ) {
      // pre-filter
      previousSamplesOutputTemp_.clear();
      const float* samplesInput = fftSampleBuffer_.getReadPointer( channel );

      // temporary buffer with only one channel
      float* samplesOutput = previousSamplesOutputTemp_.getWritePointer( 0 );

      const float* samplesInputOld_1 = previousSamplesPreFilterInput_.getReadPointer( channel );
      const float* samplesOutputOld_1 = previousSamplesPreFilterOutput_.getReadPointer( channel );

      for ( int sample = 0; sample < fftBufferSize_; ++sample ) {
         double outputSum;

         if ( sample < 2 ) {
            if ( sample == 0 ) {
               outputSum =
                  preFilterInputCoefficients_[0] * samplesInput[sample] +
                  preFilterInputCoefficients_[1] * samplesInputOld_1[1] +
                  preFilterInputCoefficients_[2] * samplesInputOld_1[0] +
                  preFilterOutputCoefficients_[1] * samplesOutputOld_1[1] +
                  preFilterOutputCoefficients_[2] * samplesOutputOld_1[0];
            } else {
               outputSum =
                  preFilterInputCoefficients_[0] * samplesInput[sample] +
                  preFilterInputCoefficients_[1] * samplesInput[sample - 1] +
                  preFilterInputCoefficients_[2] * samplesInputOld_1[1] +
                  preFilterOutputCoefficients_[1] * samplesOutput[sample - 1] +
                  preFilterOutputCoefficients_[2] * samplesOutputOld_1[1];
            }
         } else {
            outputSum =
               preFilterInputCoefficients_[0] * samplesInput[sample] +
               preFilterInputCoefficients_[1] * samplesInput[sample - 1] +
               preFilterInputCoefficients_[2] * samplesInput[sample - 2] +
               preFilterOutputCoefficients_[1] * samplesOutput[sample - 1] +
               preFilterOutputCoefficients_[2] * samplesOutput[sample - 2];
         }

         // dither output to float
         samplesOutput[sample] = dither_.ditherSample( channel, outputSum );

         // avoid underflows (1e-20f corresponds to -400 dBFS)
         if ( fabs( samplesOutput[sample] ) < 1e-20f ) {
            samplesOutput[sample] = 0.0f;
         }
      }

      previousSamplesPreFilterInput_.copyFrom(
         channel, 0, fftSampleBuffer_,
         channel, fftBufferSize_ - 2, 2 );

      previousSamplesPreFilterOutput_.copyFrom(
         channel, 0, previousSamplesOutputTemp_,
         0, fftBufferSize_ - 2, 2 );

      fftSampleBuffer_.copyFrom(
         channel, 0, previousSamplesOutputTemp_,
         0, 0, fftBufferSize_ );

      // RLB weighting filter
      previousSamplesOutputTemp_.clear();

      // clearing the buffer invalidates the pointers to its sample
      // data, so we need to update the pointers
      samplesOutput = previousSamplesOutputTemp_.getWritePointer( 0 );

      const float* samplesInputOld_2 = previousSamplesWeightingFilterInput_.getReadPointer( channel );
      const float* samplesOutputOld_2 = previousSamplesWeightingFilterOutput_.getReadPointer( channel );

      for ( int sample = 0; sample < fftBufferSize_; ++sample ) {
         double outputSum;

         if ( sample < 2 ) {
            if ( sample == 0 ) {
               outputSum =
                  weightingFilterInputCoefficients_[0] * samplesInput[sample] +
                  weightingFilterInputCoefficients_[1] * samplesInputOld_2[1] +
                  weightingFilterInputCoefficients_[2] * samplesInputOld_2[0] +
                  weightingFilterOutputCoefficients_[1] * samplesOutputOld_2[1] +
                  weightingFilterOutputCoefficients_[2] * samplesOutputOld_2[0];
            } else {
               outputSum =
                  weightingFilterInputCoefficients_[0] * samplesInput[sample] +
                  weightingFilterInputCoefficients_[1] * samplesInput[sample - 1] +
                  weightingFilterInputCoefficients_[2] * samplesInputOld_2[1] +
                  weightingFilterOutputCoefficients_[1] * samplesOutput[sample - 1] +
                  weightingFilterOutputCoefficients_[2] * samplesOutputOld_2[1];
            }
         } else {
            outputSum =
               weightingFilterInputCoefficients_[0] * samplesInput[sample] +
               weightingFilterInputCoefficients_[1] * samplesInput[sample - 1] +
               weightingFilterInputCoefficients_[2] * samplesInput[sample - 2] +
               weightingFilterOutputCoefficients_[1] * samplesOutput[sample - 1] +
               weightingFilterOutputCoefficients_[2] * samplesOutput[sample - 2];
         }

         // dither output to float
         samplesOutput[sample] = dither_.ditherSample( channel, outputSum );

         // avoid underflows (1e-20f corresponds to -400 dBFS)
         if ( fabs( samplesOutput[sample] ) < 1e-20f ) {
            samplesOutput[sample] = 0.0f;
         }
      }

      previousSamplesWeightingFilterInput_.copyFrom( channel, 0, fftSampleBuffer_, channel, fftBufferSize_ - 2, 2 );
      previousSamplesWeightingFilterOutput_.copyFrom( channel, 0, previousSamplesOutputTemp_, 0, fftBufferSize_ - 2, 2 );

      fftSampleBuffer_.copyFrom( channel, 0, previousSamplesOutputTemp_, 0, 0, fftBufferSize_ );

      convolveWithKernel( channel );
   }
}


float AverageLevelFiltered::getLevel( const int channel )
{
   jassert( isPositiveAndNotGreaterThan( channel, numberOfChannels_ ) );

   return loudnessValues_[channel];
}


// copy data from internal audio buffer to external audio buffer
void AverageLevelFiltered::copyTo( AudioBuffer<float>& destination,
                                   const int numberOfSamples )
{
   jassert( fftSampleBuffer_.getNumChannels() ==
            destination.getNumChannels() );
   jassert( fftSampleBuffer_.getNumSamples() >=
            numberOfSamples );
   jassert( destination.getNumSamples() >=
            numberOfSamples );

   int numberOfChannels = fftSampleBuffer_.getNumChannels();

   // copy data to external buffer
   for ( int channel = 0; channel < numberOfChannels; ++channel ) {
      destination.copyFrom( channel, 0,
                            fftSampleBuffer_,
                            channel, 0,
                            numberOfSamples );
   }
}


// copy data from external audio buffer to internal audio buffer
void AverageLevelFiltered::copyFrom( const AudioBuffer<float>& source,
                                     const int numberOfSamples )
{
   jassert( fftSampleBuffer_.getNumChannels() ==
            source.getNumChannels() );
   jassert( fftSampleBuffer_.getNumSamples() ==
            numberOfSamples );

   int numberOfChannels = fftSampleBuffer_.getNumChannels();

   // copy data to internal buffer
   for ( int channel = 0; channel < numberOfChannels; ++channel ) {
      fftSampleBuffer_.copyFrom( channel, 0,
                                 source,
                                 channel, 0,
                                 numberOfSamples );
   }

   // calculate loudness for all channels
   calculateLoudness();
}


void AverageLevelFiltered::calculateLoudness()
{
   float meterMinimumDecibel = MeterBallistics::getMeterMinimumDecibel();

   if ( averageAlgorithm_ == KmeterPluginParameters::selAlgorithmItuBs1770 ) {
      // filter audio data (overwrites contents of sample buffer)
      filterSamples_ItuBs1770();

      float averageLevel = 0.0f;

      for ( int channel = 0; channel < numberOfChannels_; ++channel ) {
         float averageLevelChannel = 0.0f;
         const float* sampleData = fftSampleBuffer_.getReadPointer( channel );

         // calculate mean square of the filtered input signal
         for ( int n = 0; n < fftBufferSize_; ++n ) {
            averageLevelChannel += ( sampleData[n] * sampleData[n] );
         }

         averageLevelChannel /= float( fftBufferSize_ );

         // apply weighting factor and sum channels
         //
         // L, R, C  ==> 1.00 (ignore factor)
         // LFE      ==> 0.00 (skip channel)
         // LS, RS   ==> 1.41
         // other    ==> 0.00 (skip channel)
         if ( channel < 3 ) {
            averageLevel += averageLevelChannel;
         } else if ( channel == 4 ) {
            averageLevel += 1.41f * averageLevelChannel;
         } else if ( channel == 5 ) {
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
      float loudness = -0.691f + 10.0f * log10f( averageLevel );

      if ( loudness < meterMinimumDecibel ) {
         loudness = meterMinimumDecibel;
      }

      for ( int channel = 0; channel < numberOfChannels_; ++channel ) {
         loudnessValues_.set( channel, loudness );
      }
   } else {
      for ( int channel = 0; channel < numberOfChannels_; ++channel ) {
         // filter audio data (overwrites contents of sample buffer)
         filterSamples_Rms( channel );

         float averageLevel = MeterBallistics::level2decibel(
                                 fftSampleBuffer_.getRMSLevel(
                                    channel, 0, fftBufferSize_ ) );

         // apply peak-to-average gain correction so that sine
         // waves read the same on peak and average meters
         averageLevel += peakToAverageCorrection_;

         if ( averageLevel < meterMinimumDecibel ) {
            averageLevel = meterMinimumDecibel;
         }

         loudnessValues_.set( channel, averageLevel );
      }
   }
}
