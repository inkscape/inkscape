#!/bin/bash
#
# USAGE
# osx-app [-s] [-l /path/to/libraries] -py /path/to/python/modules [-l /path/to/libraries] -b /path/to/bin/inkscape -p /path/to/Info.plist
#
# This script attempts to build an Inkscape.app package for OS X, resolving
# dynamic libraries, etc.
# 
# If the '-s' option is given, then the libraries and executable are stripped.
# 
# The Info.plist file can be found in the base inkscape directory once
# configure has been run.
#
# AUTHORS
#		 Kees Cook <kees@outflux.net>
#		 Michael Wybrow <mjwybrow@users.sourceforge.net>
#		 Jean-Olivier Irisson <jo.irisson@gmail.com>
#		 Liam P. White <inkscapebrony@gmail.com>
#		 ~suv <suv-sf@users.sourceforge.net>
# 
# Copyright (C) 2005 Kees Cook
# Copyright (C) 2005-2009 Michael Wybrow
# Copyright (C) 2007-2009 Jean-Olivier Irisson
# Copyright (C) 2014 Liam P. White
# Copyright (C) 2014 ~suv
#
# Released under GNU GPL, read the file 'COPYING' for more information
#
# Thanks to GNUnet's "build_app" script for help with library dep resolution.
#		https://gnunet.org/svn/GNUnet/contrib/OSX/build_app
# 
# NB:
# When packaging Inkscape for OS X, configure should be run with the
# "--enable-osxapp" option which sets the correct paths for support
# files inside the app bundle.
# 

# Defaults
strip_build=false
add_wrapper=true
add_python=true
python_dir=""

# If LIBPREFIX is not already set (by osx-build.sh for example) set it to blank
# (one should use the command line argument to set it correctly)
if [ -z $LIBPREFIX ]; then
	LIBPREFIX=""
fi


# Help message
#----------------------------------------------------------
help()
{
echo -e "
Create an app bundle for OS X

\033[1mUSAGE\033[0m
	$0 [-s] [-l /path/to/libraries] -py /path/to/python/modules -b /path/to/bin/inkscape -p /path/to/Info.plist

\033[1mOPTIONS\033[0m
	\033[1m-h,--help\033[0m 
		display this help message
	\033[1m-s\033[0m
		strip the libraries and executables from debugging symbols
	\033[1m-py,--with-python\033[0m
		add python modules (numpy, lxml) from given directory
		inside the app bundle
	\033[1m-l,--libraries\033[0m
		specify the path to the librairies Inkscape depends on
		(typically /sw or /opt/local)
	\033[1m-b,--binary\033[0m
		specify the path to Inkscape's binary. By default it is in
		Build/bin/ at the base of the source code directory
	\033[1m-p,--plist\033[0m
		specify the path to Info.plist. Info.plist can be found
		in the base directory of the source code once configure
		has been run
	\033[1m-v,--verbose\033[0m
		Verbose mode.

\033[1mEXAMPLE\033[0m
	$0 -s -py ~/python-modules -l /opt/local -b ../../Build/bin/inkscape -p ../../Info.plist
"
}


# Parse command line arguments
#----------------------------------------------------------
while [ "$1" != "" ]
do
	case $1 in
		-py|--with-python)
			add_python=true
			python_dir="$2"
			shift 1 ;;
		-s)
			strip_build=true
			with_dSYM=false ;;
		-l|--libraries)
			LIBPREFIX="$2"
			shift 1 ;;
		-b|--binary)
			binary="$2"
			shift 1 ;;
		-p|--plist)
			plist="$2"
			shift 1 ;;
		-v|--verbose)
			verbose_mode=true ;;
		-h|--help)
			help
			exit 0 ;;
		*)
			echo "Invalid command line option: $1"
			exit 2 ;;
	esac
	shift 1
done

echo -e "\n\033[1mCREATE INKSCAPE APP BUNDLE\033[0m\n"


# Safety tests
#----------------------------------------------------------

if [ "x$binary" == "x" ]; then
	echo "Inkscape binary path not specified." >&2
	exit 1
fi

if [ ! -x "$binary" ]; then
	echo "Inkscape executable not not found at $binary." >&2
	exit 1
fi

if [ "x$plist" == "x" ]; then
	echo "Info.plist file not specified." >&2
	exit 1
fi

if [ ! -f "$plist" ]; then
	echo "Info.plist file not found at $plist." >&2
	exit 1
fi

if [ ${add_python} = "true" ]; then
	if [ -z "$python_dir" ]; then
		echo "Python modules will be copied from MacPorts tree." >&2
	else
		if [ ! -e "$python_dir" ]; then
			echo "Python modules directory \""$python_dir"\" not found." >&2
			exit 1
		else
			if [ -e "$python_dir/i386" -o -e "$python_dir/ppc" ]; then
				echo "Outdated structure in custom python modules detected," >&2
				echo "not compatible with current packaging." >&2
				exit 1
			else
				echo "Python modules will be copied from $python_dir." >&2
			fi
		fi
	fi
fi

if [ ! -e "$LIBPREFIX" ]; then
	echo "Cannot find the directory containing the libraires: $LIBPREFIX" >&2
	exit 1
fi

if [ "x$(otool -L "$binary" | grep "libgtk-quartz")" != "x" ]; then
	if ! pkg-config --exists gtk+-quartz-2.0; then
		echo "Missing GTK+ backend -- please install gtk2 and its dependencies with variant '+quartz' and try again." >&2
		exit 1
	fi
	_backend="quartz"
else
	if ! pkg-config --exists gtk+-x11-2.0; then
	    echo "Missing GTK+ backend -- please install gtk2 and its dependencies with variant '+x11' and try again." >&2
		exit 1
	fi
	_backend="x11"
fi

if ! pkg-config --exists gtk-engines-2; then
	echo "Missing gtk-engines2 -- please install gtk-engines2 and try again." >&2
	exit 1
fi

if [ ! -e "$LIBPREFIX/lib/gtk-2.0/$(pkg-config --variable=gtk_binary_version gtk+-2.0)/engines/libmurrine.so" ]; then
	echo "Missing gtk2-murrine -- please install gtk2-murrine and try again." >&2
	exit 1
fi

if [ ! -e "$LIBPREFIX/lib/gtk-2.0/$(pkg-config --variable=gtk_binary_version gtk+-2.0)/engines/libadwaita.so" ]; then
	echo "Missing gnome-themes-standard -- please install gnome-themes-standard and try again." >&2
	exit 1
fi

if [ ! -e "$LIBPREFIX/share/icons/hicolor/index.theme" ]; then
	echo "Missing hicolor-icon-theme -- please install hicolor-icon-theme and try again." >&2
	exit 1
fi

if ! pkg-config --exists icon-naming-utils; then
	echo "Missing icon-naming-utils -- please install icon-naming-utils and try again." >&2
	exit 1
fi

# if [ "$default_theme" != "default" ] ; then
# 	if ! pkg-config --exists gnome-icon-theme; then
# 		echo "Missing gnome-icon-theme -- please install gnome-icon-theme and try again." >&2
# 		exit 1
# 	fi
# 
# 	if ! pkg-config --exists gnome-icon-theme-symbolic; then
# 		echo "Missing gnome-icon-theme-symbolic -- please install gnome-icon-theme-symbolic and try again." >&2
# 		exit 1
# 	fi
# fi

unset WITH_GNOME_VFS
if ! pkg-config --exists gnome-vfs-2.0; then
	echo "Missing gnome-vfs2 -- some features will be disabled" >&2
else
	WITH_GNOME_VFS=true
fi

# unset WITH_DBUS
# if ! pkg-config --exists dbus-1; then
# 	echo "Missing dbus -- some features will be disabled" >&2
# else
# 	WITH_DBUS=true
# fi
# 
# unset WITH_GVFS
# if [ ! -e "$LIBPREFIX/libexec/gvfsd" ]; then
# 	echo "Missing gvfs -- some features will be disabled" >&2
# elif [ ! -z "$WITH_DBUS" ]; then
# 	WITH_GVFS=true
# else
# 	echo "Missing dbus for gvfs -- some features will be disabled" >&2
# fi

if ! pkg-config --exists poppler; then
	echo "Missing poppler -- please install poppler and try again." >&2
	exit 1
fi

if ! pkg-config --exists ImageMagick; then
	echo "Missing ImageMagick -- please install ImageMagick and try again." >&2
	exit 1
fi

# FIXME: retrieve aspell version from installed files (no pkg-config support)
ASPELL_VERSION="0.60"
if [ ! -e "$LIBPREFIX/lib/aspell-$ASPELL_VERSION/en.dat" ]; then
	echo "Missing aspell en dictionary -- please install at least 'aspell-dict-en', but" >&2
	echo "preferably more dictionaries ('aspell-dict-*') and try again." >&2
	exit 1
fi

# awk on Leopard fails in fixlib(), test earlier and require gawk if test fails
awk_test="$(echo "/lib" | awk -F/ '{for (i=1;i<NF;i++) sub($i,".."); sub($NF,"",$0); print $0}')"
if [ -z "$awk_test" ]; then
	if [ ! -x "$LIBPREFIX/bin/gawk" ]; then
		echo "awk provided by system is too old, please install gawk and try again" >&2
		exit 1
	else
		awk_cmd="$LIBPREFIX/bin/gawk"
	fi
else
	awk_cmd="awk"
fi
unset awk_test


# OS X version
#----------------------------------------------------------
OSXVERSION="$(/usr/bin/sw_vers | grep ProductVersion | cut -f2)"
OSXMINORVER="$(cut -d. -f 1,2 <<< $OSXVERSION)"
OSXMINORNO="$(cut -d. -f2 <<< $OSXVERSION)"
OSXPOINTNO="$(cut -d. -f3 <<< $OSXVERSION)"
ARCH="$(uname -a | awk '{print $NF;}')"

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

# Setup
#----------------------------------------------------------
case $_backend in
	x11)
		echo "Building package with GTK+/X11." >&2
		# Handle some version specific details.
		if [ "$OSXMINORNO" -le "4" ]; then
			echo "Note: Inkscape packaging requires Mac OS X 10.5 Leopard or later."
			exit 1
		else # if [ "$OSXMINORNO" -ge "5" ]; then
			XCODEFLAGS="-configuration Deployment"
			SCRIPTEXECDIR="ScriptExec/build/Deployment/ScriptExec.app/Contents/MacOS"
			EXTRALIBS=""
		fi
		;;
	quartz)
		# quartz backend
		echo "Building package with GTK+/Quartz." >&2
		;;
	*)
		exit 1
esac


# Package always has the same name. Version information is stored in
# the Info.plist file which is filled in by the configure script.
package="Inkscape.app"

# Remove a previously existing package if necessary
if [ -d $package ]; then
	echo "Removing previous Inkscape.app"
	rm -rf $package
fi


# Set the 'macosx' directory, usually the current directory.
resdir=`pwd`

# Custom resources used to generate resources during app bundle creation.
if [ -z "$custom_res" ] ; then
	custom_res="${resdir}/Resources-extras"
fi


# Prepare Package
#----------------------------------------------------------
pkgexec="$package/Contents/MacOS"
pkgbin="$package/Contents/Resources/bin"
pkgetc="$package/Contents/Resources/etc"
pkglib="$package/Contents/Resources/lib"
pkgshare="$package/Contents/Resources/share"
pkglocale="$package/Contents/Resources/share/locale"
pkgresources="$package/Contents/Resources"

mkdir -p "$pkgexec"
mkdir -p "$pkgbin"
mkdir -p "$pkgetc"
mkdir -p "$pkglib"
mkdir -p "$pkgshare"
mkdir -p "$pkglocale"


# utility
#----------------------------------------------------------

if [ $verbose_mode ] ; then 
	cp_cmd="/bin/cp -v"
	ln_cmd="/bin/ln -sv"
	rsync_cmd="/usr/bin/rsync -av"
else
	cp_cmd="/bin/cp"
	ln_cmd="/bin/ln -s"
	rsync_cmd="/usr/bin/rsync -a"
fi


# Build and add the launcher
#----------------------------------------------------------
case $_backend in
	x11)
		(
		# Build fails if CC happens to be set (to anything other than CompileC)
		unset CC

		cd "$resdir/ScriptExec"
		echo -e "\033[1mBuilding launcher...\033[0m\n"
		xcodebuild $XCODEFLAGS clean build
		)
		$cp_cmd "$resdir/$SCRIPTEXECDIR/ScriptExec" "$pkgexec/Inkscape"
		;;
	quartz)
		$cp_cmd "$resdir/ScriptExec/launcher-quartz-no-macintegration.sh" "$pkgexec/inkscape"
		;;
	*)
		exit 1
esac


# Copy all files into the bundle
#----------------------------------------------------------
echo -e "\n\033[1mFilling app bundle...\033[0m\n"

binary_name=`basename "$binary"`
binary_dir=`dirname "$binary"`

# Inkscape's binary
if [ $_backend = "x11" ]; then
	scrpath="$pkgbin/inkscape"
	binpath="$pkgbin/inkscape-bin"
else
	scrpath="$pkgexec/inkscape"
	binpath="$pkgexec/inkscape-bin"
fi
$cp_cmd "$binary" "$binpath"
# TODO Add a "$verbose" variable and command line switch, which sets wether these commands are verbose or not

# Info.plist
$cp_cmd "$plist" "$package/Contents/Info.plist"
if [ $_backend = "quartz" ]; then
	/usr/libexec/PlistBuddy -x -c "Set :CGDisableCoalescedUpdates 1" "${package}/Contents/Info.plist"
fi

# Share files
$rsync_cmd "$binary_dir/../share/$binary_name"/* "$pkgshare/$binary_name"
$rsync_cmd "$binary_dir/../share/locale"/* "$pkglocale"

# Copy GTK shared mime information
mkdir -p "$pkgresources/share"
$cp_cmd -rp "$LIBPREFIX/share/mime" "$pkgshare/"

# Copy GTK hicolor icon theme index file
mkdir -p "$pkgresources/share/icons/hicolor"
$cp_cmd "$LIBPREFIX/share/icons/hicolor/index.theme"  "$pkgresources/share/icons/hicolor"

# GTK+ stock icons with legacy icon mapping
echo "Creating GtkStock icon theme ..."
stock_src="${custom_res}/src/icons/stock-icons" \
	./create-stock-icon-theme.sh "${pkgshare}/icons/GtkStock"
gtk-update-icon-cache --index-only "${pkgshare}/icons/GtkStock"

# GTK+ themes
$cp_cmd -RP "$LIBPREFIX/share/gtk-engines" "$pkgshare/"
for item in Adwaita Clearlooks HighContrast Industrial Raleigh Redmond ThinIce; do
	mkdir -p "$pkgshare/themes/$item"
	$cp_cmd -RP "$LIBPREFIX/share/themes/$item/gtk-2.0" "$pkgshare/themes/$item/"
done
if [ $_backend = "quartz" ]; then
	for item in Mac; do
		$cp_cmd -RP "$LIBPREFIX/share/themes/$item/gtk-2.0"* "$pkgshare/themes/$item/"
	done
fi

# Icons and the rest of the script framework
$rsync_cmd --exclude ".svn" "$resdir"/Resources/* "$pkgresources/"

# remove files not needed with GTK+/Quartz
if [ $_backend = "quartz" ]; then
	rm "$pkgresources/script"
	rm "$pkgresources/openDoc"
	rm "$pkgbin/inkscape"
fi

# activate wrapper scripts
if [ $add_wrapper = "true" ]; then
	mv "$pkgbin/gimp-wrapper.sh" "$pkgbin/gimp"
fi

# Add python modules if requested
if [ ${add_python} = "true" ]; then
	install_py_modules ()
	{
		# lxml
		$cp_cmd -RL "$packages_path/lxml" "$pkgpython"
		# numpy
		$cp_cmd -RL "$packages_path/nose" "$pkgpython"
		$cp_cmd -RL "$packages_path/numpy" "$pkgpython"
		# UniConvertor
		$cp_cmd -RL "$packages_path/PIL" "$pkgpython"
		if [ "$PYTHON_VER" == "2.5" ]; then
			$cp_cmd -RL "$packages_path/_imaging.so" "$pkgpython"
			$cp_cmd -RL "$packages_path/_imagingcms.so" "$pkgpython"
			$cp_cmd -RL "$packages_path/_imagingft.so" "$pkgpython"
			$cp_cmd -RL "$packages_path/_imagingmath.so" "$pkgpython"
		else  # we build Pillow with +tkinter
			$cp_cmd -RL "$packages_path/_tkinter.so" "$pkgpython"
		fi
		$cp_cmd -RL "$packages_path/sk1libs" "$pkgpython"
		$cp_cmd -RL "$packages_path/uniconvertor" "$pkgpython"
		# pySerial for HPGL plotting
		$cp_cmd -RL "$packages_path/serial" "$pkgpython"
		# PyGTK (optional)
		$cp_cmd -RL "$packages_path/cairo" "$pkgpython"
		$cp_cmd -RL "$packages_path/glib" "$pkgpython"
		$cp_cmd -RL "$packages_path/gobject" "$pkgpython"
		$cp_cmd -RL "$packages_path/../../../share/pygobject" "$pkgshare"
		$cp_cmd -RL "$packages_path/gtk-2.0" "$pkgpython"
		$cp_cmd -RL "$packages_path/../../../share/pygtk" "$pkgshare"
		$cp_cmd -RL "$packages_path/pygtk.pth" "$pkgpython"
		$cp_cmd -RL "$packages_path/pygtk.py" "$pkgpython"
		# ReportLab (for inkscape-hocrpdf, experimental)
		$cp_cmd -RL "$packages_path/reportlab" "$pkgpython"
		# cleanup python modules
		find "$pkgpython" -name *.pyc -print0 | xargs -0 rm -f
		find "$pkgpython" -name *.pyo -print0 | xargs -0 rm -f
		find "${pkgshare}/pygobject" -name *.pyc -print0 | xargs -0 rm -f		

		# TODO: test whether to remove hard-coded paths from *.la files or to exclude them altogether
		for la_file in $(find "$pkgpython" -name *.la); do
			sed -e "s,libdir=\'.*\',libdir=\'\',g" -i "" "$la_file"
		done
	}

	if [ $OSXMINORNO -eq "5" ]; then
		PYTHON_VERSIONS="2.5 2.6 2.7"
	elif [ $OSXMINORNO -eq "6" ]; then
		PYTHON_VERSIONS="2.6 2.7"
	else # if [ $OSXMINORNO -ge "7" ]; then
		PYTHON_VERSIONS="2.7"
	fi
	if [ -z "$python_dir" ]; then
		for PYTHON_VER in $PYTHON_VERSIONS; do
			python_dir="$(${LIBPREFIX}/bin/python${PYTHON_VER}-config --prefix)"
			packages_path="${python_dir}/lib/python${PYTHON_VER}/site-packages"
			pkgpython="${pkglib}/python${PYTHON_VER}/site-packages"
			mkdir -p $pkgpython
			install_py_modules
		done
	else
		# copy custom python site-packages. 
		# They need to be organized in a hierarchical set of directories by python major+minor version:
		#   - ${python_dir}/python2.5/site-packages/lxml
		#   - ${python_dir}/python2.5/site-packages/nose
		#   - ${python_dir}/python2.5/site-packages/numpy
		#   - ${python_dir}/python2.6/site-packages/lxml
		#   - ...
		$cp_cmd -rf "$python_dir"/* "$pkglib"
	fi
fi

sed -e "s,__build_arch__,$_build_arch,g" -i "" "$scrpath"

# PkgInfo must match bundle type and creator code from Info.plist
echo "APPLInks" > $package/Contents/PkgInfo

# Pull in extra requirements for Pango and GTK
PANGOVERSION=$(pkg-config --modversion pango)
PANGOVERSION_MINOR="$(cut -d. -f2 <<< $PANGOVERSION)"

if [ $PANGOVERSION_MINOR -lt 37 ]; then
	mkdir -p $pkgetc/pango
	touch "$pkgetc/pango/pangorc"
else
	echo "Newer pango version found, modules are built-in"
fi

# We use a modified fonts.conf file so only need the dtd
mkdir -p $pkgshare/xml/fontconfig
$cp_cmd $LIBPREFIX/share/xml/fontconfig/fonts.dtd $pkgshare/xml/fontconfig
mkdir -p $pkgetc/fonts
$cp_cmd -r $LIBPREFIX/etc/fonts/conf.d $pkgetc/fonts/
mkdir -p $pkgshare/fontconfig
$cp_cmd -r $LIBPREFIX/share/fontconfig/conf.avail $pkgshare/fontconfig/
(cd $pkgetc/fonts/conf.d && $ln_cmd ../../../share/fontconfig/conf.avail/10-autohint.conf)
(cd $pkgetc/fonts/conf.d && $ln_cmd ../../../share/fontconfig/conf.avail/70-no-bitmaps.conf)

if [ $PANGOVERSION_MINOR -lt 37 ]; then
	# Pull in modules
	pango_mod_version=`pkg-config --variable=pango_module_version pango`
	mkdir -p $pkglib/pango/$pango_mod_version/modules
	$cp_cmd $LIBPREFIX/lib/pango/$pango_mod_version/modules/*.so $pkglib/pango/$pango_mod_version/modules/
fi

gtk_version=`pkg-config --variable=gtk_binary_version gtk+-2.0`
mkdir -p $pkglib/gtk-2.0/$gtk_version/{engines,immodules,printbackends}
$cp_cmd -r $LIBPREFIX/lib/gtk-2.0/$gtk_version/* $pkglib/gtk-2.0/$gtk_version/

gdk_pixbuf_version=`pkg-config --variable=gdk_pixbuf_binary_version gdk-pixbuf-2.0`
mkdir -p $pkglib/gdk-pixbuf-2.0/$gdk_pixbuf_version/loaders
$cp_cmd $LIBPREFIX/lib/gdk-pixbuf-2.0/$gdk_pixbuf_version/loaders/*.so $pkglib/gdk-pixbuf-2.0/$gdk_pixbuf_version/loaders/

sed -e "s,__gtk_version__,$gtk_version,g" -i "" "$scrpath"
sed -e "s,__gdk_pixbuf_version__,$gdk_pixbuf_version,g" -i "" "$scrpath"
#sed -e "s,$LIBPREFIX,@loader_path/..,g" "$LIBPREFIX/etc/pango/pango.modules" > "$pkgetc/pango/pango.modules"
#sed -e "s,$LIBPREFIX,@loader_path/..,g" "$LIBPREFIX/lib/gtk-2.0/$gtk_version/immodules.cache" > "$pkglib/gtk-2.0/$gtk_version/immodules.cache"
#sed -e "s,$LIBPREFIX,@loader_path/..,g" "$LIBPREFIX/lib/gdk-pixbuf-2.0/$gtk_version/loaders.cache" > "$pkglib/gdk-pixbuf-2.0/$gtk_version/loaders.cache"

# recreate loaders and modules caches based on actually included modules

# Pango modules
if [ $PANGOVERSION_MINOR -lt 37 ]; then
	pango-querymodules "$pkglib/pango/$pango_mod_version"/modules/*.so \
		| sed -e "s,$PWD/$pkgresources,@loader_path/..,g" \
		> "$pkgetc"/pango/pango.modules
fi

# Gtk immodules
gtk-query-immodules-2.0 "$pkglib/gtk-2.0/$gtk_version"/immodules/*.so \
    | sed -e "s,$PWD/$pkgresources,@loader_path/..,g" \
    > "$pkglib/gtk-2.0/$gtk_version/"immodules.cache

# Gdk pixbuf loaders
GDK_PIXBUF_MODULEDIR="$pkglib/gdk-pixbuf-2.0/$gtk_version/"loaders gdk-pixbuf-query-loaders \
    | sed -e "s,$pkgresources,@loader_path/..,g" > "$pkglib/gdk-pixbuf-2.0/$gtk_version/"loaders.cache

# GIO modules
#gio-querymodules "$pkglib/gio/modules" 

# Gnome-vfs modules (deprecated, optional in inkscape)
if [ $WITH_GNOME_VFS ] ; then
	for item in gnome-vfs-mime-magic gnome-vfs-2.0; do
		$cp_cmd -r "$LIBPREFIX/etc/$item" "$pkgetc/"
	done
	for item in modules; do
		mkdir -p "$pkglib/gnome-vfs-2.0/$item"
		$cp_cmd "$LIBPREFIX/lib/gnome-vfs-2.0/$item"/*.so "$pkglib/gnome-vfs-2.0/$item/"
	done
fi

# ImageMagick version
IMAGEMAGICKVER="$(pkg-config --modversion ImageMagick)"
IMAGEMAGICKVER_MAJOR="$(cut -d. -f1 <<< "$IMAGEMAGICKVER")"

# ImageMagick data
# include *.la files for main libs too
for item in "$LIBPREFIX/lib/libMagick"*.la; do
	$cp_cmd "$item" "$pkglib/"
done
# ImageMagick modules
$cp_cmd -r "$LIBPREFIX/lib/ImageMagick-$IMAGEMAGICKVER" "$pkglib/"
$cp_cmd -r "$LIBPREFIX/etc/ImageMagick-$IMAGEMAGICKVER_MAJOR" "$pkgetc/"
$cp_cmd -r "$LIBPREFIX/share/ImageMagick-$IMAGEMAGICKVER_MAJOR" "$pkgshare/"
# REQUIRED: remove hard-coded paths from *.la files
for la_file in "$pkglib/libMagick"*.la; do
	sed -e "s,$LIBPREFIX/lib,,g" -i "" "$la_file"
done
for la_file in "$pkglib/ImageMagick-$IMAGEMAGICKVER/modules-Q16/coders"/*.la; do
	sed -e "s,$LIBPREFIX/lib/ImageMagick-$IMAGEMAGICKVER/modules-Q16/coders,,g" -i "" "$la_file"
done
for la_file in "$pkglib/ImageMagick-$IMAGEMAGICKVER/modules-Q16/filters"/*.la; do
	sed -e "s,$LIBPREFIX/lib/ImageMagick-$IMAGEMAGICKVER/modules-Q16/filters,,g" -i "" "$la_file"
done
sed -e "s,IMAGEMAGICKVER,$IMAGEMAGICKVER,g" -i "" "$scrpath"
sed -e "s,IMAGEMAGICKVER_MAJOR,$IMAGEMAGICKVER_MAJOR,g" -i "" "$scrpath"

# Copy aspell dictionary files:
$cp_cmd -r "$LIBPREFIX/lib/aspell-$ASPELL_VERSION" "$pkglib/"
$cp_cmd -r "$LIBPREFIX/share/aspell" "$pkgshare/"

# Copy Poppler data:
$cp_cmd -r "$LIBPREFIX/share/poppler" "$pkgshare"

# GLib2 schemas
mkdir -p "$pkgshare/glib-2.0"
$cp_cmd -RP "$LIBPREFIX/share/glib-2.0/schemas" "$pkgshare/glib-2.0/"

# Copy all linked libraries into the bundle
#----------------------------------------------------------
# get list of *.so modules from python modules
python_libs=""
for PYTHON_VER in "2.5" "2.6" "2.7"; do
	python_libs="$python_libs $(find "${pkglib}/python${PYTHON_VER}" -name *.so -or -name *.dylib)"
done
[ $verbose_mode ] && echo "Python libs: $python_libs"

# get list of included binary executables
extra_bin=$(find $pkgbin -exec file {} \; | grep executable | grep -v text | cut -d: -f1)
[ $verbose_mode ] && echo "Extra binaries: $extra_bin"

# Find out libs we need from MacPorts, Fink, or from a custom install
# (i.e. $LIBPREFIX), then loop until no changes.
a=1
nfiles=0
endl=true
while $endl; do
	echo -e "\033[1mLooking for dependencies.\033[0m Round" $a
	libs="$(otool -L \
		$pkglib/gtk-2.0/$gtk_version/{engines,immodules,printbackends}/*.{dylib,so} \
		$pkglib/gdk-pixbuf-2.0/$gtk_version/loaders/*.so \
		$pkglib/pango/$pango_version/modules/*.so \
		$pkglib/gnome-vfs-2.0/modules/*.so \
		$pkglib/gio/modules/*.so \
		$pkglib/ImageMagick-$IMAGEMAGICKVER/modules-Q16/{filters,coders}/*.so \
		$pkglib/*.{dylib,so} \
		$pkgbin/*.so \
		$python_libs \
		$extra_bin \
		$binpath \
		2>/dev/null | fgrep compatibility | cut -d\( -f1 | grep $LIBPREFIX | sort | uniq)"
	$cp_cmd -f $libs "$pkglib"
	let "a+=1"	
	nnfiles="$(ls "$pkglib" | wc -l)"
	if [ $nnfiles = $nfiles ]; then
		endl=false
	else
		nfiles=$nnfiles
	fi
done

# Some libraries don't seem to have write permission, fix this.
chmod -R u+w "$package/Contents/Resources/lib"

# Strip libraries and executables if requested
#----------------------------------------------------------
if [ "$strip_build" = "true" ]; then
    echo -e "\n\033[1mStripping debugging symbols...\033[0m\n"
    chmod +w "$pkglib"/*.dylib
    strip -x "$pkglib"/*.dylib
    strip -ur "$binpath"
fi

# Rewrite id and paths of linked libraries
#----------------------------------------------------------
# extract level for relative path to libs
echo -e "\n\033[1mRewriting library paths ...\033[0m\n"

LIBPREFIX_levels="$(echo "$LIBPREFIX"|awk -F/ '{print NF+1}')"

fixlib () {
	if [ ! -d "$1" ]; then
		fileLibs="$(otool -L $1 | fgrep compatibility | cut -d\( -f1)"
		filePath="$(echo "$2" | sed 's/.*Resources//')"
		fileType="$3"
		unset to_id
		case $fileType in
			lib)
				# TODO: verfiy correct/expected install name for relocated libs
				to_id="$package/Contents/Resources$filePath/$1"
				loader_to_res="$(echo $filePath | $awk_cmd -F/ '{for (i=1;i<NF;i++) sub($i,".."); sub($NF,"",$0); print $0}')"
				;;
			bin)
				loader_to_res="../"
				;;
			exec)
				loader_to_res="../Resources/"
				;;
			*)
				echo "Skipping loader_to_res for $1"
				;;
		esac
		[ $verbose_mode ] && echo ""
		[ $verbose_mode ] && echo "basename:          $1"
		[ $verbose_mode ] && echo "dirname:           $2"
		[ $verbose_mode ] && echo "filePath:          $filePath"
		[ $verbose_mode ] && echo "to_id:             $to_id"
		[ $verbose_mode ] && echo "loader_to_res:     $loader_to_res"
		[ $to_id ] && install_name_tool -id "$to_id" "$1"
		for lib in $fileLibs; do
			first="$(echo $lib | cut -d/ -f1-3)"
			if [ $first != /usr/lib -a $first != /usr/X11 -a $first != /opt/X11 -a $first != /System/Library ]; then
				lib_prefix_levels="$(echo $lib | $awk_cmd -F/ '{for (i=NF;i>0;i--) if($i=="lib") j=i; print j}')"
				res_to_lib="$(echo $lib | cut -d/ -f$lib_prefix_levels-)"
				unset to_path
				case $fileType in
					lib)
						to_path="@loader_path/$loader_to_res$res_to_lib"
						;;
					bin)
						to_path="@executable_path/$loader_to_res$res_to_lib"
						;;
					exec)
						to_path="@executable_path/$loader_to_res$res_to_lib"
						;;
					*)
						echo "Skipping to_path for $lib in $1"
						;;
				esac
				[ $verbose_mode ] && echo "lib:               $lib"
				[ $verbose_mode ] && echo "lib_prefix_levels: $lib_prefix_levels"
				[ $verbose_mode ] && echo "res_to_lib:        $res_to_lib"
				[ $verbose_mode ] && echo "to_path:           $to_path"
				[ $verbose_mode ] && echo "install_name_tool arguments: -change $lib $to_path $1"
				[ $to_path ] && install_name_tool -change "$lib" "$to_path" "$1"
			fi
		done
	fi
}

rewritelibpaths () {
	if [ $_backend = "quartz" ]; then
		echo -n "Rewriting dylib paths for executable ... "
		(cd "$pkgexec"; fixlib "inkscape-bin" "$package/Contents/Resources/../MacOS" "exec")
		echo "done"
	fi
	echo "Rewriting dylib paths for included binaries:"
	for file in $extra_bin; do
		echo -n "Rewriting dylib paths for $file ... "
		(cd "$(dirname $file)" ; fixlib "$(basename $file)" "$(dirname $file)" "bin")
		echo "done"
	done
	echo "Rewriting dylib paths for included libraries:"
	for file in $(find $package \( -name '*.so' -or -name '*.dylib' \) -and -not -ipath '*.dSYM*'); do
		echo -n "Rewriting dylib paths for $file ... "
		(cd "$(dirname $file)" ; fixlib "$(basename $file)" "$(dirname $file)" "lib")
		echo "done"
	done
}

rewritelibpaths


# Include debug info in app bundle
#----------------------------------------------------------
# TODO: needs more testing

if [ "$with_dSYM" = "true" ]; then

    echo -e "\n\033[1mAdding debug info to app bundle ...\033[0m\n"

    # package debug symbols for main binary
    echo "dsymutil $binpath"
    dsymutil "$binpath"

    # some of the dependencies have debug symbols in MacPorts ...
    #for item in libbz2.1.0.dylib libexif.12.dylib libopenraw.1.dylib; do
    for item in libbz2.1.0.dylib libexif.12.dylib; do
        echo "dsymutil ${pkglib}/${item}"
        dsymutil "${pkglib}/${item}"
    done

    # to debug issues with ImageMagick / libMagick++
    # Note: install ImageMagick with local portfile which includes the 'debug 1.0' portgroup
    #       use 'port -n -k upgrade --enforce-variants ImageMagick +debug' to reinstall with debug symbols
    #       (keep work dir (port -k) to allow recreation of bundled dSYMs with dsymutil)
    if [[ "$use_port" == "t" ]]; then
        if [[ "$(port echo ImageMagick and active | grep debug)" ]]; then
            # if ImageMagick was installed with debug variant
            for file in $(find $package -name 'libMagick*' -and -name '*.dylib' -and -not -ipath '*.dSYM*'); do
                echo "dsymutil $file"
                dsymutil "$file"
            done
            for file in $(find $package -ipath '*ImageMagick*' -and -name '*.so' -and -not -ipath '*.dSYM*'); do
                echo "dsymutil $file"
                dsymutil "$file"
            done
        else
            echo "Macports' ImageMagick port was not installed with +debug variant."
        fi
    else
        echo "not using MacPorts, skipping recreation of dSYMs included in app bundle."
    fi

    # for debug bundle, remove translations and tutorials (download size)
    echo "Removing translation files and tutorials (only for debug builds) ..."
    rm -rf "$pkglocale"/*
    rm -f "${pkgshare}/${binary_name}/tutorials"/*

else

    # remove dSYM files if present (local port built with +debug variant from debug port group)
    for item in $(find "${pkglib}/ImageMagick-${IMAGEMAGICKVER}" -name '*.dSYM'); do
        rm -r "$item"
    done

fi    


# All done.
#----------------------------------------------------------
echo "Inkscape.app created successfully."

exit 0
