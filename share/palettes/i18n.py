#!/usr/bin/env python

import sys
import glob
import re

palettes = sys.argv;

print "char * stringlst = ["

regex = re.compile(r'^\s*\d{1,3}\s+\d{1,3}\s+\d{1,3}\s+([^#\s].*)')

for filename in palettes:
    file = open (filename, 'r')
    print '\n/* Palette: ' + filename + ' */'
    for line in file:
        match = regex.match(line)
        if match:
            print "N_(\"" + match.group(1) + "\"),"

print "];"
