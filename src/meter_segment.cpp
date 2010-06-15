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
	setName(componentName);

	fLevelThreshold = fThreshold;
	fLevelRange = fRange;

	fLevel = 0.0f;
	bPeak = false;
	
	fBrightness = 0.0f;

	if (nColor == 0)
		fHue = 0.0f;
	else if (nColor == 1)
		fHue = 0.18f;
	else
		fHue = 0.3f;
}

MeterSegment::~MeterSegment()
{
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

	float fBrightnessOld = fBrightness;
	bool bPeakOld = bPeak;

	if (fLevel > (fLevelThreshold + fLevelRange))
		fBrightness = 1.0f;
	else if (fLevel < (fLevelThreshold))
		fBrightness = 0.0f;
	else
		fBrightness = (fLevel - fLevelThreshold) / fLevelRange;

	if ((newPeak > fLevelThreshold) & (newPeak <= (fLevelThreshold + fLevelRange)))
		bPeak = true;
	else
		bPeak = false;

	fBrightness = fBrightness * 0.72f + 0.25f;

	if ((fBrightness != fBrightnessOld) | (bPeak != bPeakOld))
		repaint();
}
