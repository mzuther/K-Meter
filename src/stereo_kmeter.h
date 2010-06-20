/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010 Martin Zuther (http://www.mzuther.de/)

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

#ifndef __STEREO_KMETER_H__
#define __STEREO_KMETER_H__

#include "juce_library_code/juce_header.h"
#include "meter_bar.h"
#include "overflow_meter.h"
#include "plugin_processor.h"


//==============================================================================
/**
*/
class StereoKmeter : public Component
{
public:
    StereoKmeter(const String &componentName, int PosX, int PosY, int nHeadroom, bool bExpanded, int nSegmentHeight);
    ~StereoKmeter();

	void setLevels(MeterBallistics* pMB);
	void paint(Graphics& g);
	void resized();
	void visibilityChanged();

private:
	int nPosX;
	int nPosY;
	int nMainSegmentHeight;
	bool isExpanded;

	int nMeterHeadroom;

	MeterBar* PeakMeterLeft;
	MeterBar* PeakMeterRight;
	MeterBar* AverageMeterLeft;
	MeterBar* AverageMeterRight;

	OverflowMeter* OverflowMeterLeft;
	OverflowMeter* OverflowMeterRight;

	void drawMarkers(Graphics& g, String& strMarker, int x, int y, int width, int height);
};


#endif  // __STEREO_KMETER_H__
