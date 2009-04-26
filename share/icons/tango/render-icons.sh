#!/bin/bash
# render-icons.sh
# (c) 2009 Krzysztof Kosiński
# Licensed under GNU GPL; see the file COPYING for details
#
# This script renders PNG icons from their SVG versions.

OLD_IFS=$IFS
IFS=$'\0'

for dir_size in *; do
if test -d $dir_size && test "`echo $dir_size | sed -n '/^[1234567890]/p'`" != ""; then
	SIZE=$(echo $dir_size | sed -e 's|x.*$||')
	find scalable -mindepth 1 -type d -a -not -regex '.*\.svn.*' -printf "$dir_size/%P\n" \
	     | xargs mkdir -p
	
	case $1 in
	# use shell mode to render discrete icons
	# right now doesn't work because of Inkscape bugs
	inkscape)
	find scalable -name '*.svg' -printf "%p -w $SIZE -h $SIZE --export-png=$dir_size/%P\n" \
	     | sed -e 's|\.svg$|.png|' \
	     | inkscape --shell
	;;
	# use shell mode to render icons extracted from icons.svg
	# right now doesn't work because of Inkscape bugs
	inkscape-icons)
	find scalable -name '*.svg' \
	     -printf "-w $SIZE -h $SIZE --export-id=%f --export-id-only --export-png=$dir_size/%P\n" \
	     | sed -e 's|\.svg$|.png|' -e 's|\.svg||' -e 's|^.*$|../icons.svg \0|'\
	     | inkscape --shell
	;;
	
	# use rsvg to render discrete icons
	rsvg|*)
	find scalable -name '*.svg' -printf "%p $dir_size/%P\n" \
	     | sed -e 's|\.svg$|.png|' \
	     | xargs -n 2 rsvg -w $SIZE -h $SIZE
	;;
	esac
fi
done

IFS=$OLD_IFS
