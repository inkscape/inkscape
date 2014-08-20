#!/usr/bin/env python

import sys
import glob
import re

sys.stdout.write("char * stringlst = [")

# Gimp palette format: R   G   B  Label (255   0   0  Red)

regex = re.compile(r'^\s*\d{1,3}\s+\d{1,3}\s+\d{1,3}\s+([^#\s].*)')
regexnoc = re.compile(r'%')

for filename in sys.argv[1:]:
    file = open (filename, 'r')
    for line in file:
        match = regex.match(line)
        if match:
            sys.stdout.write('\n/* Palette: ' + filename + ' */')
            search = regexnoc.search(match.group(1))
            if search:
                sys.stdout.write("/* xgettext:no-c-format */")
            sys.stdout.write("NC_(\"Palette\", \"" + match.group(1) + "\"),")

sys.stdout.write("];")
