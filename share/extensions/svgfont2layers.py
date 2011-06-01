'''
Copyright (C) 2011 Felipe Correa da Silva Sanches <juca@members.fsf.org>

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
from PathData import PathData

class SVGFont2Layers(inkex.Effect):
	def __init__(self):
		inkex.Effect.__init__(self)

	def create_guideline(self, label, x, y):
		namedview = self.svg.find(inkex.addNS('namedview', 'sodipodi'))
		guide = inkex.etree.SubElement(namedview, inkex.addNS('guide', 'sodipodi'))
		guide.set(inkex.addNS('label', 'inkscape'), label)
		guide.set("position" , str(x)+","+str(y))

	def get_or_create(self, parentnode, nodetype):
		node = parentnode.find(nodetype)
		if node is None:
			node = inkex.etree.SubElement(parentnode, nodetype)
		return node

	def flip_cordinate_system(self, d, emsize, baseline):
		pathdata = PathData(d)

		def flip_cordinates(coordinates, emsize, baseline, relative):
			x, y = coordinates
			if relative:
				return (x, -y)
			else:
				return (x, emsize - baseline - y)	

		return pathdata.transform_coordinate_values(flip_cordinates, int(emsize), int(baseline))

	def effect(self):
		# Get access to main SVG document element
		self.svg = self.document.getroot()
		self.defs = self.svg.find(inkex.addNS('defs', 'svg'))

		#TODO: detect files with multiple svg fonts declared. 
		# Current code only reads the first svgfont instance
		font = self.defs.find(inkex.addNS('font', 'svg'))
		setwidth = font.get("horiz-adv-x")
		baseline = font.get("horiz-origin-y")

		fontface = self.font.find(inkex.addNS('font-face', 'svg'))

		#TODO: where should we save the font family name?
		#fontfamily = fontface.get("font-family")
		emsize = fontface.get("units-per-em")

		#TODO: should we guarantee that <svg:font horiz-adv-x> equals <svg:font-face units-per-em> ?
		caps = fontface.get("cap-height")
		xheight = fontface.get("x-height")
		ascender = fontface.get("ascent")
		descender = fontface.get("descent")

		self.svg.set("width", emsize)
		self.create_guideline("baseline", 0, int(baseline))
		self.create_guideline("ascender", 0, int(baseline) + int(ascender))
		self.create_guideline("caps", 0, int(baseline) + int(caps))
		self.create_guideline("xheight", 0, int(baseline) + int(xheight))
		self.create_guideline("descender", 0, int(baseline) - int(descender))

		#TODO: missing-glyph
		glyphs = font.findall(inkex.addNS('glyph', 'svg'))
		for glyph in glyphs:
			layer = inkex.etree.SubElement(self.svg, inkex.addNS('g', 'svg'))
			unicode_char = glyph.get("unicode")
			layer.set(inkex.addNS('label', 'inkscape'), "GlyphLayer-" + unicode_char)
			layer.set(inkex.addNS('groupmode', 'inkscape'), "layer")

			############################
			#Option 1:
			# Using clone (svg:use) as childnode of svg:glyph

			#use = self.get_or_create(glyph, inkex.addNS('use', 'svg'))
			#use.set(inkex.addNS('href', 'xlink'), "#"+group.get("id"))
			#TODO: This code creates <use> nodes but they do not render on svg fonts dialog. why? 

			############################
			#Option 2:
			# Using svg:paths as childnodes of svg:glyph

			#paths = group.findall(inkex.addNS('path', 'svg'))
			#for p in paths:
			#	d = p.get("d")
			#	d = self.flip_cordinate_system(d, emsize, baseline)
			#	path = inkex.etree.SubElement(glyph, inkex.addNS('path', 'svg'))
			#	path.set("d", d)

			############################
			#Option 3:
			# Using curve description in d attribute of svg:glyph

			paths = group.findall(inkex.addNS('path', 'svg'))
			d = glyph.get("d")
			d = self.flip_cordinate_system(d, emsize, baseline)
			path = inkex.etree.SubElement(layer, inkex.addNS('path', 'svg'))
			path.set("d", d)
			#TODO: interpret options 1 and 2

if __name__ == '__main__':
	e = SVGFont2Layers()
	e.affect()

