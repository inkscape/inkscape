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

# We will use the inkex module with the predefined Effect base class.
import inkex


def propStrToList(str):
	list = []
	propList = str.split(";")
	for prop in propList:
		if not (len(prop) == 0):
			list.append(prop.strip())
	return list

def propListToDict(list):
	dictio = {}

	for prop in list:
		keyValue = prop.split(":")

		if len(keyValue) == 2:
			dictio[keyValue[0].strip()] = keyValue[1].strip()

	return dictio

class JessyInk_Effects(inkex.Effect):
	def __init__(self):
		# Call the base class constructor.
		inkex.Effect.__init__(self)

		self.OptionParser.add_option('--tab', action = 'store', type = 'string', dest = 'what')
		self.OptionParser.add_option('--viewOrder', action = 'store', type = 'string', dest = 'viewOrder', default = 1)
		self.OptionParser.add_option('--viewDuration', action = 'store', type = 'float', dest = 'viewDuration', default = 0.8)
		self.OptionParser.add_option('--removeView', action = 'store', type = 'inkbool', dest = 'removeView', default = False)

		inkex.NSS[u"jessyink"] = u"https://launchpad.net/jessyink"

	def effect(self):
		# Check version.
		scriptNodes = self.document.xpath("//svg:script[@jessyink:version='1.5.5']", namespaces=inkex.NSS)

		if len(scriptNodes) != 1:
			inkex.errormsg(_("The JessyInk script is not installed in this SVG file or has a different version than the JessyInk extensions. Please select \"install/update...\" from the \"JessyInk\" sub-menu of the \"Extensions\" menu to install or update the JessyInk script.\n\n"))

		rect = None

		for id, node in self.selected.items():
			if rect == None:
				rect = node
			else:
				inkex.errormsg(_("More than one object selected. Please select only one object.\n"))
				exit()

		if rect == None:
			inkex.errormsg(_("No object selected. Please select the object you want to assign a view to and then press apply.\n"))
			exit()

		if not self.options.removeView:
			# Remove the view that currently has the requested order number.
			for node in rect.xpath("ancestor::svg:g[@inkscape:groupmode='layer']/descendant::*[@jessyink:view]", namespaces=inkex.NSS):
				propDict = propListToDict(propStrToList(node.attrib["{" + inkex.NSS["jessyink"] + "}view"]))
	
				if propDict["order"] == self.options.viewOrder:
					del node.attrib["{" + inkex.NSS["jessyink"] + "}view"]
			
			# Set the new view.
			rect.set("{" + inkex.NSS["jessyink"] + "}view","name:view;order:" + self.options.viewOrder + ";length:" + str(int(self.options.viewDuration * 1000)))

			# Remove possible effect arguments.
			if rect.attrib.has_key("{" + inkex.NSS["jessyink"] + "}effectIn"):
				del rect.attrib["{" + inkex.NSS["jessyink"] + "}effectIn"]

			if rect.attrib.has_key("{" + inkex.NSS["jessyink"] + "}effectOut"):
				del rect.attrib["{" + inkex.NSS["jessyink"] + "}effectOut"]
		else:
			if node.attrib.has_key("{" + inkex.NSS["jessyink"] + "}view"):
				del node.attrib["{" + inkex.NSS["jessyink"] + "}view"]
		
# Create effect instance
effect = JessyInk_Effects()
effect.affect()

