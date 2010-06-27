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

#include "stereo_kmeter.h"

StereoKmeter::StereoKmeter(const String &componentName, int posX, int posY, int nHeadroom, bool bExpanded, int nSegmentHeight)
{
	setName(componentName);
	isExpanded = bExpanded;

	nPosX = posX;
	nPosY = posY;
	nMainSegmentHeight = nSegmentHeight;

	if (nHeadroom == 0)
		nMeterHeadroom = 0;
	else if (nHeadroom == 12)
		nMeterHeadroom = 12;
	else if (nHeadroom == 14)
		nMeterHeadroom = 14;
	else
		nMeterHeadroom = 20;

	PeakMeterLeft = new MeterBar(T("Peak Meter Left"), 3, 28, 9, nMeterHeadroom, bExpanded, nMainSegmentHeight, T("left"));
	PeakMeterRight = new MeterBar(T("Peak Meter Right"), 94, 28, 9, nMeterHeadroom, bExpanded, nMainSegmentHeight, T("right"));
	
	AverageMeterLeft = new MeterBar(T("Average Meter Left"), 17, 28, 18, nMeterHeadroom, bExpanded, nMainSegmentHeight, T("center"));
	AverageMeterRight = new MeterBar(T("Average Meter Right"), 71, 28, 18, nMeterHeadroom, bExpanded, nMainSegmentHeight, T("center"));

	addAndMakeVisible(PeakMeterLeft);
	addAndMakeVisible(PeakMeterRight);
	
	addAndMakeVisible(AverageMeterLeft);
	addAndMakeVisible(AverageMeterRight);

	OverflowMeterLeft = new OverflowMeter(T("Overflows Left"));
	OverflowMeterLeft->setBounds(3, 3, 32, 16);
	addAndMakeVisible(OverflowMeterLeft);

	OverflowMeterRight = new OverflowMeter(T("Overflows Right"));
	OverflowMeterRight->setBounds(71, 3, 32, 16);
	addAndMakeVisible(OverflowMeterRight);
}

StereoKmeter::~StereoKmeter()
{
	deleteAllChildren();
}
	
void StereoKmeter::visibilityChanged()
{
	int height = 134 * nMainSegmentHeight + 32;
	setBounds(nPosX, nPosY, 106, height);
}

void StereoKmeter::paint(Graphics& g)
{
	g.fillAll(Colours::grey.withAlpha(0.1f));

	g.setColour(Colours::darkgrey);
	g.drawRect(0, 0, getWidth() - 1, getHeight() - 1);

	g.setColour(Colours::darkgrey.darker(0.8f));
	g.drawRect(1, 1, getWidth() - 1, getHeight() - 1);

	g.setColour(Colours::darkgrey.darker(0.4f));
	g.drawRect(1, 1, getWidth() - 2, getHeight() - 2);

	int x = 3;
	int y = 23;
	int width = 24;
	int height = 11;
	String strMarker;

	g.setColour(Colours::white);
	g.setFont(11.0f);

	if (isExpanded)
	{
		y -= 10 * nMainSegmentHeight;
		int nStart = 0;

		if (nMeterHeadroom == 0)
			nStart = 0;
		else if (nMeterHeadroom == 12)
			nStart = 12;
		else if (nMeterHeadroom == 14)
			nStart = 14;
		else
			nStart = 20;

		for (int n=0; n >= -13; n -= 1)
		{
			if ((nStart + n) > 0)
				strMarker = T("+") + String(nStart + n);
			else
				strMarker = String(nStart + n);

			y += 10 * nMainSegmentHeight;
			drawMarkers(g, strMarker, x, y, width, height);
		}
	}
	else if (nMeterHeadroom == 0)
	{
		y -= 8 * nMainSegmentHeight;

		for (int n=0; n >= -40; n -= 4)
		{
			if (n > 0)
				strMarker = T("+") + String(n);
			else
				strMarker = String(n);

			y += 8 * nMainSegmentHeight;
			drawMarkers(g, strMarker, x, y, width, height);
		}

		for (int n=-50; n >= -80; n -= 10)
		{
			strMarker = String(n);

			y += 10 * nMainSegmentHeight;
			drawMarkers(g, strMarker, x, y, width, height);
		}
	}
	else if (nMeterHeadroom == 12)
	{
		y -= 8 * nMainSegmentHeight;

		for (int n=12; n >= -28; n -= 4)
		{
			if (n > 0)
				strMarker = T("+") + String(n);
			else
				strMarker = String(n);

			y += 8 * nMainSegmentHeight;
			drawMarkers(g, strMarker, x, y, width, height);
		}

		y -= 6 * nMainSegmentHeight;

		for (int n=-30; n >= -60; n -= 10)
		{
			strMarker = String(n);

			y += 10 * nMainSegmentHeight;
			drawMarkers(g, strMarker, x, y, width, height);
		}
	}
	else if (nMeterHeadroom == 14)
	{
		strMarker = String(T("+14"));
		drawMarkers(g, strMarker, x, y, width, height);
		y -= 4 * nMainSegmentHeight;

		for (int n=12; n >= -28; n -= 4)
		{
			if (n > 0)
				strMarker = T("+") + String(n);
			else
				strMarker = String(n);

			y += 8 * nMainSegmentHeight;
			drawMarkers(g, strMarker, x, y, width, height);
		}

		y -= 6 * nMainSegmentHeight;

		for (int n=-30; n >= -60; n -= 10)
		{
			strMarker = String(n);

			y += 10 * nMainSegmentHeight;
			drawMarkers(g, strMarker, x, y, width, height);
		}
	}
	else
	{
		y -= 8 * nMainSegmentHeight;

		for (int n=20; n >= -24; n -= 4)
		{
			if (n > 0)
				strMarker = T("+") + String(n);
			else
				strMarker = String(n);

			y += 8 * nMainSegmentHeight;
			drawMarkers(g, strMarker, x, y, width, height);
		}

		y -= 4 * nMainSegmentHeight;

		for (int n=-30; n >= -60; n -= 10)
		{
			strMarker = String(n);

			y += 10 * nMainSegmentHeight;
			drawMarkers(g, strMarker, x, y, width, height);
		}
	}
}

void StereoKmeter::resized()
{
}

void StereoKmeter::setLevels(MeterBallistics* pMB)
{
	PeakMeterLeft->setLevels(pMB->getPeakMeterLeft(), pMB->getPeakMeterLeftPeak());
	PeakMeterRight->setLevels(pMB->getPeakMeterRight(), pMB->getPeakMeterRightPeak());

	AverageMeterLeft->setLevels(pMB->getAverageMeterLeft(), pMB->getAverageMeterLeftPeak());
	AverageMeterRight->setLevels(pMB->getAverageMeterRight(), pMB->getAverageMeterRightPeak());

	OverflowMeterLeft->setOverflows(pMB->getOverflowsLeft());
	OverflowMeterRight->setOverflows(pMB->getOverflowsRight());
}

void StereoKmeter::drawMarkers(Graphics& g, String& strMarker, int x, int y, int width, int height)
{
	g.setColour(Colours::white);
	g.drawFittedText(strMarker, x + 38, y, width, height, Justification::centred, 1, 1.0f);

	g.setColour(Colours::grey);

	int nMarkerX = x + 10;
	int nMarkerY = y + 5;

	g.setPixel(nMarkerX++, nMarkerY);
	g.setPixel(nMarkerX++, nMarkerY);
	g.setPixel(nMarkerX++, nMarkerY);

	nMarkerX = x + 87;

	g.setPixel(nMarkerX++, nMarkerY);
	g.setPixel(nMarkerX++, nMarkerY);
	g.setPixel(nMarkerX++, nMarkerY);
}
