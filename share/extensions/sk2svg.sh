#!/bin/sh

rc=0

TMPDIR="${TMPDIR-/tmp}"
UNIQTMPDIR=`mktemp -d 2>/dev/null || (mkdir "$TMPDIR/sk2svg.$$" && echo "$TMPDIR/sk2svg.$$") || echo "$TMPDIR"`
TMPSVG="$UNIQTMPDIR/sk2svg$$.svg"
skconvert "$1" "$TMPSVG" > /dev/null 2>&1 || rc=1
cat < "$TMPSVG" || RC=1
rm -f "$TMPSVG"
[ "$UNIQTMPDIR" = "$TMPDIR" ] || rmdir "$UNIQTMPDIR"
exit $rc
