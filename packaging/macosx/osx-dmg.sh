#!/bin/sh
#
# Inkscape packaging script for Mac OS X
#
# The script creates a read-write disk image, 
# copies Inkscape in it, customizes its appearance through
# an applescript and compresses the disk image for distribution
#
# Authors:
#	Jean-Olivier Irisson <jo.irisson@gmail.com>
#	Michael Wybrow <mjwybrow@users.sourceforge.net>
#
# Copyright 2006
# Licensed under GNU General Public License
#

RWNAME="RWinkscape.dmg"
VOLNAME="Inkscape"
FIRSTTIME="false"
TMPDIR="/tmp/dmg-$$"

# Create temp directory with desired contents of the release volume.
rm -rf "$TMPDIR"
mkdir "$TMPDIR"

# Copy Inkscape.app folder.
cp -rf ../Inkscape.app "$TMPDIR"/

# link to Applications in order to drag and drop inkscape onto it.
ln -sf /Applications "$TMPDIR"/
	
# Copy a background image inside a hidden directory so the image
# file itself won't be shown.
mkdir "$TMPDIR/.background"
cp dmg_background.png "$TMPDIR/.background/background.png"

# Copy the .DS_Store file which contains information about window size,
# appearance, etc.  Most of this can be set with Apple script but involves
# user intervention so we just keep a copy of the correct settings and
# use that instead.
cp inkscape.ds_store "$TMPDIR/.DS_Store"

# Create a new RW image from the temp directory.
rm -f "$RWNAME"
/usr/bin/hdiutil create -srcfolder "tmp-dmg" -volname "$VOLNAME" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW "$RWNAME"

# We're finished with the temp directory, remove it.
rm -rf "$TMPDIR"

# Mount the created image.
MOUNT_DIR="/Volumes/$VOLNAME"
DEV_NAME=`/usr/bin/hdiutil attach -readwrite -noverify -noautoopen "$RWNAME" | egrep '^/dev/' | sed 1q | awk '{print $1}'`

# /usr/bin/osascript dmg_set_style.scpt

# Have the disk image window open automatically when mounted.
bless -openfolder /Volumes/$VOLNAME

# Unmount the disk image.
hdiutil detach "$DEV_NAME"

# Create the offical release image by compressing the RW one.
DATE=`date "+%Y%m%d"`
/usr/bin/hdiutil convert "$RWNAME" -format UDZO -imagekey zlib-level=9 -o "../Inkscape_$DATE.dmg"
rm -f "$RWNAME"

