#!/bin/sh
#
# USAGE
# osx-dmg [-s] -p /path/to/Inkscape.app
#
# The script creates a read-write disk image, 
# copies Inkscape in it, customizes its appearance using a 
# previously created .DS_Store file (inkscape.ds_store),
# and then compresses the disk image for distribution.
#
# AUTHORS
#	Jean-Olivier Irisson <jo.irisson@gmail.com>
#	Michael Wybrow <mjwybrow@users.sourceforge.net>
#
# Copyright (C) 2006-2007
# Released under GNU GPL, read the file 'COPYING' for more information
#
#
# How to update the disk image layout:
# ------------------------------------
#
# Modify the 'dmg_background.svg' file and generate a new 
# 'dmg_background.png' file.
#
# Update the AppleScript file 'dmg_set_style.scpt'.
#
# Run this script with the '-s' option.  It will apply the
# 'dmg_set_style.scpt' AppleScript file, and then prompt the
# user to check the window size and position before writing
# a new 'inkscape.ds_store' file to work around a bug in Finder
# and AppleScript.  The updated 'inkscape.ds_store' will need 
# to be commited to the repository when this is done.
#

# Defaults
set_ds_store=false
ds_store_file="inkscape.ds_store"
package=""
rw_name="RWinkscape.dmg"
volume_name="Inkscape"
tmp_dir="/tmp/dmg-$$"
auto_open_opt=

# Help message
#----------------------------------------------------------
help()
{
echo -e "
Create a custom dmg file to distribute Inkscape

\033[1mUSAGE\033[0m
	$0 [-s] -p /path/to/Inkscape.app

\033[1mOPTIONS\033[0m
	\033[1m-h,--help\033[0m 
		display this help message
	\033[1m-s\033[0m
		set a new apperance (do not actually creates a bundle)
	\033[1m-p,--package\033[0m
		set the path to the Inkscape.app that should be copie
		in the dmg
"
}

# Parse command line arguments
while [ "$1" != "" ]
do
	case $1 in
	  	-h|--help)
			help
			exit 0 ;;
	  	-s)
			set_ds_store=true ;;
	  	-p|--package)
			package="$2"
			shift 1 ;;
		*)
			echo "Invalid command line option" 
			exit 2 ;;
	esac
	shift 1
done

# Safety checks
if [ ! -e "$package" ]; then
	echo "Cannot find package: $package"
	exit 1
fi

echo "\n\033[1mCREATE INKSCAPE DISK IMAGE\033[0m\n"

# Create temp directory with desired contents of the release volume.
rm -rf "$tmp_dir"
mkdir "$tmp_dir"

echo "\033[1mCopying files to temp directory\033[0m"
# Inkscape itself
# copy Inkscape.app
cp -rf "$package" "$tmp_dir"/
# link to Applications in order to drag and drop inkscape onto it
ln -sf /Applications "$tmp_dir"/

# Copy a background image inside a hidden directory so the image file itself won't be shown.
mkdir "$tmp_dir/.background"
cp dmg_background.png "$tmp_dir/.background/background.png"

# If the appearance settings are not to be modified we just copy them
if [ ${set_ds_store} = "false" ]; then
	# Copy the .DS_Store file which contains information about
	# window size, appearance, etc.  Most of this can be set
	# with Apple script but involves user intervention so we
	# just keep a copy of the correct settings and use that instead.
	cp $ds_store_file "$tmp_dir/.DS_Store"
	auto_open_opt=-noautoopen
fi

# Create a new RW image from the temp directory.
echo "\033[1mCreating a temporary disk image\033[0m"
rm -f "$rw_name"
/usr/bin/hdiutil create -srcfolder "$tmp_dir" -volname "$volume_name" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW "$rw_name"

# We're finished with the temp directory, remove it.
rm -rf "$tmp_dir"

# Mount the created image.
MOUNT_DIR="/Volumes/$volume_name"
DEV_NAME=`/usr/bin/hdiutil attach -readwrite -noverify $auto_open_opt  "$rw_name" | egrep '^/dev/' | sed 1q | awk '{print $1}'`

# Have the disk image window open automatically when mounted.
bless -openfolder /Volumes/$volume_name

# In case the apperance has to be modified, mount the image and apply the base settings to it via Applescript
if [ ${set_ds_store} = "true" ]; then
	/usr/bin/osascript dmg_set_style.scpt

	open "/Volumes/$volume_name"
	# BUG: one needs to move and close the window manually for the
	# changes in appearance to be retained... 
        echo " 
        ************************************** 
        *  Please move the disk image window * 
        *    to the center of the screen     *  
        *   then close it and press enter    * 
        ************************************** 
        " 
        read -e DUMB

	# .DS_Store files aren't written till the disk is unmounted, or finder is restarted.
	hdiutil detach "$DEV_NAME"
	auto_open_opt=-noautoopen
	DEV_NAME=`/usr/bin/hdiutil attach -readwrite -noverify $auto_open_opt  "$rw_name" | egrep '^/dev/' | sed 1q | awk '{print $1}'`
	echo
	echo "New $ds_store_file file written. Re-run $0 without the -s option to use it"
	cp /Volumes/$volume_name/.DS_Store ./$ds_store_file
	SetFile -a v ./$ds_store_file

	# Unmount the disk image.
	hdiutil detach "$DEV_NAME"
	rm -f "$rw_name"

	exit 0
fi

# Unmount the disk image.
hdiutil detach "$DEV_NAME"

# Create the offical release image by compressing the RW one.
echo "\033[1mCompressing the final disk image\033[0m"
img_name="Inkscape.dmg"
# TODO make this a command line option
if [ -e "$img_name" ]; then
	echo "$img_name already exists."
	rm -i "$img_name"
fi
/usr/bin/hdiutil convert "$rw_name" -format UDZO -imagekey zlib-level=9 -o "$img_name"
rm -f "$rw_name"

exit 0
