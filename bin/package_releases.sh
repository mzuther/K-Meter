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

version="2.8.2"

binary_dir="./final"
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

    cd /tmp || exit

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

    cd "$old_dir" || exit
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

./finalise_binaries.sh

mkdir -p "./releases/linux/i386"
mkdir -p "./releases/linux/amd64"

mkdir -p "./releases/windows/x32"
mkdir -p "./releases/windows/x64"
mkdir -p "./releases/windows/debug_symbols"


# ----- GNU/Linux Standalone (32 bit) -----

echo "  === GNU/Linux Standalone $version (32 bit) ==="
echo

archive_dir="kmeter-linux32-standalone_$version"

archive_create

archive_add "kmeter_stereo" "$binary_dir"
archive_add "kmeter_surround" "$binary_dir"

archive_add "kmeter.pdf" "$binary_dir"
archive_add "kmeter/doc" "$binary_dir"
archive_add "kmeter/skins/Default" "$binary_dir"
archive_add "kmeter/skins/Default.skin" "$binary_dir"

archive_compress "gzip"
archive_store "gzip" "$release_dir/linux"


# ----- GNU/Linux VST2 (32 bit) -----

echo "  === GNU/Linux VST2 $version (32 bit) ==="
echo

archive_dir="kmeter-linux32-vst2_$version"

archive_create

archive_add "kmeter_stereo_vst2.so" "$binary_dir"
archive_add "kmeter_surround_vst2.so" "$binary_dir"

archive_add "kmeter.pdf" "$binary_dir"
archive_add "kmeter/doc" "$binary_dir"
archive_add "kmeter/skins/Default" "$binary_dir"
archive_add "kmeter/skins/Default.skin" "$binary_dir"

archive_compress "gzip"
archive_store "gzip" "$release_dir/linux"


# ----- GNU/Linux Standalone (64 bit) -----

echo "  === GNU/Linux Standalone $version (64 bit) ==="
echo

archive_dir="kmeter-linux64-standalone_$version"

archive_create

archive_add "kmeter_stereo_x64" "$binary_dir"
archive_add "kmeter_surround_x64" "$binary_dir"

archive_add "kmeter.pdf" "$binary_dir"
archive_add "kmeter/doc" "$binary_dir"
archive_add "kmeter/skins/Default" "$binary_dir"
archive_add "kmeter/skins/Default.skin" "$binary_dir"

archive_compress "gzip"
archive_store "gzip" "$release_dir/linux"


# ----- GNU/Linux VST2 (64 bit) -----

echo "  === GNU/Linux VST2 $version (64 bit) ==="
echo

archive_dir="kmeter-linux64-vst2_$version"

archive_create

archive_add "kmeter_stereo_vst2_x64.so" "$binary_dir"
archive_add "kmeter_surround_vst2_x64.so" "$binary_dir"

archive_add "kmeter.pdf" "$binary_dir"
archive_add "kmeter/doc" "$binary_dir"
archive_add "kmeter/skins/Default" "$binary_dir"
archive_add "kmeter/skins/Default.skin" "$binary_dir"

archive_compress "gzip"
archive_store "gzip" "$release_dir/linux"


# ----- Windows Standalone (32 bit) -----

echo "  === Windows Standalone $version (32 bit) ==="
echo

archive_dir="kmeter-w32-standalone_$version"

archive_create

archive_add "K-Meter (Stereo).exe" "$binary_dir"
archive_add "K-Meter (Surround).exe" "$binary_dir"

archive_add "kmeter.pdf" "$binary_dir"
archive_add "kmeter/doc" "$binary_dir"
archive_add "kmeter/skins/Default" "$binary_dir"
archive_add "kmeter/skins/Default.skin" "$binary_dir"
archive_add "kmeter/fftw" "$binary_dir"
archive_del "kmeter/fftw/libfftw3f-3_x64.dll"

archive_compress "zip"
archive_store "zip" "$release_dir/windows"


# ----- Windows VST2 (32 bit) -----

echo "  === Windows VST2 $version (32 bit) ==="
echo

archive_dir="kmeter-w32-vst2_$version"

archive_create

archive_add "K-Meter (Stereo).dll" "$binary_dir"
archive_add "K-Meter (Surround).dll" "$binary_dir"

archive_add "kmeter.pdf" "$binary_dir"
archive_add "kmeter/doc" "$binary_dir"
archive_add "kmeter/skins/Default" "$binary_dir"
archive_add "kmeter/skins/Default.skin" "$binary_dir"
archive_add "kmeter/fftw" "$binary_dir"
archive_del "kmeter/fftw/libfftw3f-3_x64.dll"

archive_compress "zip"
archive_store "zip" "$release_dir/windows"


# ----- Windows VST3 (32 bit) -----

echo "  === Windows VST3 $version (32 bit) ==="
echo

archive_dir="kmeter-w32-vst3_$version"

archive_create

archive_add "K-Meter (Stereo).vst3" "$binary_dir"
archive_add "K-Meter (Surround).vst3" "$binary_dir"

archive_add "kmeter.pdf" "$binary_dir"
archive_add "kmeter/doc" "$binary_dir"
archive_add "kmeter/skins/Default" "$binary_dir"
archive_add "kmeter/skins/Default.skin" "$binary_dir"
archive_add "kmeter/fftw" "$binary_dir"
archive_del "kmeter/fftw/libfftw3f-3_x64.dll"

archive_compress "zip"
archive_store "zip" "$release_dir/windows"


# ----- Windows Standalone (64 bit) -----

echo "  === Windows Standalone $version (64 bit) ==="
echo

archive_dir="kmeter-w64-standalone_$version"

archive_create

archive_add "K-Meter (Stereo x64).exe" "$binary_dir"
archive_add "K-Meter (Surround x64).exe" "$binary_dir"

archive_add "kmeter.pdf" "$binary_dir"
archive_add "kmeter/doc" "$binary_dir"
archive_add "kmeter/skins/Default" "$binary_dir"
archive_add "kmeter/skins/Default.skin" "$binary_dir"
archive_add "kmeter/fftw" "$binary_dir"
archive_del "kmeter/fftw/libfftw3f-3.dll"

archive_compress "zip"
archive_store "zip" "$release_dir/windows"


# ----- Windows VST2 (64 bit) -----

echo "  === Windows VST2 $version (64 bit) ==="
echo

archive_dir="kmeter-w64-vst2_$version"

archive_create

archive_add "K-Meter (Stereo x64).dll" "$binary_dir"
archive_add "K-Meter (Surround x64).dll" "$binary_dir"

archive_add "kmeter.pdf" "$binary_dir"
archive_add "kmeter/doc" "$binary_dir"
archive_add "kmeter/skins/Default" "$binary_dir"
archive_add "kmeter/skins/Default.skin" "$binary_dir"
archive_add "kmeter/fftw" "$binary_dir"
archive_del "kmeter/fftw/libfftw3f-3.dll"

archive_compress "zip"
archive_store "zip" "$release_dir/windows"


# ----- Windows VST3 (64 bit) -----

echo "  === Windows VST3 $version (64 bit) ==="
echo

archive_dir="kmeter-w64-vst3_$version"

archive_create

archive_add "K-Meter (Stereo x64).vst3" "$binary_dir"
archive_add "K-Meter (Surround x64).vst3" "$binary_dir"

archive_add "kmeter.pdf" "$binary_dir"
archive_add "kmeter/doc" "$binary_dir"
archive_add "kmeter/skins/Default" "$binary_dir"
archive_add "kmeter/skins/Default.skin" "$binary_dir"
archive_add "kmeter/fftw" "$binary_dir"
archive_del "kmeter/fftw/libfftw3f-3.dll"

archive_compress "zip"
archive_store "zip" "$release_dir/windows"


# ----- Windows debug symbols -----

echo "  === Windows debug symbols ==="
echo

archive_dir="debug-symbols_$version"

archive_create

archive_add "standalone" "$binary_dir/debug_symbols"
archive_add "vst2" "$binary_dir/debug_symbols"
archive_add "vst3" "$binary_dir/debug_symbols"

archive_compress "zip"
archive_store "zip" "$release_dir/windows"
