#!/usr/bin/env python

from xml.dom import minidom
import sys

elements = ["inkscape:_name", "inkscape:_shortdesc", "inkscape:_keywords"]

sys.stdout.write("char * stringlst = [")

for filename in sys.argv[1:]:
    doc = minidom.parse(filename)
    templates = doc.getElementsByTagName('inkscape:_templateinfo')
    
    if templates:
        for element in elements:
            lines = templates[0].getElementsByTagName(element)
            if lines:
                sys.stdout.write("N_(\"" + lines[0].firstChild.nodeValue + "\"),")

sys.stdout.write("];")
