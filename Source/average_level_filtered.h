/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2021 Martin Zuther (http://www.mzuther.de/)

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

#ifndef KMETER_AVERAGE_LEVEL_FILTERED_H
#define KMETER_AVERAGE_LEVEL_FILTERED_H

#include "FrutHeader.h"
#include "meter_ballistics.h"


class AverageLevelFiltered :
   public frut::dsp::FIRFilterBox
{
public:
   static constexpr int KMETER_MAXIMUM_FILTER_STAGES = 3;

   AverageLevelFiltered( const int numberOfChannels,
                         const double sampleRate,
                         const int fftBufferSize,
                         const int averageAlgorithm );

   virtual ~AverageLevelFiltered();
   virtual void reset();

   int getAlgorithm() const;
   void setAlgorithm( const int averageAlgorithm );

   float getLevel( const int channel );

   void copyTo( AudioBuffer<float>& destination,
                const int numberOfSamples );

   void copyFrom( const AudioBuffer<float>& source,
                  const int numberOfSamples );

private:
   JUCE_LEAK_DETECTOR( AverageLevelFiltered );

   void calculateFilterKernel();
   void calculateFilterKernel_Rms();
   void calculateFilterKernel_ItuBs1770();

   void calculateLoudness();
   void filterSamples_Rms( const int channel );
   void filterSamples_ItuBs1770();

   double sampleRate_;

   Array<float> loudnessValues_;

   Array<double> preFilterInputCoefficients_;
   Array<double> preFilterOutputCoefficients_;

   Array<double> weightingFilterInputCoefficients_;
   Array<double> weightingFilterOutputCoefficients_;

   AudioBuffer<float> previousSamplesPreFilterInput_;
   AudioBuffer<float> previousSamplesPreFilterOutput_;

   AudioBuffer<float> previousSamplesWeightingFilterInput_;
   AudioBuffer<float> previousSamplesWeightingFilterOutput_;

   AudioBuffer<float> previousSamplesOutputTemp_;

   frut::dsp::Dither dither_;

   int averageAlgorithm_;
   float peakToAverageCorrection_;
};

#endif  // KMETER_AVERAGE_LEVEL_FILTERED_H
