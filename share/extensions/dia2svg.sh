#! /bin/sh

rc=0

# dia version 0.93 (the only version I've tested) allows `--export=-', but then
# ruins it by writing other cruft to stdout.  So we'll have to use a temp file.
TMPDIR="${TMPDIR-/tmp}"
TEMPFILENAME=`mktemp 2>/dev/null || echo "$TMPDIR/tmpdia$$.svg"`
dia -n --export="${TEMPFILENAME}" --export-to-format=svg "$1" > /dev/null 2>&1 || rc=1

cat < "${TEMPFILENAME}" || rc=1
rm -f "${TEMPFILENAME}"
exit $rc
