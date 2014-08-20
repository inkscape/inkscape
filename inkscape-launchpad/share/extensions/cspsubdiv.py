#!/usr/bin/env python
from bezmisc import *
from ffgeom import *

def maxdist(((p0x,p0y),(p1x,p1y),(p2x,p2y),(p3x,p3y))):
    p0 = Point(p0x,p0y)
    p1 = Point(p1x,p1y)
    p2 = Point(p2x,p2y)
    p3 = Point(p3x,p3y)

    s1 = Segment(p0,p3)
    return max(s1.distanceToPoint(p1),s1.distanceToPoint(p2))
    

def cspsubdiv(csp,flat):
    for sp in csp:
        subdiv(sp,flat)

def subdiv(sp,flat,i=1):
    while i < len(sp):
        p0 = sp[i-1][1]
        p1 = sp[i-1][2]
        p2 = sp[i][0]
        p3 = sp[i][1]
        
        b = (p0,p1,p2,p3)
        m = maxdist(b)
        if m <= flat:
            i += 1
        else:
            one, two = beziersplitatt(b,0.5)
            sp[i-1][2] = one[1]
            sp[i][0] = two[2]
            p = [one[2],one[3],two[1]]
            sp[i:1] = [p]

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
