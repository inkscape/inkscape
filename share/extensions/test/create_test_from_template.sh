#!/bin/bash

exFile="$1"

if test -z "$exFile" -o \! -f "$exFile"; then
  echo "
  This script is a dummy help for the creation of a
  extension unity test file.

  Usage:
  $( basename "$0" ) ../extenion.py
  "
  exit 0
fi

module=$( echo "$exFile" | sed -r 's/^.*\/([^\/.]+)..*$/\1/' )

num=0
testFile="$module.test.py"
while test -e "$testFile"; do
  let num++
  testFile="$module.test$num.py"
done

if grep -q '^\s*class .*[( ]inkex.Effect[ )]' $exFile; then
  pyClass=$( grep '^\s*class .*[( ]inkex.Effect[ )]' $exFile | sed -r 's/^\s*class\s+([^ ]+)\s*\(.*$/\1/' )
else
  echo "
  ERROR: Incompatible Format or Inheritance.
    At this time this script only knows how to make unit tests
    for Python effects based on inkex.Effect.
    The $testFile will not be created.
  "
  exit 1
fi

echo ">> Creating $testFile"

exFileRE="$( echo "$exFile" | sed -r 's/\./\\./g; s/\//\\\//g' )"

sed -r "s/%MODULE%/$module/g; s/%CLASS%/$pyClass/g; s/%FILE%/$exFileRE/g" \
       test_template.py.txt > "$testFile"

chmod +x "$testFile"