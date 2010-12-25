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

#ifndef __ABOUT_WINDOW_H__
#define __ABOUT_WINDOW_H__

#include "juce_library_code/juce_header.h"
#include "resources.h"


class ProhibitingBoundsConstrainer : public ComponentBoundsConstrainer
{
 public:
  // inherit from ComponentBoundsConstrainer
  ProhibitingBoundsConstrainer() : ComponentBoundsConstrainer()
  {
  }

  // do nothing till you hear from me
  ~ProhibitingBoundsConstrainer()
  {
  }

  void checkBounds(Rectangle<int> &bounds, const Rectangle<int> &previousBounds, const Rectangle<int> &limits, bool isStretchingTop, bool isStretchingLeft, bool isStretchingBottom, bool isStretchingRight)
  {
	 // prohibit window movement and resizing by simply ignoring the
	 // change
    bounds = previousBounds;
  }
};


class AboutWindow : public ResizableWindow, ButtonListener
{
public:
  AboutWindow(int nWidth, int nHeight);
  ~AboutWindow();

  void paint(Graphics& g);
  void buttonClicked(Button* button);

private:
  Component* contentComponent;
  ProhibitingBoundsConstrainer* pConstrainer;

  TextEditor* TextEditorAbout;
  TextButton* ButtonAbout;
  ImageButton* ButtonGpl;

  Image* ImageButtonGplNormal;
  Image* ImageButtonGplOver;
  Image* ImageButtonGplDown;
};


#endif  // __ABOUT_WINDOW_H__


// Local Variables:
// ispell-local-dictionary: "british"
// End:
