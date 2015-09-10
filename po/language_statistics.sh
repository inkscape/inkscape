#!/usr/bin/env bash

#
# usage() : short help
#
usage() {
    cat <<EOF
Usage :
    $0 [--help] [-e|--export-html] languages
EOF
}

#
# count_msgids() : count the original strings
#
count_msgids() {
    cat | grep -E '^msgid\s+' | wc -l
}

#
# count_original_words() : count the words in the original strings
#
count_original_words() {
    cat | grep ^msgid | sed 's/^msgid "//g;s/"$//g' | wc -w
}

#
# statistics() : process statistics on translations
#
statistics() {
    PO_FILES="$(mktemp -t XXXXXXXXXX.pofiles)"
    find . -iname "$lang.po" > $PO_FILES
    PO_MESSAGES="$(mktemp -t XXXXXXXXXX.po)"
    msgcat --files-from=$PO_FILES --output=$PO_MESSAGES
    REV_DATE=$(grep "PO-Revision-Date: [1-9]" $PO_MESSAGES |cut -d " " -f2)
    TOTAL=$(msgattrib --force-po --no-obsolete $PO_MESSAGES | count_msgids)
    TOTAL_WC=$(msgattrib --force-po --no-obsolete --no-wrap $PO_MESSAGES | count_original_words)
    FUZZY=$(msgattrib --force-po --only-fuzzy --no-obsolete $PO_MESSAGES | count_msgids)
    # Fully translated files always return one remaining fuzzy entry... 
    if [ $FUZZY = 1 ]
        then FUZZY=0
    fi
    TRANSLATED=$(msgattrib --force-po --translated --no-fuzzy --no-obsolete $PO_MESSAGES | count_msgids)
    TRANSLATED_WC=$(msgattrib --force-po --translated --no-fuzzy --no-obsolete --no-wrap $PO_MESSAGES | count_original_words)
    rm -f $PO_FILES $PO_MESSAGES
}

#
# show_text() : show the statistics in a readable text format
#
show_text() {
    for lang in $LANGUAGES ; do
        statistics
        echo "   $lang: $(($TRANSLATED*100/$TOTAL))% ($TRANSLATED/$TOTAL) translated, $(($FUZZY*100/$TOTAL))% ($FUZZY) fuzzy, $(($TRANSLATED_WC*100/$TOTAL_WC))% ($TRANSLATED_WC/$TOTAL_WC) words translated (rev. date: $REV_DATE)"
    done
}

#
# show_html() : show the statistics in HTML format
#
show_html() {
    echo "<html>
  <body>
    <table>
      <caption>Translation status of the Inkscape user interface</caption>
      <thead>
        <tr>
          <td>Language</td><td>Status</td><td>Untranslated</td><td>Fuzzy</td><td>Total</td><td>Last changed</td>
        </tr>
      </thead>
      <tbody>"
    for lang in $LANGUAGES ; do
        statistics
        echo "        <tr><td>$lang</td><td><progress max='100' value='$(($TRANSLATED*100/$TOTAL))' title='$(($TRANSLATED*100/$TOTAL))%'>$(($TRANSLATED*100/$TOTAL))%</progress></td><td>$(($TOTAL-$TRANSLATED-$FUZZY)) ($((($TOTAL-$TRANSLATED-$FUZZY)*100/$TOTAL))%)</td><td>$FUZZY ($(($FUZZY*100/$TOTAL))%)</td><td>$TOTAL</td><td>$REV_DATE</td></tr>"
    done
    echo "      </tbody>
    </table>
  </body>
</html>"
}

user_lang=
export_html=0

# Command line options
while test $# -gt 0
do
    case $1 in
    -h | --help)
        usage
        exit 0
        ;;
    -e | --export-html)
        export_html=1
        ;;
    -*)  echo "$0 : invalid option $1" >&2
        usage
        exit 1
        ;;
    *)
        user_lang=$@
        break
        ;;
    esac
    shift
done


set -eu
#set -o pipefail

if [ "$user_lang" ]
    then LANGUAGES="$user_lang"
    else LANGUAGES="$(grep -v "^\#" ./LINGUAS)"
fi

if [ $export_html = 1 ]
    then show_html
    else show_text
fi


# -*- mode: sh; sh-basic-offset: 4; indent-tabs-mode: nil; -*-
# vim: set filetype=sh sw=4 sts=4 expandtab autoindent:
