#! /bin/bash
#
# Usage: osx-app [-s] /path/to/bin/inkscape Info.plist
#
# This script attempts to build an Inkscape.app package for OS X, resolving
# dynamic libraries, etc.  It strips the executable and libraries if
# '-s' is given.
#
#
# Authors:
#    Kees Cook <kees@outflux.net>
#    Michael Wybrow <mjwybrow@users.sourceforge.net>
#    Jean-Olivier Irisson <jo.irisson@gmail.com>
# 
# Copyright (C) 2005 Kees Cook
# Copyright (C) 2005-2007 Michael Wybrow
# Copyright (C) 2007 Jean-Olivier Irisson
#
#
# Released under GNU GPL, read the file 'COPYING' for more information
#
# Thanks to GNUnet's "build_app" script for help with library dep resolution.
# https://gnunet.org/svn/GNUnet/contrib/OSX/build_app
# 
#
# Notes:
# The Info.plist file can be found in the base inkscape directory once
# configure has been run.
#
# When packaging Inkscape for OS X, configure should be run with the 
# "--enable-osxapp" option which sets the correct paths for support
# files inside the app bundle.
# 
# Thus, the usual use of this file would be to run it from the within the
# inkscape/packaging/macosx directory, substituting in the inkscape binary path:
# 	./osx-app.sh /path/to/bin/inkscape ../../Info.plist



# Handle some version specific details.
VERSION=`/usr/bin/sw_vers | grep ProductVersion | cut -f2 -d'.'`
if [ "$VERSION" -ge "4" ]; then
  # We're on Tiger (10.4) or later.
  # XCode behaves a little differently in Tiger and later.
  XCODEFLAGS="-configuration Deployment"
  SCRIPTEXECDIR="ScriptExec/build/Deployment/ScriptExec.app/Contents/MacOS"
  # libXinerama.1.dylib is not installed as part of X11 on Panther but
  # is introduced as a dependency if Inkscape is compiled on Tiger or
  # later.  Thus, add the library to the bundle for Panther users
  # Building on Tiger gives app bundles incompatible with Panther anyway
  # so we do not need to add this now
  # EXTRALIBS="/usr/X11R6/lib/libXinerama.1.dylib"
  EXTRALIBS=""
else
  # Panther (10.3) or earlier.
  XCODEFLAGS="-buildstyle Deployment"
  SCRIPTEXECDIR="ScriptExec/build/ScriptExec.app/Contents/MacOS"
  EXTRALIBS=""
fi

# set LIBPREFIX only if it has not been set before 
# (by osx-build.sh for example)
if [ -z $LIBPREFIX ]
then
	export LIBPREFIX="$HOME/ws-fat"
fi


# Package always has the same name.  Version information is stored in
# the Info.plist file which is filled in by the configure script.
pkg=Inkscape
package="$pkg.app"

# Remove a previously existing package
if [ -d $package ]; then
	echo "Removing previous Inkscape.app"
	rm -Rf $package
fi

strip=false
while getopts 's' flag; do
	case $flag in
		s)  strip=true ;;
		\?) echo "Invalid command line option."; exit 1 ;;
		*)  echo "Internal error in getopts."; exit 2 ;;
	esac
done
shift `expr $OPTIND - 1`;


binary="$1"
if [ ! -x "$binary" ]; then
        echo "Not executable: $binary" >&2
        exit 1
fi

plist="$2"
if [ ! -f "$plist" ]; then
	echo "Need plist file" >&2
	exit 1
fi

# Set the 'macosx' directory, usually the current directory.
resdir=`pwd`

# Fix a given executable or library to be relocatable
fixlib () {
if [ ! -d "$1" ]; then
  echo $1
  libs="`otool -L $1 | fgrep compatibility | cut -d\( -f1`"
  for lib in $libs; do
    echo "  $lib"
    base=`echo $lib | awk -F/ '{print $NF}'`
    first=`echo $lib | cut -d/ -f1-3`
    to=@executable_path/../lib/$base
    if [ $first != /usr/lib -a $first != /usr/X11R6 ]; then
      /usr/bin/install_name_tool -change $lib $to $1
      if [ "`echo $lib | fgrep libcrypto`" = "" ]; then
        /usr/bin/install_name_tool -id $to ../lib/$base
        for ll in $libs; do
          base=`echo $ll | awk -F/ '{print $NF}'`
          first=`echo $ll | cut -d/ -f1-3`
          to=@executable_path/../lib/$base
          if [ $first != /usr/lib -a $first != /usr/X11R6 -a "`echo $ll | fgrep libcrypto`" = "" ]; then
            /usr/bin/install_name_tool -change $ll $to ../lib/$base
          fi
        done
      fi
    fi
  done
fi
}



mkdir -p "$package"/Contents/MacOS
mkdir -p "$package"/Contents/Resources/bin
mkdir -p "$package"/Contents/Resources/lib
mkdir -p "$package"/Contents/Resources/locale

binname=`basename "$binary"`
binpath="$package/Contents/Resources/bin/inkscape-bin"

cp "$binary" "$binpath"


# Build and add the launcher.
(
  # Build fails if CC happens to be set (to anything other than CompileC)
  unset CC
  
  cd "$resdir/ScriptExec"
  xcodebuild $XCODEFLAGS clean build
)
cp "$resdir/$SCRIPTEXECDIR/ScriptExec" "$package/Contents/MacOS/Inkscape"

# Pull down all the share files
binary_dir=`dirname "$binary"`
rsync -av "$binary_dir/../share/$binname"/* "$package/Contents/Resources/"
cp "$plist" "$package/Contents/Info.plist"
rsync -av "$binary_dir/../share/locale"/* "$package/Contents/Resources/locale"

# PkgInfo must match bundle type and creator code from Info.plist
echo "APPLInks" > $package/Contents/PkgInfo

# Pull in extra requirements.
pkgetc="$package/Contents/Resources/etc"
mkdir -p $pkgetc/pango
cp $LIBPREFIX/etc/pango/pangox.aliases $pkgetc/pango/
# Need to adjust path and quote incase of spaces in path.
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


pkglib="$package/Contents/Resources/lib"
pangover=`ls $LIBPREFIX/lib/pango/`
mkdir -p $pkglib/pango/$pangover/modules
cp $LIBPREFIX/lib/pango/$pangover/modules/*.so $pkglib/pango/$pangover/modules/

gtkver=`ls $LIBPREFIX/lib/gtk-2.0/ | grep "2\."`
mkdir -p $pkglib/gtk-2.0/$gtkver/{engines,immodules,loaders}
cp -r $LIBPREFIX/lib/gtk-2.0/$gtkver/engines/* $pkglib/gtk-2.0/$gtkver/engines/
cp $LIBPREFIX/lib/gtk-2.0/$gtkver/immodules/*.so $pkglib/gtk-2.0/$gtkver/immodules/
cp $LIBPREFIX/lib/gtk-2.0/$gtkver/loaders/*.so $pkglib/gtk-2.0/$gtkver/loaders/

mkdir -p $pkglib/gnome-vfs-2.0/modules
cp $LIBPREFIX/lib/gnome-vfs-2.0/modules/*.so $pkglib/gnome-vfs-2.0/modules/

# Find out libs we need from fink, darwinports, or from a custom install
# (e.g. $LIBPREFIX), then loop until no changes.
a=1
nfiles=0
endl=true
while $endl; do
  echo "Looking for dependencies. Round " $a
  libs="`otool -L $pkglib/gtk-2.0/$gtkver/loaders/* $pkglib/gtk-2.0/$gtkver/immodules/* $pkglib/gtk-2.0/$gtkver/engines/*.so $pkglib/pango/$pangover/modules/* $pkglib/gnome-vfs-2.0/modules/* $package/Contents/Resources/lib/* $binary 2>/dev/null | fgrep compatibility | cut -d\( -f1 | grep $LIBPREFIX | sort | uniq`"
  cp -f $libs $package/Contents/Resources/lib
  let "a+=1"  
  nnfiles=`ls $package/Contents/Resources/lib | wc -l`
  if [ $nnfiles = $nfiles ]; then
    endl=false
  else
    nfiles=$nnfiles
  fi
done

for libfile in $EXTRALIBS
do
  cp -f $libfile $package/Contents/Resources/lib
done

if [ "$strip" = "true" ]; then
  chmod +w "$package"/Contents/Resources/lib/*.dylib
  strip -x "$package"/Contents/Resources/lib/*.dylib
  strip -ur "$binpath"
fi

# NOTE: This works for all the dylibs but causes GTK to crash at startup.
#       Instead we leave them with their original install_names and set
#       DYLD_LIBRARY_PATH within the app bundle before running Inkscape.
#
# Fix package deps
#(cd "$package/Contents/MacOS/bin"
# for file in *; do
#    fixlib "$file"
# done
# cd ../lib
# for file in *; do
#    fixlib "$file"
# done)

# Get all the icons and the rest of the script framework
rsync -av --exclude ".svn" $resdir/Resources/* $package/Contents/Resources/

exit 0
