#!/bin/bash

# ----------------------------------------------------------------------------
#
#  K-Meter
#  =======
#  Implementation of a K-System meter according to Bob Katz' specifications
#
#  Copyright (c) 2010 Martin Zuther (http://www.mzuther.de/)
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

KMETER_EXECUTABLE_DIR="final"
KMETER_RELEASE_DIR="releases"
KMETER_DOCUMENTATION_DIR="../doc"

function move_new_executable
{
	if [ -f "$1" ]; then
		mv "$1" "$KMETER_EXECUTABLE_DIR"/
	fi
}

function delete_old_archive
{
	if [ -f "$1" ]; then
		echo "  Deleting old archive \"$1\"..."
		rm "$1"
	fi
}

function create_new_archive
{
	echo "  Creating new archive \"$1\"..."
	mkdir "$2"
	echo

	cp "$KMETER_EXECUTABLE_DIR/$3" "$2"/
	cp "$KMETER_DOCUMENTATION_DIR/LICENSE" "$2"/
	cp "$KMETER_DOCUMENTATION_DIR/kmeter.pdf" "$2"/

	if [ "$4" = "bzip2" ]; then
		tar --create --bzip2 --verbose --file "$1" "$2"/* | gawk ' { print "  adding: " $1 } '
	elif [ "$4" = "zip" ]; then
		zip "$1" "$2"/*
	fi

	rm -r "$2"/

	echo
}

echo



# ----- GNU/Linux Standalone (32 bit) -----

echo "  === GNU/Linux Standalone (32 bit) ==="
echo

move_new_executable "kmeter"

delete_old_archive "$KMETER_RELEASE_DIR/linux32/kmeter-standalone.tar.bz2"

create_new_archive "$KMETER_RELEASE_DIR/linux32/kmeter-standalone.tar.bz2" "kmeter" "kmeter" "bzip2"



# ----- GNU/Linux VST (32 bit) -----

echo "  === GNU/Linux VST (32 bit) ==="
echo

move_new_executable "kmeter_vst.so"

delete_old_archive "$KMETER_RELEASE_DIR/linux32/kmeter-vst.tar.bz2"

create_new_archive "$KMETER_RELEASE_DIR/linux32/kmeter-vst.tar.bz2" "kmeter-vst" "kmeter_vst.so" "bzip2"



# ----- Windows VST (32 bit) -----

echo "  === Windows VST (32 bit) ==="
echo

move_new_executable "K-Meter.dll"

delete_old_archive "$KMETER_RELEASE_DIR/w32/kmeter-vst.zip"

create_new_archive "$KMETER_RELEASE_DIR/w32/kmeter-vst.zip" "kmeter-vst" "K-Meter.dll" "zip"
