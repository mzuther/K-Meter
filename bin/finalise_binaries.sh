#! /usr/bin/env bash

#  ----------------------------------------------------------------------------
#  
#  K-Meter
#  =======
#  Implementation of a K-System meter according to Bob Katz' specifications
#  
#  Copyright (c) 2010-2024 Martin Zuther (http://www.mzuther.de/)
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
#  WARNING: this file was auto-generated, please do not edit!
#
###############################################################################

binary_dir="./final"


function finalise_binary
{
    input_file=$1
    output_file="$binary_dir/$input_file"

    if [ -f "./$input_file" ]; then
        if [ ! -f "$output_file" ] || [ "./$input_file" -nt "$output_file" ]; then
            printf "  Finalising binary:   %s -->\n" "$input_file"
            printf "                       %s\n"     "$output_file"

            mkdir -p "$(dirname "./$output_file")"
            cp "./$input_file" "./$output_file"

            printf "\n"
        fi
    fi
}


function finalise_symbols
{
    input_file=$1
    output_file="$binary_dir/debug_symbols/$input_file"

    if [ -f "./$input_file" ]; then
        if [ ! -f "$output_file" ] || [ "./$input_file" -nt "$output_file" ]; then
            printf "  Finalising symbols:  %s -->\n" "$input_file"
            printf "                       %s\n"     "$output_file"

            mkdir -p "$(dirname "./$output_file")"
            cp "./$input_file" "./$output_file"

            printf "\n"
        fi
    fi
}


printf "\n  === Finalising binaries ===\n\n"

mkdir -p "$binary_dir/debug_symbols"

finalise_binary  "standalone/kmeter_x64"
finalise_binary  "standalone/kmeter_surround_x64"


finalise_binary  "vst2/kmeter_vst2_x64.so"
finalise_binary  "vst2/kmeter_vst2_surround_x64.so"

finalise_binary  "vst3/K-Meter.vst3/Contents/x86_64-linux/K-Meter.so"

finalise_binary  "standalone/K-Meter.exe"
finalise_symbols "standalone/K-Meter.pdb"
finalise_binary  "standalone/K-Meter (surround).exe"
finalise_symbols "standalone/K-Meter (surround).pdb"

finalise_binary  "vst2/K-Meter.dll"
finalise_symbols "vst2/K-Meter.pdb"
finalise_binary  "vst2/K-Meter (surround).dll"
finalise_symbols "vst2/K-Meter (surround).pdb"

finalise_binary  "vst3/K-Meter.vst3/Contents/x86-win/K-Meter.vst3"
finalise_symbols "vst3/K-Meter.vst3/Contents/x86-win/K-Meter.pdb"

finalise_binary  "standalone/K-Meter x64.exe"
finalise_symbols "standalone/K-Meter x64.pdb"
finalise_binary  "standalone/K-Meter (surround) x64.exe"
finalise_symbols "standalone/K-Meter (surround) x64.pdb"

finalise_binary  "vst2/K-Meter x64.dll"
finalise_symbols "vst2/K-Meter x64.pdb"
finalise_binary  "vst2/K-Meter (surround) x64.dll"
finalise_symbols "vst2/K-Meter (surround) x64.pdb"

finalise_binary  "vst3/K-Meter.vst3/Contents/x86_64-win/K-Meter.vst3"
finalise_symbols "vst3/K-Meter.vst3/Contents/x86_64-win/K-Meter.pdb"

printf "  Done.\n\n\n"
