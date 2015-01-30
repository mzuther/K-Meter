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

#include "state_label.h"

StateLabel::StateLabel(const String &componentName)
{
    setName(componentName);

    nSpacingLeft = 0;
    nSpacingTop = 0;
    isActivated = false;

    imageOff = Image();
    imageOn = Image();

    pLabel = new Label("Label", "");
    pLabel->setBorderSize(BorderSize<int>(0, 0, 0, 0));
    pLabel->setJustificationType(Justification::topRight);
    addAndMakeVisible(pLabel);

    pBackgroundImage = new ImageComponent("Background Image");
    addAndMakeVisible(pBackgroundImage);

    pBackgroundImage->toBack();
}


StateLabel::~StateLabel()
{
    delete pLabel;
    pLabel = nullptr;

    delete pBackgroundImage;
    pBackgroundImage = nullptr;
}


void StateLabel::resized()
{
    pLabel->setBounds(nSpacingLeft, nSpacingTop, getWidth() - 2 * nSpacingLeft, getHeight() - 2 * nSpacingTop);
    pBackgroundImage->setBounds(0, 0, getWidth(), getHeight());
}


void StateLabel::setState(bool isActivatedNew)
{
    isActivated = isActivatedNew;
    updateState();
}


void StateLabel::updateState()
{
    if (isActivated)
    {
        pBackgroundImage->setImage(imageOn);
    }
    else
    {
        pBackgroundImage->setImage(imageOff);
    }
}


void StateLabel::setImages(Image &imageOffNew, Image &imageOnNew, int nSpacingLeftNew, int nSpacingTopNew, int nFontSize)
{
    nSpacingLeft = nSpacingLeftNew;
    nSpacingTop = nSpacingTopNew;
    pLabel->setFont(Font((float) nFontSize, Font::bold));
    pLabel->setColour(Label::textColourId, Colours::white.withAlpha(0.8f));

    imageOff = Image(imageOffNew);
    imageOn = Image(imageOnNew);

    jassert(imageOff.getBounds() == imageOn.getBounds());

    updateState();
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
