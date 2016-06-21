#!/usr/bin/env python
# Copyright 2008, 2009 Hannes Hochreiner
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.

# These lines are only needed if you don't put the script directly into
# the installation directory
import sys
# Unix
sys.path.append('/usr/share/inkscape/extensions')
# OS X
sys.path.append('/Applications/Inkscape.app/Contents/Resources/extensions')
# Windows
sys.path.append('C:\Program Files\Inkscape\share\extensions')

import inkex, os.path
import subprocess
import tempfile
import os
import zipfile
import glob
import re

def propStrToDict(inStr):
	dictio = {}

	for prop in inStr.split(";"):
		values = prop.split(":")

		if (len(values) == 2):
			dictio[values[0].strip()] = values[1].strip()

	return dictio

def dictToPropStr(dictio):
	str = ""

	for key in dictio.keys():
		str += " " + key + ":" + dictio[key] + ";" 

	return str[1:]

def setStyle(node, propKey, propValue):
	props = {}

	if node.attrib.has_key("style"):
		props = propStrToDict(node.get("style"))
	
	props[propKey] = propValue
	node.set("style", dictToPropStr(props))

class MyEffect(inkex.Effect):
	inkscapeCommand = None
	zipFile = None

	def __init__(self):
		inkex.Effect.__init__(self)

		self.OptionParser.add_option('--tab', action = 'store', type = 'string', dest = 'what')
		self.OptionParser.add_option('--type', action = 'store', type = 'string', dest = 'type', default = '')
		self.OptionParser.add_option('--resolution', action = 'store', type = 'string', dest = 'resolution', default = '')

		# Register jessyink namespace.
		inkex.NSS[u"jessyink"] = u"https://launchpad.net/jessyink"

		# Set inkscape command.
		self.inkscapeCommand = self.findInkscapeCommand()

		if (self.inkscapeCommand == None):
			inkex.errormsg(_("Could not find Inkscape command.\n"))
			sys.exit(1)

	def output(self):
		pass

	def effect(self):
		# Remove any temporary files that might be left from last time.
		self.removeJessyInkFilesInTempDir()

		# Check whether the JessyInk-script is present (indicating that the presentation has not been properly exported).
		scriptNodes = self.document.xpath("//svg:script[@jessyink:version]", namespaces=inkex.NSS)

		if len(scriptNodes) != 0:
			inkex.errormsg(_("The JessyInk script is not installed in this SVG file or has a different version than the JessyInk extensions. Please select \"install/update...\" from the \"JessyInk\" sub-menu of the \"Extensions\" menu to install or update the JessyInk script.\n\n"))

		zipFileDesc, zpFile = tempfile.mkstemp(suffix=".zip", prefix="jessyInk__")

		output = zipfile.ZipFile(zpFile, "w", compression=zipfile.ZIP_STORED)

		# Find layers.
		exportNodes = self.document.xpath("//svg:g[@inkscape:groupmode='layer']", namespaces=inkex.NSS)

		if len(exportNodes) < 1:
			sys.stderr.write("No layers found.")

		for node in exportNodes:
			setStyle(node, "display", "none")

		for node in exportNodes:
			setStyle(node, "display", "inherit")
			setStyle(node, "opacity", "1")
			self.takeSnapshot(output, node.attrib["{" + inkex.NSS["inkscape"] + "}label"])
			setStyle(node, "display", "none")
				
		# Write temporary zip file to stdout.
		output.close()
		out = open(zpFile,'rb')

		# Switch stdout to binary on Windows.
		if sys.platform == "win32":
			import os, msvcrt
			msvcrt.setmode(sys.stdout.fileno(), os.O_BINARY)

		# Output the file.
		sys.stdout.write(out.read())
		sys.stdout.close()
		out.close()

		# Delete temporary files.
		self.removeJessyInkFilesInTempDir()

	# Function to export the current state of the file using Inkscape.
	def takeSnapshot(self, output, fileName):
		# Write the svg file.
		svgFileDesc, svgFile = tempfile.mkstemp(suffix=".svg", prefix="jessyInk__")
		self.document.write(os.fdopen(svgFileDesc, "wb"))
		
		ext = str(self.options.type).lower()

		# Prepare output file.
		outFileDesc, outFile = tempfile.mkstemp(suffix="." + ext, prefix="jessyInk__")
				
		proc = subprocess.Popen([self.inkscapeCommand + " --file=" + svgFile  + " --without-gui --export-dpi=" + str(self.options.resolution) + " --export-" + ext + "=" + outFile], shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		stdout_value, stderr_value = proc.communicate()
		
		output.write(outFile, fileName + "." + ext)

	# Function to remove any temporary files created during the export.
	def removeJessyInkFilesInTempDir(self):
		for infile in glob.glob(os.path.join(tempfile.gettempdir(), 'jessyInk__*')):
			try:
				os.remove(infile)
			except:
				pass
				
	# Function to try and find the correct command to invoke Inkscape.
	def findInkscapeCommand(self):
		commands = []
		commands.append("inkscape")
		commands.append("C:\Program Files\Inkscape\inkscape.exe")
		commands.append("/Applications/Inkscape.app/Contents/Resources/bin/inkscape")

		for command in commands:
			proc = subprocess.Popen([command + " --without-gui --version"], shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
			stdout_value, stderr_value = proc.communicate()

			if proc.returncode == 0:
				return command

		return None

e = MyEffect()
e.affect()
