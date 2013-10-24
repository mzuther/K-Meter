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

    bReloadMeters = false;
    bIsValidating = false;

    bRotateMeters = false;
    fTinyScale = 1.0f;

    nInputChannels = nNumChannels;
    nStereoInputChannels = (nNumChannels + (nNumChannels % 2)) / 2;
    nCrestFactor = 0;

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
    LabelDebug = NULL;
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

    // This is where our plug-in editor's size is set.
    resizeEditor();

    AffineTransform transformation = AffineTransform();

    if (nInputChannels <= 2)
    {
        if (bRotateMeters)
        {
            stereoMeter = new StereoMeter("Stereo Meter", 10, nWidth - 41, 106, 13);
            addAndMakeVisible(stereoMeter);

            phaseCorrelationMeter = new PhaseCorrelationMeter("Correlation Meter", 10, nWidth - 24, 106, 13);
            addAndMakeVisible(phaseCorrelationMeter);

            transformation = transformation.rotated(M_PI / 2.0f, 10.0f, 10.0f);
            transformation = transformation.translated(nWidth - 20.0f, 0.0f);
        }
        else
        {
            stereoMeter = new StereoMeter("Stereo Meter", 10, nHeight - 41, 106, 13);
            addAndMakeVisible(stereoMeter);

            phaseCorrelationMeter = new PhaseCorrelationMeter("Correlation Meter", 10, nHeight - 24, 106, 13);
            addAndMakeVisible(phaseCorrelationMeter);
        }

        if (fTinyScale < 1.0f)
        {
            transformation = transformation.scaled(fTinyScale, fTinyScale, 10.0f, 10.0f);
        }

        stereoMeter->setTransform(transformation);
        phaseCorrelationMeter->setTransform(transformation);
    }
    else
    {
        stereoMeter = NULL;
        phaseCorrelationMeter = NULL;
    }

    pProcessor->addActionListenerParameters(this);

    kmeter = NULL;

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


void KmeterAudioProcessorEditor::setBoundsButtonColumn(Component* component, int x, int y, int width, int height)
{
    component->setBounds(nButtonColumnLeft + x, nButtonColumnTop + y, width, height);
}


void KmeterAudioProcessorEditor::resizeEditor()
{
    if (bRotateMeters)
    {
        if (nInputChannels <= 2)
        {
            nWidth = 648;
            nButtonColumnTop = nStereoInputChannels * Kmeter::KMETER_STEREO_WIDTH + 24;
        }
        else
        {
            nWidth = 630;

            if (pProcessor->getAverageAlgorithm() == KmeterPluginParameters::selAlgorithmItuBs1770)
            {
                nButtonColumnTop = Kmeter::KMETER_STEREO_WIDTH + 24;
            }
            else
            {
                nButtonColumnTop = nStereoInputChannels * (Kmeter::KMETER_STEREO_WIDTH + 6) + 18;
            }
        }

        nButtonColumnLeft = 10;
        nHeight = nButtonColumnTop + 56;
        setSize(nWidth, nHeight);

        setBoundsButtonColumn(ButtonK20, 0, 0, 60, 20);
        setBoundsButtonColumn(ButtonK14, 66, 0, 60, 20);
        setBoundsButtonColumn(ButtonK12, 132, 0, 60, 20);
        setBoundsButtonColumn(ButtonNormal, 198, 0, 60, 20);

        setBoundsButtonColumn(ButtonItuBs1770, 0, 25, 60, 20);
        setBoundsButtonColumn(ButtonRms, 66, 25, 60, 20);

        setBoundsButtonColumn(ButtonInfiniteHold, 280, 0, 60, 20);
        setBoundsButtonColumn(ButtonDisplayPeakMeter, 346, 0, 60, 20);
        setBoundsButtonColumn(ButtonExpanded, 412, 0, 60, 20);

        setBoundsButtonColumn(ButtonMono, 280, 25, 60, 20);
        setBoundsButtonColumn(ButtonReset, 346, 25, 60, 20);

        setBoundsButtonColumn(ButtonValidation, nWidth - 80, 0, 60, 20);
        setBoundsButtonColumn(ButtonAbout, nWidth - 80, 25, 60, 20);

        if (LabelDebug)
        {
            setBoundsButtonColumn(LabelDebug, 198, 25, 60, 16);
        }
    }
    else
    {
        if (nInputChannels <= 2)
        {
            nHeight = 648;
            nButtonColumnLeft = nStereoInputChannels * Kmeter::KMETER_STEREO_WIDTH + 24;
        }
        else
        {
            nHeight = 630;

            if (pProcessor->getAverageAlgorithm() == KmeterPluginParameters::selAlgorithmItuBs1770)
            {
                nButtonColumnLeft = Kmeter::KMETER_STEREO_WIDTH + 24;
            }
            else
            {
                nButtonColumnLeft = nStereoInputChannels * (Kmeter::KMETER_STEREO_WIDTH + 6) + 18;
            }
        }

        nButtonColumnTop = 10;
        nWidth = nButtonColumnLeft + 70;
        setSize(nWidth, nHeight);

        setBoundsButtonColumn(ButtonK20, 0, 0, 60, 20);
        setBoundsButtonColumn(ButtonK14, 0, 25, 60, 20);
        setBoundsButtonColumn(ButtonK12, 0, 50, 60, 20);
        setBoundsButtonColumn(ButtonNormal, 0, 75, 60, 20);

        setBoundsButtonColumn(ButtonItuBs1770, 0, 115, 60, 20);
        setBoundsButtonColumn(ButtonRms, 0, 140, 60, 20);

        setBoundsButtonColumn(ButtonInfiniteHold, 0, 180, 60, 20);
        setBoundsButtonColumn(ButtonDisplayPeakMeter, 0, 205, 60, 20);
        setBoundsButtonColumn(ButtonExpanded, 0, 230, 60, 20);

        setBoundsButtonColumn(ButtonMono, 0, 270, 60, 20);
        setBoundsButtonColumn(ButtonReset, 0, 295, 60, 20);

        setBoundsButtonColumn(ButtonValidation, 0, nHeight - 66, 60, 20);
        setBoundsButtonColumn(ButtonAbout, 0, nHeight - 41, 60, 20);

        if (LabelDebug)
        {
            setBoundsButtonColumn(LabelDebug, 0, nHeight - 102, 60, 16);
        }
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
            kmeter->setLevels(pMeterBallistics);

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
        else // K-20
        {
            nCrestFactor = nValue;
            bReloadMeters = true;

            ButtonK20->setToggleState(true, false);
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

        if (nStereoInputChannels > 2)
        {
            resizeEditor();
        }

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

    reloadMeters();
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
            kmeter = NULL;
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

            kmeter = new Kmeter("K-Meter", 10, 10, nCrestFactor, 1, strUnit, isSurround, ButtonExpanded->getToggleState(), ButtonDisplayPeakMeter->getToggleState(), 4);
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

            kmeter = new Kmeter("K-Meter", 10, 10, nCrestFactor, nInputChannels, strUnit, isSurround, ButtonExpanded->getToggleState(), ButtonDisplayPeakMeter->getToggleState(), 4);
        }

        addAndMakeVisible(kmeter);

        AffineTransform transformation = AffineTransform();

        if (bRotateMeters)
        {
            transformation = transformation.rotated(M_PI / 2.0f, 10.0f, 10.0f);
            transformation = transformation.translated(nWidth - 20.0f, 0.0f);
        }

        if (fTinyScale < 1.0f)
        {
            transformation = transformation.scaled(fTinyScale, fTinyScale, 10.0f, 10.0f);
        }

        kmeter->setTransform(transformation);
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
        WindowValidation* windowValidation = new WindowValidation(getWidth(), getHeight(), bRotateMeters, pProcessor);
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

    reloadMeters();
}

void KmeterAudioProcessorEditor::resized()
{
}



// Local Variables:
// ispell-local-dictionary: "british"
// End:
