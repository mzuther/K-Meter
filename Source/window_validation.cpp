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

#include "window_validation.h"


WindowValidation::WindowValidation(Component *pEditorWindow, KmeterAudioProcessor *processor)
    : DocumentWindow("Validation", Colours::white, 0, true)
    // create new window child
{
    int nWidth = 170;
    int nHeight = 250;

    pProcessor = processor;

    // stops any running validation and resets all meters
    pProcessor->preValidation(true);

    fileValidation = pProcessor->getParameterValidationFile();

    // set dimensions to those passed to the function ...
    setSize(nWidth, nHeight + getTitleBarHeight());

    // ... center window on editor ...
    centreAroundComponent(pEditorWindow, getWidth(), getHeight());

    // ... and keep the new window on top
    setAlwaysOnTop(true);

    // this window does not have any transparent areas (increases
    // performance on redrawing)
    setOpaque(true);

    // empty windows are boring, so let's prepare a space for some
    // window components
    setContentOwned(&contentComponent, false);

    LabelFileSelection.setText(fileValidation.getFileName(), dontSendNotification);
    LabelFileSelection.setMinimumHorizontalScale(1.0f);
    LabelFileSelection.setColour(Label::textColourId, Colours::black);
    LabelFileSelection.setColour(Label::backgroundColourId, Colours::white.darker(0.15f));
    LabelFileSelection.setColour(Label::outlineColourId, Colours::grey);

    // add and display the label
    contentComponent.addAndMakeVisible(LabelFileSelection);

    ButtonFileSelection.setButtonText("...");
    ButtonFileSelection.addListener(this);
    contentComponent.addAndMakeVisible(ButtonFileSelection);

    LabelSampleRate.setText("Host SR: ", dontSendNotification);
    LabelSampleRate.setColour(Label::textColourId, Colours::black);
    contentComponent.addAndMakeVisible(LabelSampleRate);

    int nSampleRate = (int) pProcessor->getSampleRate();
    String strSampleRateThousands = String(nSampleRate / 1000);
    String strSampleRateOnes = String(nSampleRate % 1000).paddedLeft('0', 3);
    String strSampleRate = strSampleRateThousands + " " + strSampleRateOnes + " Hz";

    LabelSampleRateValue.setText(strSampleRate, dontSendNotification);
    LabelSampleRateValue.setMinimumHorizontalScale(1.0f);
    LabelSampleRateValue.setColour(Label::textColourId, Colours::black);
    LabelSampleRateValue.setColour(Label::backgroundColourId, Colours::white.darker(0.15f));
    LabelSampleRateValue.setColour(Label::outlineColourId, Colours::grey);

    // add and display the label
    contentComponent.addAndMakeVisible(LabelSampleRateValue);

    LabelDumpSelectedChannel.setText("Channel: ", dontSendNotification);
    LabelDumpSelectedChannel.setColour(Label::textColourId, Colours::black);
    contentComponent.addAndMakeVisible(LabelDumpSelectedChannel);

    SliderDumpSelectedChannel.setNumberOfChannels(pProcessor->getNumChannels() - 1);
    SliderDumpSelectedChannel.setColour(GenericChannelSlider::textBoxTextColourId, Colours::black);
    SliderDumpSelectedChannel.setColour(GenericChannelSlider::textBoxBackgroundColourId, Colours::white.darker(0.15f));
    SliderDumpSelectedChannel.setColour(GenericChannelSlider::textBoxOutlineColourId, Colours::grey);

    ButtonDumpCSV.setButtonText("CSV format");
    ButtonDumpCSV.setColour(ToggleButton::textColourId, Colours::black);
    ButtonDumpCSV.setToggleState(pProcessor->getBoolean(KmeterPluginParameters::selValidationCSVFormat), dontSendNotification);
    contentComponent.addAndMakeVisible(ButtonDumpCSV);

    SliderDumpSelectedChannel.setValue(pProcessor->getRealInteger(KmeterPluginParameters::selValidationSelectedChannel), dontSendNotification);
    contentComponent.addAndMakeVisible(SliderDumpSelectedChannel);

    ButtonDumpAverageMeterLevel.setButtonText("Average meter level");
    ButtonDumpAverageMeterLevel.setColour(ToggleButton::textColourId, Colours::black);
    ButtonDumpAverageMeterLevel.setToggleState(pProcessor->getBoolean(KmeterPluginParameters::selValidationAverageMeterLevel), dontSendNotification);
    contentComponent.addAndMakeVisible(ButtonDumpAverageMeterLevel);

    ButtonDumpPeakMeterLevel.setButtonText("Peak meter level");
    ButtonDumpPeakMeterLevel.setColour(ToggleButton::textColourId, Colours::black);
    ButtonDumpPeakMeterLevel.setToggleState(pProcessor->getBoolean(KmeterPluginParameters::selValidationPeakMeterLevel), dontSendNotification);
    contentComponent.addAndMakeVisible(ButtonDumpPeakMeterLevel);

    ButtonDumpMaximumPeakLevel.setButtonText("Maximum peak level");
    ButtonDumpMaximumPeakLevel.setColour(ToggleButton::textColourId, Colours::black);
    ButtonDumpMaximumPeakLevel.setToggleState(pProcessor->getBoolean(KmeterPluginParameters::selValidationMaximumPeakLevel), dontSendNotification);
    contentComponent.addAndMakeVisible(ButtonDumpMaximumPeakLevel);

    ButtonDumpStereoMeterValue.setButtonText("Stereo meter value");
    ButtonDumpStereoMeterValue.setColour(ToggleButton::textColourId, Colours::black);
    ButtonDumpStereoMeterValue.setToggleState(pProcessor->getBoolean(KmeterPluginParameters::selValidationStereoMeterValue), dontSendNotification);
    contentComponent.addAndMakeVisible(ButtonDumpStereoMeterValue);

    ButtonDumpPhaseCorrelation.setButtonText("Phase correlation");
    ButtonDumpPhaseCorrelation.setColour(ToggleButton::textColourId, Colours::black);
    ButtonDumpPhaseCorrelation.setToggleState(pProcessor->getBoolean(KmeterPluginParameters::selValidationPhaseCorrelation), dontSendNotification);
    contentComponent.addAndMakeVisible(ButtonDumpPhaseCorrelation);

    // create and position a "validation" button which closes the
    // window and runs the selected audio file when clicked
    ButtonValidation.setButtonText("Validate");
    ButtonValidation.setColour(TextButton::textColourOffId, Colours::black);
    ButtonValidation.setColour(TextButton::buttonColourId, Colours::red);
    ButtonValidation.setColour(TextButton::buttonOnColourId, Colours::grey);

    // add "validation" window as button listener and display the
    // button
    ButtonValidation.addListener(this);
    contentComponent.addAndMakeVisible(ButtonValidation);

    // create and position a "validation" button which closes the
    // window when clicked
    ButtonCancel.setButtonText("Cancel");
    ButtonCancel.setColour(TextButton::textColourOffId, Colours::black);
    ButtonCancel.setColour(TextButton::buttonColourId, Colours::yellow);
    ButtonCancel.setColour(TextButton::buttonOnColourId, Colours::grey);

    // add "validation" window as button listener and display the button
    ButtonCancel.addListener(this);
    contentComponent.addAndMakeVisible(ButtonCancel);

    int nPositionX = 4;
    int nPositionY = 7;

    LabelFileSelection.setBounds(nPositionX + 4, nPositionY, 120, 20);
    ButtonFileSelection.setBounds(nPositionX + 127, nPositionY, 30, 20);

    nPositionY += 24;
    LabelSampleRate.setBounds(nPositionX, nPositionY, 75, 20);
    LabelSampleRateValue.setBounds(nPositionX + 66, nPositionY, 82, 20);

    nPositionY += 24;
    LabelDumpSelectedChannel.setBounds(nPositionX, nPositionY, 75, 20);
    SliderDumpSelectedChannel.setBounds(nPositionX + 66, nPositionY, 70, 20);

    nPositionY += 30;
    ButtonDumpPeakMeterLevel.setBounds(nPositionX + 1, nPositionY, 180, 20);

    nPositionY += 20;
    ButtonDumpAverageMeterLevel.setBounds(nPositionX + 1, nPositionY, 180, 20);

    nPositionY += 20;
    ButtonDumpMaximumPeakLevel.setBounds(nPositionX + 1, nPositionY, 180, 20);

    nPositionY += 20;
    ButtonDumpStereoMeterValue.setBounds(nPositionX + 1, nPositionY, 180, 20);

    nPositionY += 20;
    ButtonDumpPhaseCorrelation.setBounds(nPositionX + 1, nPositionY, 180, 20);

    nPositionY += 20;
    ButtonDumpCSV.setBounds(nPositionX + 1, nPositionY, 180, 20);

    nPositionY += 31;
    ButtonValidation.setBounds(18, nPositionY, 60, 20);
    ButtonCancel.setBounds(88, nPositionY, 60, 20);

    // finally, display window
    setVisible(true);
}


WindowValidation::~WindowValidation()
{
    pProcessor->preValidation(false);
}


void WindowValidation::buttonClicked(Button *button)
{
    // find out which button has been clicked
    if (button == &ButtonValidation)
    {
        // file name has not been set
        if (fileValidation.getFileName().isEmpty())
        {
            DBG("[K-Meter] file name for validation not set.");

            // prevent closing of window
            return;
        }

        int nSelectedChannel = (int) SliderDumpSelectedChannel.getValue();
        float fSelectedChannel = SliderDumpSelectedChannel.getFloat();
        pProcessor->setParameter(KmeterPluginParameters::selValidationSelectedChannel, fSelectedChannel);

        bool bReportCSV = ButtonDumpCSV.getToggleState();
        pProcessor->setParameter(KmeterPluginParameters::selValidationCSVFormat, bReportCSV ? 1.0f : 0.0f);

        bool bAverageMeterLevel = ButtonDumpAverageMeterLevel.getToggleState();
        pProcessor->setParameter(KmeterPluginParameters::selValidationAverageMeterLevel, bAverageMeterLevel ? 1.0f : 0.0f);

        bool bPeakMeterLevel = ButtonDumpPeakMeterLevel.getToggleState();
        pProcessor->setParameter(KmeterPluginParameters::selValidationPeakMeterLevel, bPeakMeterLevel ? 1.0f : 0.0f);

        bool bMaximumPeakLevel = ButtonDumpMaximumPeakLevel.getToggleState();
        pProcessor->setParameter(KmeterPluginParameters::selValidationMaximumPeakLevel, bMaximumPeakLevel ? 1.0f : 0.0f);

        bool bStereoMeterValue = ButtonDumpStereoMeterValue.getToggleState();
        pProcessor->setParameter(KmeterPluginParameters::selValidationStereoMeterValue, bStereoMeterValue ? 1.0f : 0.0f);

        bool bPhaseCorrelation = ButtonDumpPhaseCorrelation.getToggleState();
        pProcessor->setParameter(KmeterPluginParameters::selValidationPhaseCorrelation, bPhaseCorrelation ? 1.0f : 0.0f);

        // validation file has already been initialised
        pProcessor->startValidation(fileValidation, nSelectedChannel, bReportCSV, bAverageMeterLevel, bPeakMeterLevel, bMaximumPeakLevel, bStereoMeterValue, bPhaseCorrelation);

        int exitValue = 2;
        exitModalState(exitValue);
    }
    else if (button == &ButtonCancel)
    {
        int exitValue = 1;
        exitModalState(exitValue);
    }
    else if (button == &ButtonFileSelection)
    {
        AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
        String strWildcards = formatManager.getWildcardForAllFormats();

        FileChooser browser("Open audio file for validation", fileValidation, strWildcards, true);

        if (browser.showDialog(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, nullptr))
        {
            File selectedFile = browser.getResult();
            pProcessor->setParameterValidationFile(selectedFile);
            fileValidation = pProcessor->getParameterValidationFile();
            LabelFileSelection.setText(fileValidation.getFileName(), dontSendNotification);
        }
    }
}


void WindowValidation::closeButtonPressed()
{
    int exitValue = 0;
    exitModalState(exitValue);
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
