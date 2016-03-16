#!/usr/bin/env python
'''
Copyright (C) 2006 Jean-Francois Barraud, barraud@math.univ-lille1.fr

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
barraud@math.univ-lille1.fr

'''

import inkex, cubicsuperpath, bezmisc, pathmodifier
import copy, math, re

class RubberStretch(pathmodifier.Diffeo):
    def __init__(self):
        pathmodifier.Diffeo.__init__(self)
        self.OptionParser.add_option("-r", "--ratio",
                        action="store", type="float", 
                        dest="ratio", default=0.5)
        self.OptionParser.add_option("-c", "--curve",
                        action="store", type="float", 
                        dest="curve", default=0.5)

    def applyDiffeo(self,bpt,vects=()):
        for v in vects:
            v[0]-=bpt[0]
            v[1]-=bpt[1]
            v[1]*=-1
        bpt[1]*=-1
        a=self.options.ratio/100
        b=min(self.options.curve/100,0.99)
        x0= (self.bbox[0]+self.bbox[1])/2
        y0=-(self.bbox[2]+self.bbox[3])/2
        w,h=(self.bbox[1]-self.bbox[0])/2,(self.bbox[3]-self.bbox[2])/2
        
        x,y=(bpt[0]-x0),(bpt[1]-y0)
        sx=(1+b*(x/w+1)*(x/w-1))*2**(-a)
        sy=(1+b*(y/h+1)*(y/h-1))*2**(-a)
        bpt[0]=x0+x*sy
        bpt[1]=y0+y/sx
        for v in vects:
            dx,dy=v
            dXdx=sy
            dXdy= x*2*b*y/h/h*2**(-a)
            dYdx=-y*2*b*x/w/w*2**(-a)/sx/sx
            dYdy=1/sx
            v[0]=dXdx*dx+dXdy*dy
            v[1]=dYdx*dx+dYdy*dy
    
        #--spherify
        #s=((x*x+y*y)/(w*w+h*h))**(-a/2)
        #bpt[0]=x0+s*x
        #bpt[1]=y0+s*y
        #for v in vects:
        #    dx,dy=v
        #    v[0]=(1-a/2/(x*x+y*y)*2*x*x)*s*dx+( -a/2/(x*x+y*y)*2*y*x)*s*dy
        #    v[1]=( -a/2/(x*x+y*y)*2*x*y)*s*dx+(1-a/2/(x*x+y*y)*2*y*y)*s*dy
    
        for v in vects:
            v[0]+=bpt[0]
            v[1]+=bpt[1]
            v[1]*=-1
        bpt[1]*=-1

if __name__ == '__main__':
    e = RubberStretch()
    e.affect()

    
# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
