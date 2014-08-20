#!/usr/bin/env python

from xml.dom import minidom
import sys

sys.stdout.write("char * stringlst = [")

for filename in sys.argv[1:]:
    doc = minidom.parse(filename)
    symbols = doc.getElementsByTagName('title')
    
    if symbols:
        for symbol in symbols:
            sys.stdout.write("\n/* Symbols: " + filename + " */  NC_(\"Symbol\", \"" + symbol.firstChild.nodeValue + "\"),")

sys.stdout.write("];")
