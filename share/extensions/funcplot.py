#!/usr/bin/env python 
'''
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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

This program is a modified version of wavy.py by Aaron Spike.

'''
import inkex, simplepath, simplestyle
from math import *
from random import *

def drawfunction(xstart, xend, ybottom, ytop, samples, width, height, left, top, 
    fx = "sin(x)", fpx = "cos(x)", fponum = True, times2pi = False):
    
    if times2pi == True:
        xstart = 2 * pi * xstart
        xend   = 2 * pi * xend   
    
    # step is the distance between nodes on x
    step = (xend - xstart) / (samples-1)
    third = step / 3.0
    
    # coords and scales based on the source rect
    scalex = width / (xend - xstart)
    xoff = left
    coordx = lambda x: (x - xstart) * scalex + xoff  #convert x-value to coordinate
    scaley = height / (ytop - ybottom)
    yoff = top
    coordy = lambda y: (ytop-y) * scaley + yoff  #convert y-value to coordinate

    # functions specified by the user
    if fx != "":
        f = eval('lambda x: ' + fx)
    if fpx != "":
        fp = eval('lambda x: ' + fpx)

    # initialize function and derivative for 0;
    # they are carried over from one iteration to the next, to avoid extra function calculations         
    y0 = f(xstart) 
    if fponum == True: # numerical derivative, using 0.001*step as the small differential
        d0 = (f(xstart + 0.001*step) - y0)/(0.001*step)
    else: # derivative given by the user
        d0 = fp(xstart)

    a = [] # path array 
    a.append(['M',[coordx(xstart), coordy(y0)]]) # initial moveto

    for i in range(int(samples-1)):
        x = (i+1) * step + xstart
        y1 = f(x)
        if fponum == True: # numerical derivative
            d1 = (y1 - f(x - 0.001*step))/(0.001*step)
        else: # derivative given by the user
            d1 = fp(x)
        # create curve
        a.append(['C',[coordx(x - step + third), coordy(y0 + (d0 * third)), 
            coordx(x - third), coordy(y1 - (d1 * third)),
            coordx(x), coordy(y1)]])
        y0 = y1 # next segment's y0 is this segment's y1
        d0 = d1 # we assume the function is smooth everywhere, so carry over the derivative too
            
    return a

class FuncPlot(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--xstart",
                        action="store", type="float", 
                        dest="xstart", default=0.0,
                        help="Start x-value")
        self.OptionParser.add_option("--xend",
                        action="store", type="float", 
                        dest="xend", default=1.0,
                        help="End x-value")
        self.OptionParser.add_option("--times2pi",
                        action="store", type="inkbool", 
                        dest="times2pi", default=True,
                        help="Multiply x-range by 2*pi")    
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
                        dest="fofx", default="sin(x)",
                        help="f(x) for plotting")    
        self.OptionParser.add_option("--fponum",
                        action="store", type="inkbool", 
                        dest="fponum", default=True,
                        help="Calculate the first derivative numerically")    
        self.OptionParser.add_option("--fpofx",
                        action="store", type="string", 
                        dest="fpofx", default="cos(x)",
                        help="f'(x) for plotting") 
        self.OptionParser.add_option("--tab",
                        action="store", type="string", 
                        dest="tab", default="sampling",
                        help="The selected UI-tab when OK was pressed") 
        self.OptionParser.add_option("--pythonfunctions",
                        action="store", type="string", 
                        dest="pythonfunctions", default="",
                        help="dummy") 

    def effect(self):
        for id, node in self.selected.iteritems():
            if node.tagName == 'rect':
                # create new path with basic dimensions of selected rectangle
                newpath = self.document.createElement('svg:path')
                x = float(node.attributes.getNamedItem('x').value)
                y = float(node.attributes.getNamedItem('y').value)
                w = float(node.attributes.getNamedItem('width').value)
                h = float(node.attributes.getNamedItem('height').value)

                #copy attributes of rect
                s = node.attributes.getNamedItem('style').value
                newpath.setAttribute('style', s)
                try:
                    t = node.attributes.getNamedItem('transform').value
                    newpath.setAttribute('transform', t)
                except AttributeError:
                    pass
                    
                newpath.setAttribute('d', simplepath.formatPath(
                            drawfunction(self.options.xstart,
                                self.options.xend,
                                self.options.ybottom,
                                self.options.ytop,
                                self.options.samples, 
                                w,h,x,y,
                                self.options.fofx, 
                                self.options.fpofx,
                                self.options.fponum,
                                self.options.times2pi)))
                newpath.setAttribute('title', self.options.fofx)
                
                #newpath.setAttribute('desc', '!func;' + self.options.fofx + ';' 
                #                                      + self.options.fpofx + ';'
                #                                      + `self.options.fponum` + ';'
                #                                      + `self.options.xstart` + ';'
                #                                      + `self.options.xend` + ';'
                #                                      + `self.options.samples`)
                                
                # add path into SVG structure
                node.parentNode.appendChild(newpath)
                # TODO: make an option wether to remove the rectangle or not.
                node.parentNode.removeChild(node)
                
e = FuncPlot()
e.affect()