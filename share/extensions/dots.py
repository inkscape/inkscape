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
import inkex, simplestyle, simplepath, math

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
        self.OptionParser.add_option("-s", "--start",
                        action="store", type="int",
                        dest="start", default="1",
                        help="First number in the sequence, assigned to the first node")
        self.OptionParser.add_option("-t", "--step",
                        action="store", type="int",
                        dest="step", default="1",
                        help="Numbering step between two nodes")
        self.OptionParser.add_option("--tab",
                        action="store", type="string",
                        dest="tab",
                        help="The selected UI-tab when OK was pressed")

    def effect(self):
        selection = self.selected
        if (selection):
            for id, node in selection.iteritems():
                if node.tag == inkex.addNS('path','svg'):
                    self.addDot(node)
        else:
            inkex.errormsg("Please select an object.")

    def separateLastAndFirst(self, p):
        # Separate the last and first dot if they are togheter
        lastDot = -1
        if p[lastDot][1] == []: lastDot = -2
        if round(p[lastDot][1][-2]) == round(p[0][1][-2]) and \
                round(p[lastDot][1][-1]) == round(p[0][1][-1]):
                x1 = p[lastDot][1][-2]
                y1 = p[lastDot][1][-1]
                x2 = p[lastDot-1][1][-2]
                y2 = p[lastDot-1][1][-1]
                dx = abs( max(x1,x2) - min(x1,x2) )
                dy = abs( max(y1,y2) - min(y1,y2) )
                dist = math.sqrt( dx**2 + dy**2 )
                x = dx/dist
                y = dy/dist
                if x1 > x2: x *= -1
                if y1 > y2: y *= -1
                p[lastDot][1][-2] += x * self.unittouu(self.options.dotsize)
                p[lastDot][1][-1] += y * self.unittouu(self.options.dotsize)

    def addDot(self, node):
        self.group = inkex.etree.SubElement( node.getparent(), inkex.addNS('g','svg') )
        self.dotGroup = inkex.etree.SubElement( self.group, inkex.addNS('g','svg') )
        self.numGroup = inkex.etree.SubElement( self.group, inkex.addNS('g','svg') )
        
        try:
            t = node.get('transform')
            self.group.set('transform', t)
        except:
            pass

        style = simplestyle.formatStyle({ 'stroke': 'none', 'fill': '#000' })
        a = []
        p = simplepath.parsePath(node.get('d'))

        self.separateLastAndFirst(p)

        num = self.options.start
        for cmd,params in p:
            if cmd != 'Z' and cmd != 'z':
                dot_att = {
                  'style': style,
                  'r':  str( self.unittouu(self.options.dotsize) / 2 ),
                  'cx': str( params[-2] ),
                  'cy': str( params[-1] )
                }
                inkex.etree.SubElement(
                  self.dotGroup,
                  inkex.addNS('circle','svg'),
                  dot_att )
                self.addText(
                  self.numGroup,
                  params[-2] + ( self.unittouu(self.options.dotsize) / 2 ),
                  params[-1] - ( self.unittouu(self.options.dotsize) / 2 ),
                  num )
                num += self.options.step
        node.getparent().remove( node )

    def addText(self,node,x,y,text):
                new = inkex.etree.SubElement(node,inkex.addNS('text','svg'))
                s = {'font-size': self.unittouu(self.options.fontsize), 'fill-opacity': '1.0', 'stroke': 'none',
                    'font-weight': 'normal', 'font-style': 'normal', 'fill': '#999'}
                new.set('style', simplestyle.formatStyle(s))
                new.set('x', str(x))
                new.set('y', str(y))
                new.text = str(text)

if __name__ == '__main__':
    e = Dots()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
