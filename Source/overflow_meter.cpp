/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2016 Martin Zuther (http://www.mzuther.de/)

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

#include "overflow_meter.h"


OverflowMeter::OverflowMeter()
{
    nOverflows = 0;

    setLabelColour(Colours::white);
    setLabelText(String(nOverflows));
}


void OverflowMeter::setOverflows(int Overflows)
{
    int nOverflowsOld = nOverflows;
    nOverflows = Overflows;

    if (nOverflows != nOverflowsOld)
    {
        setLabelText(String(nOverflows));

        if (nOverflows == 0)
        {
            setState(frut::widget::StateLabel::State::off);
        }
        else
        {
            setState(frut::widget::StateLabel::State::on);
        }
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
