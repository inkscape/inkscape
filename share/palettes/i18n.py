#!/usr/bin/env python

import sys
import glob
import re

print "char * stringlst = ["

# Gimp palette format: R   G   B  Label (255   0   0  Red)

regex = re.compile(r'^\s*\d{1,3}\s+\d{1,3}\s+\d{1,3}\s+([^#\s].*)')
regexnoc = re.compile(r'%')

for filename in sys.argv[1:]:
    file = open (filename, 'r')
    for line in file:
        match = regex.match(line)
        if match:
            print '\n/* Palette: ' + filename + ' */'
            search = regexnoc.search(match.group(1))
            if search:
                print "/* xgettext:no-c-format */"
            print "NC_(\"Palette\", \"" + match.group(1) + "\"),"

print "];"
