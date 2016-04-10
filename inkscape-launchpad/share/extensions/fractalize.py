#!/usr/bin/env python 
'''
Copyright (C) 2005 Carsten Goetze c.goetze@tu-bs.de

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
import random, math, inkex, simplepath

def calculateSubdivision(x1,y1,x2,y2,smoothness):
    """ Calculate the vector from (x1,y1) to (x2,y2) """
    x3 = x2 - x1
    y3 = y2 - y1
    """ Calculate the point half-way between the two points """
    hx = x1 + x3/2
    hy = y1 + y3/2
    """ Calculate normalized vector perpendicular to the vector (x3,y3) """
    length = math.sqrt(x3*x3 + y3*y3)
    if length != 0:
        nx = -y3/length
        ny = x3/length
    else:
        nx = 1
        ny = 0
    """ Scale perpendicular vector by random factor """
    r = random.uniform(-length/(1+smoothness),length/(1+smoothness))
    nx = nx * r
    ny = ny * r
    """ add scaled perpendicular vector to the half-way point to get the final
        displaced subdivision point """
    x = hx + nx
    y = hy + ny
    return [x, y]

class PathFractalize(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-s", "--subdivs",
                        action="store", type="int", 
                        dest="subdivs", default="6",
                        help="Number of subdivisons")
        self.OptionParser.add_option("-f", "--smooth",
                        action="store", type="float", 
                        dest="smooth", default="4.0",
                        help="Smoothness of the subdivision")
    def effect(self):
        for id, node in self.selected.iteritems():
            if node.tag == inkex.addNS('path','svg'):
                d = node.get('d')
                p = simplepath.parsePath(d)
                
                a = []
                first = 1
                for cmd,params in p:
                    if cmd != 'Z':
                        if first == 1:
                            x1 = params[-2]
                            y1 = params[-1]
                            a.append(['M',params[-2:]])
                            first = 2
                        else :
                            x2 = params[-2]
                            y2 = params[-1]
                            self.fractalize(a,x1,y1,x2,y2,self.options.subdivs,self.options.smooth)
                            x1 = x2
                            y1 = y2
                            a.append(['L',params[-2:]])

                node.set('d', simplepath.formatPath(a))

    def fractalize(self,a,x1,y1,x2,y2,s,f):
        subdivPoint = calculateSubdivision(x1,y1,x2,y2,f)
        
        if s > 0 :
            """ recursively subdivide the segment left of the subdivision point """
            self.fractalize(a,x1,y1,subdivPoint[-2],subdivPoint[-1],s-1,f)
            a.append(['L',subdivPoint])
            """ recursively subdivide the segment right of the subdivision point """
            self.fractalize(a,subdivPoint[-2],subdivPoint[-1],x2,y2,s-1,f)
             
if __name__ == '__main__':
    e = PathFractalize()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
