#!/bin/sh
# Report the version of distro and tools building inkscape
#
# You can get the latest distro command from 
# distro web page: http://distro.pipfield.ca/ 

# Please add a tool you want to check
TOOLS="m4 autoconf autoheader automake automake-1.7 automake-1.8 automake-1.9 aclocal aclocal-1.7 aclocal-1.8 aclocal-1.9 intltoolize gettextize "
ENVPATTERN='PATH\|FLAGS\|LANG'

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

echo '============================================================================='
echo 'When you report a trouble about building BZR version of inkscape,            '
echo 'Please report following information about distro and tools version, too.     '
echo 
(echo '--1. distribution------------------------------------------------------------'
$srcdir/distro -a
echo )

(echo '--2. tools-------------------------------------------------------------------'
for x in $TOOLS; do 
    loc=`which $x 2>/dev/null`
    if [ -z "$loc" ]; then
        echo "$x: not found"
    else 
        echo -n "$loc: "
        y=`echo $x | cut -f1 -d-`
        $x --version </dev/null | grep $y
    fi
done 
echo )

(echo '--3. environment variables---------------------------------------------------'
env | grep -e $ENVPATTERN
echo )
echo '============================================================================='
