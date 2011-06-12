#!/usr/bin/env python

from xml.dom import minidom
import sys

doc = minidom.parse(sys.argv[1])

filters = doc.getElementsByTagName('pattern')

sys.stdout.write("char * stringlst = [")

for filter in filters:
	id = filter.getAttribute('id')
	stockid = filter.getAttribute('inkscape:stockid')

	sys.stdout.write("N_(\"" + stockid + "\"),")

sys.stdout.write("];")
