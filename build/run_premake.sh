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

function correct_premake4_bug_3015312
{
	PREMAKE_TMP=$(mktemp)

	if [ ! -w "$1" ]; then
		echo "[ERR] File $1 not found."
		return
	fi

	echo "Correcting $1..."
	gawk '{ print gensub(/\$\(SILENT\) \$\(CXX\) \$\(CXXFLAGS\) -o \$@ -c \$</, "$(SILENT) $(CXX) $(CXXFLAGS) -o \"$@\" -c \"$<\"", "G" ); }' "$1" > "$PREMAKE_TMP"
	mv "$PREMAKE_TMP" "$1"
}

cd $(dirname $0)

echo
premake4 --cc=gcc --os=windows gmake

echo
premake4 --cc=gcc --os=linux gmake

echo "Done."
echo
