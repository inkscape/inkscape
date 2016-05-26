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
import os
import re
from lxml import etree
from copy import deepcopy

class JessyInk_Effects(inkex.Effect):
	def __init__(self):
		# Call the base class constructor.
		inkex.Effect.__init__(self)

		self.OptionParser.add_option('--tab', action = 'store', type = 'string', dest = 'what')

		inkex.NSS[u"jessyink"] = u"https://launchpad.net/jessyink"

	def effect(self):
		# Check version.
		scriptNodes = self.document.xpath("//svg:script[@jessyink:version='1.5.5']", namespaces=inkex.NSS)

		if len(scriptNodes) != 1:
			inkex.errormsg(_("The JessyInk script is not installed in this SVG file or has a different version than the JessyInk extensions. Please select \"install/update...\" from the \"JessyInk\" sub-menu of the \"Extensions\" menu to install or update the JessyInk script.\n\n"))

		baseView = self.document.xpath("//sodipodi:namedview[@id='base']", namespaces=inkex.NSS)

		if len(baseView) != 1:
			inkex.errormsg(_("Could not obtain the selected layer for inclusion of the video element.\n\n"))

		layer = self.document.xpath("//svg:g[@id='" + baseView[0].attrib["{" + inkex.NSS["inkscape"] + "}current-layer"] + "']", namespaces=inkex.NSS)

		if (len(layer) != 1):
			inkex.errormsg(_("Could not obtain the selected layer for inclusion of the video element.\n\n"))

		# Parse template file.
		tmplFile = open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'jessyInk_video.svg'), 'r')
		tmplRoot = etree.fromstring(tmplFile.read())
		tmplFile.close()

		elem = deepcopy(tmplRoot.xpath("//svg:g[@jessyink:element='core.video']", namespaces=inkex.NSS)[0])
		nodeDict = findInternalLinks(elem, tmplRoot)

		deleteIds(elem)

		idSubst = {}

		for key in nodeDict:
			idSubst[key] = getNewId("jessyink.core.video", self.document)
			deleteIds(nodeDict[key])
			nodeDict[key].attrib['id'] = idSubst[key]
			elem.insert(0, nodeDict[key])

		for ndIter in elem.iter():
			for attrIter in ndIter.attrib:
				for entryIter in idSubst:
					ndIter.attrib[attrIter] = ndIter.attrib[attrIter].replace("#" + entryIter, "#" + idSubst[entryIter])

		# Append element.
		layer[0].append(elem)

def findInternalLinks(node, docRoot, nodeDict = {}):
	for entry in re.findall("url\(#.*\)", etree.tostring(node)):
		linkId = entry[5:len(entry) - 1]

		if not nodeDict.has_key(linkId):
			nodeDict[linkId] = deepcopy(docRoot.xpath("//*[@id='" + linkId + "']", namespaces=inkex.NSS)[0])
			nodeDict = findInternalLinks(nodeDict[linkId], docRoot, nodeDict)

	for entry in node.iter():
		if entry.attrib.has_key('{' + inkex.NSS['xlink'] + '}href'):
			linkId = entry.attrib['{' + inkex.NSS['xlink'] + '}href'][1:len(entry.attrib['{' + inkex.NSS['xlink'] + '}href'])]
	
			if not nodeDict.has_key(linkId):
				nodeDict[linkId] = deepcopy(docRoot.xpath("//*[@id='" + linkId + "']", namespaces=inkex.NSS)[0])
				nodeDict = findInternalLinks(nodeDict[linkId], docRoot, nodeDict)

	return nodeDict

def getNewId(prefix, docRoot):
	import datetime

	number = datetime.datetime.now().microsecond

	while len(docRoot.xpath("//*[@id='" + prefix + str(number) + "']", namespaces=inkex.NSS)) > 0:
		number += 1

	return prefix + str(number)

def deleteIds(node):
	for entry in node.iter():
		if entry.attrib.has_key('id'):
			del entry.attrib['id']

# Create effect instance
effect = JessyInk_Effects()
effect.affect()

