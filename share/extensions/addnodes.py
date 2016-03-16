#!/usr/bin/env python 
'''
This extension either adds nodes to a path so that
    a) no segment is longer than a maximum value 
    or
    b) so that each segment is divided into a given number of equal segments

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
'''

import inkex
import cubicsuperpath, simplestyle, copy, math, re, bezmisc

def numsegs(csp):
    return sum([len(p)-1 for p in csp])
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
def csplength(csp):
    total = 0
    lengths = []
    for sp in csp:
        lengths.append([])
        for i in xrange(1,len(sp)):
            l = cspseglength(sp[i-1],sp[i])
            lengths[-1].append(l)
            total += l            
    return lengths, total
def numlengths(csplen):
    retval = 0
    for sp in csplen:
        for l in sp:
            if l > 0:
                retval += 1
    return retval

class SplitIt(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--segments",
                        action="store", type="int", 
                        dest="segments", default=2,
                        help="Number of segments to divide the path into")
        self.OptionParser.add_option("--max",
                        action="store", type="float", 
                        dest="max", default=2,
                        help="Number of segments to divide the path into")
        self.OptionParser.add_option("--method",
                        action="store", type="string", 
                        dest="method", default='',
                        help="The kind of division to perform")

    def effect(self):

        for id, node in self.selected.iteritems():
            if node.tag == inkex.addNS('path','svg'):
                p = cubicsuperpath.parsePath(node.get('d'))
                
                #lens, total = csplength(p)
                #avg = total/numlengths(lens)
                #inkex.debug("average segment length: %s" % avg)

                new = []
                for sub in p:
                    new.append([sub[0][:]])
                    i = 1
                    while i <= len(sub)-1:
                        length = cspseglength(new[-1][-1], sub[i])
                        
                        if self.options.method == 'bynum':
                            splits = self.options.segments
                        else:
                            splits = math.ceil(length/self.options.max)

                        for s in xrange(int(splits),1,-1):
                            new[-1][-1], next, sub[i] = cspbezsplitatlength(new[-1][-1], sub[i], 1.0/s)
                            new[-1].append(next[:])
                        new[-1].append(sub[i])
                        i+=1
                    
                node.set('d',cubicsuperpath.formatPath(new))

if __name__ == '__main__':
    e = SplitIt()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
