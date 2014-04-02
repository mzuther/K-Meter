/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2013 Martin Zuther (http://www.mzuther.de/)

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
    // the editor window does not have any transparent areas
    // (increases performance on redrawing)
    setOpaque(true);

    // prevent meter reload during initialisation
    bInitialising = true;

    bIsValidating = false;

    nInputChannels = nNumChannels;
    nStereoInputChannels = (nNumChannels + (nNumChannels % 2)) / 2;
    nCrestFactor = 0;

    pSkin = new Skin(nInputChannels, nCrestFactor, -1);

    // The plug-in editor's size as well as the location of buttons
    // and labels will be set later on in this constructor.

    pProcessor = ownerFilter;
    pProcessor->addActionListener(this);

    ButtonK20 = new TextButton("K-20");
    ButtonK20->setRadioGroupId(1);
    ButtonK20->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonK20->setColour(TextButton::buttonOnColourId, Colours::green);

    ButtonK20->addListener(this);
    addAndMakeVisible(ButtonK20);

    ButtonK14 = new TextButton("K-14");
    ButtonK14->setRadioGroupId(1);
    ButtonK14->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonK14->setColour(TextButton::buttonOnColourId, Colours::yellow);

    ButtonK14->addListener(this);
    addAndMakeVisible(ButtonK14);

    ButtonK12 = new TextButton("K-12");
    ButtonK12->setRadioGroupId(1);
    ButtonK12->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonK12->setColour(TextButton::buttonOnColourId, Colours::yellow);

    ButtonK12->addListener(this);
    addAndMakeVisible(ButtonK12);

    ButtonNormal = new TextButton("Normal");
    ButtonNormal->setRadioGroupId(1);
    ButtonNormal->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonNormal->setColour(TextButton::buttonOnColourId, Colours::red);

    ButtonNormal->addListener(this);
    addAndMakeVisible(ButtonNormal);

    ButtonItuBs1770 = new TextButton("ITU-R");
    ButtonItuBs1770->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonItuBs1770->setColour(TextButton::buttonOnColourId, Colours::green);

    ButtonItuBs1770->addListener(this);
    addAndMakeVisible(ButtonItuBs1770);

    ButtonRms = new TextButton("RMS");
    ButtonRms->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonRms->setColour(TextButton::buttonOnColourId, Colours::yellow);

    ButtonRms->addListener(this);
    addAndMakeVisible(ButtonRms);

    updateAverageAlgorithm(false);

    ButtonInfiniteHold = new TextButton("Hold");
    ButtonInfiniteHold->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonInfiniteHold->setColour(TextButton::buttonOnColourId, Colours::yellow);

    ButtonInfiniteHold->addListener(this);
    addAndMakeVisible(ButtonInfiniteHold);

    ButtonDisplayPeakMeter = new TextButton("Peaks");
    ButtonDisplayPeakMeter->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonDisplayPeakMeter->setColour(TextButton::buttonOnColourId, Colours::yellow);

    ButtonDisplayPeakMeter->addListener(this);
    addAndMakeVisible(ButtonDisplayPeakMeter);

    ButtonExpanded = new TextButton("Expand");
    ButtonExpanded->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonExpanded->setColour(TextButton::buttonOnColourId, Colours::yellow);

    ButtonExpanded->addListener(this);
    addAndMakeVisible(ButtonExpanded);

    ButtonSkin = new TextButton("Skin");
    ButtonSkin->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonSkin->setColour(TextButton::buttonOnColourId, Colours::yellow);

    ButtonSkin->addListener(this);
    addAndMakeVisible(ButtonSkin);

    ButtonMono = new TextButton("Mono");
    ButtonMono->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonMono->setColour(TextButton::buttonOnColourId, Colours::red);

    ButtonMono->addListener(this);
    addAndMakeVisible(ButtonMono);

    ButtonReset = new TextButton("Reset");
    ButtonReset->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonReset->setColour(TextButton::buttonOnColourId, Colours::red);

    ButtonReset->addListener(this);
    addAndMakeVisible(ButtonReset);

#ifdef DEBUG
    LabelDebug = new Label("Debug Notification", "DEBUG");
    LabelDebug->setColour(Label::textColourId, Colours::red);
    LabelDebug->setJustificationType(Justification::centred);
    addAndMakeVisible(LabelDebug);
#else
    LabelDebug = nullptr;
#endif

    ButtonValidation = new TextButton("Validate");
    ButtonValidation->setColour(TextButton::textColourOnId, Colours::white);
    ButtonValidation->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonValidation->setColour(TextButton::buttonOnColourId, Colours::blue);

    ButtonValidation->addListener(this);
    addAndMakeVisible(ButtonValidation);

    ButtonAbout = new TextButton("About");
    ButtonAbout->setColour(TextButton::buttonColourId, Colours::grey);
    ButtonAbout->setColour(TextButton::buttonOnColourId, Colours::yellow);

    ButtonAbout->addListener(this);
    addAndMakeVisible(ButtonAbout);

    if (nInputChannels <= 2)
    {
        stereoMeter = new StereoMeter("Stereo Meter");
        addAndMakeVisible(stereoMeter);

        phaseCorrelationMeter = new PhaseCorrelationMeter("Correlation Meter");
        addAndMakeVisible(phaseCorrelationMeter);
    }
    else
    {
        stereoMeter = nullptr;
        phaseCorrelationMeter = nullptr;
    }

    pProcessor->addActionListenerParameters(this);

    kmeter = nullptr;

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

    // force meter reload after initialisation ...
    bInitialising = false;
    bReloadMeters = true;
    reloadMeters();

    // ... and set our plug-in editor's size.
    resizeEditor();
}


KmeterAudioProcessorEditor::~KmeterAudioProcessorEditor()
{
    pProcessor->removeActionListener(this);
    pProcessor->removeActionListenerParameters(this);

    delete pSkin;
    pSkin = nullptr;

    deleteAllChildren();
}


void KmeterAudioProcessorEditor::resizeEditor()
{
    if (nInputChannels <= 2)
    {
        nHeight = 648;
        nWidth = nStereoInputChannels * Kmeter::KMETER_STEREO_WIDTH + 94;
    }
    else
    {
        nHeight = 630;

        if (pProcessor->getAverageAlgorithm() == KmeterPluginParameters::selAlgorithmItuBs1770)
        {
            nWidth = Kmeter::KMETER_STEREO_WIDTH + 94;
        }
        else
        {
            nWidth = nStereoInputChannels * (Kmeter::KMETER_STEREO_WIDTH + 6) + 88;
        }
    }

    setSize(nWidth, nHeight);

    if (nInputChannels <= 2)
    {
        stereoMeter->setBounds(10, nHeight - 41, 106, 13);
        phaseCorrelationMeter->setBounds(10, nHeight - 24, 106, 13);
    }

    pSkin->placeComponent(ButtonK20, "button_k20");
    pSkin->placeComponent(ButtonK14, "button_k14");
    pSkin->placeComponent(ButtonK12, "button_k12");
    pSkin->placeComponent(ButtonNormal, "button_normal");

    pSkin->placeComponent(ButtonItuBs1770, "button_itu");
    pSkin->placeComponent(ButtonRms, "button_rms");

    pSkin->placeComponent(ButtonInfiniteHold, "button_hold");
    pSkin->placeComponent(ButtonDisplayPeakMeter, "button_peaks");
    pSkin->placeComponent(ButtonExpanded, "button_expand");

    pSkin->placeComponent(ButtonMono, "button_mono");
    pSkin->placeComponent(ButtonReset, "button_reset");
    pSkin->placeComponent(ButtonSkin, "button_skin");

    pSkin->placeComponent(ButtonValidation, "button_validate");
    pSkin->placeComponent(ButtonAbout, "button_about");

    if (LabelDebug)
    {
        pSkin->placeComponent(LabelDebug, "label_debug");
    }
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
            if (kmeter)
            {
                kmeter->setLevels(pMeterBallistics);
            }

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
    MeterBallistics* pMeterBallistics = nullptr;

    switch (nIndex)
    {
    case KmeterPluginParameters::selCrestFactor:

        if (nValue == 0)
        {
            nCrestFactor = nValue;
            bReloadMeters = true;

            pSkin->updateSkin(nInputChannels, nCrestFactor, pProcessor->getAverageAlgorithm());
            ButtonNormal->setToggleState(true, dontSendNotification);
        }
        else if (nValue == 12)
        {
            nCrestFactor = nValue;
            bReloadMeters = true;

            pSkin->updateSkin(nInputChannels, nCrestFactor, pProcessor->getAverageAlgorithm());
            ButtonK12->setToggleState(true, dontSendNotification);
        }
        else if (nValue == 14)
        {
            nCrestFactor = nValue;
            bReloadMeters = true;

            pSkin->updateSkin(nInputChannels, nCrestFactor, pProcessor->getAverageAlgorithm());
            ButtonK14->setToggleState(true, dontSendNotification);
        }
        else // K-20
        {
            nCrestFactor = 20;
            bReloadMeters = true;

            pSkin->updateSkin(nInputChannels, nCrestFactor, pProcessor->getAverageAlgorithm());
            ButtonK20->setToggleState(true, dontSendNotification);
        }

        break;

    case KmeterPluginParameters::selAverageAlgorithm:

        // the "RMS" and "ITU-R" buttons will be updated from the code
        // that actually switches the level averaging alghorithm, thus
        // making sure that the correct button is lit in any given
        // situation
        //
        // we just need to make make sure that this code is actually
        // executed...
        pProcessor->setAverageAlgorithm(nValue);

        if (nInputChannels > 2)
        {
            resizeEditor();
        }

        break;

    case KmeterPluginParameters::selExpanded:
        bReloadMeters = true;
        ButtonExpanded->setToggleState(nValue != 0, dontSendNotification);
        break;

        // case KmeterPluginParameters::selSkin:
        //     bReloadMeters = true;

        //     ButtonSkin->setToggleState(false, dontSendNotification);

        //     pSkin->updateSkin(nInputChannels, nCrestFactor, pProcessor->getAverageAlgorithm());
        //     resizeEditor();
        //     break;

    case KmeterPluginParameters::selPeak:
        bReloadMeters = true;
        ButtonDisplayPeakMeter->setToggleState(nValue != 0, dontSendNotification);
        break;

    case KmeterPluginParameters::selInfiniteHold:
        pMeterBallistics = pProcessor->getLevels();

        if (pMeterBallistics)
        {
            pMeterBallistics->setPeakMeterInfiniteHold(nValue != 0);
            pMeterBallistics->setAverageMeterInfiniteHold(nValue != 0);
        }

        ButtonInfiniteHold->setToggleState(nValue != 0, dontSendNotification);
        break;

    case KmeterPluginParameters::selMono:
        ButtonMono->setToggleState(nValue != 0, dontSendNotification);
        break;
    }

    // prevent meter reload during initialisation
    if (!bInitialising)
    {
        reloadMeters();
    }
}


void KmeterAudioProcessorEditor::reloadMeters()
{
    if (bReloadMeters)
    {
        bReloadMeters = false;
        bool isSurround = (nInputChannels > 2);

        if (kmeter)
        {
            removeChildComponent(kmeter);
            delete kmeter;
            kmeter = nullptr;
        }

        if (pProcessor->getAverageAlgorithm() == KmeterPluginParameters::selAlgorithmItuBs1770)
        {
            String strUnit;

            if (ButtonDisplayPeakMeter->getToggleState())
            {
                strUnit = String("LK|dB");
            }
            else
            {
                strUnit = String("LK");
            }

            kmeter = new Kmeter("K-Meter", 10, 10, nCrestFactor, 1, strUnit, isSurround, ButtonExpanded->getToggleState(), false, ButtonDisplayPeakMeter->getToggleState(), 4);
        }
        else
        {
            String strUnit;

            if (ButtonDisplayPeakMeter->getToggleState())
            {
                strUnit = String("dB|dB");
            }
            else
            {
                strUnit = String("dB");
            }

            kmeter = new Kmeter("K-Meter", 10, 10, nCrestFactor, nInputChannels, strUnit, isSurround, ButtonExpanded->getToggleState(), false, ButtonDisplayPeakMeter->getToggleState(), 4);
        }

        addAndMakeVisible(kmeter);
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
    // else if (button == ButtonSkin)
    // {
    //     pProcessor->changeParameter(KmeterPluginParameters::selSkin, button->getToggleState());
    // }
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
        windowAbout = nullptr;
    }
    else if (button == ButtonValidation)
    {
        WindowValidation* windowValidation = new WindowValidation(getWidth(), getHeight(), pProcessor);
        addAndMakeVisible(windowValidation);

        windowValidation->runModalLoop();

        removeChildComponent(windowValidation);
        delete windowValidation;
        windowValidation = nullptr;
    }
}


void KmeterAudioProcessorEditor::updateAverageAlgorithm(bool reload_meters)
{
    if (pProcessor->getAverageAlgorithm() == KmeterPluginParameters::selAlgorithmItuBs1770)
    {
        ButtonItuBs1770->setToggleState(true, dontSendNotification);
        ButtonRms->setToggleState(false, dontSendNotification);
    }
    else
    {
        ButtonItuBs1770->setToggleState(false, dontSendNotification);
        ButtonRms->setToggleState(true, dontSendNotification);
    }

    bReloadMeters = reload_meters;
    MeterBallistics* pMeterBallistics = pProcessor->getLevels();

    if (pMeterBallistics)
    {
        pMeterBallistics->reset();
    }

    pSkin->updateSkin(nInputChannels, nCrestFactor, pProcessor->getAverageAlgorithm());

    if (!bInitialising)
    {
        resizeEditor();
        reloadMeters();
    }
}

void KmeterAudioProcessorEditor::resized()
{
}



// Local Variables:
// ispell-local-dictionary: "british"
// End:
