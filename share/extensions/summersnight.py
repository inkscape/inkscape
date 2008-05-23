#!/usr/bin/env python
"""
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

"""
import inkex, os, simplepath, cubicsuperpath
from ffgeom import *

class Project(inkex.Effect):
    def __init__(self):
            inkex.Effect.__init__(self)
    def effect(self):
        if len(self.options.ids) < 2:
            inkex.debug("Requires two selected paths. The second must be exactly four nodes long.")
            exit()            
            
        #obj is selected second
        obj = self.selected[self.options.ids[0]]
        trafo = self.selected[self.options.ids[1]]
        if obj.tag == inkex.addNS('path','svg') and trafo.tag == inkex.addNS('path','svg'):
            #distil trafo into four node points
            trafo = cubicsuperpath.parsePath(trafo.get('d'))
            trafo = [[Point(csp[1][0],csp[1][1]) for csp in subs] for subs in trafo][0][:4]

            #vectors pointing away from the trafo origin
            self.t1 = Segment(trafo[0],trafo[1])
            self.t2 = Segment(trafo[1],trafo[2])
            self.t3 = Segment(trafo[3],trafo[2])
            self.t4 = Segment(trafo[0],trafo[3])
    
            #query inkscape about the bounding box of obj
            self.q = {'x':0,'y':0,'width':0,'height':0}
            file = self.args[-1]
            id = self.options.ids[0]
            for query in self.q.keys():
                _,f,err = os.popen3('inkscape --query-%s --query-id=%s "%s"' % (query,id,file))
                self.q[query] = float(f.read())
                f.close()
                err.close()

            #process path
            d = obj.get('d')
            p = cubicsuperpath.parsePath(d)
            for subs in p:
                for csp in subs:
                    csp[0] = self.trafopoint(csp[0])
                    csp[1] = self.trafopoint(csp[1])
                    csp[2] = self.trafopoint(csp[2])
            obj.set('d',cubicsuperpath.formatPath(p))

    def trafopoint(self,(x,y)):
        #Transform algorithm thanks to Jose Hevia (freon)
        vector = Segment(Point(self.q['x'],self.q['y']),Point(x,y))
        xratio = abs(vector.delta_x())/self.q['width']
        yratio = abs(vector.delta_y())/self.q['height']
    
        horz = Segment(self.t1.pointAtRatio(xratio),self.t3.pointAtRatio(xratio))
        vert = Segment(self.t4.pointAtRatio(yratio),self.t2.pointAtRatio(yratio))

        p = intersectSegments(vert,horz)
        return [p['x'],p['y']]    

if __name__ == '__main__':
    e = Project()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 encoding=utf-8 textwidth=99
