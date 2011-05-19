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

class SetupTypographyCanvas(inkex.Effect):
	def __init__(self):
		inkex.Effect.__init__(self)
		self.OptionParser.add_option("-w", "--setwidth",
						action="store", type="int",
						dest="setwidth", default=1000,
						help="Set Width")
		self.OptionParser.add_option("-a", "--ascender",
						action="store", type="int",
						dest="ascender", default='750',
						help="Ascender")
		self.OptionParser.add_option("-c", "--caps",
						action="store", type="int",
						dest="caps", default='700',
						help="Caps Height")
		self.OptionParser.add_option("-x", "--xheight",
						action="store", type="int",
						dest="xheight", default='500',
						help="x-height")
		self.OptionParser.add_option("-d", "--descender",
						action="store", type="int",
						dest="descender", default='-250',
						help="Descender")
		self.OptionParser.add_option("-l", "--lbearing",
						action="store", type="int",
						dest="lbearing", default='100',
						help="Left Side Bearing")
		self.OptionParser.add_option("-r", "--rbearing",
						action="store", type="int",
						dest="rbearing", default='100',
						help="Right Side Bearing")

	def create_horizontal_guideline(self, name, position):
		self.create_guideline(name, "0,1", 0, position)

	def create_vertical_guideline(self, name, position):
		self.create_guideline(name, "1,0", position, 0)

	def create_guideline(self, label, orientation, x,y):
		namedview = self.svg.find(inkex.addNS('namedview', 'sodipodi'))
		guide = inkex.etree.SubElement(namedview, inkex.addNS('guide', 'sodipodi'))
		guide.set("orientation", orientation)
		guide.set("position", str(x)+","+str(y))
		guide.set(inkex.addNS('label', 'inkscape'), label)

	def effect(self):
		# Get all the options
		setwidth = self.options.setwidth
		ascender = self.options.ascender
		caps = self.options.caps
		xheight = self.options.xheight
		descender = self.options.descender
		lbearing = self.options.lbearing
		rbearing = self.options.rbearing

		# Get access to main SVG document element
		self.svg = self.document.getroot()
		self.svg.set("width", str(setwidth))
		self.svg.set("height", str(setwidth))

		baseline = -descender
		# Create guidelines
		self.create_horizontal_guideline("baseline", baseline)
		self.create_horizontal_guideline("ascender", baseline+ascender)
		self.create_horizontal_guideline("caps", baseline+caps)
		self.create_horizontal_guideline("xheight", baseline+xheight)
		self.create_horizontal_guideline("descender", baseline+descender)
		self.create_vertical_guideline("lbearing", lbearing)
		self.create_vertical_guideline("rbearing", setwidth-rbearing)

if __name__ == '__main__':
	e = SetupTypographyCanvas()
	e.affect()

