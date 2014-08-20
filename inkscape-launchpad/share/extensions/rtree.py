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
import inkex, simplestyle, pturtle, random

def rtree(turtle, size, min):
    if size < min:
        return
    turtle.fd(size)
    turn = random.uniform(20, 40)
    turtle.lt(turn)
    rtree(turtle, size*random.uniform(0.5,0.9), min)
    turtle.rt(turn)
    turn = random.uniform(20, 40)
    turtle.rt(turn)
    rtree(turtle, size*random.uniform(0.5,0.9), min)
    turtle.lt(turn)
    turtle.bk(size)

class RTreeTurtle(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-s", "--size",
                        action="store", type="float", 
                        dest="size", default=100.0,
                        help="initial branch size")
        self.OptionParser.add_option("-m", "--minimum",
                        action="store", type="float", 
                        dest="minimum", default=4.0,
                        help="minimum branch size")
    def effect(self):
        self.options.size = self.unittouu(str(self.options.size) + 'px')
        self.options.minimum = self.unittouu(str(self.options.minimum) + 'px')
        s = {'stroke-linejoin': 'miter', 'stroke-width': str(self.unittouu('1px')), 
            'stroke-opacity': '1.0', 'fill-opacity': '1.0', 
            'stroke': '#000000', 'stroke-linecap': 'butt', 
            'fill': 'none'}
        t = pturtle.pTurtle()
        t.pu()
        t.setpos(self.view_center)
        t.pd()
        rtree(t, self.options.size, self.options.minimum)
        
        attribs = {'d':t.getPath(),'style':simplestyle.formatStyle(s)}
        inkex.etree.SubElement(self.current_layer, inkex.addNS('path','svg'), attribs)

if __name__ == '__main__':
    e = RTreeTurtle()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
