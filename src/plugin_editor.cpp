/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2012 Martin Zuther (http://www.mzuther.de/)

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
KmeterAudioProcessorEditor::KmeterAudioProcessorEditor(KmeterAudioProcessor* ownerFilter, int nNumChannels)
    : AudioProcessorEditor(ownerFilter)
{
    bIsValidating = false;
    bReloadMeters = false;

    nInputChannels = nNumChannels;
    nStereoInputChannels = (nNumChannels + (nNumChannels % 2)) / 2;
    nCrestFactor = 0;
    nRightColumnStart = nStereoInputChannels * Kmeter::KMETER_STEREO_WIDTH + 20;

    if (nInputChannels <= 2)
    {
        nHeight = 649;
    }
    else
    {
        nHeight = 630;
    }

    // This is where our plug-in's editor size is set.
    setSize(nRightColumnStart + 70, nHeight);

    pProcessor = ownerFilter;
    pProcessor->addActionListener(this);

    ButtonK20 = new TextButton(T("K-20"));
    ButtonK20->setBounds(nRightColumnStart, 10, 60, 20);
    ButtonK20->setRadioGroupId(1);
    ButtonK20->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonK20->setColour(TextButton::buttonOnColourId, Colours::green);

    ButtonK20->addButtonListener(this);
    addAndMakeVisible(ButtonK20);

    ButtonK14 = new TextButton(T("K-14"));
    ButtonK14->setBounds(nRightColumnStart, 35, 60, 20);
    ButtonK14->setRadioGroupId(1);
    ButtonK14->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonK14->setColour(TextButton::buttonOnColourId, Colours::yellow);

    ButtonK14->addButtonListener(this);
    addAndMakeVisible(ButtonK14);

    ButtonK12 = new TextButton(T("K-12"));
    ButtonK12->setBounds(nRightColumnStart, 60, 60, 20);
    ButtonK12->setRadioGroupId(1);
    ButtonK12->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonK12->setColour(TextButton::buttonOnColourId, Colours::yellow);

    ButtonK12->addButtonListener(this);
    addAndMakeVisible(ButtonK12);

    ButtonNormal = new TextButton(T("Normal"));
    ButtonNormal->setBounds(nRightColumnStart, 85, 60, 20);
    ButtonNormal->setRadioGroupId(1);
    ButtonNormal->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonNormal->setColour(TextButton::buttonOnColourId, Colours::red);

    ButtonNormal->addButtonListener(this);
    addAndMakeVisible(ButtonNormal);

    ButtonItuBs1770 = new TextButton(T("ITU-R"));
    ButtonItuBs1770->setBounds(nRightColumnStart, 125, 60, 20);
    ButtonItuBs1770->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonItuBs1770->setColour(TextButton::buttonOnColourId, Colours::green);

    ButtonItuBs1770->addButtonListener(this);
    addAndMakeVisible(ButtonItuBs1770);

    ButtonRms = new TextButton(T("RMS"));
    ButtonRms->setBounds(nRightColumnStart, 150, 60, 20);
    ButtonRms->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonRms->setColour(TextButton::buttonOnColourId, Colours::yellow);

    ButtonRms->addButtonListener(this);
    addAndMakeVisible(ButtonRms);

    updateAverageAlgorithm(false);

    ButtonInfiniteHold = new TextButton(T("Hold"));
    ButtonInfiniteHold->setBounds(nRightColumnStart, 190, 60, 20);
    ButtonInfiniteHold->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonInfiniteHold->setColour(TextButton::buttonOnColourId, Colours::yellow);

    ButtonInfiniteHold->addButtonListener(this);
    addAndMakeVisible(ButtonInfiniteHold);

    ButtonDisplayPeakMeter = new TextButton(T("Peaks"));
    ButtonDisplayPeakMeter->setBounds(nRightColumnStart, 215, 60, 20);
    ButtonDisplayPeakMeter->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonDisplayPeakMeter->setColour(TextButton::buttonOnColourId, Colours::yellow);

    ButtonDisplayPeakMeter->addButtonListener(this);
    addAndMakeVisible(ButtonDisplayPeakMeter);

    ButtonExpanded = new TextButton(T("Expand"));
    ButtonExpanded->setBounds(nRightColumnStart, 240, 60, 20);
    ButtonExpanded->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonExpanded->setColour(TextButton::buttonOnColourId, Colours::yellow);

    ButtonExpanded->addButtonListener(this);
    addAndMakeVisible(ButtonExpanded);

    ButtonMono = new TextButton(T("Mono"));
    ButtonMono->setBounds(nRightColumnStart, 280, 60, 20);
    ButtonMono->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonMono->setColour(TextButton::buttonOnColourId, Colours::red);

    ButtonMono->addButtonListener(this);
    addAndMakeVisible(ButtonMono);

    ButtonReset = new TextButton(T("Reset"));
    ButtonReset->setBounds(nRightColumnStart, 305, 60, 20);
    ButtonReset->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonReset->setColour(TextButton::buttonOnColourId, Colours::red);

    ButtonReset->addButtonListener(this);
    addAndMakeVisible(ButtonReset);

#ifdef DEBUG
    Label* LabelDebug = new Label(T("Debug Notification"), "DEBUG");
    LabelDebug->setBounds(nRightColumnStart, nHeight - 82, 60, 16);
    LabelDebug->setColour(Label::textColourId, Colours::red);
    LabelDebug->setJustificationType(Justification::centred);
    addAndMakeVisible(LabelDebug);
#endif

    ButtonValidation = new TextButton(T("Validate"));
    ButtonValidation->setBounds(nRightColumnStart, nHeight - 56, 60, 20);
    ButtonValidation->setColour(TextButton::textColourOnId, Colours::white);
    ButtonValidation->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonValidation->setColour(TextButton::buttonOnColourId, Colours::blue);

    ButtonValidation->addButtonListener(this);
    addAndMakeVisible(ButtonValidation);

    ButtonAbout = new TextButton(T("About"));
    ButtonAbout->setBounds(nRightColumnStart, nHeight - 31, 60, 20);
    ButtonAbout->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonAbout->setColour(TextButton::buttonOnColourId, Colours::yellow);

    ButtonAbout->addButtonListener(this);
    addAndMakeVisible(ButtonAbout);

    if (nInputChannels <= 2)
    {
        stereoMeter = new StereoMeter(T("Stereo Meter"), 10, nHeight - 44, 105, 15);
        addAndMakeVisible(stereoMeter);

        phaseCorrelationMeter = new PhaseCorrelationMeter(T("Correlation Meter"), 10, nHeight - 24, 105, 13);
        addAndMakeVisible(phaseCorrelationMeter);
    }
    else
    {
        stereoMeter = NULL;
        phaseCorrelationMeter = NULL;
    }

    pProcessor->addActionListenerParameters(this);

    kmeter = NULL;
    dynamicRangeLabel = NULL;

    int nIndex = KmeterPluginParameters::selCrestFactor;
    changeParameter(nIndex, pProcessor->getParameterAsInt(nIndex));

    nIndex = KmeterPluginParameters::selAverageAlgorithm;
    changeParameter(nIndex, pProcessor->getParameterAsInt(nIndex));

    nIndex = KmeterPluginParameters::selExpanded;
    changeParameter(nIndex, pProcessor->getParameterAsInt(nIndex));

    nIndex = KmeterPluginParameters::selPeak;
    changeParameter(nIndex, pProcessor->getParameterAsInt(nIndex));

    nIndex = KmeterPluginParameters::selInfiniteHold;
    changeParameter(nIndex, pProcessor->getParameterAsInt(nIndex));

    nIndex = KmeterPluginParameters::selMono;
    changeParameter(nIndex, pProcessor->getParameterAsInt(nIndex));
}


KmeterAudioProcessorEditor::~KmeterAudioProcessorEditor()
{
    pProcessor->removeActionListener(this);
    pProcessor->removeActionListenerParameters(this);

    deleteAllChildren();
}


void KmeterAudioProcessorEditor::actionListenerCallback(const String& message)
{
    // "PC" --> parameter changed, followed by a hash and the
    // parameter's ID
    if (message.startsWith("PC#"))
    {
        String strIndex = message.substring(3);
        int nIndex = strIndex.getIntValue();
        jassert(nIndex >= 0);
        jassert(nIndex < pProcessor->getNumParameters());

        changeParameter(nIndex);
    }
    // "UM" --> update meters
    else if (!message.compare("UM"))
    {
        MeterBallistics* pMeterBallistics = pProcessor->getLevels();

        if (pMeterBallistics)
        {
            kmeter->setLevels(pMeterBallistics);
            // dynamicRangeLabel->setValue(pMeterBallistics->getDynamicRangeValue());

            if (stereoMeter)
            {
                stereoMeter->setValue(pMeterBallistics->getStereoMeterValue());
            }

            if (phaseCorrelationMeter)
            {
                phaseCorrelationMeter->setValue(pMeterBallistics->getPhaseCorrelation());
            }
        }

        if (bIsValidating && !pProcessor->isValidating())
        {
            bIsValidating = false;
            ButtonValidation->setColour(TextButton::buttonColourId, Colours::grey);
        }
    }
    // "AC" --> algorithm changed
    else if (!message.compare("AC"))
    {
        updateAverageAlgorithm(true);
    }
    // "V+" --> validation started
    else if ((!message.compare("V+")) && pProcessor->isValidating())
    {
        bIsValidating = true;
        ButtonValidation->setColour(TextButton::buttonColourId, Colours::red);
    }
    // "V-" --> validation stopped
    else if (!message.compare("V-"))
    {
        // do nothing till you hear from me... :)
    }
    else
    {
        DBG("[K-Meter] Received unknown action message \"" + message + "\".");
    }
}


void KmeterAudioProcessorEditor::changeParameter(int nIndex)
{
    if (pProcessor->isParameterMarked(nIndex))
    {
        int nValue = pProcessor->getParameterAsInt(nIndex);
        changeParameter(nIndex, nValue);
        pProcessor->UnmarkParameter(nIndex);
    }
}


void KmeterAudioProcessorEditor::changeParameter(int nIndex, int nValue)
{
    MeterBallistics* pMeterBallistics = NULL;

    switch (nIndex)
    {
    case KmeterPluginParameters::selCrestFactor:

        if (nValue == 0)
        {
            nCrestFactor = nValue;
            bReloadMeters = true;

            ButtonNormal->setToggleState(true, false);
        }
        else if (nValue == 12)
        {
            nCrestFactor = nValue;
            bReloadMeters = true;

            ButtonK12->setToggleState(true, false);
        }
        else if (nValue == 14)
        {
            nCrestFactor = nValue;
            bReloadMeters = true;

            ButtonK14->setToggleState(true, false);
        }
        else
        {
            nCrestFactor = 20;
            bReloadMeters = true;

            ButtonK20->setToggleState(true, false);
        }

        break;

    case KmeterPluginParameters::selAverageAlgorithm:

        // do nothing till you hear from me... :)
        //
        // the "RMS" and "ITU-R" buttons will be updated from the code
        // that actually switches the level averaging alghorithm, thus
        // making sure that the correct button is lit in any given
        // situation

        break;

    case KmeterPluginParameters::selExpanded:
        bReloadMeters = true;
        ButtonExpanded->setToggleState(nValue != 0, false);
        break;

    case KmeterPluginParameters::selPeak:
        bReloadMeters = true;
        ButtonDisplayPeakMeter->setToggleState(nValue != 0, false);
        break;

    case KmeterPluginParameters::selInfiniteHold:
        pMeterBallistics = pProcessor->getLevels();

        if (pMeterBallistics)
        {
            pMeterBallistics->setPeakMeterInfiniteHold(nValue != 0);
            pMeterBallistics->setAverageMeterInfiniteHold(nValue != 0);
        }

        ButtonInfiniteHold->setToggleState(nValue != 0, false);
        break;

    case KmeterPluginParameters::selMono:
        ButtonMono->setToggleState(nValue != 0, false);
        break;
    }

    if (bReloadMeters)
    {
        bReloadMeters = false;

        if (kmeter)
        {
            removeChildComponent(kmeter);
            delete kmeter;
            kmeter = NULL;
        }

        // if (dynamicRangeLabel)
        // {
        //     removeChildComponent(dynamicRangeLabel);
        //     delete dynamicRangeLabel;
        //     dynamicRangeLabel = NULL;
        // }

        kmeter = new Kmeter(T("K-Meter"), 10, 10, nCrestFactor, nInputChannels, ButtonExpanded->getToggleState(), ButtonDisplayPeakMeter->getToggleState(), 4);
        addAndMakeVisible(kmeter);

        // dynamicRangeLabel = new DynamicRangeLabel(T("Dynamic Range"));
        // dynamicRangeLabel->setBounds(nRightColumnStart + 10, 500, 40, 20);
        // addAndMakeVisible(dynamicRangeLabel);
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
    if (button == ButtonNormal)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selCrestFactor, 0);
    }
    else if (button == ButtonK12)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selCrestFactor, 12);
    }
    else if (button == ButtonK14)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selCrestFactor, 14);
    }
    else if (button == ButtonK20)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selCrestFactor, 20);
    }
    else if (button == ButtonInfiniteHold)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selInfiniteHold, !button->getToggleState());
    }
    else if (button == ButtonRms)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selAverageAlgorithm, KmeterPluginParameters::selAlgorithmRms);
    }
    else if (button == ButtonItuBs1770)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selAverageAlgorithm, KmeterPluginParameters::selAlgorithmItuBs1770);
    }
    else if (button == ButtonExpanded)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selExpanded, !button->getToggleState());
    }
    else if (button == ButtonDisplayPeakMeter)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selPeak, !button->getToggleState());
    }
    else if (button == ButtonReset)
    {
        MeterBallistics* pMeterBallistics = pProcessor->getLevels();

        if (pMeterBallistics)
        {
            pMeterBallistics->reset();
        }
    }
    else if (button == ButtonMono)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selMono, !button->getToggleState());
    }
    else if (button == ButtonAbout)
    {
        WindowAbout* windowAbout = new WindowAbout(getWidth(), getHeight());
        addAndMakeVisible(windowAbout);

        windowAbout->runModalLoop();

        removeChildComponent(windowAbout);
        delete windowAbout;
        windowAbout = NULL;
    }
    else if (button == ButtonValidation)
    {
        WindowValidation* windowValidation = new WindowValidation(getWidth(), getHeight(), pProcessor);
        addAndMakeVisible(windowValidation);

        windowValidation->runModalLoop();

        removeChildComponent(windowValidation);
        delete windowValidation;
        windowValidation = NULL;
    }
}


void KmeterAudioProcessorEditor::updateAverageAlgorithm(bool reload_meters)
{
    if (pProcessor->getAverageAlgorithm() == KmeterPluginParameters::selAlgorithmItuBs1770)
    {
        ButtonItuBs1770->setToggleState(true, false);
        ButtonRms->setToggleState(false, false);
    }
    else
    {
        ButtonItuBs1770->setToggleState(false, false);
        ButtonRms->setToggleState(true, false);
    }

    bReloadMeters = reload_meters;
    MeterBallistics* pMeterBallistics = pProcessor->getLevels();

    if (pMeterBallistics)
    {
        pMeterBallistics->reset();
    }

}

void KmeterAudioProcessorEditor::resized()
{
}



// Local Variables:
// ispell-local-dictionary: "british"
// End:
