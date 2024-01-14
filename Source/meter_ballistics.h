/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2024 Martin Zuther (http://www.mzuther.de/)

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

#ifndef KMETER_METER_BALLISTICS_H
#define KMETER_METER_BALLISTICS_H

#include "FrutHeader.h"
#include "plugin_parameters.h"


class MeterBallistics
{
public:
   MeterBallistics( int nChannels,
                    int AverageAlgorithm,
                    bool bPeakMeterInfiniteHold,
                    bool bAverageMeterInfiniteHold );

   void setAverageAlgorithm( int AverageAlgorithm );
   void setPeakMeterInfiniteHold( bool bInfiniteHold );
   void setAverageMeterInfiniteHold( bool bInfiniteHold );
   void reset();

   int getNumberOfChannels();

   float getPeakMeterLevel( int nChannel );
   float getPeakMeterPeakLevel( int nChannel );

   float getTruePeakMeterLevel( int nChannel );
   float getTruePeakMeterPeakLevel( int nChannel );

   float getAverageMeterLevel( int nChannel );
   float getAverageMeterPeakLevel( int nChannel );

   float getMaximumPeakLevel( int nChannel );
   float getMaximumTruePeakLevel( int nChannel );
   int getNumberOfOverflows( int nChannel );

   float getStereoMeterValue();
   void setStereoMeterValue( float fTimePassed,
                             float fStereoMeterValueNew );

   float getPhaseCorrelation();
   void setPhaseCorrelation( float fTimePassed,
                             float fPhaseCorrelationNew );

   void updateChannel( int nChannel,
                       float fTimePassed,
                       float fPeak,
                       float fTruePeak,
                       float fAverageFiltered,
                       int nOverflows );

   static float level2decibel( float fLevel );
   static double decibel2level_double( double dDecibels );

   static float decibel2level( float fDecibels );
   static double level2decibel_double( double dLevel );

   static float getMeterMinimumDecibel();
private:
   JUCE_LEAK_DETECTOR( MeterBallistics );

   int nNumberOfChannels;
   int nAverageAlgorithm;

   static float fMeterMinimumDecibel;
   static float fPeakToAverageCorrection;

   Array<float> arrPeakMeterLevels;
   Array<float> arrPeakMeterPeakLevels;

   Array<float> arrTruePeakMeterLevels;
   Array<float> arrTruePeakMeterPeakLevels;

   Array<float> arrAverageMeterLevels;
   Array<float> arrAverageMeterPeakLevels;

   Array<float> arrMaximumPeakLevels;
   Array<float> arrMaximumTruePeakLevels;
   Array<int> arrNumberOfOverflows;

   Array<float> arrPeakMeterPeakLastChanged;
   Array<float> arrTruePeakMeterPeakLastChanged;
   Array<float> arrAverageMeterPeakLastChanged;

   float fStereoMeterValue;
   float fPhaseCorrelation;

   float PeakMeterBallistics( float fTimePassed,
                              float fPeakLevelCurrent,
                              float fPeakLevelOld );

   float PeakMeterPeakBallistics( float fTimePassed,
                                  float& fLastChanged,
                                  float fPeakLevelCurrent,
                                  float fPeakLevelOld );

   void AverageMeterBallistics( int nChannel,
                                float fTimePassed,
                                float fAverageLevelCurrent );

   float AverageMeterPeakBallistics( float fTimePassed,
                                     float& fLastChanged,
                                     float fAverageLevelCurrent,
                                     float fAverageLevelOld );

   void StereoMeterBallistics( float fTimePassed,
                               float fStereoMeterCurrent );

   void PhaseCorrelationMeterBallistics( float fTimePassed,
                                         float fPhaseCorrelationCurrent );

   void LogMeterBallistics( float fMeterInertia,
                            float fTimePassed,
                            float fLevel,
                            float& fReadout );
};

#endif  // KMETER_METER_BALLISTICS_H
