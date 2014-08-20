#!/usr/bin/env python 
'''
Copyright (C) 2011 Felipe Correa da Silva Sanches

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
'''

import inkex
import sys

class NewGlyphLayer(inkex.Effect):
	def __init__(self):
		inkex.Effect.__init__(self)
		self.OptionParser.add_option("-u", "--unicodechars",
						action="store", type="string",
						dest="unicodechars", default='',
						help="Unicode chars")

	def effect(self):
		# Get all the options
		unicode_chars = self.options.unicodechars

		#TODO: remove duplicate chars

		# Get access to main SVG document element
		svg = self.document.getroot()

		for char in unicode_chars:
			# Create a new layer.
			layer = inkex.etree.SubElement(svg, 'g')
			layer.set(inkex.addNS('label', 'inkscape'), 'GlyphLayer-'+char)
			layer.set(inkex.addNS('groupmode', 'inkscape'), 'layer')
			layer.set('style', 'display:none') #initially not visible

			#TODO: make it optional ("Use current selection as template glyph")
			# Move selection to the newly created layer
			for id,node in self.selected.iteritems():
				layer.append(node)

if __name__ == '__main__':
	e = NewGlyphLayer()
	e.affect()

