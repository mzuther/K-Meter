#!/bin/bash
SAXON_DIR="$HOME/Dokumente/xml/common/saxon"

echo
echo "  Processing \"$1.xml\"..."
java -jar "$SAXON_DIR/saxon9he.jar" -a:on -dtd:on -o:"$1.html" "$1.xml"
echo "  Done."
echo
