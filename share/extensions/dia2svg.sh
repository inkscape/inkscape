#!/bin/sh

rc=0

# dia version 0.93 (the only version I've tested) allows `--export=-', but then
# ruins it by writing other cruft to stdout.  So we'll have to use a temp file.
# dia 0.95 removes --export-to-format but still allows -t.
TMPDIR="${TMPDIR-/tmp}"
TEMPFILENAME=`mktemp 2>/dev/null || echo "$TMPDIR/tmpdia$$.svg"`
dia -n --export="${TEMPFILENAME}" -t svg "$1" > /dev/null 2>&1 || rc=1

cat < "${TEMPFILENAME}" || rc=1
rm -f "${TEMPFILENAME}"
exit $rc
