#!/usr/bin/env python 
'''
This extension converts a path into a dashed line using 'stroke-dasharray'
It is a modification of the file addnodes.py

Copyright (C) 2005,2007 Aaron Spike, aaron@ekips.org
Copyright (C) 2009 Alvin Penner, penner@vaxxine.com

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
# local library
import inkex
import cubicsuperpath
import bezmisc
import simplestyle

def tpoint((x1,y1), (x2,y2), t = 0.5):
    return [x1+t*(x2-x1),y1+t*(y2-y1)]
def cspbezsplit(sp1, sp2, t = 0.5):
    m1=tpoint(sp1[1],sp1[2],t)
    m2=tpoint(sp1[2],sp2[0],t)
    m3=tpoint(sp2[0],sp2[1],t)
    m4=tpoint(m1,m2,t)
    m5=tpoint(m2,m3,t)
    m=tpoint(m4,m5,t)
    return [[sp1[0][:],sp1[1][:],m1], [m4,m,m5], [m3,sp2[1][:],sp2[2][:]]]
def cspbezsplitatlength(sp1, sp2, l = 0.5, tolerance = 0.001):
    bez = (sp1[1][:],sp1[2][:],sp2[0][:],sp2[1][:])
    t = bezmisc.beziertatlength(bez, l, tolerance)
    return cspbezsplit(sp1, sp2, t)
def cspseglength(sp1,sp2, tolerance = 0.001):
    bez = (sp1[1][:],sp1[2][:],sp2[0][:],sp2[1][:])
    return bezmisc.bezierlength(bez, tolerance)    

class SplitIt(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.not_converted = []

    def effect(self):
        for i, node in self.selected.iteritems():
            self.convert2dash(node)
        if len(self.not_converted):
            inkex.errormsg(_('Total number of objects not converted: {}\n').format(len(self.not_converted)))
            # return list of IDs in case the user needs to find a specific object
            inkex.debug(self.not_converted)

    def convert2dash(self, node):
        if node.tag == inkex.addNS('g', 'svg'):
            for child in node:
                self.convert2dash(child)
        else:
            if node.tag == inkex.addNS('path','svg'):
                dashes = []
                offset = 0
                style = simplestyle.parseStyle(node.get('style'))
                if style.has_key('stroke-dasharray'):
                    if style['stroke-dasharray'].find(',') > 0:
                        dashes = [float (dash) for dash in style['stroke-dasharray'].split(',')]
                if style.has_key('stroke-dashoffset'):
                    offset = style['stroke-dashoffset']
                if dashes:
                    p = cubicsuperpath.parsePath(node.get('d'))
                    new = []
                    for sub in p:
                        idash = 0
                        dash = dashes[0]
                        length = float (offset)
                        while dash < length:
                            length = length - dash
                            idash = (idash + 1) % len(dashes)
                            dash = dashes[idash]
                        new.append([sub[0][:]])
                        i = 1
                        while i < len(sub):
                            dash = dash - length
                            length = cspseglength(new[-1][-1], sub[i])
                            while dash < length:
                                new[-1][-1], next, sub[i] = cspbezsplitatlength(new[-1][-1], sub[i], dash/length)
                                if idash % 2:           # create a gap
                                    new.append([next[:]])
                                else:                   # splice the curve
                                    new[-1].append(next[:])
                                length = length - dash
                                idash = (idash + 1) % len(dashes)
                                dash = dashes[idash]
                            if idash % 2:
                                new.append([sub[i]])
                            else:
                                new[-1].append(sub[i])
                            i+=1
                    node.set('d',cubicsuperpath.formatPath(new))
                    del style['stroke-dasharray']
                    node.set('style', simplestyle.formatStyle(style))
                    if node.get(inkex.addNS('type','sodipodi')):
                        del node.attrib[inkex.addNS('type', 'sodipodi')]
            else:
                self.not_converted.append(node.get('id'))

if __name__ == '__main__':
    e = SplitIt()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
