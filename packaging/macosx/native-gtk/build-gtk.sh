#!/bin/sh
#
# Based on the Imendio 'build-gtk.sh' script.
#
# Inkscape build additions by Michael Wybrow <mjwybrow@users.sf.net>
#
# See the following page for build instructions:
# http://developer.imendio.com/projects/gtk-macosx/build-instructions
#
# Usage:
# export PREFIX=/your/install/prefix
# ./build-gtk bootstrap
# ./build-gtk build inkscape
#


version=1.2-inkscape

export PREFIX=${PREFIX-/opt/gtk}
export PATH=$PREFIX/bin:$PATH
#export PATH=$PREFIX/bin:/bin:/sbin:/usr/bin:/usr/sbin:/usr/X11R6/bin:
export LIBTOOLIZE=$PREFIX/bin/libtoolize

# FIXME: We might need some more intelligent way to get the path here.
export PYTHONPATH=$PREFIX/lib/python2.3/site-packages

# Needed for glib etc to pick up gettext
export LDFLAGS=-L$PREFIX/lib
export CPPFLAGS=-I$PREFIX/include

export XDG_DATA_DIRS=$PREFIX/share

# Support install-check from jhbuild to speed up compilation
if [ -x $PREFIX/bin/install-check ]; then
    export INSTALL=$PREFIX/bin/install-check
fi

COMMON_OPTIONS="--prefix=$PREFIX --disable-static --enable-shared \
--disable-gtk-doc --disable-scrollkeeper"

SOURCE=${SOURCE-$HOME/Source/gtk}
GNOMECVSROOT=${GNOMECVSROOT-:pserver:anonymous@anoncvs.gnome.org:/cvs/gnome}
CAIROCVSROOT=${CAIROCVSROOT-:pserver:anoncvs@cvs.freedesktop.org:/cvs/cairo}
INKSCAPEREPURL="https://svn.sourceforge.net/svnroot/inkscape/inkscape/trunk"

if [ x$1 = xrun ]; then
    cmd="$2"
    shift 2
    exec $cmd $*
fi

if [ $# -eq 0 -o "x`echo "$*" | grep shell`" = xshell ]; then
    # Can be used in .bashrc to set a fancy prompt...
    export INSIDE_GTK_BUILD=1
    bash
    exit 0
fi

CORE_MODULES="cairo gnome-common glib pango atk gtk+"
EXTRA_MODULES="libxml2 libxslt loudmouth libglade gossip gtk-engines"
PYGTK_MODULES=" pycairo pygobject pygtk"
INKSCAPE_MODULES="$CORE_MODULES libxml2 libxslt gc lcms libsig++ doxygen glibmm cairomm gtkmm popt inkscape"

# Could add those (orbit requires popt though)
MORE_MODULES="libIDL ORBit2 gconf"

function print_usage
{
    echo
    echo "GTK+ on Mac OS X build script version $version."
    echo 
    echo "Usage:"
    echo "`basename $0` [bootstrap|[shell]|run <cmd>|build [<modules>]], modules are:"
    echo " Core: $CORE_MODULES"
    echo " Extra: $EXTRA_MODULES"
    echo " Python: $PYGTK_MODULES"
    echo " Inkscape: $INKSCAPE_MODULES"
    echo
    echo "Setup: This script defaults to downloading source to ~/Source/gtk and"
    echo "installing in /opt/gtk. Make sure your user has write access to the"
    echo "latter directory. You can override those directories by setting the"
    echo "SOURCE and PREFIX environment variables. Anoncvs is used by default"
    echo "for access to GNOME CVS, if you wish to override, set the environment"
    echo "variable GNOMECVSROOT to your own account."
    echo
    echo "While in the shell that this script provides, the environment variable"
    echo "INSIDE_GTK_BUILD is set, which makes it possible to put something like"
    echo "the following in ~/.bashrc:"
    echo
    echo " if [ x\$INSIDE_GTK_BUILD != x ]; then"
    echo "     PS1=\"[GTK] \u@\h \W $ \""
    echo " fi"
    echo
    echo "Start by bootstrapping. This will install the necessary build tools."
    echo "Then build GTK+ & co by using the \"build\" command. If no modules are"
    echo "specified, only the ones needed for GTK+ will be built. The special"
    echo "modules \"core\" and \"all\" can be used to build just the core or all"
    echo "modules."
    echo
    echo "If you want to build something manually or run something, use the "
    echo "\"shell\" command (or no command) to get a shell with the environment"
    echo "properly setup."
    echo 
    echo "Tip: if you build and install \"install-check\" from jhbuild into your"
    echo "PREFIX, recompiling when hacking on GTK+ & co will be a lot faster."
    echo
}

function download
{
    BASENAME=`basename $1`

    if [ -s $BASENAME ]; then
	echo "Already downloaded"
	return 0
    fi

    curl $1 > $BASENAME || return 1

    return 0
}

function should_build
{
    if [ -f $1/BUILT ]; then
	echo "Already built"
	return 1
    fi

    return 0
}

function tarball_get_and_build
{
    BASENAME=`basename $1`
    DIRNAME=`echo $BASENAME | sed -e s,.src.,., | sed -e s,.tar.*,,`
    INSTCMD="make install"
    PREFIXARG="--prefix=$PREFIX"
    COMMONOPTS="$COMMON_OPTIONS"
    
    echo
    echo "Building $DIRNAME"
    echo -ne "\033]0;Building $DIRNAME\007"
    
    # Special case jpeg... :/
    if [ x`echo $DIRNAME | grep jpeg` != x ]; then
	INSTCMD="make install-lib"
    fi
    
    if [ x`echo $BASENAME | grep bz2` != x ]; then
	COMP="j"
    else
	COMP="z"
    fi
    
    # Doxygen doesn't have a standard configure script.
    if [ x`echo $BASENAME | grep doxygen` != x ]; then
	PREFIXARG="--prefix $PREFIX"
    	COMMONOPTS="--shared"
    fi
    
    cd $SOURCE || return 1
    download $1 || return 1
    should_build $DIRNAME || return 0
    tar ${COMP}xf $BASENAME && \
	cd $DIRNAME && \
	echo "./configure $PREFIXARG $COMMONOPTS $2" && \
	./configure $PREFIXARG $COMMONOPTS $2 && make && $INSTCMD && touch BUILT
}

function cpan_get_and_build
{
    BASENAME=`basename $1`
    DIRNAME=`echo $BASENAME | sed -e s,.tar.*,,`
    
    echo
    echo "Building $DIRNAME"
    echo -ne "\033]0;Building $DIRNAME\007"
    
    if [ x`echo $BASENAME | grep bz2` != x ]; then
	COMP="j"
    else
	COMP="z"
    fi
    
    cd $SOURCE || return 1
    download $1 || return 1
    should_build $DIRNAME || return 0
    tar ${COMP}xf $BASENAME && \
	cd $DIRNAME && \
	perl Makefile.PL $2 && \
	make && \
	(echo "Enter your password to istall $BASENAME"; make install) && \
	touch BUILT
}

function git_get_and_build
{
    if !(echo "$MODULES" | grep -w $2) >/dev/null; then
	return 0
    fi
    
    echo
    echo "Building $2"
    echo -ne "\033]0;Building $2\007"
    
    cd $SOURCE
    if [ -d $2 ]; then
	cd $2
	cg-update || return
    else
	cg-clone $1/$2 || return
	cd $2
    fi
    
    echo "./autogen.sh $COMMON_OPTIONS $3"
    (./autogen.sh $COMMON_OPTIONS $3 && make && make install)
}

function cvs_get_and_build
{
    if !(echo "$MODULES" | grep -w $2) >/dev/null; then
	return 0
    fi
    
    echo
    echo "Building $2"
    echo -ne "\033]0;Building $2\007"
    
    cd $SOURCE
    if [ -d $2 ]; then
	cd $2
	cvs up -dP || return
    else
	cvs -d $1 co -P $2 || return
	cd $2
    fi
    
    echo "./autogen.sh $COMMON_OPTIONS $3"
    (./autogen.sh $COMMON_OPTIONS $3 && make && make install)
}

function svn_get_and_build
{
    if !(echo "$MODULES" | grep -w $2) >/dev/null; then
	return 0
    fi
    
    echo
    echo "Building $2"
    echo -ne "\033]0;Building $2\007"
    
    cd $SOURCE
    if [ -d $2 ]; then
	cd $2
	svn up || return
    else
	svn co $1 $2 || return
	cd $2
    fi
    
    echo "./autogen.sh $COMMON_OPTIONS $3"
    (./autogen.sh $COMMON_OPTIONS $3 && ./configure --prefix=$PREFIX $COMMON_OPTIONS $3 && make && make install)
}

function set_automake
{
    old_AUTOMAKE=$AUTOMAKE
    old_ACLOCAL=$ACLOCAL

    export AUTOMAKE=automake-$1
    export ACLOCAL=aclocal-$1
}

function restore_automake
{
    if [ x$old_AUTOMAKE != x ]; then
	export AUTOMAKE=$old_AUTOMAKE
    else
	unset AUTOMAKE
    fi

    if [ x$old_ACLOCAL != x ]; then
	export ACLOCAL=$old_ACLOCAL
    else
	unset ACLOCAL
    fi
}

function do_exit
{
    echo -ne "\033]0;\007"
    exit
}

# Make sure to restore the title when done.
trap do_exit EXIT SIGINT SIGTERM

if (echo "$*" | grep bootstrap) >/dev/null; then
    if [ "x`cg-version 2>/dev/null`" == "x" ]; then
	echo "You need the cogito to get cairo from git. It's available e.g. in Darwin ports."
	exit 1
    fi
    if [ "x`which svn 2>/dev/null`" == "x" ]; then
	echo "You need the svn client to get inkscape."
	exit 1
    fi
    
    mkdir -p $SOURCE 2>/dev/null || (echo "Error: Couldn't create source checkout dir $SOURCE"; exit 1)
    mkdir -p $PREFIX/bin 2>/dev/null || (echo "Error: Couldn't create bin dir $PREFIX/bin"; exit 1)
    
    echo "Building bootstrap packages."
    
    PACKAGES=" \
	http://pkgconfig.freedesktop.org/releases/pkg-config-0.20.tar.gz \
	http://ftp.gnu.org/gnu/libtool/libtool-1.5.22.tar.gz \
	http://ftp.gnu.org/gnu/autoconf/autoconf-2.59.tar.bz2 \
	http://ftp.gnu.org/pub/gnu/automake/automake-1.7.9.tar.bz2 \
	http://ftp.gnu.org/gnu/automake/automake-1.9.6.tar.bz2 \
	http://heanet.dl.sourceforge.net/sourceforge/libpng/libpng-1.2.12.tar.bz2 \
	ftp://ftp.remotesensing.org/pub/libtiff/tiff-3.8.2.tar.gz \
	http://people.imendio.com/richard/gtk-osx/files/jpeg-6b.tar.gz \
	http://ftp.gnu.org/gnu/gettext/gettext-0.14.5.tar.gz \
	http://heanet.dl.sourceforge.net/sourceforge/expat/expat-2.0.0.tar.gz \
	http://heanet.dl.sourceforge.net/sourceforge/freetype/freetype-2.1.10.tar.bz2 \
	http://fontconfig.org/release/fontconfig-2.3.2.tar.gz \
	http://people.imendio.com/richard/gtk-osx/files/docbook-files-1.tar.gz \
	http://www.cs.mu.oz.au/~mjwybrow/gtk-osx/gnome-doc-utils-fake-1.tar.gz \
	"

	#http://people.imendio.com/richard/gtk-osx/files/popt-1.7.tar.gz
    
    for PACKAGE in $PACKAGES; do
	tarball_get_and_build $PACKAGE || exit 1
    done
    
    PACKAGE=http://ftp.gnome.org/pub/GNOME/sources/gtk-doc/1.6/gtk-doc-1.6.tar.bz2
    tarball_get_and_build $PACKAGE "--with-xml-catalog=$PREFIX/etc/xml/catalog" || exit 1
    
    PACKAGE=ftp://ftp4.freebsd.org/pub/FreeBSD/ports/distfiles/XML-Parser-2.34.tar.gz
    cpan_get_and_build $PACKAGE "PREFIX=$PREFIX INSTALLDIRS=perl EXPATLIBPATH=$PREFIX/lib EXPATINCPATH=$PREFIX/include" || exit 1

    PACKAGES=" \
	http://ftp.gnome.org/pub/GNOME/sources/intltool/0.35/intltool-0.35.0.tar.bz2 \
	http://icon-theme.freedesktop.org/releases/hicolor-icon-theme-0.9.tar.gz \
	http://ftp.gnome.org/pub/GNOME/sources/gnome-icon-theme/2.14/gnome-icon-theme-2.14.2.tar.bz2 \
	"
    
    for PACKAGE in $PACKAGES; do
	tarball_get_and_build $PACKAGE || exit 1
    done
    
    # Setup glibtool* links since some stuff expects them to be named like that on OSX
    if [ -z $PREFIX/bin/glibtoolize ]; then
	ln -s $PREFIX/bin/libtoolize $PREFIX/bin/glibtoolize
	ln -s $PREFIX/bin/libtool $PREFIX/bin/glibtool
    fi

    echo
    echo "Done bootstrapping. Continue with \"build\" or \"shell\"."
    exit 0
fi

if [ "x$1" != xbuild ]; then
    print_usage
    exit 1
fi

shift

MODULES=$*
if [ $# -eq 0 ]; then
    echo "Building core modules."
    MODULES="$CORE_MODULES"
elif [ "x$1" = xcore ]; then
    echo "Building core modules."
    MODULES="$CORE_MODULES"
elif [ "x$1" = xpython ]; then
    echo "Building python modules."
    MODULES="$PYGTK_MODULES"
elif [ "x$1" = xall ]; then
    echo "Building core+extra+python modules."
    MODULES="$CORE_MODULES $EXTRA_MODULES $PYGTK_MODULES"
elif [ "x$1" = xinkscape ]; then
    echo "Building inkscape modules."
    MODULES="$INKSCAPE_MODULES"
fi

git_get_and_build git://git.cairographics.org/git cairo "--enable-pdf --enable-atsui --enable-quartz --disable-xlib" || exit 1

tarball_get_and_build http://www.hpl.hp.com/personal/Hans_Boehm/gc/gc_source/gc6.7.tar.gz || exit 1
tarball_get_and_build http://www.littlecms.com/lcms-1.15.tar.gz || exit 1
tarball_get_and_build ftp://ftp.gnome.org/mirror/gnome.org/sources/libsigc++/2.0/libsigc++-2.0.17.tar.gz || exit 1
tarball_get_and_build http://ftp.stack.nl/pub/users/dimitri/doxygen-1.5.1.src.tar.gz || exit 1
tarball_get_and_build ftp://ftp.rpm.org/pub/rpm/dist/rpm-4.1.x/popt-1.7.tar.gz || exit 1

cvs_get_and_build $GNOMECVSROOT libxml2 || exit 1
cvs_get_and_build $GNOMECVSROOT libxslt || exit 1
cvs_get_and_build $GNOMECVSROOT gnome-common || exit 1
cvs_get_and_build $GNOMECVSROOT glib || exit 1
cvs_get_and_build $GNOMECVSROOT atk || exit 1
cvs_get_and_build $GNOMECVSROOT pango "--without-x" || exit 1
cvs_get_and_build $GNOMECVSROOT gtk+ "--with-gdktarget=quartz" || exit 1
cvs_get_and_build $GNOMECVSROOT gtk-engines || exit 1
cvs_get_and_build $GNOMECVSROOT loudmouth "--with-ssl=openssl" || exit 1
cvs_get_and_build $GNOMECVSROOT libglade || exit 1
# gossip needs xml2po from gnome-doc-utils.
cvs_get_and_build $GNOMECVSROOT gossip "--with-backend=cocoa" || exit 1
cvs_get_and_build $CAIROCVSROOT pycairo || exit 1
cvs_get_and_build $GNOMECVSROOT pygobject "--disable-docs" || exit 1
cvs_get_and_build $GNOMECVSROOT pygtk "--disable-docs" || exit 1

cvs_get_and_build $GNOMECVSROOT glibmm "--disable-docs --disable-fulldocs" || exit 1
cvs_get_and_build $CAIROCVSROOT cairomm || exit 1
cvs_get_and_build $GNOMECVSROOT gtkmm "--disable-docs --disable-examples --disable-demos" || exit 1

svn_get_and_build $INKSCAPEREPURL inkscape || exit 1

#cvs_get_and_build $GNOMECVSROOT gimp || exit 1
# For gimp:
# libart_lgpl, needs automake 1.4 and doesn't run libtoolize
# gtkhtml2 (optional)
# libpoppler (optional)
# ./autogen.sh --prefix=/opt/gimp --disable-gtk-doc 

echo "Done."
