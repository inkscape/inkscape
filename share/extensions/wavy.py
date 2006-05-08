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



drawwave() was translated into python from the postscript version
described at http://www.ghostscript.com/person/toby/ and located
at http://www.telegraphics.com.au/sw/sine.ps . 
http://www.tinaja.com/glib/bezsine.pdf shows another method for
approximating sine with beziers.

The orginal postscript version displayed the following copyright 
notice and was released under the terms of the GPL:
Copyright (C) 2001-3 Toby Thain, toby@telegraphics.com.au
'''
import inkex, simplepath, simplestyle
from math import *
from random import *

def drawwave(samples, periods, width, height, left, top, 
    fx = "sin(x)", fpx = "cos(x)", fponum = True):

    # step is the distance between nodes on x
    step = 2*pi / samples
    third = step / 3.0
    
    # coords and scales based on the source rect
    xoff = left
    yoff = top + (height / 2)
    scalex = width / (2*pi * periods)
    scaley = height / 2
    procx = lambda x: x * scalex + xoff
    procy = lambda y: y * scaley + yoff

    # functions specified by the user
    if fx != "":
        f = eval('lambda x: ' + fx)
    if fpx != "":
        fp = eval('lambda x: ' + fpx)

    # initialize function and derivative for 0;
    # they are carried over from one iteration to the next, to avoid extra function calculations         
    y0 = f(0) 
    if fponum == True: # numerical derivative, using 0.001*step as the small differential
        d0 = (f(0 + 0.001*step) - y0)/(0.001*step)
    else: # derivative given by the user
        d0 = fp(0)

    a = [] # path array 
    a.append(['M',[procx(0.0), procy(y0)]]) # initial moveto

    for i in range(int(samples * periods)):
        x = i * step 
        y1 = f(x + step)
        if fponum == True: # numerical derivative
            d1 = (y1 - f(x + step - 0.001*step))/(0.001*step)
        else: # derivative given by the user
            d1 = fp(x + step)
        # create curve
        a.append(['C',[procx(x + third), procy(y0 + (d0 * third)), 
            procx(x + (step - third)), procy(y1 - (d1 * third)),
            procx(x + step), procy(y1)]])
        y0 = y1 # next segment's y0 is this segment's y1
        d0 = d1 # we assume the function is smooth everywhere, so carry over the derivative too
            
    return a

class Wavy(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-p", "--periods",
                        action="store", type="float", 
                        dest="periods", default=4.0,
                        help="Periods (2*Pi each)")
        self.OptionParser.add_option("-s", "--samples",
                        action="store", type="int", 
                        dest="samples", default=8,
                        help="Samples per period")    
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
    def effect(self):
        for id, node in self.selected.iteritems():
            if node.tagName == 'rect':
                new = self.document.createElement('svg:path')
                x = float(node.attributes.getNamedItem('x').value)
                y = float(node.attributes.getNamedItem('y').value)
                w = float(node.attributes.getNamedItem('width').value)
                h = float(node.attributes.getNamedItem('height').value)

                s = node.attributes.getNamedItem('style').value
                new.setAttribute('style', s)
                try:
                    t = node.attributes.getNamedItem('transform').value
                    new.setAttribute('transform', t)
                except AttributeError:
                    pass
                new.setAttribute('d', simplepath.formatPath(
                            drawwave(self.options.samples, 
                                self.options.periods,
                                w,h,x,y,
                                self.options.fofx, 
                                self.options.fpofx,
                                self.options.fponum)))
                node.parentNode.appendChild(new)
                node.parentNode.removeChild(node)

e = Wavy()
e.affect()
