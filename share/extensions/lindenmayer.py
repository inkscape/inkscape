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
import inkex, simplestyle, pturtle

class LSystem(inkex.Effect):
	def __init__(self):
		inkex.Effect.__init__(self)
		self.OptionParser.add_option("-o", "--order",
						action="store", type="int", 
						dest="order", default=3,
						help="number of iteration")
		self.OptionParser.add_option("-a", "--angle",
						action="store", type="float", 
						dest="angle", default=16.0,
						help="angle for turn commands")
		self.OptionParser.add_option("-s", "--step",
						action="store", type="float", 
						dest="step", default=25.0,
						help="step size")
		self.OptionParser.add_option("-x", "--axiom",
						action="store", type="string", 
						dest="axiom", default="++F",
						help="initial state of system")
		self.OptionParser.add_option("-r", "--rules",
						action="store", type="string", 
						dest="rules", default="F=FF-[-F+F+F]+[+F-F-F]",
						help="replacement rules")
		self.stack = []
		self.turtle = pturtle.pTurtle()
	def iterate(self):
		self.rules = dict([i.split("=") for i in self.options.rules.upper().split(";") if i.count("=")==1])
		self.__recurse(self.options.axiom.upper(),0)
		return self.turtle.getPath()
	def __recurse(self,rule,level):
		for c in rule:
			if level < self.options.order:
				try:
					self.__recurse(self.rules[c],level+1)
				except KeyError:
					pass
			
			if c == 'F':
				self.turtle.pd()
				self.turtle.fd(self.options.step)
			elif c == 'G':
				self.turtle.pu()
				self.turtle.fd(self.options.step)
			elif c == '+':
				self.turtle.lt(self.options.angle)
			elif c == '-':
				self.turtle.rt(self.options.angle)
			elif c == '|':
				self.turtle.lt(180)
			elif c == '[':
				self.stack.append([self.turtle.getpos(), self.turtle.getheading()])
			elif c == ']':
				self.turtle.pu()
				pos,heading = self.stack.pop()
				self.turtle.setpos(pos)
				self.turtle.setheading(heading)
	def effect(self):
		new = self.document.createElement('svg:path')
		s = {'stroke-linejoin': 'miter', 'stroke-width': '1.0px', 
			'stroke-opacity': '1.0', 'fill-opacity': '1.0', 
			'stroke': '#000000', 'stroke-linecap': 'butt', 
			'fill': 'none'}
		new.setAttribute('style', simplestyle.formatStyle(s))
		new.setAttribute('d', self.iterate())
		self.document.documentElement.appendChild(new)

e = LSystem()
e.affect()
