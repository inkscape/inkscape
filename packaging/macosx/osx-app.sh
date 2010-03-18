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
# 
# Copyright (C) 2005 Kees Cook
# Copyright (C) 2005-2009 Michael Wybrow
# Copyright (C) 2007-2009 Jean-Olivier Irisson
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
strip=false
add_python=false
python_dir=""

# If LIBPREFIX is not already set (by osx-build.sh for example) set it to blank (one should use the command line argument to set it correctly)
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
	\033[1m-b--binary\033[0m
		specify the path to Inkscape's binary. By default it is in
		Build/bin/ at the base of the source code directory
	\033[1m-p,--plist\033[0m
		specify the path to Info.plist. Info.plist can be found
		in the base directory of the source code once configure
		has been run

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
			strip=true ;;
		-l|--libraries)
			LIBPREFIX="$2"
			shift 1 ;;
		-b|--binary)
			binary="$2"
			shift 1 ;;
		-p|--plist)
			plist="$2"
			shift 1 ;;
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

PYTHONPACKURL="http://inkscape.modevia.com/macosx-snap/Python-packages.dmg"

if [ "x$python_dir" == "x" ]; then
	echo "Python modules directory not specified." >&2
	echo "Python modules can be downloaded from:" >&2
	echo "    $PYTHONPACKURL" >&2
	exit 1
fi

if [ ! -e "$python_dir/i386" -o ! -e "$python_dir/ppc" ]; then
	echo "Directory does not appear to contain the i386 and ppc python modules:" >&2
	echo "    $python_dir" >&2
	echo "Python modules can be downloaded from:" >&2
	echo "    $PYTHONPACKURL" >&2
	exit 1
fi

if [ ! -e "$LIBPREFIX" ]; then
	echo "Cannot find the directory containing the libraires: $LIBPREFIX" >&2
	exit 1
fi

if ! pkg-config --exists gtk-engines-2; then
	echo "Missing gtk-engines2 -- please install gtk-engines2 and try again." >&2
	exit 1
fi

if ! pkg-config --exists gnome-vfs-2.0; then
	echo "Missing gnome-vfs2 -- please install gnome-vfs2 and try again." >&2
	exit 1
fi

if ! pkg-config --exists poppler; then
	echo "Missing poppler -- please install poppler and try again." >&2
	exit 1
fi

if ! pkg-config --modversion ImageMagick >/dev/null 2>&1; then
	echo "Missing ImageMagick -- please install ImageMagick and try again." >&2
	exit 1
fi

if [ ! -e "$LIBPREFIX/lib/aspell-0.60/en.dat" ]; then
	echo "Missing aspell en dictionary -- please install at least 'aspell-dict-en', but" >&2
	echo "preferably all dictionaries ('aspell-dict-*') and try again." >&2
	exit 1
fi


# Handle some version specific details.
VERSION=`/usr/bin/sw_vers | grep ProductVersion | cut -f2 -d'.'`
if [ "$VERSION" -ge "4" ]; then
	# We're on Tiger (10.4) or later.
	# XCode behaves a little differently in Tiger and later.
	XCODEFLAGS="-configuration Deployment"
	SCRIPTEXECDIR="ScriptExec/build/Deployment/ScriptExec.app/Contents/MacOS"
	EXTRALIBS=""
else
	# Panther (10.3) or earlier.
	XCODEFLAGS="-buildstyle Deployment"
	SCRIPTEXECDIR="ScriptExec/build/ScriptExec.app/Contents/MacOS"
	EXTRALIBS=""
fi


# Package always has the same name. Version information is stored in
# the Info.plist file which is filled in by the configure script.
package="Inkscape.app"

# Remove a previously existing package if necessary
if [ -d $package ]; then
	echo "Removing previous Inkscape.app"
	rm -Rf $package
fi


# Set the 'macosx' directory, usually the current directory.
resdir=`pwd`


# Prepare Package
#----------------------------------------------------------
pkgexec="$package/Contents/MacOS"
pkgbin="$package/Contents/Resources/bin"
pkglib="$package/Contents/Resources/lib"
pkglocale="$package/Contents/Resources/locale"
pkgpython="$package/Contents/Resources/python/site-packages/"
pkgresources="$package/Contents/Resources"

mkdir -p "$pkgexec"
mkdir -p "$pkgbin"
mkdir -p "$pkglib"
mkdir -p "$pkglocale"
mkdir -p "$pkgpython"

mkdir -p "$pkgresources/Dutch.lprj"
mkdir -p "$pkgresources/English.lprj"
mkdir -p "$pkgresources/French.lprj"
mkdir -p "$pkgresources/German.lprj"
mkdir -p "$pkgresources/Italian.lprj"
mkdir -p "$pkgresources/Spanish.lprj"
mkdir -p "$pkgresources/fi.lprj"
mkdir -p "$pkgresources/no.lprj"
mkdir -p "$pkgresources/sv.lprj"

# Build and add the launcher
#----------------------------------------------------------
(
	# Build fails if CC happens to be set (to anything other than CompileC)
	unset CC
	
	cd "$resdir/ScriptExec"
	echo -e "\033[1mBuilding launcher...\033[0m\n"
	xcodebuild $XCODEFLAGS clean build
)
cp "$resdir/$SCRIPTEXECDIR/ScriptExec" "$pkgexec/Inkscape"


# Copy all files into the bundle
#----------------------------------------------------------
echo -e "\n\033[1mFilling app bundle...\033[0m\n"

binary_name=`basename "$binary"`
binary_dir=`dirname "$binary"`

# Inkscape's binary
binpath="$pkgbin/inkscape-bin"
cp -v "$binary" "$binpath"
# TODO Add a "$verbose" variable and command line switch, which sets wether these commands are verbose or not

# Share files
rsync -av "$binary_dir/../share/$binary_name"/* "$pkgresources/"
cp "$plist" "$package/Contents/Info.plist"
rsync -av "$binary_dir/../share/locale"/* "$pkgresources/locale"

# Copy GTK shared mime information
mkdir -p "$pkgresources/share"
cp -rp "$LIBPREFIX/share/mime" "$pkgresources/share/"

# Icons and the rest of the script framework
rsync -av --exclude ".svn" "$resdir"/Resources/* "$pkgresources/"

# Update the ImageMagick path in startup script.
IMAGEMAGICKVER=`pkg-config --modversion ImageMagick`
sed -e "s,IMAGEMAGICKVER,$IMAGEMAGICKVER,g" -i "" $pkgbin/inkscape

# Add python modules if requested
if [ ${add_python} = "true" ]; then
	# copy python site-packages. They need to be organized in a hierarchical set of directories, by architecture and python major+minor version, e.g. i386/2.3/ for Ptyhon 2.3 on Intel; ppc/2.4/ for Python 2.4 on PPC
	cp -rvf "$python_dir"/* "$pkgpython"
fi

# PkgInfo must match bundle type and creator code from Info.plist
echo "APPLInks" > $package/Contents/PkgInfo

# Pull in extra requirements for Pango and GTK
pkgetc="$package/Contents/Resources/etc"
mkdir -p $pkgetc/pango
cp $LIBPREFIX/etc/pango/pangox.aliases $pkgetc/pango/
# Need to adjust path and quote in case of spaces in path.
sed -e "s,$LIBPREFIX,\"\${CWD},g" -e 's,\.so ,.so" ,g' $LIBPREFIX/etc/pango/pango.modules > $pkgetc/pango/pango.modules
cat > $pkgetc/pango/pangorc <<END_PANGO
[Pango]
ModuleFiles=\${HOME}/.inkscape-etc/pango.modules
[PangoX]
AliasFiles=\${HOME}/.inkscape-etc/pangox.aliases
END_PANGO

# We use a modified fonts.conf file so only need the dtd
mkdir -p $pkgetc/fonts
cp $LIBPREFIX/etc/fonts/fonts.dtd $pkgetc/fonts/
cp -r $LIBPREFIX/etc/fonts/conf.avail $pkgetc/fonts/
cp -r $LIBPREFIX/etc/fonts/conf.d $pkgetc/fonts/

mkdir -p $pkgetc/gtk-2.0
sed -e "s,$LIBPREFIX,\${CWD},g" $LIBPREFIX/etc/gtk-2.0/gdk-pixbuf.loaders > $pkgetc/gtk-2.0/gdk-pixbuf.loaders
sed -e "s,$LIBPREFIX,\${CWD},g" $LIBPREFIX/etc/gtk-2.0/gtk.immodules > $pkgetc/gtk-2.0/gtk.immodules

for item in gnome-vfs-mime-magic gnome-vfs-2.0
do
	cp -r $LIBPREFIX/etc/$item $pkgetc/
done

pango_version=`pkg-config --variable=pango_module_version pango`
mkdir -p $pkglib/pango/$pango_version/modules
cp $LIBPREFIX/lib/pango/$pango_version/modules/*.so $pkglib/pango/$pango_version/modules/

gtk_version=`pkg-config --variable=gtk_binary_version gtk+-2.0`
mkdir -p $pkglib/gtk-2.0/$gtk_version/{engines,immodules,loaders,printbackends}
cp -r $LIBPREFIX/lib/gtk-2.0/$gtk_version/* $pkglib/gtk-2.0/$gtk_version/

mkdir -p $pkglib/gnome-vfs-2.0/modules
cp $LIBPREFIX/lib/gnome-vfs-2.0/modules/*.so $pkglib/gnome-vfs-2.0/modules/

cp -r "$LIBPREFIX/lib/ImageMagick-$IMAGEMAGICKVER" "$pkglib/"
cp -r "$LIBPREFIX/share/ImageMagick-$IMAGEMAGICKVER" "$pkgresources/share/"

# Copy aspell dictionary files:
cp -r "$LIBPREFIX/lib/aspell-0.60" "$pkglib/"
cp -r "$LIBPREFIX/share/aspell" "$pkgresources/share/"

# Find out libs we need from fink, darwinports, or from a custom install
# (i.e. $LIBPREFIX), then loop until no changes.
a=1
nfiles=0
endl=true
while $endl; do
	echo -e "\033[1mLooking for dependencies.\033[0m Round" $a
	libs="`otool -L $pkglib/gtk-2.0/$gtk_version/{engines,immodules,loaders,printbackends}/*.{dylib,so} $pkglib/pango/$pango_version/modules/* $pkglib/gnome-vfs-2.0/modules/* $package/Contents/Resources/lib/* $pkglib/ImageMagick-$IMAGEMAGICKVER/modules-Q16/{filters,coders}/*.so $binary 2>/dev/null | fgrep compatibility | cut -d\( -f1 | grep $LIBPREFIX | sort | uniq`"
	cp -f $libs $package/Contents/Resources/lib
	let "a+=1"	
	nnfiles=`ls $package/Contents/Resources/lib | wc -l`
	if [ $nnfiles = $nfiles ]; then
		endl=false
	else
		nfiles=$nnfiles
	fi
done

# Add extra libraries of necessary
for libfile in $EXTRALIBS
do
	cp -f $libfile $package/Contents/Resources/lib
done

# Some libraries don't seem to have write permission, fix this.
chmod -R u+w $package/Contents/Resources/lib

# Strip libraries and executables if requested
#----------------------------------------------------------
if [ "$strip" = "true" ]; then
	echo -e "\n\033[1mStripping debugging symbols...\033[0m\n"
	chmod +w "$pkglib"/*.dylib
	strip -x "$pkglib"/*.dylib
	strip -ur "$binpath"
fi

# NOTE: This works for all the dylibs but causes GTK to crash at startup.
#				Instead we leave them with their original install_names and set
#				DYLD_LIBRARY_PATH within the app bundle before running Inkscape.
#
fixlib () {
	libPath="`echo $2 | sed 's/.*Resources//'`"
	pkgPath="`echo $2 | sed 's/Resources\/.*/Resources/'`"
	# Fix a given executable or library to be relocatable
	if [ ! -d "$1" ]; then
		libs="`otool -L $1 | fgrep compatibility | cut -d\( -f1`"
		for lib in $libs; do
			echo "	$lib"
			base=`echo $lib | awk -F/ '{print $NF}'`
			first=`echo $lib | cut -d/ -f1-3`
			relative=`echo $lib | cut -d/ -f4-`
			to=@executable_path/../$relative
			if [ $first != /usr/lib -a $first != /usr/X11 -a $first != /System/Library ]; then
				/usr/bin/install_name_tool -change $lib $to $1
				if [ "`echo $lib | fgrep libcrypto`" = "" ]; then
					/usr/bin/install_name_tool -id $to $1
					for ll in $libs; do
						base=`echo $ll | awk -F/ '{print $NF}'`
						first=`echo $ll | cut -d/ -f1-3`
						relative=`echo $ll | cut -d/ -f4-`
						to=@executable_path/../$relative
						if [ $first != /usr/lib -a $first != /usr/X11 -a $first != /System/Library -a "`echo $ll | fgrep libcrypto`" = "" ]; then
							/usr/bin/install_name_tool -change $ll $to $pkgPath/$relative
						fi
					done
				fi
			fi
		done
	fi
}

rewritelibpaths () {
	# 
	# Fix package deps
	(cd "$package/Contents/Resources/lib/gtk-2.0/2.10.0/loaders"
	for file in *.so; do
		echo "Rewriting dylib paths for $file..."
		fixlib "$file" "`pwd`"
	done
	)
	(cd "$package/Contents/Resources/lib/gtk-2.0/2.10.0/engines"
	for file in *.so; do
		echo "Rewriting dylib paths for $file..."
		fixlib "$file" "`pwd`"
	done
	)
	(cd "$package/Contents/Resources/lib/gtk-2.0/2.10.0/immodules"
	for file in *.so; do
		echo "Rewriting dylib paths for $file..."
		fixlib "$file" "`pwd`"
	done
	)
	(cd "$package/Contents/Resources/lib/gtk-2.0/2.10.0/printbackends"
	for file in *.so; do
		echo "Rewriting dylib paths for $file..."
		fixlib "$file" "`pwd`"
	done
	)
	(cd "$package/Contents/Resources/lib/gnome-vfs-2.0/modules"
	for file in *.so; do
		echo "Rewriting dylib paths for $file..."
		fixlib "$file" "`pwd`"
	done
	)
	(cd "$package/Contents/Resources/lib/pango/1.6.0/modules"
	for file in *.so; do
		echo "Rewriting dylib paths for $file..."
		fixlib "$file" "`pwd`"
	done
	)
	(cd "$package/Contents/Resources/lib/ImageMagick-$IMAGEMAGICKVER/modules-Q16/filters"
	for file in *.so; do
		echo "Rewriting dylib paths for $file..."
		fixlib "$file" "`pwd`"
	done
	)
	(cd "$package/Contents/Resources/lib/ImageMagick-$IMAGEMAGICKVER/modules-Q16/coders"
	for file in *.so; do
		echo "Rewriting dylib paths for $file..."
		fixlib "$file" "`pwd`"
	done
	)
	(cd "$package/Contents/Resources/bin"
	for file in *; do
		echo "Rewriting dylib paths for $file..."
		fixlib "$file" "`pwd`"
	done
	cd ../lib
	for file in *.dylib; do
		echo "Rewriting dylib paths for $file..."
		fixlib "$file" "`pwd`"
	done
	)
}

PATHLENGTH=`echo $LIBPREFIX | wc -c`
if [ "$PATHLENGTH" -ge "50" ]; then
	# If the LIBPREFIX path is long enough to allow 
	# path rewriting, then do this.
	rewritelibpaths
else
	echo "Could not rewrite dylb paths for bundled libraries.  This requires" >&2
	echo "Macports to be installed in a PREFIX of at least 50 characters in length." >&2
	echo "" >&2
	echo "The package will still work if the following line is uncommented in" >&2
	echo "Inkscape.app/Contents/Resources/bin/inkscape:" >&2
	echo '        export DYLD_LIBRARY_PATH="$TOP/lib"' >&2
	exit 1

fi

exit 0
