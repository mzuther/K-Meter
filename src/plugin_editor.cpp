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

#include "plugin_editor.h"

//==============================================================================
KmeterAudioProcessorEditor::KmeterAudioProcessorEditor(KmeterAudioProcessor* ownerFilter)
    : AudioProcessorEditor(ownerFilter)
{
	// This is where our plugin's editor size is set.
	setSize(220, 640);

	nHeadroom = 0;
	nNumberOfChannels = 0;

	pProcessor = ownerFilter;
	pProcessor->addChangeListener(this);

	ButtonK20 = new TextButton(T("K-20"));
	ButtonK20->setBounds(140, 10, 60, 20);
	ButtonK20->setRadioGroupId(1);
	ButtonK20->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonK20->setColour(TextButton::buttonOnColourId, Colours::green);
	ButtonK20->setClickingTogglesState(true);

	ButtonK20->addButtonListener(this);
	addAndMakeVisible(ButtonK20);

	ButtonK14 = new TextButton(T("K-14"));
	ButtonK14->setBounds(140, 35, 60, 20);
	ButtonK14->setRadioGroupId(1);
	ButtonK14->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonK14->setColour(TextButton::buttonOnColourId, Colours::green);
	ButtonK14->setClickingTogglesState(true);

	ButtonK14->addButtonListener(this);
	addAndMakeVisible(ButtonK14);

	ButtonK12 = new TextButton(T("K-12"));
	ButtonK12->setBounds(140, 60, 60, 20);
	ButtonK12->setRadioGroupId(1);
	ButtonK12->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonK12->setColour(TextButton::buttonOnColourId, Colours::green);
	ButtonK12->setClickingTogglesState(true);

	ButtonK12->addButtonListener(this);
	addAndMakeVisible(ButtonK12);

	ButtonNormal = new TextButton(T("Normal"));
	ButtonNormal->setBounds(140, 85, 60, 20);
	ButtonNormal->setRadioGroupId(1);
	ButtonNormal->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonNormal->setColour(TextButton::buttonOnColourId, Colours::yellow);
	ButtonNormal->setClickingTogglesState(true);

	ButtonNormal->addButtonListener(this);
	addAndMakeVisible(ButtonNormal);

	ButtonHold = new TextButton(T("Hold"));
	ButtonHold->setBounds(140, 125, 60, 20);
	ButtonHold->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonHold->setColour(TextButton::buttonOnColourId, Colours::red);
	ButtonHold->setClickingTogglesState(true);

	ButtonHold->addButtonListener(this);
	addAndMakeVisible(ButtonHold);

	ButtonDisplayPeakMeter = new TextButton(T("Peaks"));
	ButtonDisplayPeakMeter->setBounds(140, 150, 60, 20);
	ButtonDisplayPeakMeter->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonDisplayPeakMeter->setColour(TextButton::buttonOnColourId, Colours::yellow);
	ButtonDisplayPeakMeter->setClickingTogglesState(true);

	ButtonDisplayPeakMeter->addButtonListener(this);
	addAndMakeVisible(ButtonDisplayPeakMeter);

	ButtonExpanded = new TextButton(T("Expand"));
	ButtonExpanded->setBounds(140, 175, 60, 20);
	ButtonExpanded->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonExpanded->setColour(TextButton::buttonOnColourId, Colours::yellow);
	ButtonExpanded->setClickingTogglesState(true);

	ButtonExpanded->addButtonListener(this);
	addAndMakeVisible(ButtonExpanded);

	ButtonMono = new TextButton(T("Mono"));
	ButtonMono->setBounds(140, 215, 60, 20);
	ButtonMono->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonMono->setColour(TextButton::buttonOnColourId, Colours::yellow);
	ButtonMono->setClickingTogglesState(false);

	ButtonMono->addButtonListener(this);
	addAndMakeVisible(ButtonMono);

	ButtonReset = new TextButton(T("Reset"));
	ButtonReset->setBounds(140, 240, 60, 20);
	ButtonReset->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonReset->setColour(TextButton::buttonOnColourId, Colours::red);

	ButtonReset->addButtonListener(this);
	addAndMakeVisible(ButtonReset);

	Label* LabelReadoutWarning = new Label(T("Readout Warning"), "Read-out\nnot yet\nvalidated!");
	LabelReadoutWarning->setBounds(130, 531, 80, 40);
	LabelReadoutWarning->setColour(Label::textColourId, Colours::yellow);
	LabelReadoutWarning->setJustificationType(Justification::centred);
	addAndMakeVisible(LabelReadoutWarning);

	#ifdef DEBUG
	Label* LabelDebug = new Label(T("Debug Notification"), "DEBUG");
	LabelDebug->setBounds(145, 587, 50, 16);
	LabelDebug->setColour(Label::textColourId, Colours::red);
	LabelDebug->setJustificationType(Justification::centred);
	addAndMakeVisible(LabelDebug);
	#endif

	ButtonAbout = new TextButton(T("About"));
	ButtonAbout->setBounds(140, 609, 60, 20);
	ButtonAbout->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonAbout->setColour(TextButton::buttonOnColourId, Colours::yellow);

	ButtonAbout->addButtonListener(this);
	addAndMakeVisible(ButtonAbout);

	stereoMeter = new StereoMeter(T("Stereo Meter"), 15, 600, 105, 15);
	addAndMakeVisible(stereoMeter);

	correlationMeter = new CorrelationMeter(T("Correlation Meter"), 15, 620, 105, 13);
	addAndMakeVisible(correlationMeter);

	stereoKmeter = NULL;

	// hide peak meter
	ButtonDisplayPeakMeter->setToggleState(false, false);

	// display non-expanded meter
	ButtonExpanded->setToggleState(false, false);

	// toggle default button
	ButtonK20->setToggleState(true, true);
}

KmeterAudioProcessorEditor::~KmeterAudioProcessorEditor()
{
	pProcessor->removeChangeListener(this);

	deleteAllChildren();
}

void KmeterAudioProcessorEditor::changeListenerCallback(void* objectThatHasChanged)
{
	MeterBallistics* pBallistics = pProcessor->getLevels();

	stereoKmeter->setLevels(pBallistics);
	stereoMeter->setValue(pBallistics->getStereoMeterValue());
	correlationMeter->setValue(pBallistics->getCorrelationMeterValue());

	if (pBallistics->getNumberOfChannels() != nNumberOfChannels)
	{
	  nNumberOfChannels = pBallistics->getNumberOfChannels();
	  
	  if (nNumberOfChannels == 1)
	  {
		 ButtonMono->setColour(TextButton::buttonOnColourId, Colours::red);
		 ButtonMono->setClickingTogglesState(false);
		 ButtonMono->setToggleState(true, true);
	  }
	  else
	  {
		 ButtonMono->setColour(TextButton::buttonOnColourId, Colours::yellow);
		 ButtonMono->setClickingTogglesState(true);
		 ButtonMono->setToggleState(false, true);
	  }
	}
}

//==============================================================================
void KmeterAudioProcessorEditor::paint(Graphics& g)
{
	g.setGradientFill(ColourGradient(Colours::darkgrey.darker(0.8f), 0, 0, Colours::darkgrey.darker(1.4f), 0, (float) getHeight(), false));
	g.fillAll();
}

void KmeterAudioProcessorEditor::buttonClicked(Button* button)
{
	bool reloadStereoKmeter = false;

	if (button == ButtonNormal)
	{
		nHeadroom = 0;
		reloadStereoKmeter = true;
	}
	else if (button == ButtonK12)
	{
		nHeadroom = 12;
		reloadStereoKmeter = true;
	}
	else if (button == ButtonK14)
	{
		nHeadroom = 14;
		reloadStereoKmeter = true;
	}
	else if (button == ButtonK20)
	{
		nHeadroom = 20;
		reloadStereoKmeter = true;
	}
	else if (button == ButtonHold)
	{
		MeterBallistics* pBallistics = pProcessor->getLevels();
		pBallistics->setPeakHold(button->getToggleState());
		pBallistics->setAverageHold(button->getToggleState());
	}
	else if (button == ButtonExpanded)
	{
		reloadStereoKmeter = true;
	}
	else if (button == ButtonDisplayPeakMeter)
	{
		reloadStereoKmeter = true;
	}
	else if (button == ButtonReset)
	{
		MeterBallistics* pBallistics = pProcessor->getLevels();
		pBallistics->reset();
	}
	else if (button == ButtonMono)
	{
	  pProcessor->convertMono(ButtonMono->getToggleState());
	}
	else if (button == ButtonAbout)
	{
	  AboutWindow* aboutWindow = new AboutWindow(getWidth(), getHeight());
	  addAndMakeVisible(aboutWindow);

	  aboutWindow->runModalLoop();

	  removeChildComponent(aboutWindow);
	  delete aboutWindow;
	}

	if (reloadStereoKmeter)
	{
		if (stereoKmeter)
		{
			removeChildComponent(stereoKmeter);
			delete stereoKmeter;
		}

		stereoKmeter = new StereoKmeter(T("Stereo K-Meter"), 15, 5, nHeadroom, ButtonExpanded->getToggleState(), ButtonDisplayPeakMeter->getToggleState(), 4);
		addAndMakeVisible(stereoKmeter);
	}
}

void KmeterAudioProcessorEditor::resized()
{
}

