#!/bin/bash
#
#  Inkscape compilation and packaging script for Mac OS X
#
# Please see
#  http://wiki.inkscape.org/wiki/index.php?title=CompilingMacOsX
# for more complete information
#
# Author:
#	Jean-Olivier Irisson <jo.irisson@gmail.com>
# with information from
#	Kees Cook
#	Michael Wybrow
#
# Copyright 2006
# Licensed under GNU General Public License
#

############################################################

# User modifiable parameters
#----------------------------------------------------------
#	Configure flags
CONFFLAGS="--disable-static --enable-shared --enable-osxapp"
# Libraries prefix
LIBPREFIX="/opt/local"

############################################################

# Help message
#----------------------------------------------------------
help()
{

echo -e "
Compilation script for Inkscape on Mac OS X.

\033[1mUSAGE\033[0m
  $0 [options] action[s]

\033[1mACTIONS & OPTIONS\033[0m
  \033[1mh,help\033[0m	
    display this help message
  \033[1mu,up,update\033[0m
    update an existing checkout from svn (run svn up)
  \033[1ma,auto,autogen\033[0m
    prepare configure script (run autogen.sh). This is only necessary
    for a fresh svn checkout or after make distclean.
  \033[1mc,conf,configure\033[0m
    configure the build (run configure). Edit your configuration
    options in $0
    \033[1m-p,--prefix\033[0m	specify install prefix (configure step only)
  \033[1mb,build\033[0m
    build Inkscape (run make)
  \033[1mi,install\033[0m
    install the build products locally, inside the source
    directory (run make install)
  \033[1mp,pack,package\033[0m
    package Inkscape in a double clickable .app bundle 
    \033[1m-s,--strip\033[0m	remove debugging information in Inkscape package
  \033[1md,dist,distrib\033[0m
    store Inkscape.app in a disk image (dmg) for distribution
    \033[1m-py,--with-python\033[0m	specify python packages path for inclusion into the dmg image
	
\033[1mEXAMPLES\033[0m
  \033[1m$0 conf build install\033[0m
    configure, build and install a dowloaded version of Inkscape in the default
    directory, keeping debugging information.	
  \033[1m$0 -p ~ -s -py ~/pyxml/ u a c b i p d\033[0m
    update an svn checkout, prepare configure script, configure,
    build and install Inkscape in the user home directory. 	
    Then package Inkscape withouth debugging information,
    with python packages from ~/pyxml/ and prepare it for   
    distribution."
}

# Parameters
#----------------------------------------------------------
# Paths
HERE=`pwd`
SRCROOT=$HERE/../..		# we are currently in packaging/macosx

# Defaults
if [ "$INSTALLPREFIX" = "" ]
then
	INSTALLPREFIX=$SRCROOT/Build/
fi
SVNUPDATE="f"
AUTOGEN="f"
CONFIGURE="f"
BUILD="f"
INSTALL="f"
PACKAGE="f"
DISTRIB="f"

STRIP="f"
PYTHON="f"

# Parse command line options
#----------------------------------------------------------
while [ "$1" != "" ]
do
	case $1 in
	h|help)
		help 
		exit 1 ;;
   u|up|update)
		SVNUPDATE="t" ;;
   a|auto|autogen)
		AUTOGEN="t" ;;
	c|conf|configure)
		CONFIGURE="t" ;;
	b|build)
		BUILD="t" ;;
	i|install)
		INSTALL="t" ;;
	p|pack|package)
		PACKAGE="t" ;;
	d|dist|distrib)
		DISTRIB="t" ;;
	# -p|--prefix)
	#   	INSTALLPREFIX=$2
	#   	shift 1 ;;
	-s|-strip)
	     	STRIP="t" ;;
	-py|--with-python)
		PYTHON="t" 
		PYTHONDIR="$2"
		shift 1 ;;
	esac
	shift 1
done


# Set environment variables
# ----------------------------------------------------------
export LIBPREFIX

# Specific environment variables
#  automake seach path
export CPATH="$LIBPREFIX/include"
#  configure search path
export CPPFLAGS="-I$LIBPREFIX/include"
export LDFLAGS="-L$LIBPREFIX/lib"
#  compiler arguments
export CFLAGS="-O3 -Wall"
export CXXFLAGS="$CFLAGS"
# add X11 executables and libraries [does not seem to be required now]
# export PATH="/usr/X11R6/bin:$PATH"
# export LIBRARY_PATH="/usr/X11R6/lib:$LIBPREFIX/lib"
# pkgconfig path [does not seem to be required either]
# export PKG_CONFIG_PATH="$LIBPREFIX/lib/pkgconfig"


# Actions
# ----------------------------------------------------------
if [[ "$SVNUPDATE" == "t" ]]
then
	cd $SRCROOT
	svn up
	status=$?
	if [[ $status -ne 0 ]]; then
		echo -e "\nSVN update failed"
		exit $status
	fi
	cd $HERE
fi

if [[ "$AUTOGEN" == "t" ]]
then
	cd $SRCROOT
	./autogen.sh
	status=$?
	if [[ $status -ne 0 ]]; then
		echo -e "\nautogen failed"
		exit $status
	fi
	cd $HERE
fi

if [[ "$CONFIGURE" == "t" ]]
then
	ALLCONFFLAGS=`echo "$CONFFLAGS --prefix=$INSTALLPREFIX"`
	cd $SRCROOT
	if [ ! -f configure ]
	then
		echo "Configure script not found in $SRCROOT. Run autogen.sh first"
		exit 1
	fi
	./configure $ALLCONFFLAGS
	status=$?
	if [[ $status -ne 0 ]]; then
		echo -e "\nConfigure failed"
		exit $status
	fi
	cd $HERE
fi

if [[ "$BUILD" == "t" ]]
then
	cd $SRCROOT
	make
	status=$?
	if [[ $status -ne 0 ]]; then
		echo -e "\nBuild failed"
		exit $status
	fi
	cd $HERE
fi

if [[ "$INSTALL" == "t" ]] 
then
	cd $SRCROOT
	make install
	status=$?
	if [[ $status -ne 0 ]]; then
		echo -e "\nInstall failed"
		exit $status
	fi
	cd $HERE
fi

if [[ "$PACKAGE" == "t" ]]
then
	
	# Detect strip parameter
	if [[ "$STRIP" == "t" ]]; then
		STRIPPARAM="-s"
	else
		STRIPPARAM=""
	fi
	
	# Test the existence of required files
	if [ ! -e $INSTALLPREFIX/bin/inkscape ]
	then
		echo "The inkscape executable \"$INSTALLPREFIX/bin/inkscape\" cound not be found."
		exit 1
	fi
	if [ ! -e $SRCROOT/Info.plist ]
	then
		echo "The file \"$SRCROOT/Info.plist\" could not be found, please re-run configure."
		exit 1
	fi
	
	# Create app bundle
	./osx-app.sh $STRIPPARAM $INSTALLPREFIX/bin/inkscape $SRCROOT/Info.plist
	status=$?
	if [[ $status -ne 0 ]]; then
		echo -e "\nApplication bundle creation failed"
		exit $status
	fi
fi

if [[ "$DISTRIB" == "t" ]]
then	
	# Create dmg bundle
	if [[ "$PYTHON" == "t" ]]; then
		./osx-dmg.sh -py "$PYTHONDIR"
	else
		./osx-dmg.sh
	fi
	status=$?
	if [[ $status -ne 0 ]]; then
		echo -e "\nDisk image creation failed"
		exit $status
	fi

	DATE=`date "+%Y%m%d"`
	mv Inkscape.dmg Inkscape_$DATE.dmg
	
	# Prepare information file
# 	INFOFILE=Inkscape_$DATE-info.txt
# 	echo "Version information on $DATE for `whoami`:
# 	OS X      `/usr/bin/sw_vers | grep ProductVersion | cut -f2 -d \:`
# 	DarwinPorts  `port version | cut -f2 -d \ `
# 	GCC          `gcc --version | grep GCC`
# 	GTK          `pkg-config --modversion gtk+-2.0`
# 	GTKmm        `pkg-config --modversion gtkmm-2.4`
# 	Cairo        `pkg-config --modversion cairo`
# 	Cairomm      `pkg-config --modversion cairomm-1.0`
# 	CairoPDF     `pkg-config --modversion cairo-pdf`
# 	Pango        `pkg-config --modversion pango`
# Configure options:
# 	$CONFFLAGS" > $INFOFILE
# 	if [[ "$STRIP" == "t" ]]; then
# 		echo "Debug info
# 	no" >> $INFOFILE
# 	else
# 		echo "Debug info
# 	yes" >> $INFOFILE
# 	fi
	
	# open a Finder window here
	open .
fi

exit 0
