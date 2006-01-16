#! /bin/sh
# delautogen.sh: Remove files created by autogen.sh, and files created by make
# if compiling in the source directory.
#
# Requires gnu find, gnu xargs.

set -e
mydir=`dirname "$0"`
cd "$mydir"
rm -rf autom4te.cache
rm -rf src/.libs
for d in `find -name .cvsignore -printf '%h '`; do
	(cd "$d" && rm -f *~ && rm -rf .deps && rm -f $(grep '^[][a-zA-Z0-9_.*?-]*$' .cvsignore) )
done
