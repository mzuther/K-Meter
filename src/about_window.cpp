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

#include "about_window.h"

AboutWindow::AboutWindow(int nWidth, int nHeight)
  : ResizableWindow ("About K-Meter", Colours::black, false)
{
  setBounds(0, 0, nWidth, nHeight);
  setAlwaysOnTop(true);

  pConstrainer = new ProhibitingBoundsConstrainer();
  setConstrainer(pConstrainer);

  contentComponent = new Component(T("Window Area"));
  setContentComponent(contentComponent);

  Font fontHeadline(15.0f, Font::bold);
  Font fontNormal(13.0f, Font::plain);

  TextEditorAbout = new TextEditor(T("About Text"));
  TextEditorAbout->setColour(TextEditor::backgroundColourId, Colours::black.withAlpha(0.25f));
  TextEditorAbout->setColour(TextEditor::textColourId, Colours::white);
  TextEditorAbout->setColour(TextEditor::highlightColourId, Colours::white.withAlpha(0.25f));
  TextEditorAbout->setColour(TextEditor::highlightedTextColourId, Colours::white);

  TextEditorAbout->setMultiLine(true, true);
  TextEditorAbout->setReadOnly(true);

  TextEditorAbout->setFont(fontHeadline);
  TextEditorAbout->insertTextAtCaret(String(JucePlugin_Name) + T(" ") + JucePlugin_VersionString + T("\n"));

  TextEditorAbout->setFont(fontNormal);
  TextEditorAbout->insertTextAtCaret(String(JucePlugin_Desc) + T(".\n\n"));

  TextEditorAbout->setFont(fontHeadline);
  TextEditorAbout->insertTextAtCaret("Contributors\n");

  TextEditorAbout->setFont(fontNormal);
  TextEditorAbout->insertTextAtCaret(
    "Martin Zuther\n"
    "bram@smartelectronix\n\n");

#ifdef KMETER_VST_PLUGIN

  TextEditorAbout->setFont(fontHeadline);
  TextEditorAbout->insertTextAtCaret("Trademarks\n");

  TextEditorAbout->setFont(fontNormal);
  TextEditorAbout->insertTextAtCaret(
    "VST PlugIn Technology by Steinberg\n\n");

#endif

  TextEditorAbout->setFont(fontHeadline);
  TextEditorAbout->insertTextAtCaret("Copyright\n");

  TextEditorAbout->setFont(fontNormal);
  TextEditorAbout->insertTextAtCaret(
    "(c) 2010 Contributors\n\n");

  TextEditorAbout->setFont(fontHeadline);
  TextEditorAbout->insertTextAtCaret("License\n");

  TextEditorAbout->setFont(fontNormal);
  TextEditorAbout->insertTextAtCaret(
    "This program is free software: you can redistribute it and/or modify "
    "it under the terms of the GNU General Public License as published by "
    "the Free Software Foundation, either version 3 of the License, or "
    "(at your option) any later version.\n\n"

    "This program is distributed in the hope that it will be useful, "
    "but WITHOUT ANY WARRANTY; without even the implied warranty of "
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
    "GNU General Public License for more details.\n\n"

    "You should have received a copy of the GNU General Public License "
    "along with this program.  If not, see <http://www.gnu.org/licenses/>.\n\n"

    "Thank you for using free software!\n\n");

  TextEditorAbout->setBounds(0, 0, nWidth, nHeight - 47);
  TextEditorAbout->setCaretPosition(0);
  TextEditorAbout->scrollEditorToPositionCaret(0, 0);
  contentComponent->addAndMakeVisible(TextEditorAbout);

  ButtonAbout = new TextButton(T("About"));
  ButtonAbout->setBounds(nWidth - 83, nHeight - 34, 60, 20);
  ButtonAbout->setColour(TextButton::buttonColourId, Colours::yellow);
  ButtonAbout->setColour(TextButton::buttonOnColourId, Colours::grey);

  ButtonAbout->addButtonListener(this);
  contentComponent->addAndMakeVisible(ButtonAbout);

  ImageButtonGplNormal = ImageCache::getFromMemory(resources::button_gpl_normal_png, resources::button_gpl_normal_pngSize);
  ImageButtonGplOver = ImageCache::getFromMemory(resources::button_gpl_over_png, resources::button_gpl_over_pngSize);
  ImageButtonGplDown = ImageCache::getFromMemory(resources::button_gpl_down_png, resources::button_gpl_down_pngSize);

  ButtonGpl = new ImageButton(T("GPL Link"));
  ButtonGpl->setBounds(4, nHeight - 41, 64, 32);
  ButtonGpl->setImages(true, false, true,
								 ImageButtonGplNormal, 1.0f, Colour(),
								 ImageButtonGplOver, 1.0f, Colour(),
								 ImageButtonGplDown, 1.0f, Colour());

  ButtonGpl->addButtonListener(this);
  contentComponent->addAndMakeVisible(ButtonGpl);
}

AboutWindow::~AboutWindow()
{
  contentComponent->deleteAllChildren();
}

void AboutWindow::paint(Graphics& g)
{
  g.setGradientFill(ColourGradient(Colours::darkgrey.darker(0.4f), 0, 0, Colours::darkgrey.darker(1.0f), 0, (float) getHeight(), false));
  g.fillAll();
}

void AboutWindow::buttonClicked(Button* button)
{
	if (button == ButtonAbout)
	{
	  setVisible(false);
	}
	else if (button == ButtonGpl)
	{
	  URL("http://www.gnu.org/licenses/gpl-3.0.html").launchInDefaultBrowser();
	}
}
