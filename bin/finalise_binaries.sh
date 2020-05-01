#!/bin/bash

#  ----------------------------------------------------------------------------
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
#  ----------------------------------------------------------------------------


###############################################################################
#
#  WARNING: this file is auto-generated, please do not edit!
#
###############################################################################

binary_dir="final"


function finalise_binary
{
    filepath=$1
    filename=$(basename "$1")

    if [ -f "./$filepath" ]; then
        if [ ! -f "$binary_dir/$filename" ] || [ "./$filepath" -nt "$binary_dir/$filename" ]; then
            printf "  Finalising binary:   %s -->\n" "$filepath"
            printf "                       %s\n"     "$binary_dir/$filename"

            cp "./$filepath" "./$binary_dir/$filename"

            printf "\n"
        fi
    fi
}


function finalise_symbols
{
    filepath=$1
    filename=$(basename "$1")

    if [ -f "./$filepath" ]; then
        if [ ! -f "$binary_dir/debug_symbols/$filepath" ] || [ "./$filepath" -nt "$binary_dir/debug_symbols/$filepath" ]; then
            printf "  Finalising symbols:  %s -->\n" "$filepath"
            printf "                       %s\n"     "$binary_dir/debug_symbols/$filepath"

            mkdir -p "$(dirname "./$binary_dir/debug_symbols/$filepath")"
            cp "./$filepath" "./$binary_dir/debug_symbols/$filepath"

            printf "\n"
        fi
    fi
}


printf "\n  === Finalising binaries ===\n\n"

mkdir -p "$binary_dir/debug_symbols"

finalise_binary  "standalone/kmeter_stereo"
finalise_binary  "standalone/kmeter_surround"

finalise_binary  "vst2/kmeter_stereo_vst2.so"
finalise_binary  "vst2/kmeter_surround_vst2.so"

finalise_binary  "standalone/kmeter_stereo_x64"
finalise_binary  "standalone/kmeter_surround_x64"

finalise_binary  "vst2/kmeter_stereo_vst2_x64.so"
finalise_binary  "vst2/kmeter_surround_vst2_x64.so"

finalise_binary  "standalone/K-Meter (Stereo).exe"
finalise_symbols "standalone/K-Meter (Stereo).pdb"
finalise_binary  "standalone/K-Meter (Surround).exe"
finalise_symbols "standalone/K-Meter (Surround).pdb"

finalise_binary  "vst2/K-Meter (Stereo).dll"
finalise_symbols "vst2/K-Meter (Stereo).pdb"
finalise_binary  "vst2/K-Meter (Surround).dll"
finalise_symbols "vst2/K-Meter (Surround).pdb"

finalise_binary  "vst3/K-Meter (Stereo).vst3"
finalise_symbols "vst3/K-Meter (Stereo).pdb"
finalise_binary  "vst3/K-Meter (Surround).vst3"
finalise_symbols "vst3/K-Meter (Surround).pdb"

finalise_binary  "standalone/K-Meter (Stereo x64).exe"
finalise_symbols "standalone/K-Meter (Stereo x64).pdb"
finalise_binary  "standalone/K-Meter (Surround x64).exe"
finalise_symbols "standalone/K-Meter (Surround x64).pdb"

finalise_binary  "vst2/K-Meter (Stereo x64).dll"
finalise_symbols "vst2/K-Meter (Stereo x64).pdb"
finalise_binary  "vst2/K-Meter (Surround x64).dll"
finalise_symbols "vst2/K-Meter (Surround x64).pdb"

finalise_binary  "vst3/K-Meter (Stereo x64).vst3"
finalise_symbols "vst3/K-Meter (Stereo x64).pdb"
finalise_binary  "vst3/K-Meter (Surround x64).vst3"
finalise_symbols "vst3/K-Meter (Surround x64).pdb"

printf "  Done.\n\n\n"
