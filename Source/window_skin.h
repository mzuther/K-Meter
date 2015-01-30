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

#ifndef __WINDOW_SKIN_H__
#define __WINDOW_SKIN_H__

#include "../JuceLibraryCode/JuceHeader.h"
#include "resources/resources.h"

class SkinListBoxModel;


class WindowSkin : public DocumentWindow, ButtonListener
{
public:
    WindowSkin(Component *pEditorWindow, const File &fileSkin);
    ~WindowSkin();

    void buttonClicked(Button *button);
    const String &getSelectedString();

private:
    JUCE_LEAK_DETECTOR(WindowSkin);

    Component *contentComponent;

    ListBox *pListBox;
    SkinListBoxModel *pListBoxModel;

    TextButton *ButtonSelect;
    TextButton *ButtonDefault;

    String strSkinName;
};


class SkinListBoxModel : public ListBoxModel
{
public:
    SkinListBoxModel(const File &fileSkinDirectory);
    ~SkinListBoxModel();

    int getNumRows();
    int getRow(const String &strQuery);
    const String &getValue(int nRow);
    void setDefault(int nRow);

    void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected);

private:
    WildcardFileFilter skinWildcard;
    TimeSliceThread directoryThread;

    File fileDefaultSkin;
    String strDefaultSkin;
    StringArray strValues;
};


#endif  // __WINDOW_SKIN_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
