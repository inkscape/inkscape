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

def listToPropStr(list):
	str = ""
	for prop in list:
		str += " " + prop + ";" 
	return str[1:]

class JessyInk_Uninstall(inkex.Effect):
	def __init__(self):
		# Call the base class constructor.
		inkex.Effect.__init__(self)

		self.OptionParser.add_option('--tab', action = 'store', type = 'string', dest = 'what')
		self.OptionParser.add_option('--remove_script', action = 'store', type = 'inkbool', dest = 'remove_script', default = True)
		self.OptionParser.add_option('--remove_effects', action = 'store', type = 'inkbool', dest = 'remove_effects', default = True)
		self.OptionParser.add_option('--remove_masterSlide', action = 'store', type = 'inkbool', dest = 'remove_masterSlide', default = True)
		self.OptionParser.add_option('--remove_transitions', action = 'store', type = 'inkbool', dest = 'remove_transitions', default = True)
		self.OptionParser.add_option('--remove_autoTexts', action = 'store', type = 'inkbool', dest = 'remove_autoTexts', default = True)
		self.OptionParser.add_option('--remove_views', action = 'store', type = 'inkbool', dest = 'remove_views', default = True)

		inkex.NSS[u"jessyink"] = u"https://launchpad.net/jessyink"

	def effect(self):
		# Remove script, if so desired.
		if self.options.remove_script:
			# Find and delete script node.
			for node in self.document.xpath("//svg:script[@id='JessyInk']", namespaces=inkex.NSS):
				node.getparent().remove(node)
		
			# Remove "jessyInkInit()" in the "onload" attribute, if present.
			if self.document.getroot().get("onload"):
				propList = propStrToList(self.document.getroot().get("onload"))
			else:
				propList = []
		
			for prop in propList:
				if prop == "jessyInkInit()":
					propList.remove("jessyInkInit()")
		
			if len(propList) > 0:
				self.document.getroot().set("onload", listToPropStr(propList))
			else:
				if self.document.getroot().get("onload"):
					del self.document.getroot().attrib["onload"]

		# Remove effect attributes, if so desired.
		if self.options.remove_effects:
			for node in self.document.xpath("//*[@jessyink:effectIn]", namespaces=inkex.NSS):
				del node.attrib["{" + inkex.NSS["jessyink"] + "}effectIn"]

			for node in self.document.xpath("//*[@jessyink:effectOut]", namespaces=inkex.NSS):
				del node.attrib["{" + inkex.NSS["jessyink"] + "}effectOut"]

			# Remove old style attributes as well.
			for node in self.document.xpath("//*[@jessyInk_effectIn]", namespaces=inkex.NSS):
				del node.attrib["jessyInk_effectIn"]

			for node in self.document.xpath("//*[@jessyInk_effectOut]", namespaces=inkex.NSS):
				del node.attrib["jessyInk_effectOut"]

		# Remove master slide assignment, if so desired.
		if self.options.remove_masterSlide:
			for node in self.document.xpath("//*[@jessyink:masterSlide]", namespaces=inkex.NSS):
				del node.attrib["{" + inkex.NSS["jessyink"] + "}masterSlide"]

			# Remove old style attributes as well.
			for node in self.document.xpath("//*[@jessyInk_masterSlide]", namespaces=inkex.NSS):
				del node.attrib["jessyInk_masterSlide"]

		# Remove transitions, if so desired.
		if self.options.remove_transitions:
			for node in self.document.xpath("//*[@jessyink:transitionIn]", namespaces=inkex.NSS):
				del node.attrib["{" + inkex.NSS["jessyink"] + "}transitionIn"]

			for node in self.document.xpath("//*[@jessyink:transitionOut]", namespaces=inkex.NSS):
				del node.attrib["{" + inkex.NSS["jessyink"] + "}transitionOut"]

			# Remove old style attributes as well.
			for node in self.document.xpath("//*[@jessyInk_transitionIn]", namespaces=inkex.NSS):
				del node.attrib["jessyInk_transitionIn"]

			for node in self.document.xpath("//*[@jessyInk_transitionOut]", namespaces=inkex.NSS):
				del node.attrib["jessyInk_transitionOut"]

		# Remove auto texts, if so desired.
		if self.options.remove_autoTexts:
			for node in self.document.xpath("//*[@jessyink:autoText]", namespaces=inkex.NSS):
				del node.attrib["{" + inkex.NSS["jessyink"] + "}autoText"]

			# Remove old style attributes as well.
			for node in self.document.xpath("//*[@jessyInk_autoText]", namespaces=inkex.NSS):
				del node.attrib["jessyInk_autoText"]

		# Remove views, if so desired.
		if self.options.remove_views:
			for node in self.document.xpath("//*[@jessyink:view]", namespaces=inkex.NSS):
				del node.attrib["{" + inkex.NSS["jessyink"] + "}view"]

# Create effect instance.
effect = JessyInk_Uninstall()
effect.affect()

