#!/bin/bash
# -*- mode: sh; sh-basic-offset: 4; indent-tabs-mode: nil; -*-
# vim: set filetype=sh sw=4 sts=4 expandtab autoindent:

set -eu
#set -o pipefail
LANGUAGES="$(grep -v "^\#" ./LINGUAS)"
#LANGUAGES=${@:-am ar ko}
#echo $LANGUAGES


count_msgids () {
    cat | grep -E '^msgid\s+' | wc -l
}

count_original_words () {
    cat | grep ^msgid | sed 's/^msgid "//g;s/"$//g' | wc -w
}

statistics () {
    PO_MESSAGES="$(mktemp -t XXXXXXXXXX.po)"
    msgcat --files-from=$PO_FILES --output=$PO_MESSAGES
    REV_DATE=$(grep "PO-Revision-Date: [1-9]" $PO_MESSAGES |cut -d " " -f2)
    TOTAL=$(msgattrib --force-po --no-obsolete $PO_MESSAGES | count_msgids)
    TOTAL_WC=$(msgattrib --force-po --no-obsolete --no-wrap $PO_MESSAGES | count_original_words)
    FUZZY=$(msgattrib --force-po --only-fuzzy --no-obsolete $PO_MESSAGES | count_msgids)
    TRANSLATED=$(msgattrib --force-po --translated --no-fuzzy --no-obsolete $PO_MESSAGES | count_msgids)
    TRANSLATED_WC=$(msgattrib --force-po --translated --no-fuzzy --no-obsolete --no-wrap $PO_MESSAGES | count_original_words)
    echo "   $lang: $(($TRANSLATED*100/$TOTAL))% ($TRANSLATED/$TOTAL) translated, $(($FUZZY*100/$TOTAL))% ($FUZZY) fuzzy, $(($TRANSLATED_WC*100/$TOTAL_WC))% ($TRANSLATED_WC/$TOTAL_WC) words translated (rev. date: $REV_DATE)"
    rm -f $PO_FILES $PO_MESSAGES
}

# all PO files

for lang in $LANGUAGES ; do
    PO_FILES="$(mktemp -t XXXXXXXXXX.pofiles)"
    find . -iname "$lang.po" > $PO_FILES
    statistics $PO_FILES
done
