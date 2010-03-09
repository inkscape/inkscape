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
# Copyright (C) 2006-2010
# Released under GNU GPL, read the file 'COPYING' for more information
#

############################################################

# User modifiable parameters
#----------------------------------------------------------
# Configure flags
CONFFLAGS="--enable-osxapp"
# Libraries prefix (Warning: NO trailing slash)
LIBPREFIX="/opt/local"
# User name on Modevia
MODEVIA_NAME=""

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
    update an existing checkout from bzr (run bzr pull)
  \033[1ma,auto,autogen\033[0m
    prepare configure script (run autogen.sh). This is only necessary
    for a fresh bzr checkout or after make distclean.
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
    \033[1m-py,--with-python\033[0m	specify python modules path for inclusion into the app bundle
  \033[1md,dist,distrib\033[0m
    store Inkscape.app in a disk image (dmg) for distribution
  \033[1mf,fat,universal\033[0m
    compile inkscape as a universal binary as both i386 and ppc architectures
  \033[1mput,upload\033[0m
    upload the dmg and the associate info file on Modevia server
  \033[1mall\033[0m
    do everything (update, configure, build, install, package, distribute)

\033[1mEXAMPLES\033[0m
  \033[1m$0 conf build install\033[0m
    configure, build and install a dowloaded version of Inkscape in the default
    directory, keeping debugging information.	
  \033[1m$0 u a c b -p ~ i -s -py ~/site-packages/ p d\033[0m
    update an bzr checkout, prepare configure script, configure,
    build and install Inkscape in the user home directory (~). 	
    Then package Inkscape without debugging information,
    with python packages from ~/site-packages/ and prepare 
    a dmg for distribution."
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
BZRUPDATE="f"
AUTOGEN="f"
CONFIGURE="f"
BUILD="f"
INSTALL="f"
PACKAGE="f"
DISTRIB="f"
UPLOAD="f"
UNIVERSAL="f"

STRIP=""
PYTHON_MODULES=""

# Parse command line options
#----------------------------------------------------------
while [ "$1" != "" ]
do
	case $1 in
	h|help)
		help 
		exit 1 ;;
	all)            
		BZRUPDATE="t"
		CONFIGURE="t"
		BUILD="t" 
		INSTALL="t"
		PACKAGE="t"
		DISTRIB="t" ;;
	u|up|update)
		BZRUPDATE="t" ;;
	a|auto|autogen)
		AUTOGEN="t" ;;
	c|conf|configure)
		CONFIGURE="t" ;;
	-u|--universal)
		UNIVERSAL="t" ;;
	b|build)
		BUILD="t" ;;
	i|install)
		INSTALL="t" ;;
	p|pack|package)
		PACKAGE="t" ;;
	d|dist|distrib)
		DISTRIB="t" ;;
	put|upload)
		UPLOAD="t" ;;
	-p|--prefix)
	  	INSTALLPREFIX=$2
	  	shift 1 ;;
	-s|--strip)
	     	STRIP="-s" ;;
	-py|--with-python)
		PYTHON_MODULES="$2"
		shift 1 ;;
	*)
		echo "Invalid command line option: $1" 
		exit 2 ;;
	esac
	shift 1
done

OSXMINORVER=`/usr/bin/sw_vers | grep ProductVersion | cut -d'	' -f2 | cut -f1-2 -d.`

# Set environment variables
# ----------------------------------------------------------
export LIBPREFIX

# Specific environment variables
#  automake seach path
export CPATH="$LIBPREFIX/include"
#  configure search path
export CPPFLAGS="-I$LIBPREFIX/include"
# export CPPFLAGS="-I$LIBPREFIX/include -I /System/Library/Frameworks/Carbon.framework/Versions/Current/Headers"
export LDFLAGS="-L$LIBPREFIX/lib"
#  compiler arguments
export CFLAGS="-O3 -Wall"
export CXXFLAGS="$CFLAGS"

if [[ "$UNIVERSAL" == "t" ]]
then
	MINOSXVER="$OSXMINORVER"
	
	export SDK=/Developer/SDKs/MacOSX${MINOSXVER}.sdk
	
	export CFLAGS="$CFLAGS -isysroot $SDK -arch ppc -arch i386"
	export CXXFLAGS="$CFLAGS"

	export CONFFLAGS="$CONFFLAGS --disable-dependency-tracking"
fi

# Actions
# ----------------------------------------------------------
if [[ "$BZRUPDATE" == "t" ]]
then
	cd $SRCROOT
	bzr pull
	status=$?
	if [[ $status -ne 0 ]]; then
		echo -e "\nBZR update failed"
		exit $status
	fi
	cd $HERE
fi

# Fetch some information
REVISION=`bzr version-info 2>/dev/null | grep revno | cut -d' ' -f2`
ARCH=`arch | tr [p,c] [P,C]`
OSXVERSION=`/usr/bin/sw_vers | grep ProductVersion | cut -d'	' -f2`

if [[ "$OSXMINORVER" == "10.3" ]]; then
	TARGETNAME="PANTHER"
	TARGETVERSION="10.3"
elif [[ "$OSXMINORVER" == "10.4" ]]; then
        TARGETNAME="TIGER"
	TARGETVERSION="10.4"
elif [[ "$OSXMINORVER" == "10.5" ]]; then
	TARGETNAME="LEOPARD+"
	TARGETVERSION="10.5+"
fi

TARGETARCH="$ARCH"
if [[ "$UNIVERSAL" == "t" ]]; then
	TARGETARCH="UNIVERSAL"
fi

NEWNAME="Inkscape-r$REVISION-$TARGETVERSION-$TARGETARCH"
DMGFILE="$NEWNAME.dmg"
INFOFILE="$NEWNAME-info.txt"


if [[ "$UPLOAD" == "t" ]]
then
	# If we are uploading, we are probably building nightlies and don't
	# need to build a new one if the repository hasn't changed since the
	# last.  Hence, if a dmg for this version already exists, then just
	# exit here.
	if [[ -f "$DMGFILE" ]]; then
		echo -e "\nRepository hasn't changed: $DMGFILE already exists."
		exit 0
	fi
fi

if [[ "$AUTOGEN" == "t" ]]
then
	cd $SRCROOT
	if [[ "$UNIVERSAL" == "t" ]]
	then
		# Universal builds have to be built with the option
		# --disable-dependency-tracking.  So they need to be
		# started from scratch each time.
		if [[ -f Makefile ]]; then
			make distclean
		fi
	fi
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
		echo "Configure script not found in $SRCROOT. Run '$0 autogen' first"
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
	
	# Set python command line option (if PYTHON_MODULES location is not empty, then add the python call to the command line, otherwise, stay empty)
	if [[ "$PYTHON_MODULES" != "" ]]; then
		PYTHON_MODULES="-py $PYTHON_MODULES"
		# TODO: fix this: it does not allow for spaces in the PATH under this form and cannot be quoted
	fi

	# Create app bundle
	./osx-app.sh $STRIP -b $INSTALLPREFIX/bin/inkscape -p $SRCROOT/Info.plist $PYTHON_MODULES
	status=$?
	if [[ $status -ne 0 ]]; then
		echo -e "\nApplication bundle creation failed"
		exit $status
	fi
fi

function checkversion {
	DEPVER=`pkg-config --modversion $1 2>/dev/null`
	if [[ "$?" == "1" ]]; then
		DEPVER="Not included"
	fi
	echo "$DEPVER"
}


if [[ "$DISTRIB" == "t" ]]
then
	# Create dmg bundle
	./osx-dmg.sh -p "Inkscape.app"
	status=$?
	if [[ $status -ne 0 ]]; then
		echo -e "\nDisk image creation failed"
		exit $status
	fi

	mv Inkscape.dmg $DMGFILE
	
	# Prepare information file
	echo "Build information on `date` for `whoami`:
	For OS X Ver  $TARGETNAME ($TARGETVERSION)
	Architecture  $TARGETARCH
Build system information:
	OS X Version  $OSXVERSION
	Architecture  $ARCH
	DarwinPorts   `port version | cut -f2 -d \ `
	GCC           `$CXX --version | grep GCC`
Included dependency versions:
	GTK           `checkversion gtk+-2.0`
	GTKmm         `checkversion gtkmm-2.4`
	Cairo         `checkversion cairo`
	Cairomm       `checkversion cairomm-1.0`
	CairoPDF      `checkversion cairo-pdf`
	Fontconfig    `checkversion fontconfig`
	Pango         `checkversion pango`
	LibXML2       `checkversion libxml-2.0`
	LibXSLT       `checkversion libxslt`
	LibSigC++     `checkversion sigc++-2.0`
	LibPNG        `checkversion libpng`
	GSL           `checkversion gsl`
	ImageMagick   `checkversion ImageMagick`
	Poppler       `checkversion poppler-cairo`
	LittleCMS     `checkversion lcms`
	GnomeVFS      `checkversion gnome-vfs-2.0`
	LibWPG        `checkversion libwpg-0.1`
Configure options:
	$CONFFLAGS" > $INFOFILE
	if [[ "$STRIP" == "t" ]]; then
		echo "Debug info
	no" >> $INFOFILE
	else
		echo "Debug info
	yes" >> $INFOFILE
	fi	
fi

if [[ "$UPLOAD" == "t" ]]
then
	# Provide default for user name on modevia
	if [[ "$MODEVIA_NAME" == "" ]]; then
		MODEVIA_NAME=$USER
	fi
	# Uploasd file
	scp $DMGFILE $INFOFILE "$MODEVIA_NAME"@inkscape.modevia.com:inkscape/docs/macosx-snap/
	status=$?
	if [[ $status -ne 0 ]]; then
		echo -e "\nUpload failed"
		exit $status
	fi
fi

if [[ "$PACKAGE" == "t" || "$DISTRIB" == "t" ]]; then
	# open a Finder window here to admire what we just produced
	open .
fi

exit 0
