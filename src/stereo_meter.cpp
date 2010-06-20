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

#include "stereo_meter.h"

StereoMeter::StereoMeter(const String &componentName, int PosX, int PosY, int Width, int Height)
{
	setName(componentName);

	fValue = 0.0f;

	nPosX = PosX;
	nPosY = PosY;
	nWidth = Width;
	nHeight = Height;
}

StereoMeter::~StereoMeter()
{
}

void StereoMeter::paint(Graphics& g)
{
	int width = getWidth();
	int height = getHeight();
	int x = int ((1.0f + fValue) / 2.0f * (width - 4) + 2);
	int middle = width / 2;

	g.setColour(Colours::black);
	g.fillRect(1, 2, width - 2, height - 4);

	g.setColour(Colours::darkgrey.darker(0.3f));
	g.drawRect(0, 1, width, height - 2, 1);

	g.setColour(Colours::darkgrey);
	for (int y=2; y < (height - 2); y++)
		g.setPixel(middle, y);

	g.setColour(Colours::white);

	// upper arrow
	g.setPixel(middle - 1, 0);
	g.setPixel(middle + 1, 0);
	g.setPixel(middle, 0);
	g.setPixel(middle, 1);

	// lower arrow
	g.setPixel(middle, height - 2);
	g.setPixel(middle, height - 1);
	g.setPixel(middle - 1, height - 1);
	g.setPixel(middle + 1, height - 1);

	g.setFont(11.0f);
	g.drawFittedText(T("L"), 0, 1, height - 2, height - 2, Justification::centred, 1, 1.0f);
	g.drawFittedText(T("R"), width - height + 1, 1, height - 2, height - 2, Justification::centred, 1, 1.0f);

	g.setColour(Colours::red);
	for (int y=2; y < (height - 2); y++)
		g.setPixel(x, y);

	g.setColour(Colours::red.withAlpha(0.6f));

	for (int y=2; y < (height - 2); y++)
	{
		g.setPixel(x - 1, y);
		g.setPixel(x + 1, y);
	}
}

void StereoMeter::visibilityChanged()
{
	setBounds(nPosX, nPosY, nWidth, nHeight);
}

void StereoMeter::setValue(float newValue)
{
	if (fValue == newValue)
		return;

	fValue = newValue;
	repaint();
}
