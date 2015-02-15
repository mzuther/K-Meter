/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2015 Martin Zuther (http://www.mzuther.de/)

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
KmeterAudioProcessorEditor::KmeterAudioProcessorEditor(KmeterAudioProcessor *ownerFilter, int nNumChannels)
    : AudioProcessorEditor(ownerFilter)
{
    // the editor window does not have any transparent areas
    // (increases performance on redrawing)
    setOpaque(true);

    // prevent meter reload during initialisation
    bInitialising = true;

    bIsValidating = false;
    bValidateWindow = false;

    nInputChannels = nNumChannels;
    nStereoInputChannels = (nNumChannels + (nNumChannels % 2)) / 2;
    nCrestFactor = 0;

    bExpanded = false;
    bDisplayPeakMeter = false;

    // The plug-in editor's size as well as the location of buttons
    // and labels will be set later on in this constructor.

    pProcessor = ownerFilter;
    pProcessor->addActionListener(this);

    ButtonK20 = new ImageButton("K-20");
    ButtonK20->setRadioGroupId(1);
    ButtonK20->addListener(this);
    addAndMakeVisible(ButtonK20);

    ButtonK14 = new ImageButton("K-14");
    ButtonK14->setRadioGroupId(1);
    ButtonK14->addListener(this);
    addAndMakeVisible(ButtonK14);

    ButtonK12 = new ImageButton("K-12");
    ButtonK12->setRadioGroupId(1);
    ButtonK12->addListener(this);
    addAndMakeVisible(ButtonK12);

    ButtonNormal = new ImageButton("Normal");
    ButtonNormal->setRadioGroupId(1);
    ButtonNormal->addListener(this);
    addAndMakeVisible(ButtonNormal);

    ButtonItuBs1770 = new ImageButton("ITU-R");
    ButtonItuBs1770->addListener(this);
    addAndMakeVisible(ButtonItuBs1770);

    ButtonRms = new ImageButton("RMS");
    ButtonRms->addListener(this);
    addAndMakeVisible(ButtonRms);

    updateAverageAlgorithm(false);

    ButtonInfinitePeakHold = new ImageButton("Hold");
    ButtonInfinitePeakHold->addListener(this);
    addAndMakeVisible(ButtonInfinitePeakHold);

    ButtonDisplayPeakMeter = new ImageButton("Peaks");
    ButtonDisplayPeakMeter->addListener(this);
    addAndMakeVisible(ButtonDisplayPeakMeter);

    ButtonExpanded = new ImageButton("Expand");
    ButtonExpanded->addListener(this);
    addAndMakeVisible(ButtonExpanded);

    ButtonSkin = new ImageButton("Skin");
    ButtonSkin->addListener(this);
    addAndMakeVisible(ButtonSkin);

    ButtonMono = new ImageButton("Mono");
    ButtonMono->addListener(this);
    addAndMakeVisible(ButtonMono);

    ButtonReset = new ImageButton("Reset");
    ButtonReset->addListener(this);
    addAndMakeVisible(ButtonReset);

    ButtonValidation = new ImageButton("Validate");
    ButtonValidation->addListener(this);
    addAndMakeVisible(ButtonValidation);

    ButtonAbout = new ImageButton("About");
    ButtonAbout->addListener(this);
    addAndMakeVisible(ButtonAbout);

    LabelDebug = new ImageComponent("Debug Notification");
    // moves debug label to the back of the editor's z-plane to that
    // it doesn't overlay (and thus block) any other components
    addAndMakeVisible(LabelDebug, 0);

    BackgroundImage = new ImageComponent("Background Image");
    // prevent unnecessary redrawing of plugin editor
    BackgroundImage->setOpaque(true);
    // moves background image to the back of the editor's z-plane to
    // that it doesn't overlay (and thus block) any other components
    addAndMakeVisible(BackgroundImage, 0);

    if (nInputChannels <= 2)
    {
        stereoMeter = new HorizontalMeter("Stereo Meter");
        addAndMakeVisible(stereoMeter);

        phaseCorrelationMeter = new HorizontalMeter("Correlation Meter");
        addAndMakeVisible(phaseCorrelationMeter);
    }
    else
    {
        stereoMeter = nullptr;
        phaseCorrelationMeter = nullptr;
    }

    kmeter = nullptr;

    updateParameter(KmeterPluginParameters::selCrestFactor);
    updateParameter(KmeterPluginParameters::selAverageAlgorithm);
    updateParameter(KmeterPluginParameters::selExpanded);
    updateParameter(KmeterPluginParameters::selShowPeaks);
    updateParameter(KmeterPluginParameters::selInfinitePeakHold);
    updateParameter(KmeterPluginParameters::selMono);

    // the following may or may not work on Mac
    File fileApplicationDirectory = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory();
    fileSkinDirectory = fileApplicationDirectory.getChildFile("./kmeter-skins/");

    pSkin = nullptr;
    strSkinName = pProcessor->getParameterSkinName();
    loadSkin();

    // force meter reload after initialisation ...
    bInitialising = false;
    bReloadMeters = true;

    // will also apply skin to plug-in editor
    reloadMeters();
}


KmeterAudioProcessorEditor::~KmeterAudioProcessorEditor()
{
    pProcessor->removeActionListener(this);

    delete pSkin;
    pSkin = nullptr;

    deleteAllChildren();
}


void KmeterAudioProcessorEditor::loadSkin()
{
    if (pSkin != nullptr)
    {
        delete pSkin;
        pSkin = nullptr;
    }

    File fileSkin = fileSkinDirectory.getChildFile(strSkinName + ".skin");

    if (!fileSkin.existsAsFile())
    {
        Logger::outputDebugString("[Skin] file \"" + fileSkin.getFileName() + "\" not found");

        strSkinName = "Default";
        fileSkin = fileSkinDirectory.getChildFile(strSkinName + ".skin");
    }

    pProcessor->setParameterSkinName(strSkinName);
    pSkin = new Skin(fileSkin, nInputChannels, nCrestFactor, pProcessor->getAverageAlgorithm(), bExpanded, bDisplayPeakMeter);
}


void KmeterAudioProcessorEditor::applySkin()
{
    // prevent skin application during meter initialisation
    if (bInitialising)
    {
        return;
    }

    // update skin
    pSkin->updateSkin(nInputChannels, nCrestFactor, pProcessor->getAverageAlgorithm(), bExpanded, bDisplayPeakMeter);

    // moves background image to the back of the editor's z-plane;
    // will also resize plug-in editor
    pSkin->setBackgroundImage(BackgroundImage, this);

    pSkin->placeAndSkinButton(ButtonK20, "button_k20");
    pSkin->placeAndSkinButton(ButtonK14, "button_k14");
    pSkin->placeAndSkinButton(ButtonK12, "button_k12");
    pSkin->placeAndSkinButton(ButtonNormal, "button_normal");

    pSkin->placeAndSkinButton(ButtonItuBs1770, "button_itu");
    pSkin->placeAndSkinButton(ButtonRms, "button_rms");

    pSkin->placeAndSkinButton(ButtonInfinitePeakHold, "button_hold");
    pSkin->placeAndSkinButton(ButtonDisplayPeakMeter, "button_peaks");
    pSkin->placeAndSkinButton(ButtonExpanded, "button_expand");

    pSkin->placeAndSkinButton(ButtonMono, "button_mono");
    pSkin->placeAndSkinButton(ButtonReset, "button_reset");
    pSkin->placeAndSkinButton(ButtonSkin, "button_skin");

    pSkin->placeAndSkinButton(ButtonValidation, "button_validate");
    pSkin->placeAndSkinButton(ButtonAbout, "button_about");

#ifdef DEBUG
    pSkin->placeAndSkinLabel(LabelDebug, "label_debug");
#else
    pSkin->placeComponent(LabelDebug, "label_debug");
    LabelDebug->setImage(Image());
    LabelDebug->setBounds(-1, -1, 1, 1);
#endif

    if (kmeter != nullptr)
    {
        kmeter->applySkin(pSkin);
    }

    if (stereoMeter != nullptr)
    {
        pSkin->placeAndSkinHorizontalMeter(stereoMeter, "meter_stereo");
    }

    if (phaseCorrelationMeter != nullptr)
    {
        pSkin->placeAndSkinHorizontalMeter(phaseCorrelationMeter, "meter_phase_correlation");
    }
}


void KmeterAudioProcessorEditor::actionListenerCallback(const String &strMessage)
{
    // "PC" --> parameter changed, followed by a hash and the
    // parameter's ID
    if (strMessage.startsWith("PC#"))
    {
        String strIndex = strMessage.substring(3);
        int nIndex = strIndex.getIntValue();
        jassert(nIndex >= 0);
        jassert(nIndex < pProcessor->getNumParameters());

        if (pProcessor->hasChanged(nIndex))
        {
            updateParameter(nIndex);
        }
    }
    // "UM" --> update meters
    else if (!strMessage.compare("UM"))
    {
        MeterBallistics *pMeterBallistics = pProcessor->getLevels();

        if (pMeterBallistics != nullptr)
        {
            if (kmeter != nullptr)
            {
                kmeter->setLevels(pMeterBallistics);
            }

            if (stereoMeter != nullptr)
            {
                float fStereo = pMeterBallistics->getStereoMeterValue();
                stereoMeter->setValue(fStereo / 2.0f + 0.5f);
            }

            if (phaseCorrelationMeter != nullptr)
            {
                float fPhase = pMeterBallistics->getPhaseCorrelation();
                phaseCorrelationMeter->setValue(fPhase / 2.0f + 0.5f);
            }
        }

        if (bIsValidating && !pProcessor->isValidating())
        {
            bIsValidating = false;
        }
    }
    // "AC" --> algorithm changed
    else if (!strMessage.compare("AC"))
    {
        updateAverageAlgorithm(true);
    }
    // "V+" --> validation started
    else if ((!strMessage.compare("V+")) && pProcessor->isValidating())
    {
        bIsValidating = true;
    }
    // "V-" --> validation stopped
    else if (!strMessage.compare("V-"))
    {
        if (!bValidateWindow)
        {
            ButtonValidation->setToggleState(false, dontSendNotification);
        }

        // do nothing till you hear from me... :)
    }
    else
    {
        DBG("[K-Meter] received unknown action strMessage \"" + strMessage + "\".");
    }
}


void KmeterAudioProcessorEditor::updateParameter(int nIndex)
{
    MeterBallistics *pMeterBallistics = nullptr;
    int nValue = pProcessor->getRealInteger(nIndex);

    pProcessor->clearChangeFlag(nIndex);

    switch (nIndex)
    {
    case KmeterPluginParameters::selCrestFactor:

        if (nValue == 0)
        {
            nCrestFactor = nValue;

            // will also apply skin to plug-in editor
            bReloadMeters = true;

            ButtonNormal->setToggleState(true, dontSendNotification);
        }
        else if (nValue == 12)
        {
            nCrestFactor = nValue;

            // will also apply skin to plug-in editor
            bReloadMeters = true;

            ButtonK12->setToggleState(true, dontSendNotification);
        }
        else if (nValue == 14)
        {
            nCrestFactor = nValue;

            // will also apply skin to plug-in editor
            bReloadMeters = true;

            ButtonK14->setToggleState(true, dontSendNotification);
        }
        else // K-20
        {
            nCrestFactor = 20;

            // will also apply skin to plug-in editor
            bReloadMeters = true;

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

        break;

    case KmeterPluginParameters::selExpanded:
        bExpanded = (nValue != 0);
        ButtonExpanded->setToggleState(bExpanded, dontSendNotification);

        // will also apply skin to plug-in editor
        bReloadMeters = true;
        break;

    case KmeterPluginParameters::selShowPeaks:
        bDisplayPeakMeter = (nValue != 0);
        ButtonDisplayPeakMeter->setToggleState(bDisplayPeakMeter, dontSendNotification);

        // will also apply skin to plug-in editor
        bReloadMeters = true;
        break;

    case KmeterPluginParameters::selInfinitePeakHold:
        pMeterBallistics = pProcessor->getLevels();

        if (pMeterBallistics != nullptr)
        {
            pMeterBallistics->setPeakMeterInfiniteHold(nValue != 0);
            pMeterBallistics->setAverageMeterInfiniteHold(nValue != 0);
        }

        ButtonInfinitePeakHold->setToggleState(nValue != 0, dontSendNotification);
        break;

    case KmeterPluginParameters::selMono:
        ButtonMono->setToggleState(nValue != 0, dontSendNotification);
        break;
    }

    // prevent meter reload during initialisation
    if (!bInitialising)
    {
        // will also apply skin to plug-in editor
        reloadMeters();
    }
}


void KmeterAudioProcessorEditor::reloadMeters()
{
    if (bReloadMeters)
    {
        bReloadMeters = false;

        if (kmeter != nullptr)
        {
            removeChildComponent(kmeter);
            delete kmeter;
            kmeter = nullptr;
        }

        if (pProcessor->getAverageAlgorithm() == KmeterPluginParameters::selAlgorithmItuBs1770)
        {
            kmeter = new Kmeter("K-Meter", nCrestFactor, 1, ButtonExpanded->getToggleState(), false, ButtonDisplayPeakMeter->getToggleState(), 4);
        }
        else
        {
            kmeter = new Kmeter("K-Meter", nCrestFactor, nInputChannels, ButtonExpanded->getToggleState(), false, ButtonDisplayPeakMeter->getToggleState(), 4);
        }

        // moves traKmeter to the back of the editor's z-plane so that
        // it doesn't overlay (and thus block) any other components
        addAndMakeVisible(kmeter, 0);

        // moves background image to the back of the editor's z-plane
        applySkin();
    }
}

//==============================================================================
void KmeterAudioProcessorEditor::paint(Graphics &g)
{
    g.fillAll(Colours::black);
}

void KmeterAudioProcessorEditor::buttonClicked(Button *button)
{
    if (button == ButtonNormal)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selCrestFactor, KmeterPluginParameters::selNormal / float(KmeterPluginParameters::nNumCrestFactors - 1));
    }
    else if (button == ButtonK12)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selCrestFactor, KmeterPluginParameters::selK12 / float(KmeterPluginParameters::nNumCrestFactors - 1));
    }
    else if (button == ButtonK14)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selCrestFactor, KmeterPluginParameters::selK14 / float(KmeterPluginParameters::nNumCrestFactors - 1));
    }
    else if (button == ButtonK20)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selCrestFactor, KmeterPluginParameters::selK20 / float(KmeterPluginParameters::nNumCrestFactors - 1));
    }
    else if (button == ButtonInfinitePeakHold)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selInfinitePeakHold, button->getToggleState() ? 0.0f : 1.0f);
    }
    else if (button == ButtonRms)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selAverageAlgorithm, KmeterPluginParameters::selAlgorithmRms / float(KmeterPluginParameters::nNumAlgorithms - 1));
    }
    else if (button == ButtonItuBs1770)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selAverageAlgorithm, KmeterPluginParameters::selAlgorithmItuBs1770 / float(KmeterPluginParameters::nNumAlgorithms - 1));
    }
    else if (button == ButtonExpanded)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selExpanded, button->getToggleState() ? 0.0f : 1.0f);
    }
    else if (button == ButtonSkin)
    {
        // manually activate button
        button->setToggleState(true, dontSendNotification);

        File fileSkin = fileSkinDirectory.getChildFile(strSkinName + ".skin");

        GenericWindowSkin windowSkin(this, fileSkin);
        windowSkin.runModalLoop();

        // manually deactivate button
        button->setToggleState(false, dontSendNotification);

        strSkinName = windowSkin.getSelectedString();
        loadSkin();

        // will also apply skin to plug-in editor
        bReloadMeters = true;
        reloadMeters();
    }
    else if (button == ButtonDisplayPeakMeter)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selShowPeaks, button->getToggleState() ? 0.0f : 1.0f);
    }
    else if (button == ButtonReset)
    {
        MeterBallistics *pMeterBallistics = pProcessor->getLevels();

        if (pMeterBallistics)
        {
            pMeterBallistics->reset();
        }

        loadSkin();

        // will also apply skin to plug-in editor
        bReloadMeters = true;
        reloadMeters();
    }
    else if (button == ButtonMono)
    {
        pProcessor->changeParameter(KmeterPluginParameters::selMono, button->getToggleState() ? 0.0f : 1.0f);
    }
    else if (button == ButtonAbout)
    {
        // manually activate button
        button->setToggleState(true, dontSendNotification);

        StringPairArray strArray;

        strArray.set("Copyright", "(c) 2010-2015 Martin Zuther\n");

        strArray.set("Contributors",
                     L"Bob Katz\n"
                     L"Jan Kokemüller\n"
                     L"Filipe Coelho\n"
                     L"Bram de Jong\n");

        strArray.set("Beta testing",
                     L"Rickard (Interfearing Sounds)\n");

        strArray.set("Thanks",
                     L"Thanks to Bob Katz, all contributors "
                     L"and beta testers and the open source "
                     L"community at large!\n\n"
                     L"Thank you for using free software!\n");

        strArray.set("Libraries",
#ifdef LINUX
                     L"ALSA\n"
#endif
                     L"FFTW\n"
#ifdef LINUX
                     L"FreeType\n"
                     L"JACK\n"
#endif
                     L"JUCE\n"
#if (KMETER_LV2_PLUGIN != 0)
                     L"LV2\n"
#endif
#ifdef LINUX
                     L"POSIX Threads\n"
                     L"Xlib\n"
                     L"Xext\n"
#endif
                    );

#if (JUCE_USE_VSTSDK_2_4 != 0)

        // display trademarks (but only when necessary)
        strArray.set("Trademarks",
                     L"VST PlugIn Technology by Steinberg\n");

#endif

#if (JUCE_ASIO != 0)

        // display trademarks (but only when necessary)
        strArray.set("Trademarks",
                     L"ASIO Technology by Steinberg Media Technologies GmbH\n");

#endif

        GenericWindowAbout windowAbout(this, strArray);
        windowAbout.runModalLoop();

        // manually deactivate button
        button->setToggleState(false, dontSendNotification);
    }
    else if (button == ButtonValidation)
    {
        // manually activate button
        button->setToggleState(true, dontSendNotification);

        bValidateWindow = true;
        WindowValidation windowValidation(this, pProcessor);
        windowValidation.runModalLoop();
        bValidateWindow = false;

        // manually set button according to validation state
        button->setToggleState(bIsValidating, dontSendNotification);
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
    MeterBallistics *pMeterBallistics = pProcessor->getLevels();

    if (pMeterBallistics != nullptr)
    {
        pMeterBallistics->reset();
    }

    if (!bInitialising)
    {
        // will also apply skin to plug-in editor
        reloadMeters();
    }
}

void KmeterAudioProcessorEditor::resized()
{
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
