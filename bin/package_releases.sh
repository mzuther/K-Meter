#!/bin/bash

# ----------------------------------------------------------------------------
#
#  K-Meter
#  =======
#  Implementation of a K-System meter according to Bob Katz' specifications
#
#  Copyright (c) 2010-2015 Martin Zuther (http://www.mzuther.de/)
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

KMETER_VERSION="1.40.1"

KMETER_EXECUTABLE_DIR="final"
KMETER_RELEASE_DIR="releases"

KMETER_DOCUMENTATION_DIR="../doc"
KMETER_SKIN_DIR="../skins"

function copy_new_executable
{
	if [ -f "$1" ]; then
		echo "  Finalising file $1..."
		cp "$1" "$KMETER_EXECUTABLE_DIR"/
	fi
}

function move_new_executable
{
	if [ -f "$1" ]; then
		echo "  Finalising file $1..."
		mv "$1" "$KMETER_EXECUTABLE_DIR"/
	fi
}

function fill_archive
{
	if [ ! -d "$2" ]; then
		mkdir -p "$2"
	fi

	if [ -f "$1" ]; then
		echo "    $1"
		cp "$1" "$2"
	elif [ -d "$1" ]; then
		echo "    $1"
		cp --recursive "$1" "$2"
	fi
}

function delete_old_archive
{
	if [ -f "$1" ]; then
		echo "  Deleting old archive \"$1\"..."
		rm "$1"
	else
		echo "  Old archive \"$1\" not found."
	fi
}

function create_new_archive
{
	echo "  Creating folder \"$1\"..."
	echo "  Copying files to \"$1\"..."
	mkdir -p "$1"
	echo
}

function compress_new_archive
{
	echo
	echo "  Creating archive \"$1\"..."
	echo

	if [ "$3" = "bzip2" ]; then
		tar --create --bzip2 --verbose --file "$1" "$2"/* | gawk ' { print "    adding: " $1 } '
	elif [ "$3" = "zip" ]; then
		zip --recurse-paths "$1" "$2"/* | gawk ' { print "  " $0 } '
	fi

	echo
	echo "  Removing folder \"$2\"..."

	rm -r "$2"/

	echo "  Done."
	echo
}

echo


# ----- General -----

rm -f "$KMETER_SKIN_DIR/default_skin.ini"


# ----- GNU/Linux Standalone (32 bit) -----

KMETER_ARCHIVE_DIR="kmeter-standalone_$KMETER_VERSION"

echo "  === GNU/Linux Standalone v$KMETER_VERSION (32 bit) ==="
echo

move_new_executable "kmeter_stereo"
move_new_executable "kmeter_surround"

delete_old_archive "$KMETER_RELEASE_DIR/linux32/kmeter-standalone_$KMETER_VERSION.tar.bz2"

create_new_archive "$KMETER_ARCHIVE_DIR"

fill_archive "$KMETER_EXECUTABLE_DIR/kmeter_stereo" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_EXECUTABLE_DIR/kmeter_surround" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/LICENSE" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/kmeter.pdf" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_SKIN_DIR/" "$KMETER_ARCHIVE_DIR"

compress_new_archive "$KMETER_RELEASE_DIR/linux32/$KMETER_ARCHIVE_DIR.tar.bz2" "$KMETER_ARCHIVE_DIR" "bzip2"


# ----- GNU/Linux LV2 (32 bit) -----

KMETER_ARCHIVE_DIR="kmeter-lv2_$KMETER_VERSION"
KMETER_LV2_DIR="kmeter_lv2"

echo "  === GNU/Linux LV2 v$KMETER_VERSION (32 bit) ==="
echo

move_new_executable "$KMETER_LV2_DIR/kmeter_stereo_lv2.so"
move_new_executable "$KMETER_LV2_DIR/kmeter_surround_lv2.so"

delete_old_archive "$KMETER_RELEASE_DIR/linux32/kmeter-lv2_$KMETER_VERSION.tar.bz2"

create_new_archive "$KMETER_ARCHIVE_DIR"

fill_archive "$KMETER_EXECUTABLE_DIR/kmeter_stereo_lv2.so" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_EXECUTABLE_DIR/kmeter_surround_lv2.so" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_LV2_DIR/manifest.ttl" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_LV2_DIR/kmeter_stereo.ttl" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_LV2_DIR/kmeter_surround.ttl" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/LICENSE" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/kmeter.pdf" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_SKIN_DIR/" "$KMETER_ARCHIVE_DIR"

compress_new_archive "$KMETER_RELEASE_DIR/linux32/$KMETER_ARCHIVE_DIR.tar.bz2" "$KMETER_ARCHIVE_DIR" "bzip2"


# ----- GNU/Linux VST (32 bit) -----

KMETER_ARCHIVE_DIR="kmeter-vst_$KMETER_VERSION"

echo "  === GNU/Linux VST v$KMETER_VERSION (32 bit) ==="
echo

move_new_executable "kmeter_stereo_vst.so"
move_new_executable "kmeter_surround_vst.so"

delete_old_archive "$KMETER_RELEASE_DIR/linux32/kmeter-vst_$KMETER_VERSION.tar.bz2"

create_new_archive "$KMETER_ARCHIVE_DIR"

fill_archive "$KMETER_EXECUTABLE_DIR/kmeter_stereo_vst.so" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_EXECUTABLE_DIR/kmeter_surround_vst.so" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/LICENSE" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/kmeter.pdf" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_SKIN_DIR/" "$KMETER_ARCHIVE_DIR"

compress_new_archive "$KMETER_RELEASE_DIR/linux32/$KMETER_ARCHIVE_DIR.tar.bz2" "$KMETER_ARCHIVE_DIR" "bzip2"


# ----- GNU/Linux Standalone (64 bit) -----

KMETER_ARCHIVE_DIR="kmeter-standalone_$KMETER_VERSION"

echo "  === GNU/Linux Standalone v$KMETER_VERSION (64 bit) ==="
echo

move_new_executable "kmeter_stereo_x64"
move_new_executable "kmeter_surround_x64"

delete_old_archive "$KMETER_RELEASE_DIR/linux64/kmeter-standalone_$KMETER_VERSION.tar.bz2"

create_new_archive "$KMETER_ARCHIVE_DIR"

fill_archive "$KMETER_EXECUTABLE_DIR/kmeter_stereo_x64" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_EXECUTABLE_DIR/kmeter_surround_x64" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/LICENSE" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/kmeter.pdf" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_SKIN_DIR/" "$KMETER_ARCHIVE_DIR"

compress_new_archive "$KMETER_RELEASE_DIR/linux64/$KMETER_ARCHIVE_DIR.tar.bz2" "$KMETER_ARCHIVE_DIR" "bzip2"


# ----- GNU/Linux LV2 (64 bit) -----

KMETER_ARCHIVE_DIR="kmeter-lv2_$KMETER_VERSION"
KMETER_LV2_DIR="kmeter_lv2_x64"

echo "  === GNU/Linux LV2 v$KMETER_VERSION (64 bit) ==="
echo

move_new_executable "$KMETER_LV2_DIR/kmeter_stereo_lv2_x64.so"
move_new_executable "$KMETER_LV2_DIR/kmeter_surround_lv2_x64.so"

delete_old_archive "$KMETER_RELEASE_DIR/linux64/kmeter-lv2_$KMETER_VERSION.tar.bz2"

create_new_archive "$KMETER_ARCHIVE_DIR"

fill_archive "$KMETER_EXECUTABLE_DIR/kmeter_stereo_lv2_x64.so" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_EXECUTABLE_DIR/kmeter_surround_lv2_x64.so" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_LV2_DIR/manifest.ttl" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_LV2_DIR/kmeter_stereo.ttl" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_LV2_DIR/kmeter_surround.ttl" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/LICENSE" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/kmeter.pdf" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_SKIN_DIR/" "$KMETER_ARCHIVE_DIR"

compress_new_archive "$KMETER_RELEASE_DIR/linux64/$KMETER_ARCHIVE_DIR.tar.bz2" "$KMETER_ARCHIVE_DIR" "bzip2"


# ----- GNU/Linux VST (64 bit) -----

KMETER_ARCHIVE_DIR="kmeter-vst_$KMETER_VERSION"

echo "  === GNU/Linux VST v$KMETER_VERSION (64 bit) ==="
echo

move_new_executable "kmeter_stereo_vst_x64.so"
move_new_executable "kmeter_surround_vst_x64.so"

delete_old_archive "$KMETER_RELEASE_DIR/linux64/kmeter-vst_$KMETER_VERSION.tar.bz2"

create_new_archive "$KMETER_ARCHIVE_DIR"

fill_archive "$KMETER_EXECUTABLE_DIR/kmeter_stereo_vst_x64.so" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_EXECUTABLE_DIR/kmeter_surround_vst_x64.so" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/LICENSE" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/kmeter.pdf" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_SKIN_DIR/" "$KMETER_ARCHIVE_DIR"

compress_new_archive "$KMETER_RELEASE_DIR/linux64/$KMETER_ARCHIVE_DIR.tar.bz2" "$KMETER_ARCHIVE_DIR" "bzip2"


# ----- Windows Standalone (32 bit) -----

KMETER_ARCHIVE_DIR="kmeter-standalone_$KMETER_VERSION"

echo "  === Windows Standalone v$KMETER_VERSION (32 bit) ==="
echo

move_new_executable "K-Meter (Stereo).exe"
move_new_executable "K-Meter (Surround).exe"
copy_new_executable "libfftw3f-3.dll"

delete_old_archive "$KMETER_RELEASE_DIR/w32/kmeter-standalone_$KMETER_VERSION.zip"

create_new_archive "$KMETER_ARCHIVE_DIR"

fill_archive "$KMETER_EXECUTABLE_DIR/K-Meter (Stereo).exe" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_EXECUTABLE_DIR/K-Meter (Surround).exe" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/LICENSE" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/kmeter.pdf" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_SKIN_DIR/" "$KMETER_ARCHIVE_DIR"

fill_archive "COPYRIGHT_FFTW" "$KMETER_ARCHIVE_DIR"
fill_archive "LICENSE_FFTW" "$KMETER_ARCHIVE_DIR"
fill_archive "libfftw3f-3.dll" "$KMETER_ARCHIVE_DIR"

compress_new_archive "$KMETER_RELEASE_DIR/w32/$KMETER_ARCHIVE_DIR.zip" "$KMETER_ARCHIVE_DIR" "zip"


# ----- Windows VST (32 bit) -----

KMETER_ARCHIVE_DIR="kmeter-vst_$KMETER_VERSION"

echo "  === Windows VST v$KMETER_VERSION (32 bit) ==="
echo

move_new_executable "K-Meter (Stereo).dll"
move_new_executable "K-Meter (Surround).dll"
copy_new_executable "libfftw3f-3.dll"

delete_old_archive "$KMETER_RELEASE_DIR/w32/kmeter-vst_$KMETER_VERSION.zip"

create_new_archive "$KMETER_ARCHIVE_DIR"

fill_archive "$KMETER_EXECUTABLE_DIR/K-Meter (Stereo).dll" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_EXECUTABLE_DIR/K-Meter (Surround).dll" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/LICENSE" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/kmeter.pdf" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_SKIN_DIR/" "$KMETER_ARCHIVE_DIR"

fill_archive "COPYRIGHT_FFTW" "$KMETER_ARCHIVE_DIR"
fill_archive "LICENSE_FFTW" "$KMETER_ARCHIVE_DIR"
fill_archive "libfftw3f-3.dll" "$KMETER_ARCHIVE_DIR"

compress_new_archive "$KMETER_RELEASE_DIR/w32/$KMETER_ARCHIVE_DIR.zip" "$KMETER_ARCHIVE_DIR" "zip"


# ----- Windows Standalone (64 bit) -----

KMETER_ARCHIVE_DIR="kmeter-standalone_$KMETER_VERSION"

echo "  === Windows Standalone v$KMETER_VERSION (64 bit) ==="
echo

move_new_executable "K-Meter (Stereo x64).exe"
move_new_executable "K-Meter (Surround x64).exe"
copy_new_executable "libfftw3f-3_x64.dll"

delete_old_archive "$KMETER_RELEASE_DIR/w64/kmeter-standalone_$KMETER_VERSION.zip"

create_new_archive "$KMETER_ARCHIVE_DIR"

fill_archive "$KMETER_EXECUTABLE_DIR/K-Meter (Stereo x64).exe" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_EXECUTABLE_DIR/K-Meter (Surround x64).exe" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/LICENSE" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/kmeter.pdf" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_SKIN_DIR/" "$KMETER_ARCHIVE_DIR"

fill_archive "COPYRIGHT_FFTW" "$KMETER_ARCHIVE_DIR"
fill_archive "LICENSE_FFTW" "$KMETER_ARCHIVE_DIR"
fill_archive "libfftw3f-3_x64.dll" "$KMETER_ARCHIVE_DIR"

compress_new_archive "$KMETER_RELEASE_DIR/w64/$KMETER_ARCHIVE_DIR.zip" "$KMETER_ARCHIVE_DIR" "zip"


# ----- Windows VST (64 bit) -----

KMETER_ARCHIVE_DIR="kmeter-vst_$KMETER_VERSION"

echo "  === Windows VST v$KMETER_VERSION (64 bit) ==="
echo

move_new_executable "K-Meter (Stereo x64).dll"
move_new_executable "K-Meter (Surround x64).dll"
copy_new_executable "libfftw3f-3_x64.dll"

delete_old_archive "$KMETER_RELEASE_DIR/w64/kmeter-vst_$KMETER_VERSION.zip"

create_new_archive "$KMETER_ARCHIVE_DIR"

fill_archive "$KMETER_EXECUTABLE_DIR/K-Meter (Stereo x64).dll" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_EXECUTABLE_DIR/K-Meter (Surround x64).dll" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/LICENSE" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_DOCUMENTATION_DIR/kmeter.pdf" "$KMETER_ARCHIVE_DIR"
fill_archive "$KMETER_SKIN_DIR/" "$KMETER_ARCHIVE_DIR"

fill_archive "COPYRIGHT_FFTW" "$KMETER_ARCHIVE_DIR"
fill_archive "LICENSE_FFTW" "$KMETER_ARCHIVE_DIR"
fill_archive "libfftw3f-3_x64.dll" "$KMETER_ARCHIVE_DIR"

compress_new_archive "$KMETER_RELEASE_DIR/w64/$KMETER_ARCHIVE_DIR.zip" "$KMETER_ARCHIVE_DIR" "zip"
