#!/usr/bin/env python 
'''
Copyright (C) 2005,2007,2008 Aaron Spike, aaron@ekips.org
Copyright (C) 2008 Alvin Penner, penner@vaxxine.com

- template dxf_outlines.dxf added Feb 2008 by Alvin Penner
- ROBO-Master output option added Aug 2008 by Alvin Penner

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
import inkex, simplepath, cubicsuperpath, dxf_templates, math

try:
    from numpy import *
    from numpy.linalg import solve
except:
    inkex.errormsg(_("Failed to import the numpy or numpy.linalg modules. These modules are required by this extension. Please install them and try again."))
    sys.exit()

def pointdistance((x1,y1),(x2,y2)):
    return math.sqrt(((x2 - x1) ** 2) + ((y2 - y1) ** 2))

def get_fit(u, csp, col):
    return (1-u)**3*csp[0][col] + 3*(1-u)**2*u*csp[1][col] + 3*(1-u)*u**2*csp[2][col] + u**3*csp[3][col]

class MyEffect(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-R", "--ROBO", action="store", type="string", dest="ROBO")
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
    def ROBO_spline(self,csp):
        # this spline has zero curvature at the endpoints, as in ROBO-Master
        knots = 10
        ctrls = 6
        fits = 4
        xfit = zeros((6), dtype=float64)
        xfit[0] = csp[0][0]
        xfit[1] = get_fit(.333, csp, 0)
        xfit[2] = get_fit(.667, csp, 0)
        xfit[3] = csp[3][0]
        yfit = zeros((6), dtype=float64)
        yfit[0] = csp[0][1]
        yfit[1] = get_fit(.333, csp, 1)
        yfit[2] = get_fit(.667, csp, 1)
        yfit[3] = csp[3][1]
        d1 = pointdistance((xfit[0],yfit[0]),(xfit[1],yfit[1]))
        d2 = pointdistance((xfit[1],yfit[1]),(xfit[2],yfit[2]))
        d3 = pointdistance((xfit[2],yfit[2]),(xfit[3],yfit[3]))
        u1 = d1/(d1 + d2 + d3)
        u2 = (d1 + d2)/(d1 + d2 + d3)
        solmatrix = zeros((6,6), dtype=float64)
        solmatrix[0,0] = 1
        solmatrix[1,1] = (1 - u1/u2)**2
        solmatrix[1,2] = (2*u2 - u1*u2 - u1)*u1/u2/u2
        solmatrix[1,3] = u1*u1/u2
        solmatrix[2,2] = (1 - u2)**2/(1 - u1)
        solmatrix[2,3] = (2*u2 - u1*u2 - u1)*(1 - u2)/(1 - u1)/(1 - u1)
        solmatrix[2,4] = ((u2 - u1)/(1 - u1))**2
        solmatrix[3,5] = 1
        solmatrix[4,0] = u2
        solmatrix[4,1] = -u1 - u2
        solmatrix[4,2] = u1
        solmatrix[5,3] = 1 - u2
        solmatrix[5,4] = u1 + u2 - 2
        solmatrix[5,5] = 1 - u1
        xctrl = solve(solmatrix, xfit)
        yctrl = solve(solmatrix, yfit)
        self.dxf_add("  0\nSPLINE\n  5\n%x\n100\nAcDbEntity\n  8\n0\n100\nAcDbSpline\n" % self.handle)
        self.dxf_add(" 70\n0\n 71\n3\n 72\n%d\n 73\n%d\n 74\n%d\n" % (knots, ctrls, fits))
        for i in range(4):
            self.dxf_add(" 40\n0\n")
        self.dxf_add(" 40\n%f\n" % u1)
        self.dxf_add(" 40\n%f\n" % u2)
        for i in range(4):
            self.dxf_add(" 40\n1\n")
        for i in range(6):
            self.dxf_add(" 10\n%f\n 20\n%f\n 30\n0.0\n" % (xctrl[i],yctrl[i]))
        for i in range(4):
            self.dxf_add(" 11\n%f\n 21\n%f\n 31\n0.0\n" % (xfit[i],yfit[i]))

    def effect(self):
        #References:   Minimum Requirements for Creating a DXF File of a 3D Model By Paul Bourke
        #              NURB Curves: A Guide for the Uninitiated By Philip J. Schneider
        #              The NURBS Book By Les Piegl and Wayne Tiller (Springer, 1995)
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
                    elif (self.options.ROBO == 'true'):
                        self.ROBO_spline([s[1],s[2],e[0],e[1]])
                    else:
                        self.dxf_spline([s[1],s[2],e[0],e[1]])

        self.dxf_add(dxf_templates.r14_footer)

if __name__ == '__main__':
    e = MyEffect()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 encoding=utf-8 textwidth=99
