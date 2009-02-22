#!/bin/bash
# Updates PO files from the current POT file
# Please run this program from the po/ directory
# Existing po.old files will be overwritten

type -p intltool-update >/dev/null 2>/dev/null
if [ "$?" -eq "0" ]; then
  intltool-update --pot >/dev/null 2>&1
  POT_FILENAME="`ls -1rt *.pot 2>/dev/null|tail -1 2>/dev/null`"
  if [ ! -r "$POT_FILENAME" ]; then
    echo "Could not create POT file. Exiting."
    exit
  fi
else
  echo "Could not create POT file (intltool-update not found). Exiting."
  exit
fi

PO_FILE_COUNT=0

find . -noleaf -type f -name "*.po"|sort|\
(
while read FILENAME; do
  PO_FILE_COUNT=`expr $PO_FILE_COUNT + 1`
  mv -f "$FILENAME" "$FILENAME".old     # do not ask questions, because the answers would come from the pipe
  if [ "$?" -eq "0" ]; then
    echo "$FILENAME"
    msgmerge "$FILENAME".old "$POT_FILENAME" > "$FILENAME"
    if [ "$?" -ne "0" ]; then
      echo "Could not merge \"$FILENAME.old\"."
    fi
  else
    echo "Could not rename \"$FILENAME\". File skipped."
  fi
done

echo; echo "Total number of PO files: $PO_FILE_COUNT"
)

echo
