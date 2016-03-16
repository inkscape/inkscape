#!/usr/bin/env python 
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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
'''

import inkex
import sys
import simplepath

class SVGFont2Layers(inkex.Effect):
	def __init__(self):
		self.count=0
		inkex.Effect.__init__(self)
		self.OptionParser.add_option("--limitglyphs",
						action="store", type="inkbool",
						dest="limitglyphs", default=True,
						help="Load only the first 30 glyphs from the SVGFont (otherwise the loading process may take a very long time)")

	def create_horiz_guideline(self, label, y):
		namedview = self.svg.find(inkex.addNS('namedview', 'sodipodi'))
		guide = inkex.etree.SubElement(namedview, inkex.addNS('guide', 'sodipodi'))
		guide.set(inkex.addNS('label', 'inkscape'), label)
		guide.set("orientation", "0,1")
		guide.set("position", "0,"+str(y))

	def get_or_create(self, parentnode, nodetype):
		node = parentnode.find(nodetype)
		if node is None:
			node = inkex.etree.SubElement(parentnode, nodetype)
		return node

	def flip_cordinate_system(self, d, emsize, baseline):
		pathdata = simplepath.parsePath(d)
		simplepath.scalePath(pathdata, 1,-1)
		simplepath.translatePath(pathdata, 0, int(emsize) - int(baseline))
		return simplepath.formatPath(pathdata)

	def effect(self):
		# Get access to main SVG document element
		self.svg = self.document.getroot()
		self.defs = self.svg.find(inkex.addNS('defs', 'svg'))

		#TODO: detect files with multiple svg fonts declared. 
		# Current code only reads the first svgfont instance
		font = self.defs.find(inkex.addNS('font', 'svg'))
		setwidth = font.get("horiz-adv-x")
		baseline = font.get("horiz-origin-y")
		if baseline is None:
			baseline = 0

		fontface = font.find(inkex.addNS('font-face', 'svg'))

		#TODO: where should we save the font family name?
		#fontfamily = fontface.get("font-family")
		emsize = fontface.get("units-per-em")

		#TODO: should we guarantee that <svg:font horiz-adv-x> equals <svg:font-face units-per-em> ?
		caps = fontface.get("cap-height")
		xheight = fontface.get("x-height")
		ascender = fontface.get("ascent")
		descender = fontface.get("descent")

		self.svg.set("width", emsize)
		self.create_horiz_guideline("baseline", int(baseline))
		self.create_horiz_guideline("ascender", int(baseline) + int(ascender))
		self.create_horiz_guideline("caps", int(baseline) + int(caps))
		self.create_horiz_guideline("xheight", int(baseline) + int(xheight))
		self.create_horiz_guideline("descender", int(baseline) - int(descender))

		#TODO: missing-glyph
		glyphs = font.findall(inkex.addNS('glyph', 'svg'))
		first_glyph = True
		for glyph in glyphs:
			unicode_char = glyph.get("unicode")
			if unicode_char is None:
				continue

			layer = inkex.etree.SubElement(self.svg, inkex.addNS('g', 'svg'))
			layer.set(inkex.addNS('label', 'inkscape'), "GlyphLayer-" + unicode_char)
			layer.set(inkex.addNS('groupmode', 'inkscape'), "layer")

      #glyph layers (except the first one) are innitially hidden
			if not first_glyph:
				layer.set("style", "display:none")
			first_glyph = False

			#TODO: interpret option 1

			############################
			#Option 1:
			# Using clone (svg:use) as childnode of svg:glyph

			#use = self.get_or_create(glyph, inkex.addNS('use', 'svg'))
			#use.set(inkex.addNS('href', 'xlink'), "#"+group.get("id"))
			#TODO: This code creates <use> nodes but they do not render on svg fonts dialog. why? 

			############################
			#Option 2:
			# Using svg:paths as childnodes of svg:glyph

			paths = glyph.findall(inkex.addNS('path', 'svg'))
			for path in paths:
				d = path.get("d")
				if d is None:
					continue
				d = self.flip_cordinate_system(d, emsize, baseline)
				new_path = inkex.etree.SubElement(layer, inkex.addNS('path', 'svg'))
				new_path.set("d", d)

			############################
			#Option 3:
			# Using curve description in d attribute of svg:glyph

			d = glyph.get("d")
			if d is None:
				continue
			d = self.flip_cordinate_system(d, emsize, baseline)
			path = inkex.etree.SubElement(layer, inkex.addNS('path', 'svg'))
			path.set("d", d)

			self.count+=1
			if self.options.limitglyphs and self.count>=30:
				break

if __name__ == '__main__':
	e = SVGFont2Layers()
	e.affect()

