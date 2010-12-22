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

#include "meter_segment.h"

MeterSegment::MeterSegment(const String &componentName, float fThreshold, float fRange, int nColor)
{
	// set component name 
	setName(componentName);

	// lower threshold, meter segment will be dark below this level
	fLowerThreshold = fThreshold;

	// level range above lower threshold that affects the brightness
	fThresholdRange = fRange;

	// upper threshold, meter segment will be lit above this level
	fUpperThreshold = fThreshold + fThresholdRange;

	fLevel = 0.0f;
	bPeak = false;

	// initialise meter segment's brightness (0.0f is dark, 1.0f is
	// fully lit)
	fBrightness = 0.0f;

	// set meter segment's hue from color number
	if (nColor == 0)
		// meter segment is red
		fHue = 0.0f;
	else if (nColor == 1)
		// meter segment is yellow
		fHue = 0.18f;
	else
		// meter segment is green
		fHue = 0.3f;
}

MeterSegment::~MeterSegment()
{
	// nothing to do, really
}

void MeterSegment::paint(Graphics& g)
{
	int width = getWidth();
	int height = getHeight();

	g.setColour(Colour(fHue, 1.0f, fBrightness, 1.0f));
	g.fillRect(1, 1, width - 2, height - 2);

	if (bPeak)
	{
		g.setColour(Colours::white);
		g.drawRect(0, 0, width, height);
	}
}

void MeterSegment::resized()
{
}

void MeterSegment::setLevels(float newLevel, float newPeak)
{
	fLevel = newLevel;

	// store old brightness and peak values
	float fBrightnessOld = fBrightness;
	bool bPeakOld = bPeak;

	if (fLevel > fUpperThreshold)
		fBrightness = 1.0f;
	else if (fLevel < (fLowerThreshold))
		fBrightness = 0.0f;
	else
		fBrightness = (fLevel - fLowerThreshold) / fThresholdRange;

	if ((newPeak > fLowerThreshold) && (newPeak <= fUpperThreshold))
		bPeak = true;
	else
		bPeak = false;

	fBrightness = fBrightness * 0.72f + 0.25f;

	// if brightness and/or peak have changed, re-paint meter segment
	if ((fBrightness != fBrightnessOld) || (bPeak != bPeakOld))
		repaint();
}
