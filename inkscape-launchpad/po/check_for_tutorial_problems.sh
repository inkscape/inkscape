#!/bin/bash
# check the main Inkscape tree for tutorial-related problems
# $1: full name of the root directory of the local copy of the main Inkscape tree
# usage example:
#   check_for_tutorial_problems.sh /tmp/inkscape

LANG=C
LC_ALL=C

# First pass to detect invalid links to installed tutorials (trailing
# space, typo...).
echo "===== Checking tutorials links in PO files ======"
PO_FILE_LIST=`ls -1 "$1/po/" | grep "\.po$" | sort`

echo "$PO_FILE_LIST" |\
while read PO_FILENAME; do
  echo "----- $PO_FILENAME -----------------------------------"
  TRANSLATIONS_IN_CURRENT_PO=`cat "$1/po/$PO_FILENAME" | grep -A 1 "^msgid\ \"tutorial-[^.]*\.svg\""`
  UNTRANSLATED_COUNT=`echo "$TRANSLATIONS_IN_CURRENT_PO" | grep -c "^msgstr \"\""`
  if [ $UNTRANSLATED_COUNT -gt 0 ]; then
    echo "$PO_FILENAME has $UNTRANSLATED_COUNT untranslated tutorial filenames"
  fi
  echo "$TRANSLATIONS_IN_CURRENT_PO" | grep -v "^msgstr \"\"" |\
  grep "^msgstr \"" |\
  while read TUTORIAL_FILENAME_TRANSLATION_LINE; do
    TUTORIAL_FILENAME_TRANSLATION=`echo "$TUTORIAL_FILENAME_TRANSLATION_LINE" |\
    sed 's/^msgstr \"\(.*\)\"[ 	]*$/\1/'`
    if [ ! -e "$1/share/tutorials/$TUTORIAL_FILENAME_TRANSLATION" ]; then
      echo -n "$PO_FILENAME references \"$TUTORIAL_FILENAME_TRANSLATION\""
      echo "			ERROR: THE REFERENCED FILE DOESN'T EXIST, PLEASE CHECK"
    fi
  done
  echo
done

# Second pass to check if a tutorial for a language exists but is not
# linked in the PO file.
echo
echo "===== Checking tutorials not referenced in PO files ======"
TUTORIAL_FILE_LIST=`ls -1 "$1/share/tutorials/" | grep "^tutorial-[^.]*\...\.svg$" | sort`

echo "$TUTORIAL_FILE_LIST" |\
while read TUTORIAL_FILENAME; do
  LANGUAGE_CODE=`echo "$TUTORIAL_FILENAME" | sed 's/^tutorial-[^.]*\.\(..\)\.svg$/\1/'`
  if [ -e "$1/po/$LANGUAGE_CODE.po" ]; then
    TRANSLATIONS_IN_CURRENT_PO=`cat "$1/po/$LANGUAGE_CODE.po" | grep -A 1 "^msgid\ \"tutorial-[^.]*\.svg\""`
    echo "$TRANSLATIONS_IN_CURRENT_PO" | grep -q "^msgstr \"$TUTORIAL_FILENAME\""
    if [ $? -ne 0 ]; then
      echo "WARNING: $TUTORIAL_FILENAME IS NOT REFERENCED IN $LANGUAGE_CODE.po"
    fi
  else
    echo "WARNING: \"$LANGUAGE_CODE.po\" FILE NOT FOUND"
  fi
done

# Last pass to check if tutorials are correctly added in Makefile.am
echo
echo "===== Checking Makefile.am ======"
TUTORIAL_DIRECTORY_FILELIST=`ls -1 "$1/share/tutorials/" | grep -v "^Makefile.am$" | sort`

echo "$TUTORIAL_DIRECTORY_FILELIST" |\
while read TUTORIAL_DIRECTORY_FILENAME; do
  cat "$1/share/tutorials/Makefile.am" | grep -q "^    $TUTORIAL_DIRECTORY_FILENAME "
  if [ $? -ne 0 ]; then
    cat "$1/share/tutorials/Makefile.am" | grep -q "^    $TUTORIAL_DIRECTORY_FILENAME$"
    if [ $? -ne 0 ]; then
      echo "WARNING: $TUTORIAL_DIRECTORY_FILENAME IS NOT IN Makefile.am"
    fi
  fi
done
