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

#include "window_about.h"


WindowAbout::WindowAbout(int nWidth, int nHeight)
    : ResizableWindow(T("About K-Meter"), false)
    // create new window child of width "nWidth" and height "nHeight"
{
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

    // create a word-wrapping read-only text editor component with
    // multiple lines for displaying information about the plug-in
    TextEditorAbout = new TextEditor(T("About Text"));
    TextEditorAbout->setMultiLine(true, true);
    TextEditorAbout->setReadOnly(true);

    // initialise colours of the text editor component
    TextEditorAbout->setColour(TextEditor::backgroundColourId, Colours::black.withAlpha(0.25f));
    TextEditorAbout->setColour(TextEditor::textColourId, Colours::white);
    TextEditorAbout->setColour(TextEditor::highlightColourId, Colours::white.withAlpha(0.25f));
    TextEditorAbout->setColour(TextEditor::highlightedTextColourId, Colours::white);

    // set up two fonts, one for headlines and one for regular text
    Font fontHeadline(15.0f, Font::bold);
    Font fontRegular(13.0f, Font::plain);

    // display plug-in name and version number
    TextEditorAbout->setFont(fontHeadline);
    TextEditorAbout->insertTextAtCaret(
        String("K-Meter ") + JucePlugin_VersionString + T("\n")
    );

    // display plug-in description
    TextEditorAbout->setFont(fontRegular);
    TextEditorAbout->insertTextAtCaret(
        String(JucePlugin_Desc) + T(".\n\n")
    );

    // display the contributors
    TextEditorAbout->setFont(fontHeadline);
    TextEditorAbout->insertTextAtCaret(
        T("Contributors\n")
    );

    TextEditorAbout->setFont(fontRegular);
    TextEditorAbout->insertTextAtCaret(
        L"Martin Zuther\n"
        L"Bob Katz\n"
        L"bram@smartelectronix\n\n"
    );

    // display copyright notice
    TextEditorAbout->setFont(fontHeadline);
    TextEditorAbout->insertTextAtCaret(
        T("Copyright\n")
    );

    TextEditorAbout->setFont(fontRegular);
    TextEditorAbout->insertTextAtCaret(
        T("(c) 2010-2011 Contributors\n\n")
    );

    // display used libraries
    TextEditorAbout->setFont(fontHeadline);
    TextEditorAbout->insertTextAtCaret(
        T("Libraries\n")
    );

    TextEditorAbout->setFont(fontRegular);
    TextEditorAbout->insertTextAtCaret(
#ifdef LINUX
        L"ALSA\n"
#endif
        L"FFTW\n"
#ifdef LINUX
        L"FreeType\n"
#endif
        L"JUCE\n"
#ifdef LINUX
        L"POSIX Threads\n"
        L"Xlib\n"
        L"Xext\n"
#endif
        L"\n"
    );

#if (JUCE_USE_VSTSDK_2_4 != 0)

    // display trademarks (but only when necessary)
    TextEditorAbout->setFont(fontHeadline);
    TextEditorAbout->insertTextAtCaret(
        T("Trademarks\n")
    );

    TextEditorAbout->setFont(fontRegular);
    TextEditorAbout->insertTextAtCaret(
        T("VST PlugIn Technology by Steinberg\n\n")
    );

#endif

#if (JUCE_ASIO != 0)

    // display trademarks (but only when necessary)
    TextEditorAbout->setFont(fontHeadline);
    TextEditorAbout->insertTextAtCaret(
        T("Trademarks\n")
    );

    TextEditorAbout->setFont(fontRegular);
    TextEditorAbout->insertTextAtCaret(
        T("ASIO Technology by Steinberg Media Technologies GmbH\n\n")
    );

#endif

    // display license summary
    TextEditorAbout->setFont(fontHeadline);
    TextEditorAbout->insertTextAtCaret(
        T("License\n")
    );

    TextEditorAbout->setFont(fontRegular);
    TextEditorAbout->insertTextAtCaret(
        L"This program is free software: you can redistribute it and/or modify "
        L"it under the terms of the GNU General Public License as published by "
        L"the Free Software Foundation, either version 3 of the License, or "
        L"(at your option) any later version.\n\n"

        L"This program is distributed in the hope that it will be useful, "
        L"but WITHOUT ANY WARRANTY; without even the implied warranty of "
        L"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
        L"GNU General Public License for more details.\n\n"

        L"You should have received a copy of the GNU General Public License "
        L"along with this program.  If not, see <http://www.gnu.org/licenses/>.\n\n"

        L"Thank you for using free software!\n\n"
    );

    // in case the text has become too long to fit into the text
    // editor component, scroll to the beginning
    TextEditorAbout->setCaretPosition(0);
    TextEditorAbout->scrollEditorToPositionCaret(0, 0);

    // finally, position and display the text editor component
    TextEditorAbout->setBounds(0, 0, nWidth, nHeight - 47);
    contentComponent->addAndMakeVisible(TextEditorAbout);

    // create and position an "about" button that appears as if it
    // were pressed down and which closes the window when clicked
    ButtonAbout = new TextButton(T("About"));
    ButtonAbout->setBounds(nWidth - 73, nHeight - 34, 60, 20);
    ButtonAbout->setColour(TextButton::buttonColourId, Colours::yellow);
    ButtonAbout->setColour(TextButton::buttonOnColourId, Colours::grey);

    // add "about" window as button listener and display the button
    ButtonAbout->addButtonListener(this);
    contentComponent->addAndMakeVisible(ButtonAbout);

    // prepare an image button by loading its images from memory
    ImageButtonGplNormal = ImageCache::getFromMemory(resources::button_gpl_normal_png, resources::button_gpl_normal_pngSize);
    ImageButtonGplOver = ImageCache::getFromMemory(resources::button_gpl_over_png, resources::button_gpl_over_pngSize);
    ImageButtonGplDown = ImageCache::getFromMemory(resources::button_gpl_down_png, resources::button_gpl_down_pngSize);

    // create and position the image button which opens the license
    // text in a web browser
    ButtonGpl = new ImageButton(T("GPL Link"));
    ButtonGpl->setBounds(4, nHeight - 41, 64, 32);
    ButtonGpl->setImages(true, false, true,
                         ImageButtonGplNormal, 1.0f, Colour(),
                         ImageButtonGplOver, 1.0f, Colour(),
                         ImageButtonGplDown, 1.0f, Colour()
                        );

    // add "about" window as button listener and display the button
    ButtonGpl->addButtonListener(this);
    contentComponent->addAndMakeVisible(ButtonGpl);
}


WindowAbout::~WindowAbout()
{
    delete pConstrainer;
    pConstrainer = NULL;

    // delete all children of the window; "contentComponent" will be
    // deleted by the base class, so please leave it alone!
    contentComponent->deleteAllChildren();
}


void WindowAbout::paint(Graphics& g)
{
    // fill window background with grey colour gradient
    g.setGradientFill(ColourGradient(Colours::darkgrey.darker(0.4f), 0, 0, Colours::darkgrey.darker(1.0f), 0, (float) getHeight(), false));
    g.fillAll();
}


void WindowAbout::buttonClicked(Button* button)
{
    // find out which button has been clicked
    if (button == ButtonAbout)
    {
        // close window by making it invisible
        setVisible(false);
    }
    else if (button == ButtonGpl)
    {
        // open license text in default web browser
        URL(T("http://www.gnu.org/licenses/gpl-3.0.html")).launchInDefaultBrowser();
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
