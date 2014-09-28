/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2014 Martin Zuther (http://www.mzuther.de/)

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

#include "window_skin.h"


WindowSkin::WindowSkin(Component* pEditorWindow, File& fileSkin)
    : DocumentWindow("Select skin", Colours::white, 0, true)
    // create new window child
{
    int nWidth = 150;
    int nHeight = 0;

    // empty windows are boring, so let's prepare a space for some
    // window components
    contentComponent = new Component("Window Area");
    setContentOwned(contentComponent, false);

    // prepare a list box and its model
    pListBox = new ListBox("ListBox Skins", nullptr);
    pListBoxModel = new SkinListBoxModel(fileSkin);
    pListBox->setModel(pListBoxModel);

    // calculate and set necessary height for list box
    int nHeightListBox = pListBoxModel->getNumRows() * pListBox->getRowHeight() + 2;
    pListBox->setBounds(10, 10, nWidth - 20, nHeightListBox);

    // set look of list box
    pListBox->setColour(ListBox::outlineColourId, Colours::grey);
    pListBox->setOutlineThickness(1);

    // disable multiple selections
    pListBox->setMultipleSelectionEnabled(false);

    // select current skin in list box
    strSkinName = fileSkin.getFileNameWithoutExtension();
    pListBox->selectRow(pListBoxModel->getRow(strSkinName));

    // display list box
    contentComponent->addAndMakeVisible(pListBox);

    // calculate window height from height of list box
    nHeight = nHeightListBox + 50;

    // create and position an "select" button
    ButtonSelect = new TextButton("Select");
    ButtonSelect->setBounds(10, nHeight - 30, 60, 20);
    ButtonSelect->setColour(TextButton::buttonColourId, Colours::yellow);
    ButtonSelect->setColour(TextButton::buttonOnColourId, Colours::yellow);

    // add "skin" window as button listener and display the button
    ButtonSelect->addListener(this);
    contentComponent->addAndMakeVisible(ButtonSelect);

    // set window dimensions to those passed to the function ...
    setSize(nWidth, nHeight + getTitleBarHeight());

    // ... center window on editor ...
    centreAroundComponent(pEditorWindow, getWidth(), getHeight());

    // ... and keep the new window on top
    setAlwaysOnTop(true);

    // this window does not have any transparent areas (increases
    // performance on redrawing)
    setOpaque(true);

    // finally, display window
    setVisible(true);
}


WindowSkin::~WindowSkin()
{
    delete pListBoxModel;
    pListBoxModel = nullptr;

    // delete all children of the window; "contentComponent" will be
    // deleted by the base class, so please leave it alone!
    contentComponent->deleteAllChildren();
}


void WindowSkin::buttonClicked(Button* button)
{
    // find out which button has been clicked
    if (button == ButtonSelect)
    {
        // close window by making it invisible
        setVisible(false);
    }
}


const String& WindowSkin::getSelectedString()
{
    // get selected row from list box
    int nSelectedRow = pListBox->getSelectedRow(0);

    // get and return selected value
    return pListBoxModel->getValue(nSelectedRow);
}


SkinListBoxModel::SkinListBoxModel(File& fileSkin)
    : skinWildcard("*.skin", String::empty, "skin files"),
      directoryThread("skin directory scanner")
{
    // set up scanner for skin directory
    DirectoryContentsList skinFiles(&skinWildcard, directoryThread);
    skinFiles.setDirectory(fileSkin.getParentDirectory(), false, true);
    directoryThread.startThread();

    // wait for scan of directory
    while (skinFiles.isStillLoading())
    {
    }

    // add found skins to list box
    for (int n = 0; n < skinFiles.getNumFiles(); n++)
    {
        File foundSkin = skinFiles.getFile(n);
        strValues.add(foundSkin.getFileNameWithoutExtension());
    }
}


SkinListBoxModel::~SkinListBoxModel()
{
}


int SkinListBoxModel::getNumRows()
{
    return strValues.size();
}


int SkinListBoxModel::getRow(const String& strQuery)
{
    return strValues.indexOf(strQuery);
}


const String& SkinListBoxModel::getValue(int nRow)
{
    return strValues[nRow];
}


void SkinListBoxModel::paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
    // draw selected rows in light blue
    if (rowIsSelected)
    {
        g.fillAll(Colours::lightblue);
    }

    // render row text
    g.setFont(14.0f);
    g.setColour(Colours::black);

    g.drawText(getValue(rowNumber), 2, 0, width - 4, height, Justification::centredLeft, true);
}

// Local Variables:
// ispell-local-dictionary: "british"
// End:
