#!/usr/bin/env python

from xml.dom import minidom
import sys

doc = minidom.parse(sys.argv[1])

filters = doc.getElementsByTagName('pattern')

print "char * stringlst = ["

for filter in filters:
	id = filter.getAttribute('id')
	stockid = filter.getAttribute('inkscape:stockid')

	print "N_(\"" + stockid + "\"),"

print "];"
