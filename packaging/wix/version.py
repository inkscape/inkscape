#!/usr/bin/python

import os
import struct

version = ''
versionstr = ''
architecture = ''

def is64bitArchitecture(filename):
	''' test if a executable is of x64 format @see http://stackoverflow.com/questions/1001404/check-if-unmanaged-dll-is-32-bit-or-64-bit/1002672#1002672
	@see http://www.microsoft.com/whdc/system/platform/firmware/PECOFF.mspx
      //offset to PE header is always at 0x3C
      //PE header starts with "PE\0\0" =  0x50 0x45 0x00 0x00
      //followed by 2-byte machine type field (see document above for enum)
	'''
	with open(filename, 'rb') as cofffile:
		cofffile.seek(0x3c)
		peOffset = struct.unpack('H', cofffile.read(2))[0]
		cofffile.seek(peOffset)
		peHead = struct.unpack('I', cofffile.read(4))[0]
		if peHead != 0x00004550:	# "PE\0\0", little-endian
			# throw new Exception("Can't find PE header")
			pass
		machineType = struct.unpack('H', cofffile.read(2))[0]
		if machineType in (0x8664, 0x200):
			return True
	return False

if is64bitArchitecture('..\..\inkscape\inkscape.exe'):
	architecture = '-x64'
else:
	architecture = ''

# retrieve the version information from the inkscape.rc file
#             VALUE "ProductVersion", "0.48+devel"
with open('..\..\src\inkscape.rc', 'r') as rc:
	isversioninfo = False
	
	for line in rc.readlines():
		if 'productversion' in line.lower() and 'value' in line.lower():
			items = line.split()
			versionstr = items[2]
			versionstr = versionstr.replace('"', '')
			versionstr = versionstr.replace("'", "")
			# version = version.replace("+", "_")
			print versionstr + architecture
		if 'versioninfo' in line.lower():
			isversioninfo = True
		if 'begin' in line.lower():
			isversioninfo = False
		if isversioninfo and 'productversion' in line.lower():
			items = line.split()
			''' the second element contains now version info in the form major,minor,fix,build'''
			veritems = items[1].split(',')
			version = veritems[0] + '.' + veritems[1]
			

with open('version.wxi', 'w') as wxi:
	wxi.write("<?xml version='1.0' encoding='utf-8'?>\n")
	wxi.write("<!-- do not edit, this file is created by version.py tool any changes will be lost -->\n")
	wxi.write("<Include>\n")
	wxi.write("<?define ProductVersion='" + version + "' ?>\n")
	wxi.write("<?define FullProductName='Inkscape " + versionstr + "' ?>\n")
	if 'x64' in architecture:
		wxi.write("<?define ProgramFilesFolder='ProgramFiles64Folder' ?>\n")
		wxi.write("<?define Win64='yes' ?>\n")
		wxi.write("<?define InstallerVersion='200' ?>\n")
		wxi.write("<?define Platform='x64' ?>\n")
	else:
		wxi.write("<?define ProgramFilesFolder='ProgramFilesFolder' ?>\n")
		wxi.write("<?define Win64='no' ?>\n")
		wxi.write("<?define InstallerVersion='100' ?>\n")
		wxi.write("<?define Platform='x86' ?>\n")
	wxi.write("</Include>\n")

