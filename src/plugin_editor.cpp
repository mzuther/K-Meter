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
	setSize(202, 650);

	nHeadroom = 0;
	nNumberOfChannels = 0;

	pProcessor = ownerFilter;
	pProcessor->addChangeListener(this);

	ButtonK20 = new TextButton(T("K-20"));
	ButtonK20->setBounds(132, 10, 60, 20);
	ButtonK20->setRadioGroupId(1);
	ButtonK20->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonK20->setColour(TextButton::buttonOnColourId, Colours::green);
	ButtonK20->setClickingTogglesState(true);

	ButtonK20->addButtonListener(this);
	addAndMakeVisible(ButtonK20);

	ButtonK14 = new TextButton(T("K-14"));
	ButtonK14->setBounds(132, 35, 60, 20);
	ButtonK14->setRadioGroupId(1);
	ButtonK14->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonK14->setColour(TextButton::buttonOnColourId, Colours::yellow);
	ButtonK14->setClickingTogglesState(true);

	ButtonK14->addButtonListener(this);
	addAndMakeVisible(ButtonK14);

	ButtonK12 = new TextButton(T("K-12"));
	ButtonK12->setBounds(132, 60, 60, 20);
	ButtonK12->setRadioGroupId(1);
	ButtonK12->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonK12->setColour(TextButton::buttonOnColourId, Colours::yellow);
	ButtonK12->setClickingTogglesState(true);

	ButtonK12->addButtonListener(this);
	addAndMakeVisible(ButtonK12);

	ButtonNormal = new TextButton(T("Normal"));
	ButtonNormal->setBounds(132, 85, 60, 20);
	ButtonNormal->setRadioGroupId(1);
	ButtonNormal->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonNormal->setColour(TextButton::buttonOnColourId, Colours::red);
	ButtonNormal->setClickingTogglesState(true);

	ButtonNormal->addButtonListener(this);
	addAndMakeVisible(ButtonNormal);

	ButtonHold = new TextButton(T("Hold"));
	ButtonHold->setBounds(132, 125, 60, 20);
	ButtonHold->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonHold->setColour(TextButton::buttonOnColourId, Colours::yellow);
	ButtonHold->setClickingTogglesState(true);

	ButtonHold->addButtonListener(this);
	addAndMakeVisible(ButtonHold);

	ButtonDisplayPeakMeter = new TextButton(T("Peaks"));
	ButtonDisplayPeakMeter->setBounds(132, 150, 60, 20);
	ButtonDisplayPeakMeter->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonDisplayPeakMeter->setColour(TextButton::buttonOnColourId, Colours::yellow);
	ButtonDisplayPeakMeter->setClickingTogglesState(true);

	ButtonDisplayPeakMeter->addButtonListener(this);
	addAndMakeVisible(ButtonDisplayPeakMeter);

	ButtonExpanded = new TextButton(T("Expand"));
	ButtonExpanded->setBounds(132, 175, 60, 20);
	ButtonExpanded->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonExpanded->setColour(TextButton::buttonOnColourId, Colours::yellow);
	ButtonExpanded->setClickingTogglesState(true);

	ButtonExpanded->addButtonListener(this);
	addAndMakeVisible(ButtonExpanded);

	ButtonMono = new TextButton(T("Mono"));
	ButtonMono->setBounds(132, 215, 60, 20);
	ButtonMono->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonMono->setColour(TextButton::buttonOnColourId, Colours::red);
	ButtonMono->setClickingTogglesState(false);

	ButtonMono->addButtonListener(this);
	addAndMakeVisible(ButtonMono);

	ButtonReset = new TextButton(T("Reset"));
	ButtonReset->setBounds(132, 240, 60, 20);
	ButtonReset->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonReset->setColour(TextButton::buttonOnColourId, Colours::red);

	ButtonReset->addButtonListener(this);
	addAndMakeVisible(ButtonReset);

	#ifdef DEBUG
	Label* LabelDebug = new Label(T("Debug Notification"), "DEBUG");
	LabelDebug->setBounds(145, 587, 50, 16);
	LabelDebug->setColour(Label::textColourId, Colours::red);
	LabelDebug->setJustificationType(Justification::centred);
	addAndMakeVisible(LabelDebug);
	#endif

	ButtonAbout = new TextButton(T("About"));
	ButtonAbout->setBounds(132, 619, 60, 20);
	ButtonAbout->setColour(TextButton::buttonColourId, Colours::grey);
	ButtonAbout->setColour(TextButton::buttonOnColourId, Colours::yellow);

	ButtonAbout->addButtonListener(this);
	addAndMakeVisible(ButtonAbout);

	stereoMeter = new StereoMeter(T("Stereo Meter"), 10, 605, 105, 15);
	addAndMakeVisible(stereoMeter);

	correlationMeter = new CorrelationMeter(T("Correlation Meter"), 10, 625, 105, 13);
	addAndMakeVisible(correlationMeter);

	stereoKmeter = NULL;

	int index = KmeterAudioProcessor::nSelect_Headroom;
	int nValue = pProcessor->getTranslatedParameter(index);
	changeParameter(index, nValue);

	index = KmeterAudioProcessor::nSelect_Expanded;
	nValue = pProcessor->getTranslatedParameter(index);
	changeParameter(index, nValue);

	index = KmeterAudioProcessor::nSelect_Peak;
	nValue = pProcessor->getTranslatedParameter(index);
	changeParameter(index, nValue);

	index = KmeterAudioProcessor::nSelect_Hold;
	nValue = pProcessor->getTranslatedParameter(index);
	changeParameter(index, nValue);

	index = KmeterAudioProcessor::nSelect_Mono;
	nValue = pProcessor->getTranslatedParameter(index);
	changeParameter(index, nValue);
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
		 ButtonMono->setClickingTogglesState(false);
		 ButtonMono->setToggleState(true, true);
	  }
	  else
	  {
		 ButtonMono->setClickingTogglesState(true);
		 ButtonMono->setToggleState(false, true);
	  }
	}
}

void KmeterAudioProcessorEditor::changeParameter(int index, int nValue)
{
  switch (index)
  {
  case KmeterAudioProcessor::nSelect_Headroom:
	 if (nValue == 0)
		ButtonNormal->setToggleState(true, true);
	 else if (nValue == 12)
		ButtonK12->setToggleState(true, true);
	 else if (nValue == 14)
		ButtonK14->setToggleState(true, true);
	 else
		ButtonK20->setToggleState(true, true);
	 break;

  case KmeterAudioProcessor::nSelect_Expanded:
	 ButtonExpanded->setToggleState(nValue, true);
	 break;

  case KmeterAudioProcessor::nSelect_Peak:
	 ButtonDisplayPeakMeter->setToggleState(nValue, true);
	 break;

  case KmeterAudioProcessor::nSelect_Hold:
	 ButtonHold->setToggleState(nValue, true);
	 break;

  case KmeterAudioProcessor::nSelect_Mono:
	 ButtonMono->setToggleState(nValue, true);
	 break;
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

		pProcessor->changeParameter(KmeterAudioProcessor::nSelect_Headroom, nHeadroom);
	}
	else if (button == ButtonK12)
	{
		nHeadroom = 12;
		reloadStereoKmeter = true;

		pProcessor->changeParameter(KmeterAudioProcessor::nSelect_Headroom, nHeadroom);
	}
	else if (button == ButtonK14)
	{
		nHeadroom = 14;
		reloadStereoKmeter = true;

		pProcessor->changeParameter(KmeterAudioProcessor::nSelect_Headroom, nHeadroom);
	}
	else if (button == ButtonK20)
	{
		nHeadroom = 20;
		reloadStereoKmeter = true;

		pProcessor->changeParameter(KmeterAudioProcessor::nSelect_Headroom, nHeadroom);
	}
	else if (button == ButtonHold)
	{
		MeterBallistics* pBallistics = pProcessor->getLevels();
		pBallistics->setPeakHold(button->getToggleState());
		pBallistics->setAverageHold(button->getToggleState());

		pProcessor->changeParameter(KmeterAudioProcessor::nSelect_Hold, button->getToggleState());
	}
	else if (button == ButtonExpanded)
	{
		reloadStereoKmeter = true;
		pProcessor->changeParameter(KmeterAudioProcessor::nSelect_Expanded, button->getToggleState());
	}
	else if (button == ButtonDisplayPeakMeter)
	{
		reloadStereoKmeter = true;
		pProcessor->changeParameter(KmeterAudioProcessor::nSelect_Peak, button->getToggleState());
	}
	else if (button == ButtonReset)
	{
		MeterBallistics* pBallistics = pProcessor->getLevels();
		pBallistics->reset();
	}
	else if (button == ButtonMono)
	{
	  pProcessor->changeParameter(KmeterAudioProcessor::nSelect_Mono, button->getToggleState());
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

		stereoKmeter = new StereoKmeter(T("Stereo K-Meter"), 10, 10, nHeadroom, ButtonExpanded->getToggleState(), ButtonDisplayPeakMeter->getToggleState(), 4);
		addAndMakeVisible(stereoKmeter);
	}
}

void KmeterAudioProcessorEditor::resized()
{
}

