#!/bin/bash
#
#  Inkscape compilation and packaging script for Mac OS X
#
# Please see
#  http://wiki.inkscape.org/wiki/index.php?title=CompilingMacOsX
# for more complete information
#
# Authors:
#	Jean-Olivier Irisson <jo.irisson@gmail.com>
#	Liam P. White <inkscapebrony@gmail.com>
#	~suv <suv-sf@users.sourceforge.net>
# with information from
#	Kees Cook
#	Michael Wybrow
#
# Copyright (C) 2006-2014
# Released under GNU GPL, read the file 'COPYING' for more information
#

############################################################

# User modifiable parameters
#----------------------------------------------------------
# Configure flags
CONFFLAGS="--enable-osxapp"
# Libraries prefix (Warning: NO trailing slash)
if [ -z "$LIBPREFIX" ]; then
	LIBPREFIX="/opt/local-x11"
fi

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
    \033[1m-g,--debug\033[0m	compile with debug symbols and without optimization
    \033[1m-p,--prefix\033[0m	specify install prefix (configure step only)
  \033[1mb,build\033[0m
    build Inkscape (run make)
    \033[1m-j,--jobs\033[0m   Set the number of parallel execution for make
  \033[1mi,install\033[0m
    install the build products locally, inside the source
    directory (run make install)
  \033[1mp,pack,package\033[0m
    package Inkscape in a double clickable .app bundle
    \033[1m-s,--strip\033[0m	remove debugging information in Inkscape package
    \033[1m-v,--verbose\033[0m	verbose mode
    \033[1m-py,--with-python\033[0m	specify python modules path for inclusion into the app bundle
  \033[1md,dist,distrib\033[0m
    store Inkscape.app in a disk image (dmg) for distribution
  \033[1minfo\033[0m
    create info file for current build

\033[1mEXAMPLES\033[0m
  \033[1m$0 conf build install\033[0m
    configure, build and install a dowloaded version of Inkscape in the default
    directory, keeping debugging information.
  \033[1m$0 u a c b -p ~ i -s -py ~/python_modules/ p d\033[0m
    update an bzr checkout, prepare configure script, configure,
    build and install Inkscape in the user home directory (~).
    Then package Inkscape without debugging information,
    with python packages from ~/python_modules/ and prepare
    a dmg for distribution."
}

# Parameters
#----------------------------------------------------------
# Paths
HERE="$(pwd)"
SRCROOT="$(cd ../.. && pwd)"	# we are currently in packaging/macosx

# Defaults
if [ -z "$BUILDPREFIX" ]; then
	BUILDPREFIX="$SRCROOT/build-osxapp/"
fi
if [ -z "$INSTALLPREFIX" ]; then
	INSTALLPREFIX="$SRCROOT/inst-osxapp/"
fi
BZRUPDATE="f"
AUTOGEN="f"
CONFIGURE="f"
DEBUG_BUILD="f"
BUILD="f"
NJOBS=1
INSTALL="f"
PACKAGE="f"
DISTRIB="f"
BUILD_INFO="f"

STRIP=""
VERBOSE=""
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
	-g|--debug)
		DEBUG_BUILD="t" ;;
	b|build)
		BUILD="t" ;;
	-j|--jobs)
		NJOBS=$2
		shift 1 ;;
	i|install)
		INSTALL="t" ;;
	p|pack|package)
		PACKAGE="t" ;;
	d|dist|distrib)
		DISTRIB="t" ;;
	-p|--prefix)
		INSTALLPREFIX=$2
		shift 1 ;;
	-s|--strip)
		STRIP="-s" ;;
	-py|--with-python)
		PYTHON_MODULES="$2"
		shift 1 ;;
	-v|--verbose)
		VERBOSE="-v" ;;
	info)
		BUILD_INFO="t" ;;
	*)
		echo "Invalid command line option: $1"
		exit 2 ;;
	esac
	shift 1
done

# Checks
# ----------------------------------------------------------
# OS X version
OSXVERSION="$(/usr/bin/sw_vers | grep ProductVersion | cut -f2)"
OSXMINORVER="$(cut -d. -f 1,2 <<< $OSXVERSION)"
OSXMINORNO="$(cut -d. -f2 <<< $OSXVERSION)"
OSXPOINTNO="$(cut -d. -f3 <<< $OSXVERSION)"
ARCH="$(uname -a | awk '{print $NF;}')"

# MacPorts for dependencies
[[ -x $LIBPREFIX/bin/port && -d $LIBPREFIX/etc/macports ]] && export use_port="t"

# guess default build_arch (MacPorts)
if [ "$OSXMINORNO" -ge "6" ]; then
	if [ "$(sysctl -n hw.cpu64bit_capable 2>/dev/null)" = "1" ]; then
		_build_arch="x86_64"
	else
		_build_arch="i386"
	fi
else
	if [ $ARCH = "powerpc" ]; then
		_build_arch="ppc"
	else
		_build_arch="i386"
	fi
fi

# GTK+ backend
gtk_target="$(pkg-config --variable=target gtk+-2.0 2>/dev/null)"

# Set environment variables
# ----------------------------------------------------------
export LIBPREFIX

# Specific environment variables
#  automake seach path
export CPATH="$LIBPREFIX/include"
#  configure search path
export CPPFLAGS="$CPPFLAGS -I$LIBPREFIX/include"
export LDFLAGS="$LDFLAGS -L$LIBPREFIX/lib"
#  compiler arguments
if [[ $DEBUG_BUILD == "t" ]]; then
	export CFLAGS="-g -O0"
else
	export CFLAGS="-Os"
fi

# Use system compiler and compiler flags which are known to work:
if [ "$OSXMINORNO" -le "4" ]; then
	echo "Note: Inkscape packaging requires Mac OS X 10.5 Leopard or later."
	exit 1
elif [ "$OSXMINORNO" -eq "5" ]; then
	## Apple's GCC 4.2.1 on Leopard
	TARGETNAME="LEOPARD"
	TARGETVERSION="10.5"
	export CC="/usr/bin/gcc-4.2"
	export CXX="/usr/bin/g++-4.2"
	#export CLAGS="$CFLAGS -arch $_build_arch"
	export CXXFLAGS="$CFLAGS"
	CONFFLAGS="--disable-openmp $CONFFLAGS"
elif [ "$OSXMINORNO" -eq "6" ]; then
	## Apple's LLVM-GCC 4.2.1 on Snow Leopard
	TARGETNAME="SNOW LEOPARD"
	TARGETVERSION="10.6"
	export CC="/usr/bin/llvm-gcc-4.2"
	export CXX="/usr/bin/llvm-g++-4.2"
	#export CLAGS="$CFLAGS -arch $_build_arch"
	export CXXFLAGS="$CFLAGS"
	CONFFLAGS="--disable-openmp $CONFFLAGS"
elif [ "$OSXMINORNO" -eq "7" ]; then
	## Apple's clang on Lion and later
	TARGETNAME="LION"
	TARGETVERSION="10.7"
	export CC="/usr/bin/clang"
	export CXX="/usr/bin/clang++"
	#export CLAGS="$CFLAGS -arch $_build_arch"
	export CXXFLAGS="$CFLAGS -Wno-mismatched-tags -Wno-cast-align" #-stdlib=libstdc++ -std=c++11
elif [ "$OSXMINORNO" -eq "8" ]; then
	## Apple's clang on Mountain Lion
	TARGETNAME="MOUNTAIN LION"
	TARGETVERSION="10.8"
	export CC="/usr/bin/clang"
	export CXX="/usr/bin/clang++"
	#export CLAGS="$CFLAGS -arch $_build_arch"
	export CXXFLAGS="$CFLAGS -Wno-mismatched-tags -Wno-cast-align -std=c++11 -stdlib=libstdc++"
elif [ "$OSXMINORNO" -eq "9" ]; then
	## Apple's clang on Mavericks
	TARGETNAME="MAVERICKS"
	TARGETVERSION="10.9"
	export CC="/usr/bin/clang"
	export CXX="/usr/bin/clang++"
	#export CLAGS="$CFLAGS -arch $_build_arch"
	export CXXFLAGS="$CLAGS -Wno-mismatched-tags -Wno-cast-align -std=c++11 -stdlib=libc++"
elif [ "$OSXMINORNO" -eq "10" ]; then
	## Apple's clang on Yosemite
	TARGETNAME="YOSEMITE"
	TARGETVERSION="10.10"
	export CC="/usr/bin/clang"
	export CXX="/usr/bin/clang++"
	#export CLAGS="$CFLAGS -arch $_build_arch"
	export CXXFLAGS="$CLAGS -Wno-mismatched-tags -Wno-cast-align -std=c++11 -stdlib=libc++"
	echo "Note: Detected version of OS X: $TARGETNAME $OSXVERSION"
	echo "      Inkscape packaging has not been tested on ${TARGETNAME}."
else # if [ "$OSXMINORNO" -ge "11" ]; then
	## Apple's clang after Yosemite?
	TARGETNAME="UNKNOWN"
	TARGETVERSION="10.XX"
	export CC="/usr/bin/clang"
	export CXX="/usr/bin/clang++"
	#export CLAGS="$CFLAGS -arch $_build_arch"
	export CXXFLAGS="$CLAGS -Wno-mismatched-tags -Wno-cast-align -std=c++11 -stdlib=libc++"
	echo "Note: Detected version of OS X: $TARGETNAME $OSXVERSION"
	echo "      Inkscape packaging has not been tested on this unknown version of OS X (${OSXVERSION})."
fi

# Utility functions
# ----------------------------------------------------------
getinkscapeinfo () {

	osxapp_domain="$BUILDPREFIX/Info"
	INKVERSION="$(defaults read $osxapp_domain CFBundleVersion)"
	[ $? -ne 0 ] && INKVERSION="devel"
	REVISION="$(bzr revno 2>/dev/null)"
	[ $? -ne 0 ] && REVISION="" || REVISION="-r$REVISION"
	BUILDNO=1

	TARGETARCH="$_build_arch"
	NEWNAME="Inkscape-$INKVERSION$REVISION-$BUILDNO-$gtk_target-$TARGETVERSION-$TARGETARCH"
	while [ -e "Inkscape-$INKVERSION$REVISION-$BUILDNO-$gtk_target-$TARGETVERSION-$TARGETARCH".dmg ]; do
		let BUILDNO=${BUILDNO}+1
		NEWNAME="Inkscape-$INKVERSION$REVISION-$BUILDNO-$gtk_target-$TARGETVERSION-$TARGETARCH"
	done
	DMGFILE="$NEWNAME.dmg"
	INFOFILE="$NEWNAME-info.txt"

}

checkversion () {
	DEPVER="$(pkg-config --modversion $1 2>/dev/null)"
	if [[ "$?" == "1" ]]; then
		[[ $2 ]] && DEPVER="$(checkversion-port $2)" || unset DEPVER
	fi
	if [[ ! -z "$DEPVER" ]]; then
		[[ $2 ]] && DEPVER="${DEPVER}$(checklicense-port $2)"
	else
		DEPVER="---"
	fi
	echo "$DEPVER"
}

checkversion-port () {
	if [[ "$use_port" == "t" ]]; then
		PORTVER="$(port echo $1 and active 2>/dev/null | cut -d@ -f2 | cut -d_ -f1)"
		if [ -z "$PORTVER" ]; then
			PORTVER="$(port echo ${1}-devel and active 2>/dev/null | cut -d@ -f2 | cut -d_ -f1)"
		fi
	else
		PORTVER=""
	fi
	echo "$PORTVER"
}

checklicense-port() {
	if [[ "$use_port" == "t" ]]; then
		PORTLIC="$(port info --license --line $1 2>/dev/null)"
		PORTURL="$(port info --homepage --line $1 2>/dev/null)"
		if [[ -z "$PORTLIC" ]]; then
			PORTLIC="Unknown"
		fi
		_spacer="\t\t"
		PORTLIC="$(echo -ne "${_spacer}(License: ${PORTLIC}, Homepage: ${PORTURL})")"
	else
		PORTLIC="Unknown license"
	fi
	echo "$PORTLIC"
}

checkversion-py-module () {
	# python -c "import foo; ..."
	echo "TODO."
}

buildinfofile () {
	[ -z "$INFOFILE" ] && getinkscapeinfo
	# Prepare information file
	echo "Build information on $(date) for $(whoami):
	For OS X Ver          $TARGETNAME ($TARGETVERSION)
	Architecture          $TARGETARCH
Build system information:
	OS X Version          $OSXVERSION
	Architecture          $_build_arch
	MacPorts Ver          $(port version 2>/dev/null | cut -f2 -d \ )
	Compiler              $($CXX --version | head -1)
	GTK+ backend          $gtk_target
Included dependency versions (build or runtime):
	Glib                  $(checkversion glib-2.0 glib2)
	Glibmm                $(checkversion glibmm-2.4 glibmm)
	GTK                   $(checkversion gtk+-2.0 gtk2)
	GTKmm                 $(checkversion gtkmm-2.4 gtkmm)
	GdkPixbuf             $(checkversion gdk-pixbuf-2.0 gdk-pixbuf2)
	Pixman                $(checkversion pixman-1 libpixman)
	Cairo                 $(checkversion cairo cairo)
	Cairomm               $(checkversion cairomm-1.0 cairomm)
	CairoPDF              $(checkversion cairo-pdf cairo)
	Poppler               $(checkversion poppler-cairo poppler)
	Fontconfig            $(checkversion fontconfig fontconfig)
	Freetype              $(checkversion freetype2 freetype)
	Pango                 $(checkversion pango pango)
	Pangoft2              $(checkversion pangoft2 pango)
	Harfbuzz              $(checkversion harfbuzz harfbuzz)
	LibXML2               $(checkversion libxml-2.0 libxml2)
	LibXSLT               $(checkversion libxslt libxslt)
	LibSigC++             $(checkversion sigc++-2.0 libsigcxx2)
	Boost                 $(checkversion boost boost)
	Boehm GC              $(checkversion bdw-gc boehmgc)
	GSL                   $(checkversion gsl gsl)
	LibPNG                $(checkversion libpng libpng)
	Librsvg               $(checkversion librsvg-2.0 librsvg)
	LittleCMS             $(checkversion lcms lcms)
	LittleCMS2            $(checkversion lcms2 lcms2)
	GnomeVFS              $(checkversion gnome-vfs-2.0 gnome-vfs)
	DBus                  $(checkversion dbus-1 dbus)
	Gvfs                  $(checkversion gvfs gvfs)
	ImageMagick           $(checkversion ImageMagick ImageMagick)
	Libexif               $(checkversion libexif libexif)
	JPEG                  $(checkversion jpeg jpeg)
	Icu                   $(checkversion icu-uc icu)
	LibWPD                $(checkversion libwpd-0.9 libwpd)
	LibWPG                $(checkversion libwpg-0.2 libwpg)
	Libcdr                $(checkversion libcdr-0.0 libcdr)
	Libvisio              $(checkversion libvisio-0.0 libvisio)
	Potrace               $(checkversion potrace potrace)
Included python modules:
	lxml                  $(checkversion py27-lxml py27-lxml)
	numpy                 $(checkversion py27-numpy py27-numpy)
	sk1libs               $(checkversion py27-sk1libs py27-sk1libs)
	UniConvertor          $(checkversion py27-uniconvertor py27-uniconvertor)
	Pillow                $(checkversion py27-Pillow py27-Pillow)
" > $INFOFILE

	## TODO: Pending merge adds support for:
	#LibRevenge            $(checkversion librevenge-0.0 librevenge-devel)
	#LibWPD                $(checkversion libwpd-0.10 libwpd-10.0)
	#LibWPG                $(checkversion libwpg-0.3 libwpg-0.3)
	#Libcdr                $(checkversion libcdr-0.1 libcdr-0.1)
	#Libvisio              $(checkversion libvisio-0.1 libvisio-0.1)

	## TODO: add support for gtk-mac-integration (see osxmenu branch)
	#Gtk-mac-integration   $(checkversion gtk-mac-integration gtk-osx-application)

	## TODO: how to realiably add details specific to config and build
	#if [[ ! -z "$ALLCONFFLAGS" ]]; then
	#	echo "Configure options:
	#	$ALLCONFFLAGS" >> $INFOFILE
	#fi
	#if [[ "$STRIP" == "-s" ]]; then
	#	echo "Debug info:
	#	no" >> $INFOFILE
	#else
	#	echo "Debug info:
	#	yes" >> $INFOFILE
	#fi
}

# Actions
# ----------------------------------------------------------
if [[ "$BZRUPDATE" == "t" ]]
then
	cd $SRCROOT
	if [ -z "$(bzr info | grep "checkout")" ]; then
		echo "repo is unbound (branch)" >&2
		bzr pull
	else
		echo "repo is bound (checkout)" >&2
		echo '... please update bound branch manually.' >&2
	       	false
	fi
	status=$?
	if [[ $status -ne 0 ]]; then
		echo -e "\nBZR update failed"
		exit $status
	fi
	cd $HERE
fi

if [[ "$AUTOGEN" == "t" ]]
then
	cd $SRCROOT
	export NOCONFIGURE=true && ./autogen.sh
	status=$?
	if [[ $status -ne 0 ]]; then
		echo -e "\nautogen failed"
		exit $status
	fi
	cd $HERE
fi

if [[ "$CONFIGURE" == "t" ]]
then
	ALLCONFFLAGS="$CONFFLAGS --prefix=$INSTALLPREFIX --enable-localinstall"
	cd $SRCROOT
	if [ ! -d $BUILDPREFIX ]
	then
		mkdir $BUILDPREFIX || exit 1
	fi
	cd $BUILDPREFIX
	if [ ! -f $SRCROOT/configure ]
	then
		echo "Configure script not found in $SRCROOT. Run '$0 autogen' first"
		exit 1
	fi
	$SRCROOT/configure $ALLCONFFLAGS
	status=$?
	if [[ $status -ne 0 ]]; then
		echo -e "\nConfigure failed"
		exit $status
	fi
	cd $HERE
fi

if [[ "$BUILD" == "t" ]]
then
	cd $BUILDPREFIX || exit 1
	touch "$SRCROOT/src/main.cpp" "$SRCROOT/src/ui/dialog/aboutbox.cpp"
	make -j $NJOBS
	status=$?
	if [[ $status -ne 0 ]]; then
		echo -e "\nBuild failed"
		exit $status
	fi
	cd $HERE
fi

if [[ "$INSTALL" == "t" ]]
then
	cd $BUILDPREFIX || exit 1
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
	if [ ! -e $BUILDPREFIX/Info.plist ]
	then
		echo "The file \"$BUILDPREFIX/Info.plist\" could not be found, please re-run configure."
		exit 1
	fi

	# Set python command line option (if PYTHON_MODULES location is not empty, then add the python call to the command line, otherwise, stay empty)
	if [[ "$PYTHON_MODULES" != "" ]]; then
		PYTHON_MODULES="-py $PYTHON_MODULES"
		# TODO: fix this: it does not allow for spaces in the PATH under this form and cannot be quoted
	fi

	if [[ "$DEBUG_BUILD" = "t" ]]; then
		export with_dSYM="true"
	fi

	# Create app bundle
	./osx-app.sh $STRIP $VERBOSE -b $INSTALLPREFIX/bin/inkscape -p $BUILDPREFIX/Info.plist $PYTHON_MODULES
	status=$?
	if [[ $status -ne 0 ]]; then
		echo -e "\nApplication bundle creation failed"
		exit $status
	fi
fi

if [[ "$DISTRIB" == "t" ]]
then
	getinkscapeinfo
	# Create dmg bundle
	./osx-dmg.sh -p "Inkscape.app"
	status=$?
	if [[ $status -ne 0 ]]; then
		echo -e "\nDisk image creation failed"
		exit $status
	fi

	mv Inkscape.dmg $DMGFILE

	if [[ "$DEBUG_BUILD" = "t" ]]; then
		mv "$DMGFILE" "${NEWNAME}-debug.dmg"
		ln -s "${NEWNAME}-debug.dmg" "$DMGFILE"
	fi

	# Prepare information file
	BUILD_INFO="t"
fi

if [[ "$BUILD_INFO" == "t" ]]
then
	buildinfofile
fi

if [[ "$PACKAGE" == "t" || "$DISTRIB" == "t" ]];
then
	# open a Finder window here to admire what we just produced
	open .
fi

exit 0
