#! /bin/sh

TMPDIR="${TMPDIR-/tmp}"
TEMPFILENAME=`mktemp 2>/dev/null || echo "$TMPDIR/tmp-ps-$$.epsi"`
ps2epsi "$1" "${TEMPFILENAME}" > /dev/null 2>&1
rc=0
cat < "${TEMPFILENAME}" || rc=1
rm -f "${TEMPFILENAME}"
exit $rc
