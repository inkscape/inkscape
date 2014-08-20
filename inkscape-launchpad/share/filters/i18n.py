#!/usr/bin/env python

from xml.dom import minidom
import sys

doc = minidom.parse(sys.argv[1])

filters = doc.getElementsByTagName('filter')

sys.stdout.write("char * stringlst = [")

for filter in filters:
    label = "N_(\"" + filter.getAttribute('inkscape:label') + "\")"
    menu = "N_(\"" + filter.getAttribute('inkscape:menu') + "\")"
    if (filter.getAttribute('inkscape:menu-tooltip')):
        desc = "N_(\"" + filter.getAttribute('inkscape:menu-tooltip') + "\")"
    else:
        desc = ""
    comment = ""

    if "NR" in label:
        comment = '/* TRANSLATORS: NR means non-realistic. See menu Filters > Non realistic shaders */\n'
    
    sys.stdout.write(comment + "\n" + label + ",\n" + menu + ",\n" + desc + ",\n")

sys.stdout.write("];")
