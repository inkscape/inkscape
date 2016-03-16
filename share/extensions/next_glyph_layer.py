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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
'''

import inkex

class NextLayer(inkex.Effect):
	def __init__(self):
		inkex.Effect.__init__(self)

	def effect(self):

		# Get access to main SVG document element
		self.svg = self.document.getroot()

		groups = self.svg.findall(inkex.addNS('g', 'svg'))

		count=0
		glyphs=[]
		for g in groups:
			if "GlyphLayer-" in g.get(inkex.addNS('label', 'inkscape')):
				glyphs.append(g)
				if g.get("style")=="display:inline":
					count+=1
					current = len(glyphs)-1

		if count!=1 or len(glyphs)<2:
			return
		#TODO: inform the user?

		glyphs[current].set("style", "display:none")
		glyphs[(current+1)%len(glyphs)].set("style", "display:inline")
		return

#TODO: loop 

if __name__ == '__main__':
	e = NextLayer()
	e.affect()

