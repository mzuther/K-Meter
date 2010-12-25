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

	// level range above lower threshold; this affects the brightness
	fThresholdRange = fRange;

	// upper threshold, meter segment will be fully lit above this
	// level
	fUpperThreshold = fThreshold + fThresholdRange;

	// current level in decibel (initialise to a very low level so
	// segment won't be lit by default)
	fLevel = -1000.0f;

	// show peak level marker on segment?
	bPeakMarker = false;

	// initialise meter segment's brightness (0.0f is dark, 1.0f is
	// fully lit)
	fBrightness = 0.0f;

	// set meter segment's hue from colour number
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
	// get meter segment's screen dimensions
	int width = getWidth();
	int height = getHeight();

	// initialise meter segment's colour from hue and brightness
	g.setColour(Colour(fHue, 1.0f, fBrightness, 1.0f));

	// fill meter segment with solid colour, but leave a border of one
	// pixel for peak marker
	g.fillRect(1, 1, width - 2, height - 2);

	// if peak marker is lit, draw a white rectangle around meter
	// segment (width: 1 pixel)
	if (bPeakMarker)
	{
		g.setColour(Colours::white);
		g.drawRect(0, 0, width, height);
	}
}


void MeterSegment::visibilityChanged()
{
	// if this function did not exist, the meter segment wouldn't be
	// drawn until the first level change!
}


void MeterSegment::resized()
{
}


void MeterSegment::setLevels(float newLevel, float newPeakLevel)
{
	// update current level
	fLevel = newLevel;

	// store old brightness and peak marker values
	float fBrightnessOld = fBrightness;
	bool bPeakMarkerOld = bPeakMarker;

	// current level lies on or above upper threshold, so fully light
	// meter segment
	if (fLevel >= fUpperThreshold)
		fBrightness = 0.97f;
	// current level lies on or below lower threshold, so set meter
	// segment to dark
	else if (fLevel <= (fLowerThreshold))
		fBrightness = 0.25f;
	// current level lies within thresholds, so calculate brightness
	// from current level
	else
	{
		fBrightness = (fLevel - fLowerThreshold) / fThresholdRange;

		// to look well, meter segments should be left with some
		// colour and not have maximum brightness
		fBrightness = fBrightness * 0.72f + 0.25f;
	}

	// if meter's peak level lies above lower threshold and below or
	// on upper threshold, show peak marker on segment
	if ((newPeakLevel > fLowerThreshold) && (newPeakLevel <= fUpperThreshold))
		bPeakMarker = true;
	// otherwise, do not show peak marker on segment
	else
		bPeakMarker = false;

	// re-paint meter segment only when brightness or peak marker have
	// changed,
	if ((fBrightness != fBrightnessOld) || (bPeakMarker != bPeakMarkerOld))
		repaint();
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
