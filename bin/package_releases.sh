#!/bin/bash

# ----------------------------------------------------------------------------
#
#  K-Meter
#  =======
#  Implementation of a K-System meter according to Bob Katz' specifications
#
#  Copyright (c) 2010-2018 Martin Zuther (http://www.mzuther.de/)
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

version="2.8.0"

executable_dir="./final"
include_dir="$executable_dir/kmeter"
release_dir="releases"


function archive_create
{
	rm -rf "/tmp/$archive_dir"

	echo "  Creating archive in \"/tmp/$archive_dir\":"
	mkdir -p "/tmp/$archive_dir"
	echo
}


function archive_add
{
	filename="$1"
	source_dir="$2"
	target_dir=$(dirname "/tmp/$archive_dir/$1")

	if [ ! -d "$target_dir" ]; then
		mkdir -p "$target_dir"
	fi

	if [ -f "$source_dir/$filename" ]; then
		echo "  [+] $filename"
		cp --dereference "$source_dir/$filename" "/tmp/$archive_dir/$1"
	elif [ -d "$source_dir/$filename" ]; then
		echo "  [+] $filename/*"
		cp --dereference --recursive "$source_dir/$filename/" "/tmp/$archive_dir/$1"
	else
		echo "  [ ] $filename  --> not added"
	fi
}


function archive_del
{
	filename="$1"

	if [ -f "/tmp/$archive_dir/$filename" ]; then
		echo "  [-] $filename"
		rm "/tmp/$archive_dir/$filename"
	elif [ -d "/tmp/$archive_dir/$filename" ]; then
		echo "  [-] $filename/*"
		rm -rf "/tmp/$archive_dir/$filename/"
	else
		echo "  [ ] $filename  --> not deleted"
	fi
}


function archive_compress
{
	archive_type=$1
	old_dir=$(pwd)

	echo
	echo "  Compressing archive..."

	cd /tmp

	if [ "$archive_type" = "bzip2" ]; then
		archive_name="$archive_dir.tar.bz2"
		rm -f "$archive_name"
		tar --create --bzip2 --verbose --file "$archive_name" "$archive_dir" > /dev/null
	elif [ "$archive_type" = "gzip" ]; then
		archive_name="$archive_dir.tar.gz"
		rm -f "$archive_name"
		tar --create --gzip --verbose --file "$archive_name" "$archive_dir" > /dev/null
	elif [ "$archive_type" = "zip" ]; then
		archive_name="$archive_dir.zip"
		rm -f "$archive_name"
		zip --recurse-paths "$archive_name" "$archive_dir" > /dev/null
	fi

	cd $old_dir
}


function archive_store
{
	archive_type=$1
	destination_dir=$2

	if [ "$archive_type" = "bzip2" ]; then
		archive_name="$archive_dir.tar.bz2"
	elif [ "$archive_type" = "gzip" ]; then
		archive_name="$archive_dir.tar.gz"
	elif [ "$archive_type" = "zip" ]; then
		archive_name="$archive_dir.zip"
	fi

	rm -rf "/tmp/$archive_dir/"

	if [ -f "$destination_dir/$archive_name" ]; then
		echo "  Overwriting \"$destination_dir/$archive_name\"..."
	else
		echo "  Storing at \"$destination_dir/$archive_name\"..."
	fi

	mv "/tmp/$archive_name" "$destination_dir/$archive_name"

	echo "  Done."
	echo
	echo
}


# ----- General -----

./finalise_executables


# ----- GNU/Linux Standalone (32 bit) -----

echo "  === GNU/Linux Standalone $version (32 bit) ==="
echo

archive_dir="kmeter-standalone_i386_$version"

archive_create

archive_add "kmeter_stereo" "$executable_dir"
archive_add "kmeter_surround" "$executable_dir"

archive_add "kmeter/doc" "$executable_dir"
archive_add "kmeter/skins/Default" "$executable_dir"
archive_add "kmeter/skins/Default.skin" "$executable_dir"

archive_compress "gzip"
archive_store "gzip" "$release_dir/linux"


# ----- GNU/Linux LV2 (32 bit) -----

echo "  === GNU/Linux LV2 $version (32 bit) ==="
echo

archive_dir="kmeter-lv2_i386_$version"
lv2_dir="./lv2/kmeter_lv2"

archive_create

archive_add "kmeter_stereo_lv2.so" "$executable_dir"
archive_add "kmeter_surround_lv2.so" "$executable_dir"

archive_add "manifest.ttl" "$lv2_dir"
archive_add "kmeter_stereo.ttl" "$lv2_dir"
archive_add "kmeter_surround.ttl" "$lv2_dir"

archive_add "kmeter/doc" "$executable_dir"
archive_add "kmeter/skins/Default" "$executable_dir"
archive_add "kmeter/skins/Default.skin" "$executable_dir"

archive_compress "gzip"
archive_store "gzip" "$release_dir/linux"


# ----- GNU/Linux VST2 (32 bit) -----

echo "  === GNU/Linux VST2 $version (32 bit) ==="
echo

archive_dir="kmeter-vst2_i386_$version"

archive_create

archive_add "kmeter_stereo_vst.so" "$executable_dir"
archive_add "kmeter_surround_vst.so" "$executable_dir"

archive_add "kmeter/doc" "$executable_dir"
archive_add "kmeter/skins/Default" "$executable_dir"
archive_add "kmeter/skins/Default.skin" "$executable_dir"

archive_compress "gzip"
archive_store "gzip" "$release_dir/linux"


# ----- GNU/Linux Standalone (64 bit) -----

echo "  === GNU/Linux Standalone $version (64 bit) ==="
echo

archive_dir="kmeter-standalone_amd64_$version"

archive_create

archive_add "kmeter_stereo_x64" "$executable_dir"
archive_add "kmeter_surround_x64" "$executable_dir"

archive_add "kmeter/doc" "$executable_dir"
archive_add "kmeter/skins/Default" "$executable_dir"
archive_add "kmeter/skins/Default.skin" "$executable_dir"

archive_compress "gzip"
archive_store "gzip" "$release_dir/linux"


# ----- GNU/Linux LV2 (64 bit) -----

echo "  === GNU/Linux LV2 $version (64 bit) ==="
echo

archive_dir="kmeter-lv2_amd64_$version"
lv2_dir="./lv2/kmeter_lv2_x64"

archive_create

archive_add "kmeter_stereo_lv2_x64.so" "$executable_dir"
archive_add "kmeter_surround_lv2_x64.so" "$executable_dir"

archive_add "manifest.ttl" "$lv2_dir"
archive_add "kmeter_stereo.ttl" "$lv2_dir"
archive_add "kmeter_surround.ttl" "$lv2_dir"

archive_add "kmeter/doc" "$executable_dir"
archive_add "kmeter/skins/Default" "$executable_dir"
archive_add "kmeter/skins/Default.skin" "$executable_dir"

archive_compress "gzip"
archive_store "gzip" "$release_dir/linux"


# ----- GNU/Linux VST2 (64 bit) -----

echo "  === GNU/Linux VST2 $version (64 bit) ==="
echo

archive_dir="kmeter-vst2_amd64_$version"

archive_create

archive_add "kmeter_stereo_vst_x64.so" "$executable_dir"
archive_add "kmeter_surround_vst_x64.so" "$executable_dir"

archive_add "kmeter/doc" "$executable_dir"
archive_add "kmeter/skins/Default" "$executable_dir"
archive_add "kmeter/skins/Default.skin" "$executable_dir"

archive_compress "gzip"
archive_store "gzip" "$release_dir/linux"


# ----- Windows Standalone (32 bit) -----

echo "  === Windows Standalone $version (32 bit) ==="
echo

archive_dir="kmeter-standalone_x86_$version"

archive_create

archive_add "K-Meter (Stereo).exe" "$executable_dir"
archive_add "K-Meter (Surround).exe" "$executable_dir"

archive_add "kmeter/doc" "$executable_dir"
archive_add "kmeter/fftw" "$executable_dir"
archive_add "kmeter/skins/Default" "$executable_dir"
archive_add "kmeter/skins/Default.skin" "$executable_dir"
archive_del "kmeter/fftw/libfftw3f-3_x64.dll"

archive_compress "zip"
archive_store "zip" "$release_dir/windows"


# ----- Windows VST2 (32 bit) -----

echo "  === Windows VST2 $version (32 bit) ==="
echo

archive_dir="kmeter-vst2_x86_$version"

archive_create

archive_add "K-Meter (Stereo).dll" "$executable_dir"
archive_add "K-Meter (Surround).dll" "$executable_dir"

archive_add "kmeter/doc" "$executable_dir"
archive_add "kmeter/fftw" "$executable_dir"
archive_add "kmeter/skins/Default" "$executable_dir"
archive_add "kmeter/skins/Default.skin" "$executable_dir"
archive_del "kmeter/fftw/libfftw3f-3_x64.dll"

archive_compress "zip"
archive_store "zip" "$release_dir/windows"


# ----- Windows VST3 (32 bit) -----

echo "  === Windows VST3 $version (32 bit) ==="
echo

archive_dir="kmeter-vst3_x86_$version"

archive_create

archive_add "K-Meter (Stereo).vst3" "$executable_dir"
archive_add "K-Meter (Surround).vst3" "$executable_dir"

archive_add "kmeter/doc" "$executable_dir"
archive_add "kmeter/fftw" "$executable_dir"
archive_add "kmeter/skins/Default" "$executable_dir"
archive_add "kmeter/skins/Default.skin" "$executable_dir"
archive_del "kmeter/fftw/libfftw3f-3_x64.dll"

archive_compress "zip"
archive_store "zip" "$release_dir/windows"


# ----- Windows Standalone (64 bit) -----

echo "  === Windows Standalone $version (64 bit) ==="
echo

archive_dir="kmeter-standalone_x64_$version"

archive_create

archive_add "K-Meter (Stereo x64).exe" "$executable_dir"
archive_add "K-Meter (Surround x64).exe" "$executable_dir"

archive_add "kmeter/doc" "$executable_dir"
archive_add "kmeter/fftw" "$executable_dir"
archive_add "kmeter/skins/Default" "$executable_dir"
archive_add "kmeter/skins/Default.skin" "$executable_dir"
archive_del "kmeter/fftw/libfftw3f-3.dll"

archive_compress "zip"
archive_store "zip" "$release_dir/windows"


# ----- Windows VST2 (64 bit) -----

echo "  === Windows VST2 $version (64 bit) ==="
echo

archive_dir="kmeter-vst2_x64_$version"

archive_create

archive_add "K-Meter (Stereo x64).dll" "$executable_dir"
archive_add "K-Meter (Surround x64).dll" "$executable_dir"

archive_add "kmeter/doc" "$executable_dir"
archive_add "kmeter/fftw" "$executable_dir"
archive_add "kmeter/skins/Default" "$executable_dir"
archive_add "kmeter/skins/Default.skin" "$executable_dir"
archive_del "kmeter/fftw/libfftw3f-3.dll"

archive_compress "zip"
archive_store "zip" "$release_dir/windows"


# ----- Windows VST3 (64 bit) -----

echo "  === Windows VST3 $version (64 bit) ==="
echo

archive_dir="kmeter-vst3_x64_$version"

archive_create

archive_add "K-Meter (Stereo x64).vst3" "$executable_dir"
archive_add "K-Meter (Surround x64).vst3" "$executable_dir"

archive_add "kmeter/doc" "$executable_dir"
archive_add "kmeter/fftw" "$executable_dir"
archive_add "kmeter/skins/Default" "$executable_dir"
archive_add "kmeter/skins/Default.skin" "$executable_dir"
archive_del "kmeter/fftw/libfftw3f-3.dll"

archive_compress "zip"
archive_store "zip" "$release_dir/windows"
