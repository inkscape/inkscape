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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
'''
import inkex, simplestyle, pturtle, random
from simpletransform import computePointInNode

def stripme(s):
    return s.strip()

class LSystem(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-o", "--order",
                        action="store", type="int", 
                        dest="order", default=3,
                        help="number of iteration")
        self.OptionParser.add_option("-l", "--langle",
                        action="store", type="float", 
                        dest="langle", default=16.0,
                        help="angle for turning left")
        self.OptionParser.add_option("-r", "--rangle",
                        action="store", type="float", 
                        dest="rangle", default=16.0,
                        help="angle for turning right")
        self.OptionParser.add_option("-s", "--step",
                        action="store", type="float", 
                        dest="step", default=25.0,
                        help="step size")
        self.OptionParser.add_option("-p", "--randomizestep",
                        action="store", type="float", 
                        dest="randomizestep", default=0.0,
                        help="randomize step")
        self.OptionParser.add_option("-z", "--randomizeangle",
                        action="store", type="float", 
                        dest="randomizeangle", default=0.0,
                        help="randomize angle")
        self.OptionParser.add_option("-x", "--axiom",
                        action="store", type="string", 
                        dest="axiom", default="++F",
                        help="initial state of system")
        self.OptionParser.add_option("-u", "--rules",
                        action="store", type="string", 
                        dest="rules", default="F=FF-[-F+F+F]+[+F-F-F]",
                        help="replacement rules")
        self.OptionParser.add_option("-t", "--tab",
                        action="store", type="string",
                        dest="tab")
        self.stack = []
        self.turtle = pturtle.pTurtle()
    def iterate(self):
        self.rules = dict([map(stripme, i.split("=")) for i in self.options.rules.upper().split(";") if i.count("=")==1])
        string = self.__recurse(self.options.axiom.upper(),0)
        self.__compose_path(string)
        return self.turtle.getPath()
    def __compose_path(self, string):
        self.turtle.pu()
        self.turtle.setpos(computePointInNode(list(self.view_center), self.current_layer))
        self.turtle.pd()
        for c in string:
            if c in 'ABCDEF':
                self.turtle.pd()
                self.turtle.fd(self.options.step * (random.normalvariate(1.0, 0.01 * self.options.randomizestep)))
            elif c in 'GHIJKL':
                self.turtle.pu()
                self.turtle.fd(self.options.step * (random.normalvariate(1.0,  0.01 * self.options.randomizestep)))
            elif c == '+':
                self.turtle.lt(self.options.langle * (random.normalvariate(1.0,  0.01 * self.options.randomizeangle)))
            elif c == '-':
                self.turtle.rt(self.options.rangle * (random.normalvariate(1.0,  0.01 * self.options.randomizeangle)))
            elif c == '|':
                self.turtle.lt(180)
            elif c == '[':
                self.stack.append([self.turtle.getpos(), self.turtle.getheading()])
            elif c == ']':
                self.turtle.pu()
                pos,heading = self.stack.pop()
                self.turtle.setpos(pos)
                self.turtle.setheading(heading)

    def __recurse(self,rule,level):
        level_string = ''
        for c in rule:
            if level < self.options.order:
                try:
                    level_string = level_string + self.__recurse(self.rules[c],level+1)
                except KeyError:
                    level_string = level_string + c
            else:
                level_string = level_string + c 
        return level_string
            
    def effect(self):
        self.options.step = self.unittouu(str(self.options.step) + 'px')
        s = {'stroke-linejoin': 'miter', 'stroke-width': str(self.unittouu('1px')), 
            'stroke-opacity': '1.0', 'fill-opacity': '1.0', 
            'stroke': '#000000', 'stroke-linecap': 'butt', 
            'fill': 'none'}
        attribs = {'style':simplestyle.formatStyle(s),'d':self.iterate()}
        inkex.etree.SubElement(self.current_layer,inkex.addNS('path','svg'),attribs)

if __name__ == '__main__':
    e = LSystem()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
