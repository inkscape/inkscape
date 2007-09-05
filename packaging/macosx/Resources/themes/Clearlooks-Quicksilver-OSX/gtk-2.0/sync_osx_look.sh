#!/bin/sh
#
#	Reads defaults from Apple preferences and modifies GTK accordingly
#
#	(c) 2007 JiHO <jo.irisson@gmail.com>
#	GNU General Public License http://www.gnu.org/copyleft/gpl.html
#

# Appearance setting
aquaStyle=`defaults read "Apple Global Domain" AppleAquaColorVariant`
# 1 for aqua, 6 for graphite

# Highlight Color setting
hiliColor=`defaults read "Apple Global Domain" AppleHighlightColor`
# a RGB value, with components between 0 and 1

# Menu items color
if [[ aquaStyle -eq 1 ]]; then
	menuColor="#4a76cd"
else
	menuColor="#7c8da4"
fi
# Format highlight color as a GTK rgb value
hiliColorFormated=`echo $hiliColor | awk -F " " '{print "\\\{"$1","$2","$3"\\\}"}'`

# echo $menuColor
# echo $hiliColorFormated

# Modify the gtkrc
#	- with the correct colors
#	- to point to the correct scrollbars folder
sed 's/OSX_HILI_COLOR_PLACEHOLDER/'$hiliColorFormated'/g' pre_gtkrc | sed 's/OSX_MENU_COLOR_PLACEHOLDER/\"'$menuColor'\"/g' | sed 's/AQUASTYLE_PLACEHOLDER/'$aquaStyle'/g' > gtkrc
