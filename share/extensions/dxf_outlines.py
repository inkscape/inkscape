#!/usr/bin/env python 
'''
Copyright (C) 2005,2007 Aaron Spike, aaron@ekips.org

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
import inkex, simplepath, cubicsuperpath

class MyEffect(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.dxf = ''
    def output(self):
        print self.dxf
    def dxf_add(self, str):
        self.dxf += str
    def dxf_line(self,csp):
        line = "\n0\nLINE\n8\n2\n62\n4\n10\n%f\n20\n%f\n30\n0\n11\n%f\n21\n%f\n31\n0" % (csp[0][0],csp[0][1],csp[1][0],csp[1][1])
        self.dxf_add(line)
    def dxf_spline(self,csp):
        knots = 8
        ctrls = 4
        self.dxf_add("\n  0\nSPLINE\n  5\n43\n  8\n0\n 62\n256\n370\n-1\n  6\nByLayer")
        self.dxf_add("\n100\nAcDbEntity\n100\nAcDbSpline\n 70\n8\n 71\n3\n 72\n%d\n 73\n%d\n 74\n0" % (knots, ctrls))
        for i in range(2):
            for j in range(4): 
                self.dxf_add("\n 40\n%d" % i)
        for i in csp:
            self.dxf_add("\n 10\n%f\n 20\n%f\n 30\n0" % (i[0],i[1]))
    def effect(self):
        #References:   Minimum Requirements for Creating a DXF File of a 3D Model By Paul Bourke
        #              NURB Curves: A Guide for the Uninitiated By Philip J. Schneider
        self.dxf_add("999\nDXF created by Inkscape\n0\nSECTION\n2\nENTITIES")
        
        scale = 25.4/90.0
        h = inkex.unittouu(self.document.getroot().xpath('@height',inkex.NSS)[0])
        
        path = '//svg:path'
        for node in self.document.getroot().xpath(path,inkex.NSS):
            d = node.get('d')
            sim = simplepath.parsePath(d)
            simplepath.scalePath(sim,scale,-scale)
            simplepath.translatePath(sim,0,h*scale)            
            p = cubicsuperpath.CubicSuperPath(sim)
            for sub in p:
                for i in range(len(sub)-1):
                    s = sub[i]
                    e = sub[i+1]
                    if s[1] == s[2] and e[0] == e[1]:
                        self.dxf_line([s[1],e[1]])
                    else:
                        self.dxf_spline([s[1],s[2],e[0],e[1]])
        self.dxf_add("\n0\nENDSEC\n0\nEOF\n")


e = MyEffect()
e.affect()
