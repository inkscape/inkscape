#!/bin/bash 

# This script does all the magic calls to automake/autoconf and
# friends that are needed to configure a cvs checkout.  As described in
# the file HACKING you need a couple of extra tools to run this script
# successfully.
#
# If you are compiling from a released tarball you don't need these
# tools and you shouldn't use this script.  Just call ./configure
# directly.


PROJECT="Inkscape"
TEST_TYPE=-f
FILE=inkscape.spec.in

AUTOCONF_REQUIRED_VERSION=2.52
AUTOMAKE_REQUIRED_VERSION=1.10
GLIB_REQUIRED_VERSION=2.0.0
INTLTOOL_REQUIRED_VERSION=0.17

srcdir=`dirname "$0"`
test -z "$srcdir" && srcdir=.
ORIGDIR=`pwd`
cd "$srcdir"

./tools-version.sh

check_version ()
{
MAJOR1=`echo "$1" | cut -d"." -f1`;
MINOR1=`echo "$1" | cut -s -d"." -f2`;
MAJOR2=`echo "$2" | cut -d"." -f1`;
MINOR2=`echo "$2" | cut -d"." -f2;`
test -z "$MINOR1" && MINOR1="0";

if [ "$MAJOR1" -gt "$MAJOR2" ] || [ "$MAJOR1" -eq "$MAJOR2" -a "$MINOR1" -ge "$MINOR2" ]; then
        echo "yes (version $1)"
    else
        echo "Too old (found version $1)!"
        DIE=1
    fi
}

attempt_command () {
    IGNORE=$1
    shift

    echo "Running $@ ..."
    ERR="`$@ 2>&1`"
    errcode=$?
    if [ "x$IGNORE" = "x" ]; then
        ERR=`echo "$ERR"`
    else
        ERR=`echo "$ERR" | egrep -v "$IGNORE"`
    fi
    if [ "x$ERR" != "x" ]; then
        echo "$ERR" | awk '{print "  " $0}'
    fi
    if [ $errcode -gt 0 ]; then
        echo "Please fix the error conditions and try again."
        exit 1
    fi
}

echo
echo "I am testing that you have the required versions of autoconf,"
echo "automake, glib-gettextize and intltoolize. This test is not foolproof and"
echo "if anything goes wrong, there may be guidance in the file HACKING.txt"
echo

DIE=0

echo -n "checking for autoconf >= $AUTOCONF_REQUIRED_VERSION ... "
if (autoconf --version) < /dev/null > /dev/null 2>&1; then
    VER=`autoconf --version \
         | grep -iw autoconf | sed -n 's/.* \([0-9.]*\)[-a-z0-9]*$/\1/p'`
    check_version "$VER" "$AUTOCONF_REQUIRED_VERSION"
else
    echo
    echo "  You must have autoconf installed to compile $PROJECT."
    echo "  Download the appropriate package for your distribution,"
    echo "  or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
    DIE=1;
fi

echo -n "checking for automake >= $AUTOMAKE_REQUIRED_VERSION ... "
# Prefer earlier versions just so that the earliest supported version gets test coverage by developers.
if (automake-1.11 --version) < /dev/null > /dev/null 2>&1; then
   AUTOMAKE=automake-1.11
   ACLOCAL=aclocal-1.11
elif (automake-1.10 --version) < /dev/null > /dev/null 2>&1; then
   AUTOMAKE=automake-1.10
   ACLOCAL=aclocal-1.10
elif (automake --version) < /dev/null > /dev/null 2>&1; then
   # Leave unversioned automake for a last resort: it may be a version earlier
   # than what we require.
   # (In particular, it might mean automake 1.4: that version didn't default to
   #  installing a versioned name.)
   AUTOMAKE=automake
   ACLOCAL=aclocal
else
    echo
    echo "  You must have automake 1.10 or newer installed to compile $PROJECT."
    DIE=1
fi
if test x$AUTOMAKE != x; then
    VER=`$AUTOMAKE --version \
         | grep automake | sed -n 's/.* \([0-9.]*\)[-a-z0-9]*$/\1/p'`
    check_version "$VER" "$AUTOMAKE_REQUIRED_VERSION"
fi

echo -n "checking for glib-gettextize >= $GLIB_REQUIRED_VERSION ... "
if (glib-gettextize --version) < /dev/null > /dev/null 2>&1; then
    VER=`glib-gettextize --version \
         | grep glib-gettextize | sed -n 's/.* \([0-9.]*\)/\1/p'`
    check_version "$VER" "$GLIB_REQUIRED_VERSION"
else
    echo
    echo "  You must have glib-gettextize installed to compile $PROJECT."
    echo "  glib-gettextize is part of glib-2.0, so you should already"
    echo "  have it. Make sure it is in your PATH."
    DIE=1
fi

echo -n "checking for intltool >= $INTLTOOL_REQUIRED_VERSION ... "
if (intltoolize --version) < /dev/null > /dev/null 2>&1; then
    VER=`intltoolize --version \
         | grep intltoolize | sed -n 's/.* \([0-9.]*\)/\1/p'`
    check_version "$VER" "$INTLTOOL_REQUIRED_VERSION"
else
    echo
    echo "  You must have intltool installed to compile $PROJECT."
    echo "  Get the latest version from"
    echo "  ftp://ftp.gnome.org/pub/GNOME/sources/intltool/"
    DIE=1
fi

if test "$DIE" -eq 1; then
    echo
    echo "Please install/upgrade the missing tools and call me again."
    echo	
    exit 1
fi


test $TEST_TYPE $FILE || {
    echo
    echo "You must run this script in the top-level $PROJECT directory."
    echo
    exit 1
}


if test -z "$ACLOCAL_FLAGS"; then

    acdir=`$ACLOCAL --print-ac-dir`
    m4list="glib-2.0.m4 glib-gettext.m4 gtk-2.0.m4 intltool.m4 pkg.m4 libtool.m4"

    for file in $m4list
    do
    if [ ! -f "$acdir/$file" ]; then
        echo
        echo "WARNING: aclocal's directory is $acdir, but..."
            echo "         no file $acdir/$file"
            echo "         You may see fatal macro warnings below."
            echo "         If these files are installed in /some/dir, set the ACLOCAL_FLAGS "
            echo "         environment variable to \"-I /some/dir\", or install"
            echo "         $acdir/$file."
            echo
        fi
    done
fi

echo ""

attempt_command 'underquoted definition of|[\)\#]Extending' \
	$ACLOCAL $ACLOCAL_FLAGS

# optionally feature autoheader
(autoheader --version)  < /dev/null > /dev/null 2>&1 && {
	attempt_command '' autoheader
}

attempt_command '' libtoolize
attempt_command '' $AUTOMAKE --copy --force --add-missing
attempt_command '' autoconf
attempt_command '^(Please add the files|  codeset|  progtest|from the|or directly|You will also|ftp://ftp.gnu.org|$)' \
	glib-gettextize --copy --force
attempt_command '' intltoolize --copy --force --automake

echo ""
echo "Done!  Please run './configure' now."
