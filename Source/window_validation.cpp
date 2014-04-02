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

#include "window_validation.h"


WindowValidation::WindowValidation(int nWidth, int nHeight, bool horizontal_layout, KmeterAudioProcessor* processor)
    : ResizableWindow("Validation K-Meter", false)
    // create new window child of width "nWidth" and height "nHeight"
{
    bHorizontalLayout = horizontal_layout;

    pProcessor = processor;
    pProcessor->stopValidation();
    fileValidation = pProcessor->getParameterValidationFile();

    // set dimensions to those passed to the function ...
    setBounds(0, 0, nWidth, nHeight);

    // ... and keep the new window on top
    setAlwaysOnTop(true);

    // this window does not have any transparent areas (increases
    // performance on redrawing)
    setOpaque(true);

    // prohibit movement of the new window
    pConstrainer = new ProhibitingBoundsConstrainer();
    setConstrainer(pConstrainer);

    // empty windows are boring, so let's prepare a space for some
    // window components
    contentComponent = new Component("Window Area");
    setContentOwned(contentComponent, false);

    LabelFileSelection = new Label("Label FileSelection", fileValidation.getFileName());
    LabelFileSelection->setMinimumHorizontalScale(1.0f);
    LabelFileSelection->setColour(Label::textColourId, Colours::white);
    LabelFileSelection->setColour(Label::backgroundColourId, Colours::grey.darker(0.6f));
    LabelFileSelection->setColour(Label::outlineColourId, Colours::black);

    // add and display the label
    contentComponent->addAndMakeVisible(LabelFileSelection);

    ButtonFileSelection = new TextButton("...");
    ButtonFileSelection->addListener(this);
    contentComponent->addAndMakeVisible(ButtonFileSelection);

    LabelSampleRate = new Label("Label SampleRate", "Host SR: ");
    LabelSampleRate->setColour(Label::textColourId, Colours::white);
    contentComponent->addAndMakeVisible(LabelSampleRate);

    int nSampleRate = (int) pProcessor->getSampleRate();
    String strSampleRateThousands = String(nSampleRate / 1000);
    String strSampleRateOnes = String(nSampleRate % 1000).paddedLeft('0', 3);
    String strSampleRate = strSampleRateThousands + " " + strSampleRateOnes + " Hz";

    LabelSampleRateValue = new Label("Label SampleRateValue", strSampleRate);
    LabelSampleRateValue->setMinimumHorizontalScale(1.0f);
    LabelSampleRateValue->setColour(Label::textColourId, Colours::white);
    LabelSampleRateValue->setColour(Label::backgroundColourId, Colours::grey.darker(0.6f));
    LabelSampleRateValue->setColour(Label::outlineColourId, Colours::black);

    // add and display the label
    contentComponent->addAndMakeVisible(LabelSampleRateValue);

    LabelDumpSelectedChannel = new Label("Selected channel", "Channel: ");
    LabelDumpSelectedChannel->setColour(Label::textColourId, Colours::white);
    contentComponent->addAndMakeVisible(LabelDumpSelectedChannel);

    SliderDumpSelectedChannel = new ChannelSlider("Selected channel", pProcessor->getNumChannels() - 1);
    SliderDumpSelectedChannel->setColour(ChannelSlider::textBoxTextColourId, Colours::white);
    SliderDumpSelectedChannel->setColour(ChannelSlider::textBoxBackgroundColourId, Colours::grey.darker(0.6f));
    SliderDumpSelectedChannel->setColour(ChannelSlider::textBoxOutlineColourId, Colours::black);

    ButtonDumpCSV = new ToggleButton("CSV format");
    ButtonDumpCSV->setColour(ToggleButton::textColourId, Colours::white);
    ButtonDumpCSV->setToggleState(pProcessor->getParameterAsBool(KmeterPluginParameters::selValidationCSVFormat), dontSendNotification);
    contentComponent->addAndMakeVisible(ButtonDumpCSV);

    SliderDumpSelectedChannel->setValue(pProcessor->getParameterAsInt(KmeterPluginParameters::selValidationSelectedChannel), dontSendNotification);
    contentComponent->addAndMakeVisible(SliderDumpSelectedChannel);

    ButtonDumpAverageMeterLevel = new ToggleButton("Average meter level");
    ButtonDumpAverageMeterLevel->setColour(ToggleButton::textColourId, Colours::white);
    ButtonDumpAverageMeterLevel->setToggleState(pProcessor->getParameterAsBool(KmeterPluginParameters::selValidationAverageMeterLevel), dontSendNotification);
    contentComponent->addAndMakeVisible(ButtonDumpAverageMeterLevel);

    ButtonDumpPeakMeterLevel = new ToggleButton("Peak meter level");
    ButtonDumpPeakMeterLevel->setColour(ToggleButton::textColourId, Colours::white);
    ButtonDumpPeakMeterLevel->setToggleState(pProcessor->getParameterAsBool(KmeterPluginParameters::selValidationPeakMeterLevel), dontSendNotification);
    contentComponent->addAndMakeVisible(ButtonDumpPeakMeterLevel);

    ButtonDumpMaximumPeakLevel = new ToggleButton("Maximum peak level");
    ButtonDumpMaximumPeakLevel->setColour(ToggleButton::textColourId, Colours::white);
    ButtonDumpMaximumPeakLevel->setToggleState(pProcessor->getParameterAsBool(KmeterPluginParameters::selValidationMaximumPeakLevel), dontSendNotification);
    contentComponent->addAndMakeVisible(ButtonDumpMaximumPeakLevel);

    ButtonDumpStereoMeterValue = new ToggleButton("Stereo meter value");
    ButtonDumpStereoMeterValue->setColour(ToggleButton::textColourId, Colours::white);
    ButtonDumpStereoMeterValue->setToggleState(pProcessor->getParameterAsBool(KmeterPluginParameters::selValidationStereoMeterValue), dontSendNotification);
    contentComponent->addAndMakeVisible(ButtonDumpStereoMeterValue);

    ButtonDumpPhaseCorrelation = new ToggleButton("Phase correlation");
    ButtonDumpPhaseCorrelation->setColour(ToggleButton::textColourId, Colours::white);
    ButtonDumpPhaseCorrelation->setToggleState(pProcessor->getParameterAsBool(KmeterPluginParameters::selValidationPhaseCorrelation), dontSendNotification);
    contentComponent->addAndMakeVisible(ButtonDumpPhaseCorrelation);

    // create and position a "validation" button which closes the
    // window and runs the selected audio file when clicked
    ButtonValidation = new TextButton("Validate");
    ButtonValidation->setColour(TextButton::textColourOffId, Colours::black);
    ButtonValidation->setColour(TextButton::buttonColourId, Colours::red);
    ButtonValidation->setColour(TextButton::buttonOnColourId, Colours::grey);

    // add "validation" window as button listener and display the
    // button
    ButtonValidation->addListener(this);
    contentComponent->addAndMakeVisible(ButtonValidation);

    // create and position a "validation" button which closes the
    // window when clicked
    ButtonCancel = new TextButton("Cancel");
    ButtonCancel->setColour(TextButton::textColourOffId, Colours::black);
    ButtonCancel->setColour(TextButton::buttonColourId, Colours::yellow);
    ButtonCancel->setColour(TextButton::buttonOnColourId, Colours::grey);

    // add "validation" window as button listener and display the button
    ButtonCancel->addListener(this);
    contentComponent->addAndMakeVisible(ButtonCancel);

    if (bHorizontalLayout)
    {
        ButtonFileSelection->setBounds(nWidth - 209, nHeight - 170, 30, 20);
        LabelFileSelection->setBounds(nWidth - 358, nHeight - 170, 144, 20);

        LabelSampleRate->setBounds(nWidth - 362, nHeight - 144, 75, 20);
        LabelSampleRateValue->setBounds(nWidth - 296, nHeight - 144, 82, 20);

        LabelDumpSelectedChannel->setBounds(nWidth - 362, nHeight - 118, 75, 20);
        SliderDumpSelectedChannel->setBounds(nWidth - 296, nHeight - 118, 70, 20);
        ButtonDumpCSV->setBounds(nWidth - 362, nHeight - 94, 180, 20);

        ButtonDumpPeakMeterLevel->setBounds(nWidth - 165, nHeight - 174, 180, 20);
        ButtonDumpAverageMeterLevel->setBounds(nWidth - 165, nHeight - 154, 180, 20);
        ButtonDumpMaximumPeakLevel->setBounds(nWidth - 165, nHeight - 134, 180, 20);
        ButtonDumpStereoMeterValue->setBounds(nWidth - 165, nHeight - 114, 180, 20);
        ButtonDumpPhaseCorrelation->setBounds(nWidth - 165, nHeight - 94, 180, 20);

        ButtonValidation->setBounds(nWidth - 73, nHeight - 59, 60, 20);
        ButtonCancel->setBounds(nWidth - 138, nHeight - 59, 60, 20);
    }
    else
    {
        ButtonFileSelection->setBounds(nWidth - 45, nHeight - 269, 30, 20);
        LabelFileSelection->setBounds(nWidth - 188, nHeight - 269, 138, 20);

        LabelSampleRate->setBounds(nWidth - 192, nHeight - 244, 75, 20);
        LabelSampleRateValue->setBounds(nWidth - 126, nHeight - 244, 82, 20);

        LabelDumpSelectedChannel->setBounds(nWidth - 192, nHeight - 219, 75, 20);
        SliderDumpSelectedChannel->setBounds(nWidth - 126, nHeight - 219, 70, 20);

        ButtonDumpPeakMeterLevel->setBounds(nWidth - 192, nHeight - 189, 180, 20);
        ButtonDumpAverageMeterLevel->setBounds(nWidth - 192, nHeight - 169, 180, 20);
        ButtonDumpMaximumPeakLevel->setBounds(nWidth - 192, nHeight - 149, 180, 20);
        ButtonDumpStereoMeterValue->setBounds(nWidth - 192, nHeight - 129, 180, 20);
        ButtonDumpPhaseCorrelation->setBounds(nWidth - 192, nHeight - 109, 180, 20);
        ButtonDumpCSV->setBounds(nWidth - 192, nHeight - 89, 180, 20);

        ButtonValidation->setBounds(nWidth - 73, nHeight - 59, 60, 20);
        ButtonCancel->setBounds(nWidth - 138, nHeight - 59, 60, 20);
    }
}


WindowValidation::~WindowValidation()
{
    delete pConstrainer;
    pConstrainer = nullptr;

    // delete all children of the window; "contentComponent" will be
    // deleted by the base class, so please leave it alone!
    contentComponent->deleteAllChildren();
}


void WindowValidation::paint(Graphics& g)
{
    int nHeight = getHeight();
    int nWidth = getWidth();

    // fill window background with grey colour gradient
    g.setGradientFill(ColourGradient(Colours::darkgrey.darker(0.4f), 0, 0, Colours::darkgrey.darker(1.0f), 0, (float) nHeight, false));
    g.fillAll();

    if (bHorizontalLayout)
    {
        g.setColour(Colours::white);
        g.setOpacity(0.15f);
        g.drawRect(nWidth - 362, nHeight - 174, 358, 146);

        g.setColour(Colours::white);
        g.setOpacity(0.05f);
        g.fillRect(nWidth - 362, nHeight - 173, 356, 144);
    }
    else
    {
        g.setColour(Colours::white);
        g.setOpacity(0.15f);
        g.drawRect(nWidth - 193, nHeight - 273, 188, 244);

        g.setColour(Colours::white);
        g.setOpacity(0.05f);
        g.fillRect(nWidth - 192, nHeight - 272, 186, 242);
    }
}


void WindowValidation::buttonClicked(Button* button)
{
    // find out which button has been clicked
    if (button == ButtonValidation)
    {
        int nSelectedChannel = (int) SliderDumpSelectedChannel->getValue();
        float fSelectedChannel = (nSelectedChannel + 1.0f) / 100.0f;
        pProcessor->setParameter(KmeterPluginParameters::selValidationSelectedChannel, fSelectedChannel);

        bool bReportCSV = ButtonDumpCSV->getToggleState();
        pProcessor->setParameter(KmeterPluginParameters::selValidationCSVFormat, bReportCSV ? 1.0f : 0.0f);

        bool bAverageMeterLevel = ButtonDumpAverageMeterLevel->getToggleState();
        pProcessor->setParameter(KmeterPluginParameters::selValidationAverageMeterLevel, bAverageMeterLevel ? 1.0f : 0.0f);

        bool bPeakMeterLevel = ButtonDumpPeakMeterLevel->getToggleState();
        pProcessor->setParameter(KmeterPluginParameters::selValidationPeakMeterLevel, bPeakMeterLevel ? 1.0f : 0.0f);

        bool bMaximumPeakLevel = ButtonDumpMaximumPeakLevel->getToggleState();
        pProcessor->setParameter(KmeterPluginParameters::selValidationMaximumPeakLevel, bMaximumPeakLevel ? 1.0f : 0.0f);

        bool bStereoMeterValue = ButtonDumpStereoMeterValue->getToggleState();
        pProcessor->setParameter(KmeterPluginParameters::selValidationStereoMeterValue, bStereoMeterValue ? 1.0f : 0.0f);

        bool bPhaseCorrelation = ButtonDumpPhaseCorrelation->getToggleState();
        pProcessor->setParameter(KmeterPluginParameters::selValidationPhaseCorrelation, bPhaseCorrelation ? 1.0f : 0.0f);

        // validation file has already been initialised
        pProcessor->startValidation(fileValidation, nSelectedChannel, bReportCSV, bAverageMeterLevel, bPeakMeterLevel, bMaximumPeakLevel, bStereoMeterValue, bPhaseCorrelation);

        // close window by making it invisible
        setVisible(false);
    }
    else if (button == ButtonCancel)
    {
        // close window by making it invisible
        setVisible(false);
    }
    else if (button == ButtonFileSelection)
    {
        WildcardFileFilter wildcardFilter("*.wav;*.aiff;*.flac", "", "Audio files (*.wav, *.aiff, *.flac)");

        FileBrowserComponent browser(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, fileValidation, &wildcardFilter, nullptr);

        FileChooserDialogBox dialogBox("Open audio file", "Please select an audio file to inject into K-Meter's audio path.", browser, true, getLookAndFeel().findColour(AlertWindow::backgroundColourId));

        if (dialogBox.show())
        {
            File selectedFile = browser.getSelectedFile(0);
            pProcessor->setParameterValidationFile(selectedFile);
            fileValidation = pProcessor->getParameterValidationFile();
            LabelFileSelection->setText(fileValidation.getFileName(), dontSendNotification);
        }
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
