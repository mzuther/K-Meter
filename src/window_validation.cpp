/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2011 Martin Zuther (http://www.mzuther.de/)

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


WindowValidation::WindowValidation(int nWidth, int nHeight, KmeterAudioProcessor* processor)
    : ResizableWindow(T("Validation K-Meter"), false)
    // create new window child of width "nWidth" and height "nHeight"
{
    pProcessor = processor;
    pProcessor->stopValidation();
    fileValidation = pProcessor->getParameterValidationFile();

    // set dimensions to those passed to the function ...
    setBounds(0, 0, nWidth, nHeight);
    // ... and keep the new window on top
    setAlwaysOnTop(true);

    // prohibit movement of the new window
    pConstrainer = new ProhibitingBoundsConstrainer();
    setConstrainer(pConstrainer);

    // empty windows are boring, so let's prepare a space for some
    // window components
    contentComponent = new Component(T("Window Area"));
    setContentComponent(contentComponent);

    LabelFileSelection = new Label("Label FileSelection", fileValidation.getFileName());
    LabelFileSelection->setBounds(nWidth - 188, nHeight - 209, 138, 20);
    LabelFileSelection->setMinimumHorizontalScale(1.0f);
    LabelFileSelection->setColour(Label::textColourId, Colours::white);
    LabelFileSelection->setColour(Label::backgroundColourId, Colours::grey.darker(0.6f));
    LabelFileSelection->setColour(Label::outlineColourId, Colours::black);

    // add and display the label
    contentComponent->addAndMakeVisible(LabelFileSelection);

    // create and position an "about" button that appears as if it
    // were pressed down and which closes the window when clicked
    ButtonFileSelection = new TextButton(T("..."));
    ButtonFileSelection->setBounds(nWidth - 45, nHeight - 209, 30, 20);

    // add "about" window as button listener and display the button
    ButtonFileSelection->addButtonListener(this);
    contentComponent->addAndMakeVisible(ButtonFileSelection);

    LabelDumpSelectedChannel = new Label(T("Selected channel"), T("Channel: "));
    LabelDumpSelectedChannel->setBounds(nWidth - 192, nHeight - 179, 75, 20);
    LabelDumpSelectedChannel->setColour(Label::textColourId, Colours::white);
    contentComponent->addAndMakeVisible(LabelDumpSelectedChannel);

    SliderDumpSelectedChannel = new ChannelSlider(T("Selected channel"), pProcessor->getNumChannels() - 1);
    SliderDumpSelectedChannel->setBounds(nWidth - 126, nHeight - 179, 70, 20);
    SliderDumpSelectedChannel->setColour(ChannelSlider::textBoxTextColourId, Colours::white);
    SliderDumpSelectedChannel->setColour(ChannelSlider::textBoxBackgroundColourId, Colours::grey.darker(0.6f));
    SliderDumpSelectedChannel->setColour(ChannelSlider::textBoxOutlineColourId, Colours::black);

    SliderDumpSelectedChannel->setValue(pProcessor->getParameterAsInt(KmeterPluginParameters::selValidationSelectedChannel), false, false);
    contentComponent->addAndMakeVisible(SliderDumpSelectedChannel);

    ButtonDumpPeakMeterLevel = new ToggleButton(T("Peak meter level"));
    ButtonDumpPeakMeterLevel->setBounds(nWidth - 192, nHeight - 154, 180, 20);
    ButtonDumpPeakMeterLevel->setColour(ToggleButton::textColourId, Colours::white);
    ButtonDumpPeakMeterLevel->setToggleState(pProcessor->getParameterAsBool(KmeterPluginParameters::selValidationPeakMeterLevel), false);
    contentComponent->addAndMakeVisible(ButtonDumpPeakMeterLevel);

    ButtonDumpAverageMeterLevel = new ToggleButton(T("Average meter level"));
    ButtonDumpAverageMeterLevel->setBounds(nWidth - 192, nHeight - 134, 180, 20);
    ButtonDumpAverageMeterLevel->setColour(ToggleButton::textColourId, Colours::white);
    ButtonDumpAverageMeterLevel->setToggleState(pProcessor->getParameterAsBool(KmeterPluginParameters::selValidationAverageMeterLevel), false);
    contentComponent->addAndMakeVisible(ButtonDumpAverageMeterLevel);

    ButtonDumpStereoMeterValue = new ToggleButton(T("Stereo meter value"));
    ButtonDumpStereoMeterValue->setBounds(nWidth - 192, nHeight - 114, 180, 20);
    ButtonDumpStereoMeterValue->setColour(ToggleButton::textColourId, Colours::white);
    ButtonDumpStereoMeterValue->setToggleState(pProcessor->getParameterAsBool(KmeterPluginParameters::selValidationStereoMeterValue), false);
    contentComponent->addAndMakeVisible(ButtonDumpStereoMeterValue);

    ButtonDumpPhaseCorrelation = new ToggleButton(T("Stereo phase correlation"));
    ButtonDumpPhaseCorrelation->setBounds(nWidth - 192, nHeight - 94, 180, 20);
    ButtonDumpPhaseCorrelation->setColour(ToggleButton::textColourId, Colours::white);
    ButtonDumpPhaseCorrelation->setToggleState(pProcessor->getParameterAsBool(KmeterPluginParameters::selValidationPhaseCorrelation), false);
    contentComponent->addAndMakeVisible(ButtonDumpPhaseCorrelation);

    // create and position a "validation" button which closes the
    // window and runs the selected audio file when clicked
    ButtonValidation = new TextButton(T("Validate"));
    ButtonValidation->setBounds(nWidth - 73, nHeight - 59, 60, 20);
    ButtonValidation->setColour(TextButton::textColourOffId, Colours::black);
    ButtonValidation->setColour(TextButton::buttonColourId, Colours::red);
    ButtonValidation->setColour(TextButton::buttonOnColourId, Colours::grey);

    // add "validation" window as button listener and display the
    // button
    ButtonValidation->addButtonListener(this);
    contentComponent->addAndMakeVisible(ButtonValidation);

    // create and position a "validation" button which closes the
    // window when clicked
    ButtonCancel = new TextButton(T("Cancel"));
    ButtonCancel->setBounds(nWidth - 138, nHeight - 59, 60, 20);
    ButtonCancel->setColour(TextButton::textColourOffId, Colours::black);
    ButtonCancel->setColour(TextButton::buttonColourId, Colours::yellow);
    ButtonCancel->setColour(TextButton::buttonOnColourId, Colours::grey);

    // add "validation" window as button listener and display the button
    ButtonCancel->addButtonListener(this);
    contentComponent->addAndMakeVisible(ButtonCancel);
}


WindowValidation::~WindowValidation()
{
    delete pConstrainer;
    pConstrainer = NULL;

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

    g.setColour(Colours::white);
    g.setOpacity(0.15f);
    g.drawRect(nWidth - 189, nHeight - 210, 182, 178);

    g.setColour(Colours::white);
    g.setOpacity(0.05f);
    g.fillRect(nWidth - 188, nHeight - 209, 180, 176);
}


void WindowValidation::buttonClicked(Button* button)
{
    // find out which button has been clicked
    if (button == ButtonValidation)
    {
        int nSelectedChannel = (int) SliderDumpSelectedChannel->getValue();
        float fSelectedChannel = (nSelectedChannel + 1.0f) / 100.0f;
        pProcessor->setParameter(KmeterPluginParameters::selValidationSelectedChannel, fSelectedChannel);

        bool bPeakMeterLevel = ButtonDumpPeakMeterLevel->getToggleState();
        pProcessor->setParameter(KmeterPluginParameters::selValidationPeakMeterLevel, bPeakMeterLevel ? 1.0f : 0.0f);

        bool bAverageMeterLevel = ButtonDumpAverageMeterLevel->getToggleState();
        pProcessor->setParameter(KmeterPluginParameters::selValidationAverageMeterLevel, bAverageMeterLevel ? 1.0f : 0.0f);

        bool bStereoMeterValue = ButtonDumpStereoMeterValue->getToggleState();
        pProcessor->setParameter(KmeterPluginParameters::selValidationStereoMeterValue, bStereoMeterValue ? 1.0f : 0.0f);

        bool bPhaseCorrelation = ButtonDumpPhaseCorrelation->getToggleState();
        pProcessor->setParameter(KmeterPluginParameters::selValidationPhaseCorrelation, bPhaseCorrelation ? 1.0f : 0.0f);

        // validation file has already been initialised
        pProcessor->startValidation(fileValidation, nSelectedChannel, bPeakMeterLevel, bAverageMeterLevel, bStereoMeterValue, bPhaseCorrelation);

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

        FileBrowserComponent browser(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, fileValidation, &wildcardFilter, NULL);

        FileChooserDialogBox dialogBox("Open audio file", "Please select an audio file to inject into K-Meter's audio path.", browser, true, getLookAndFeel().findColour(AlertWindow::backgroundColourId));

        if (dialogBox.show())
        {
            File selectedFile = browser.getSelectedFile(0);
            pProcessor->setParameterValidationFile(selectedFile);
            fileValidation = pProcessor->getParameterValidationFile();
            LabelFileSelection->setText(fileValidation.getFileName(), false);
        }
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
