#!/usr/bin/env python
'''
Copyright (C) 2009 Michel Chatelain.
Copyright (C) 2007 Tavmjong Bah, tavmjong@free.fr
Copyright (C) 2006 Georg Wiora, xorx@quarkbox.de
Copyright (C) 2006 Johan Engelen, johan@shouraizou.nl
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

Changes:
 * This program is derived by Michel Chatelain from funcplot.py. His changes are in the Public Domain.
 * Michel Chatelain, 17-18 janvier 2009, a partir de funcplot.py
 * 20 janvier 2009 : adaptation a la version 0.46 a partir de la nouvelle version de funcplot.py

'''

import inkex, simplepath, simplestyle
from math import *
from random import *

def drawfunction(t_start, t_end, xleft, xright, ybottom, ytop, samples, width, height, left, bottom,
    fx = "cos(3*t)", fy = "sin(5*t)", times2pi = False, isoscale = True, drawaxis = True):

    if times2pi == True:
        t_start = 2 * pi * t_start
        t_end   = 2 * pi * t_end

    # coords and scales based on the source rect
    scalex = width / (xright - xleft)
    xoff = left
    coordx = lambda x: (x - xleft) * scalex + xoff  #convert x-value to coordinate
    scaley = height / (ytop - ybottom)
    yoff = bottom
    coordy = lambda y: (ybottom - y) * scaley + yoff  #convert y-value to coordinate

    # Check for isotropic scaling and use smaller of the two scales, correct ranges
    if isoscale:
      if scaley<scalex:
        # compute zero location
        xzero = coordx(0)
        # set scale
        scalex = scaley
        # correct x-offset
        xleft = (left-xzero)/scalex
        xright = (left+width-xzero)/scalex
      else :
        # compute zero location
        yzero = coordy(0)
        # set scale
        scaley = scalex
        # correct x-offset
        ybottom = (yzero-bottom)/scaley
        ytop = (bottom+height-yzero)/scaley

    # functions specified by the user
    if fx != "":
        f1 = eval('lambda t: ' + fx.strip('"'))
    if fy != "":
        f2 = eval('lambda t: ' + fy.strip('"'))

    # step is increment of t
    step = (t_end - t_start) / (samples-1)
    third = step / 3.0
    ds = step * 0.001 # Step used in calculating derivatives

    a = [] # path array
    # add axis
    if drawaxis :
      # check for visibility of x-axis
      if ybottom<=0 and ytop>=0:
        # xaxis
        a.append(['M ',[left, coordy(0)]])
        a.append([' l ',[width, 0]])
      # check for visibility of y-axis
      if xleft<=0 and xright>=0:
        # xaxis
        a.append([' M ',[coordx(0),bottom]])
        a.append([' l ',[0, -height]])

    # initialize functions and derivatives for 0;
    # they are carried over from one iteration to the next, to avoid extra function calculations.
    x0 = f1(t_start)
    y0 = f2(t_start)

    # numerical derivatives, using 0.001*step as the small differential
    t1 = t_start + ds # Second point AFTER first point (Good for first point)
    x1 = f1(t1)
    y1 = f2(t1)
    dx0 = (x1 - x0)/ds
    dy0 = (y1 - y0)/ds

    # Start curve
    a.append([' M ',[coordx(x0), coordy(y0)]]) # initial moveto
    for i in range(int(samples-1)):
        t1 = (i+1) * step + t_start
        t2 = t1 - ds # Second point BEFORE first point (Good for last point)
        x1 = f1(t1)
        x2 = f1(t2)
        y1 = f2(t1)
        y2 = f2(t2)

        # numerical derivatives
        dx1 = (x1 - x2)/ds
        dy1 = (y1 - y2)/ds

        # create curve
        a.append([' C ',
                  [coordx(x0 + (dx0 * third)), coordy(y0 + (dy0 * third)),
                   coordx(x1 - (dx1 * third)), coordy(y1 - (dy1 * third)),
                   coordx(x1),                 coordy(y1)]
                  ])
        t0  = t1  # Next segment's start is this segments end
        x0  = x1
        y0  = y1
        dx0 = dx1 # Assume the functions are smooth everywhere, so carry over the derivatives too
        dy0 = dy1
    return a

class ParamCurves(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--t_start",
                        action="store", type="float",
                        dest="t_start", default=0.0,
                        help="Start t-value")
        self.OptionParser.add_option("--t_end",
                        action="store", type="float",
                        dest="t_end", default=1.0,
                        help="End t-value")
        self.OptionParser.add_option("--times2pi",
                        action="store", type="inkbool",
                        dest="times2pi", default=True,
                        help="Multiply t-range by 2*pi")
        self.OptionParser.add_option("--xleft",
                        action="store", type="float",
                        dest="xleft", default=-1.0,
                        help="x-value of rectangle's left")
        self.OptionParser.add_option("--xright",
                        action="store", type="float",
                        dest="xright", default=1.0,
                        help="x-value of rectangle's right")
        self.OptionParser.add_option("--ybottom",
                        action="store", type="float",
                        dest="ybottom", default=-1.0,
                        help="y-value of rectangle's bottom")
        self.OptionParser.add_option("--ytop",
                        action="store", type="float",
                        dest="ytop", default=1.0,
                        help="y-value of rectangle's top")
        self.OptionParser.add_option("-s", "--samples",
                        action="store", type="int",
                        dest="samples", default=8,
                        help="Samples")
        self.OptionParser.add_option("--fofx",
                        action="store", type="string",
                        dest="fofx", default="cos(3*t)",
                        help="fx(t) for plotting")
        self.OptionParser.add_option("--fofy",
                        action="store", type="string",
                        dest="fofy", default="sin(5*t)",
                        help="fy(t) for plotting")
        self.OptionParser.add_option("--remove",
                        action="store", type="inkbool",
                        dest="remove", default=True,
                        help="If True, source rectangle is removed")
        self.OptionParser.add_option("--isoscale",
                        action="store", type="inkbool",
                        dest="isoscale", default=True,
                        help="If True, isotropic scaling is used")
        self.OptionParser.add_option("--drawaxis",
                        action="store", type="inkbool",
                        dest="drawaxis", default=True,
                        help="If True, axis are drawn")
        self.OptionParser.add_option("--tab",
                        action="store", type="string",
                        dest="tab", default="sampling",
                        help="The selected UI-tab when OK was pressed")
        self.OptionParser.add_option("--paramcurvesuse",
                        action="store", type="string",
                        dest="paramcurvesuse", default="",
                        help="dummy")
        self.OptionParser.add_option("--pythonfunctions",
                        action="store", type="string",
                        dest="pythonfunctions", default="",
                        help="dummy")

    def effect(self):
        for id, node in self.selected.iteritems():
            if node.tag == inkex.addNS('rect','svg'):
                # create new path with basic dimensions of selected rectangle
                newpath = inkex.etree.Element(inkex.addNS('path','svg'))
                x = float(node.get('x'))
                y = float(node.get('y'))
                w = float(node.get('width'))
                h = float(node.get('height'))

                #copy attributes of rect
                s = node.get('style')
                if s:
                    newpath.set('style', s)

                t = node.get('transform')
                if t:
                    newpath.set('transform', t)

                # top and bottom were exchanged
                newpath.set('d', simplepath.formatPath(
                            drawfunction(self.options.t_start,
                                self.options.t_end,
                                self.options.xleft,
                                self.options.xright,
                                self.options.ybottom,
                                self.options.ytop,
                                self.options.samples,
                                w,h,x,y+h,
                                self.options.fofx,
                                self.options.fofy,
                                self.options.times2pi,
                                self.options.isoscale,
                                self.options.drawaxis)))
                newpath.set('title', self.options.fofx + " " + self.options.fofy)

                #newpath.set('desc', '!func;' + self.options.fofx + ';' + self.options.fofy + ';'
                #                                      + `self.options.t_start` + ';'
                #                                      + `self.options.t_end` + ';'
                #                                      + `self.options.samples`)

                # add path into SVG structure
                node.getparent().append(newpath)
                # option wether to remove the rectangle or not.
                if self.options.remove:
                  node.getparent().remove(node)

if __name__ == '__main__':
    e = ParamCurves()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
