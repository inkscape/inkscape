#!/usr/bin/python

import os

version = ''

# retrieve the version information from the inkscape.rc file
#             VALUE "ProductVersion", "0.48+devel"
with open('..\..\src\inkscape.rc', 'r') as rc:
	for line in rc.readlines():
		if 'productversion' in line.lower() and 'value' in line.lower():
			items = line.split()
			version = items[2]
			version = version.replace('"', '')
			version = version.replace("'", "")
			# version = version.replace("+", "_")
			print version
			

with open('version.wxi', 'w') as wxi:
	wxi.write("<?xml version='1.0' encoding='utf-8'?>\n")
	wxi.write("<!-- do not edit, this file is created by version.py tool any changes will be lost -->\n")
	wxi.write("<Include>\n")
	wxi.write("<?define ProductVersion='" + version + "' ?>\n")
	wxi.write("<?define FullProductName='Inkscape " + version + "' ?>\n")
	wxi.write("</Include>\n")

	

