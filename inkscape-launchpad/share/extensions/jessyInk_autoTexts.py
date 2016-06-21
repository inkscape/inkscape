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

class JessyInk_AutoTexts(inkex.Effect):
	def __init__(self):
		# Call the base class constructor.
		inkex.Effect.__init__(self)

		self.OptionParser.add_option('--tab', action = 'store', type = 'string', dest = 'what')
		self.OptionParser.add_option('--autoText', action = 'store', type = 'string', dest = 'autoText', default = 'none')
		
		inkex.NSS[u"jessyink"] = u"https://launchpad.net/jessyink"

	def effect(self):
		# Check version.
		scriptNodes = self.document.xpath("//svg:script[@jessyink:version='1.5.5']", namespaces=inkex.NSS)

		if len(scriptNodes) != 1:
			inkex.errormsg(_("The JessyInk script is not installed in this SVG file or has a different version than the JessyInk extensions. Please select \"install/update...\" from the \"JessyInk\" sub-menu of the \"Extensions\" menu to install or update the JessyInk script.\n\n"))

		if len(self.selected) == 0:
			inkex.errormsg(_("To assign an effect, please select an object.\n\n"))

		for id, node in self.selected.items():
			nodes = node.xpath("./svg:tspan", namespaces=inkex.NSS)

			if len(nodes) != 1:
				inkex.errormsg(_("Node with id '{0}' is not a suitable text node and was therefore ignored.\n\n").format(str(id)))
			else:
				if self.options.autoText == "slideTitle":
					nodes[0].set("{" + inkex.NSS["jessyink"] + "}autoText","slideTitle")
				elif self.options.autoText == "slideNumber":
					nodes[0].set("{" + inkex.NSS["jessyink"] + "}autoText","slideNumber")
				elif self.options.autoText == "numberOfSlides":
					nodes[0].set("{" + inkex.NSS["jessyink"] + "}autoText","numberOfSlides")
				else:
					if nodes[0].attrib.has_key("{" + inkex.NSS["jessyink"] + "}autoText"):
						del nodes[0].attrib["{" + inkex.NSS["jessyink"] + "}autoText"]

# Create effect instance
effect = JessyInk_AutoTexts()
effect.affect()

