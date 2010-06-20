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

#include "meter_bar.h"

MeterBar::MeterBar(const String &componentName, int posX, int posY, int Width, int nHeadroom, bool bExpanded, int nSegmentHeight, String justify)
{
	setName(componentName);
	isExpanded = bExpanded;

	if (isExpanded)
	{
		nNumberOfBars = 134;

		if (nHeadroom == 0)
		{
			nMeterHeadroom = 0;

			nLimitTopBars = 40;
			nLimitRedBars = nLimitTopBars;
			nLimitAmberBars = nLimitRedBars + 40;
			nLimitGreenBars_1 = 800;
			nLimitGreenBars_2 = nLimitGreenBars_1;
		}
		else if (nHeadroom == 12)
		{
			nMeterHeadroom = 12;

			nLimitTopBars = 80;
			nLimitRedBars = nLimitTopBars;
			nLimitAmberBars = nLimitRedBars + 40;
			nLimitGreenBars_1 = 800;
			nLimitGreenBars_2 = nLimitGreenBars_1;
		}
		else if (nHeadroom == 14)
		{
			nMeterHeadroom = 14;

			nLimitTopBars = 100;
			nLimitRedBars = nLimitTopBars;
			nLimitAmberBars = nLimitRedBars + 40;
			nLimitGreenBars_1 = 800;
			nLimitGreenBars_2 = nLimitGreenBars_1;
		}
		else
		{
			nMeterHeadroom = 20;

			nLimitTopBars = 160;
			nLimitRedBars = nLimitTopBars;
			nLimitAmberBars = nLimitRedBars + 40;
			nLimitGreenBars_1 = 800;
			nLimitGreenBars_2 = nLimitGreenBars_1;
		}
	}
	else
	{
		if (nHeadroom == 0) {
			nMeterHeadroom = 0;
			nNumberOfBars = 47;

			nLimitTopBars = 4;
			nLimitRedBars = 6;
			nLimitAmberBars = 14;
			nLimitGreenBars_1 = 42;
			nLimitGreenBars_2 = nLimitGreenBars_1;
		}
		else if (nHeadroom == 12)
		{
			nMeterHeadroom = 12;
			nNumberOfBars = 48;

			nLimitTopBars = 4;
			nLimitRedBars = 10;
			nLimitAmberBars = 14;
			nLimitGreenBars_1 = 44;
			nLimitGreenBars_2 = nLimitGreenBars_1;
		}
		else if (nHeadroom == 14)
		{
			nMeterHeadroom = 14;
			nNumberOfBars = 50;

			nLimitTopBars = 4;
			nLimitRedBars = 12;
			nLimitAmberBars = 16;
			nLimitGreenBars_1 = 46;
			nLimitGreenBars_2 = nLimitGreenBars_1;
		}
		else
		{
			nMeterHeadroom = 20;
			nNumberOfBars = 51;

			nLimitTopBars = 4;
			nLimitRedBars = 18;
			nLimitAmberBars = 22;
			nLimitGreenBars_1 = 46;
			nLimitGreenBars_2 = 47;
		}
	}

	nPosX = posX;
	nPosY = posY;
	nWidth = Width;
	nMainSegmentHeight = nSegmentHeight;
	justifyMeter = justify;

	fLevel = 0.0f;

	float fThreshold = 0.0f;
	float fRange = 0.0f;
	int nColor = 0;

	MeterArray = new MeterSegment*[nNumberOfBars];

	for (int n=0; n < nNumberOfBars; n++)
	{
		if (isExpanded)
		{
			fRange = 0.1f;
			nColor = 0;
		}
		else
		{
			if (n < nLimitTopBars)
				fRange = 0.5f;
			else if (n < nLimitGreenBars_1)
				fRange = 1.0f;
			else if (n < nLimitGreenBars_2)
				fRange = 6.0f;
			else
				fRange = 10.0f;
		}

		if (n < nLimitRedBars)
			nColor = 0;
		else if (n < nLimitAmberBars)
			nColor = 1;
		else
			nColor = 2;

		fThreshold -= fRange;
		MeterArray[n] = new MeterSegment(String(T("MeterSegment ") + n), fThreshold, fRange, nColor);

		addAndMakeVisible(MeterArray[n]);
	}
}

MeterBar::~MeterBar()
{
	for (int n = 0; n < nNumberOfBars; n++)
	{
		removeChildComponent(MeterArray[n]);
		delete MeterArray[n];
	}

	delete [] MeterArray;

	deleteAllChildren();
}

void MeterBar::visibilityChanged()
{
	int x = 0;
	int y = 0;
	int width = nWidth;
	int height = 134 * nMainSegmentHeight + 1;
	int segment_height = nMainSegmentHeight;

	setBounds(nPosX, nPosY, width, height);

	for (int n=0; n < nNumberOfBars; n++)
	{
		if (n < nLimitRedBars)
		{
			width = nWidth;
			x = 0;
		}
		else if (n < nLimitAmberBars)
		{
			if (justifyMeter == T("left"))
			{
				width = (int) (nWidth * 0.85f);
				x = nWidth - width;
			}
			else if (justifyMeter == T("right"))
			{
				width = (int) (nWidth * 0.85f);
				x = 0;
			}
			else
			{
				width = nWidth;
				x = 0;
			}
		}
		else
		{
			if (justifyMeter == T("left"))
			{
				width = (int) (nWidth * 0.75f);
				x = nWidth - width;
			}
			else if (justifyMeter == T("right"))
			{
				width = (int) (nWidth * 0.75f);
				x = 0;
			}
			else
			{
				width = nWidth;
				x = 0;
			}
		}

		if (isExpanded)
			segment_height = nMainSegmentHeight;
		else if (n < nLimitTopBars)
			segment_height = nMainSegmentHeight;
		else if (n < nLimitGreenBars_1)
			segment_height = 2 * nMainSegmentHeight;
		else if (n < nLimitGreenBars_2)
			segment_height = 6 * nMainSegmentHeight;
		else if (n == nNumberOfBars - 1)
		{
			if (nMeterHeadroom == 0)
				segment_height = 14 * nMainSegmentHeight;
			else if (nMeterHeadroom == 12)
				segment_height = 20 * nMainSegmentHeight;
			else if (nMeterHeadroom == 14)
				segment_height = 16 * nMainSegmentHeight;
			else
				segment_height = 10 * nMainSegmentHeight;
		}
		else
			segment_height = 10 * nMainSegmentHeight;

		MeterArray[n]->setBounds(x, y, width, segment_height + 1);
		y += segment_height;
	}
}

void MeterBar::paint(Graphics& g)
{
	g.fillAll(Colours::black);
}

void MeterBar::resized()
{
}

void MeterBar::setLevels(float newLevel, float newPeak)
{
	if ((fLevel != newLevel) | (fPeak != newPeak))
	{
		fLevel = newLevel;
		fPeak = newPeak;

		for (int n=0; n < nNumberOfBars; n++)
			MeterArray[n]->setLevels(fLevel, fPeak);
	}
}
