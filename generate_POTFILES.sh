#!/bin/bash

echo "Generating updated POTFILES list..."

echo "# List of source files containing translatable strings.
# Please keep this file sorted alphabetically.
# Generated from script by mfx at" `date` "
[encoding: UTF-8]
inkscape.desktop.in" >po/POTFILES.in.new

grep -r -l -I "_(" src/ | grep -E ".(cpp|c|h)$" | sort >>po/POTFILES.in.new
find share/extensions -name "*.inx" | sort | xargs -n 1 printf "[type: gettext/xml] %s\n" >>po/POTFILES.in.new
diff po/POTFILES.in po/POTFILES.in.new -q
mv po/POTFILES.in.new po/POTFILES.in
echo "Done."
