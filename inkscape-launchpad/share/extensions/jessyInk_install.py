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

import os

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

class JessyInk_Install(inkex.Effect):
	def __init__(self):
		# Call the base class constructor.
		inkex.Effect.__init__(self)

		self.OptionParser.add_option('--tab', action = 'store', type = 'string', dest = 'what')

		inkex.NSS[u"jessyink"] = u"https://launchpad.net/jessyink"

	def effect(self):
		# Find and delete old script node
		for node in self.document.xpath("//svg:script[@id='JessyInk']", namespaces=inkex.NSS):
			node.getparent().remove(node)
	
		# Create new script node
		scriptElm = inkex.etree.Element(inkex.addNS("script", "svg"))
		scriptElm.text = open(os.path.join(os.path.dirname(__file__),	"jessyInk.js")).read()
		scriptElm.set("id","JessyInk")
		scriptElm.set("{" + inkex.NSS["jessyink"] + "}version", '1.5.5')
		self.document.getroot().append(scriptElm)

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

		# Update effect attributes.
		for node in self.document.xpath("//*[@jessyInk_effectIn]", namespaces=inkex.NSS):
			node.attrib["{" + inkex.NSS["jessyink"] + "}effectIn"] = node.attrib["jessyInk_effectIn"]
			del node.attrib["jessyInk_effectIn"]

		for node in self.document.xpath("//*[@jessyink:effectIn]", namespaces=inkex.NSS):
			node.attrib["{" + inkex.NSS["jessyink"] + "}effectIn"] = node.attrib["{" + inkex.NSS["jessyink"] + "}effectIn"].replace("=", ":")

		for node in self.document.xpath("//*[@jessyInk_effectOut]", namespaces=inkex.NSS):
			node.attrib["{" + inkex.NSS["jessyink"] + "}effectOut"] = node.attrib["jessyInk_effectOut"]
			del node.attrib["jessyInk_effectOut"]

		for node in self.document.xpath("//*[@jessyink:effectOut]", namespaces=inkex.NSS):
			node.attrib["{" + inkex.NSS["jessyink"] + "}effectOut"] = node.attrib["{" + inkex.NSS["jessyink"] + "}effectOut"].replace("=", ":")

		# Update master slide assignment.
		for node in self.document.xpath("//*[@jessyInk_masterSlide]", namespaces=inkex.NSS):
			node.attrib["{" + inkex.NSS["jessyink"] + "}masterSlide"] = node.attrib["jessyInk_masterSlide"]
			del node.attrib["jessyInk_masterSlide"]

		for node in self.document.xpath("//*[@jessyink:masterSlide]", namespaces=inkex.NSS):
			node.attrib["{" + inkex.NSS["jessyink"] + "}masterSlide"] = node.attrib["{" + inkex.NSS["jessyink"] + "}masterSlide"].replace("=", ":")

		# Udpate transitions.
		for node in self.document.xpath("//*[@jessyInk_transitionIn]", namespaces=inkex.NSS):
			node.attrib["{" + inkex.NSS["jessyink"] + "}transitionIn"] = node.attrib["jessyInk_transitionIn"]
			del node.attrib["jessyInk_transitionIn"]

		for node in self.document.xpath("//*[@jessyink:transitionIn]", namespaces=inkex.NSS):
			node.attrib["{" + inkex.NSS["jessyink"] + "}transitionIn"] = node.attrib["{" + inkex.NSS["jessyink"] + "}transitionIn"].replace("=", ":")

		for node in self.document.xpath("//*[@jessyInk_transitionOut]", namespaces=inkex.NSS):
			node.attrib["{" + inkex.NSS["jessyink"] + "}transitionOut"] = node.attrib["jessyInk_transitionOut"]
			del node.attrib["jessyInk_transitionOut"]

		for node in self.document.xpath("//*[@jessyink:transitionOut]", namespaces=inkex.NSS):
			node.attrib["{" + inkex.NSS["jessyink"] + "}transitionOut"] = node.attrib["{" + inkex.NSS["jessyink"] + "}transitionOut"].replace("=", ":")

		# Update auto texts.
		for node in self.document.xpath("//*[@jessyInk_autoText]", namespaces=inkex.NSS):
			node.attrib["{" + inkex.NSS["jessyink"] + "}autoText"] = node.attrib["jessyInk_autoText"]
			del node.attrib["jessyInk_autoText"]

		for node in self.document.xpath("//*[@jessyink:autoText]", namespaces=inkex.NSS):
			node.attrib["{" + inkex.NSS["jessyink"] + "}autoText"] = node.attrib["{" + inkex.NSS["jessyink"] + "}autoText"].replace("=", ":")

# Create effect instance
effect = JessyInk_Install()
effect.affect()

