#!/usr/bin/env python 
'''
Copyright (C) 2005,2007,2008 Aaron Spike, aaron@ekips.org
Copyright (C) 2008 Alvin Penner, penner@vaxxine.com

- template dxf_outlines.dxf added Feb 2008 by Alvin Penner, penner@vaxxine.com

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
import inkex, simplepath, cubicsuperpath, dxf_templates

class MyEffect(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.dxf = []
        self.handle = 255                       # initiallize handle for DXF ENTITY
    def output(self):
        print ''.join(self.dxf)
    def dxf_add(self, str):
        self.dxf.append(str)
    def dxf_line(self,csp):
        self.dxf_add("  0\nLINE\n  5\n%x\n100\nAcDbEntity\n  8\n0\n100\nAcDbLine\n" % self.handle)
        self.dxf_add(" 10\n%f\n 20\n%f\n 30\n0.0\n 11\n%f\n 21\n%f\n 31\n0.0\n" % (csp[0][0],csp[0][1],csp[1][0],csp[1][1]))
    def dxf_spline(self,csp):
        knots = 8
        ctrls = 4
        self.dxf_add("  0\nSPLINE\n  5\n%x\n100\nAcDbEntity\n  8\n0\n100\nAcDbSpline\n" % self.handle)
        self.dxf_add(" 70\n8\n 71\n3\n 72\n%d\n 73\n%d\n 74\n0\n" % (knots, ctrls))
        for i in range(2):
            for j in range(4): 
                self.dxf_add(" 40\n%d\n" % i)
        for i in csp:
            self.dxf_add(" 10\n%f\n 20\n%f\n 30\n0.0\n" % (i[0],i[1]))
    def effect(self):
        #References:   Minimum Requirements for Creating a DXF File of a 3D Model By Paul Bourke
        #              NURB Curves: A Guide for the Uninitiated By Philip J. Schneider
        self.dxf_add("999\nDXF created by Inkscape\n")
        self.dxf_add(dxf_templates.r14_header)

        scale = 25.4/90.0
        h = inkex.unittouu(self.document.getroot().xpath('@height', namespaces=inkex.NSS)[0])
        path = '//svg:path'
        for node in self.document.getroot().xpath(path, namespaces=inkex.NSS):
            d = node.get('d')
            sim = simplepath.parsePath(d)
            simplepath.scalePath(sim,scale,-scale)
            simplepath.translatePath(sim,0,h*scale)            
            p = cubicsuperpath.CubicSuperPath(sim)
            for sub in p:
                for i in range(len(sub)-1):
                    # generate unique handle for DXF ENTITY
                    self.handle += 1
                    s = sub[i]
                    e = sub[i+1]
                    if s[1] == s[2] and e[0] == e[1]:
                        self.dxf_line([s[1],e[1]])
                    else:
                        self.dxf_spline([s[1],s[2],e[0],e[1]])

        self.dxf_add(dxf_templates.r14_footer)

e = MyEffect()
e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 encoding=utf-8 textwidth=99
