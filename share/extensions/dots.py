#!/usr/bin/env python 
'''
Copyright (C) 2005 Aaron Spike, aaron@ekips.org

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
import inkex, simplestyle, simplepath

class Dots(inkex.Effect):
	def __init__(self):
		inkex.Effect.__init__(self)
		self.OptionParser.add_option("-d", "--dotsize",
						action="store", type="string", 
						dest="dotsize", default="10px",
						help="Size of the dots placed at path nodes")
		self.OptionParser.add_option("-f", "--fontsize",
						action="store", type="string", 
						dest="fontsize", default="20",
						help="Size of node label numbers")	
	def effect(self):
		for id, node in self.selected.iteritems():
			if node.tagName == 'path':
				self.group = self.document.createElement('svg:g')
				node.parentNode.appendChild(self.group)
				new = self.document.createElement('svg:path')
				
				try:
					t = node.attributes.getNamedItem('transform').value
					self.group.setAttribute('transform', t)
				except AttributeError:
					pass

				s = simplestyle.parseStyle(node.attributes.getNamedItem('style').value)
				s['stroke-linecap']='round'
				s['stroke-width']=self.options.dotsize
				new.setAttribute('style', simplestyle.formatStyle(s))

				a =[]
				p = simplepath.parsePath(node.attributes.getNamedItem('d').value)
				num = 1
				for cmd,params in p:
					if cmd != 'Z':
						a.append(['M',params[-2:]])
						a.append(['L',params[-2:]])
						self.addText(self.group,params[-2],params[-1],num)
						num += 1
				new.setAttribute('d', simplepath.formatPath(a))
				self.group.appendChild(new)
				node.parentNode.removeChild(node)

				
	def addText(self,node,x,y,text):
				new = self.document.createElement('svg:text')
				s = {'font-size': self.options.fontsize, 'fill-opacity': '1.0', 'stroke': 'none',
					'font-weight': 'normal', 'font-style': 'normal', 'fill': '#000000'}
				new.setAttribute('style', simplestyle.formatStyle(s))
				new.setAttribute('x', str(x))
				new.setAttribute('y', str(y))
				new.appendChild(self.document.createTextNode(str(text)))
				node.appendChild(new)

e = Dots()
e.affect()
