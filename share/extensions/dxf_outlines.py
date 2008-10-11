#!/usr/bin/env python 
'''
Copyright (C) 2005,2007,2008 Aaron Spike, aaron@ekips.org
Copyright (C) 2008 Alvin Penner, penner@vaxxine.com

- template dxf_outlines.dxf added Feb 2008 by Alvin Penner
- ROBO-Master output option added Aug 2008 by Alvin Penner
- ROBO-Master multispline output added Sept 2008 by Alvin Penner

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
    inkex.errormsg("Failed to import the numpy or numpy.linalg modules. These modules are required by this extension. Please install them and try again.")
    inkex.sys.exit()

def pointdistance((x1,y1),(x2,y2)):
    return math.sqrt(((x2 - x1) ** 2) + ((y2 - y1) ** 2))

def get_fit(u, csp, col):
    return (1-u)**3*csp[0][col] + 3*(1-u)**2*u*csp[1][col] + 3*(1-u)*u**2*csp[2][col] + u**3*csp[3][col]

def get_matrix(u, i, j):
    if j == i + 2:
        return (u[i]-u[i-1])*(u[i]-u[i-1])/(u[i+2]-u[i-1])/(u[i+1]-u[i-1])
    elif j == i + 1:
        return ((u[i]-u[i-1])*(u[i+2]-u[i])/(u[i+2]-u[i-1]) + (u[i+1]-u[i])*(u[i]-u[i-2])/(u[i+1]-u[i-2]))/(u[i+1]-u[i-1])
    elif j == i:
        return (u[i+1]-u[i])*(u[i+1]-u[i])/(u[i+1]-u[i-2])/(u[i+1]-u[i-1])
    else:
        return 0

class MyEffect(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-R", "--ROBO", action="store", type="string", dest="ROBO")
        self.dxf = []
        self.handle = 255                       # handle for DXF ENTITY
        self.csp_old = [[0.0,0.0]]*4            # previous spline
        self.d = array([0], float)              # knot vector
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
        if (abs(csp[0][0] - self.csp_old[3][0]) > .0001
            or abs(csp[0][1] - self.csp_old[3][1]) > .0001
            or abs((csp[1][1]-csp[0][1])*(self.csp_old[3][0]-self.csp_old[2][0]) - (csp[1][0]-csp[0][0])*(self.csp_old[3][1]-self.csp_old[2][1])) > .001):
            self.ROBO_output()                              # terminate current spline
            self.xfit = array([csp[0][0]], float)           # initiallize new spline
            self.yfit = array([csp[0][1]], float)
            self.d = array([0], float)
        self.xfit = concatenate((self.xfit, zeros((3))))    # append to current spline
        self.yfit = concatenate((self.yfit, zeros((3))))
        self.d = concatenate((self.d, zeros((3))))
        for i in range(1, 4):
            j = len(self.d) + i - 4
            self.xfit[j] = get_fit(i/3.0, csp, 0)
            self.yfit[j] = get_fit(i/3.0, csp, 1)
            self.d[j] = self.d[j-1] + pointdistance((self.xfit[j-1],self.yfit[j-1]),(self.xfit[j],self.yfit[j]))
        self.csp_old = csp
    def ROBO_output(self):
        if len(self.d) == 1:
            return
        fits = len(self.d)
        ctrls = fits + 2
        knots = ctrls + 4
        self.xfit = concatenate((self.xfit, zeros((2))))    # pad with 2 endpoint constraints
        self.yfit = concatenate((self.yfit, zeros((2))))    # pad with 2 endpoint constraints
        self.d = concatenate((self.d, zeros((6))))          # pad with 3 duplicates at each end
        self.d[fits+2] = self.d[fits+1] = self.d[fits] = self.d[fits-1]
        solmatrix = zeros((ctrls,ctrls), dtype=float)
        for i in range(fits):
            solmatrix[i,i]   = get_matrix(self.d, i, i)
            solmatrix[i,i+1] = get_matrix(self.d, i, i+1)
            solmatrix[i,i+2] = get_matrix(self.d, i, i+2)
        solmatrix[fits, 0]   = self.d[2]/self.d[fits-1]     # curvature at start = 0
        solmatrix[fits, 1]   = -(self.d[1] + self.d[2])/self.d[fits-1]
        solmatrix[fits, 2]   = self.d[1]/self.d[fits-1]
        solmatrix[fits+1, fits-1] = (self.d[fits-1] - self.d[fits-2])/self.d[fits-1]   # curvature at end = 0
        solmatrix[fits+1, fits]   = (self.d[fits-3] + self.d[fits-2] - 2*self.d[fits-1])/self.d[fits-1]
        solmatrix[fits+1, fits+1] = (self.d[fits-1] - self.d[fits-3])/self.d[fits-1]
        xctrl = solve(solmatrix, self.xfit)
        yctrl = solve(solmatrix, self.yfit)
        self.dxf_add("  0\nSPLINE\n  5\n%x\n100\nAcDbEntity\n  8\n0\n100\nAcDbSpline\n" % self.handle)
        self.dxf_add(" 70\n0\n 71\n3\n 72\n%d\n 73\n%d\n 74\n%d\n" % (knots, ctrls, fits))
        for i in range(knots):
            self.dxf_add(" 40\n%f\n" % self.d[i-3])
        for i in range(ctrls):
            self.dxf_add(" 10\n%f\n 20\n%f\n 30\n0.0\n" % (xctrl[i],yctrl[i]))
        for i in range(fits):
            self.dxf_add(" 11\n%f\n 21\n%f\n 31\n0.0\n" % (self.xfit[i],self.yfit[i]))

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
            if len(sim):
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
        if self.options.ROBO == 'true':
            self.handle += 1
            self.ROBO_output()
        self.dxf_add(dxf_templates.r14_footer)

if __name__ == '__main__':
    e = MyEffect()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 encoding=utf-8 textwidth=99
