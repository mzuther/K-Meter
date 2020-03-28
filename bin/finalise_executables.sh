#!/bin/bash

# ----------------------------------------------------------------------------
#
#  K-Meter
#  =======
#  Implementation of a K-System meter according to Bob Katz' specifications
#
#  Copyright (c) 2010-2020 Martin Zuther (http://www.mzuther.de/)
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#  Thank you for using free software!
#
# ----------------------------------------------------------------------------

executable_dir="final"


function finalise_executable
{
	filepath=$1
	filename=`basename "$1"`

	if [ -f "./$filepath" ]; then
		echo "  Finalising:  $filepath -->"
		echo "               $executable_dir/$filename"

		mv "./$filepath" "./$executable_dir/$filename"

        echo
	fi
}


function finalise_symbols
{
	filepath=$1
	filename=`basename "$1"`

	if [ -f "./$filepath" ]; then
		echo "  Finalising:  $filepath -->"
		echo "               $executable_dir/debug_symbols/$filepath"

        mkdir -p `dirname "./$executable_dir/debug_symbols/$filepath"`
		mv "./$filepath" "./$executable_dir/debug_symbols/$filepath"

        echo
	fi
}


echo
echo "  === Finalising executables ==="
echo

finalise_executable "standalone/kmeter_stereo"
finalise_executable "standalone/kmeter_surround"

finalise_executable "lv2/kmeter_stereo_lv2.so"
finalise_executable "lv2/kmeter_surround_lv2.so"

finalise_executable "vst/kmeter_stereo_vst.so"
finalise_executable "vst/kmeter_surround_vst.so"

finalise_executable "standalone/kmeter_stereo_x64"
finalise_executable "standalone/kmeter_surround_x64"

finalise_executable "lv2/kmeter_stereo_lv2_x64.so"
finalise_executable "lv2/kmeter_surround_lv2_x64.so"

finalise_executable "vst/kmeter_stereo_vst_x64.so"
finalise_executable "vst/kmeter_surround_vst_x64.so"

finalise_executable "standalone/K-Meter (Stereo).exe"
finalise_symbols    "standalone/K-Meter (Stereo).pdb"
finalise_executable "standalone/K-Meter (Surround).exe"
finalise_symbols    "standalone/K-Meter (Surround).pdb"

finalise_executable "vst/K-Meter (Stereo).dll"
finalise_symbols    "vst/K-Meter (Stereo).pdb"
finalise_executable "vst/K-Meter (Surround).dll"
finalise_symbols    "vst/K-Meter (Surround).pdb"

finalise_executable "vst3/K-Meter (Stereo).vst3"
finalise_symbols    "vst3/K-Meter (Stereo).pdb"
finalise_executable "vst3/K-Meter (Surround).vst3"
finalise_symbols    "vst3/K-Meter (Surround).pdb"

finalise_executable "standalone/K-Meter (Stereo x64).exe"
finalise_symbols    "standalone/K-Meter (Stereo x64).pdb"
finalise_executable "standalone/K-Meter (Surround x64).exe"
finalise_symbols    "standalone/K-Meter (Surround x64).pdb"

finalise_executable "vst/K-Meter (Stereo x64).dll"
finalise_symbols    "vst/K-Meter (Stereo x64).pdb"
finalise_executable "vst/K-Meter (Surround x64).dll"
finalise_symbols    "vst/K-Meter (Surround x64).pdb"

finalise_executable "vst3/K-Meter (Stereo x64).vst3"
finalise_symbols    "vst3/K-Meter (Stereo x64).pdb"
finalise_executable "vst3/K-Meter (Surround x64).vst3"
finalise_symbols    "vst3/K-Meter (Surround x64).pdb"

echo "  Done."
echo
echo
