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
		self.OptionParser.add_option("-u", "--unicodechar",
						action="store", type="string",
						dest="unicodechar", default='',
						help="Unicode char")

	def effect(self):
		# Get all the options
		unicode_char = self.options.unicodechar

		# Get access to main SVG document element
		svg = self.document.getroot()

		# Create a new layer.
		layer = inkex.etree.SubElement(svg, 'g')
		layer.set(inkex.addNS('label', 'inkscape'), 'GlyphLayer-'+unicode_char)
		layer.set(inkex.addNS('groupmode', 'inkscape'), 'layer')

if __name__ == '__main__':
	e = NewGlyphLayer()
	e.affect()

