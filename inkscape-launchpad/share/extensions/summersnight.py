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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

"""
# standard library
import os
# local library
import cubicsuperpath
import inkex
import simplepath
import simpletransform
from ffgeom import *

try:
    from subprocess import Popen, PIPE
    bsubprocess = True
except:
    bsubprocess = False

class Project(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)

    def effect(self):
        if len(self.options.ids) < 2:
            inkex.errormsg(_("This extension requires two selected paths. \nThe second path must be exactly four nodes long."))
            exit()

        #obj is selected second
        scale = self.unittouu('1px')    # convert to document units
        obj = self.selected[self.options.ids[0]]
        trafo = self.selected[self.options.ids[1]]
        if obj.get(inkex.addNS('type','sodipodi')):
            inkex.errormsg(_("The first selected object is of type '%s'.\nTry using the procedure Path->Object to Path." % obj.get(inkex.addNS('type','sodipodi'))))
            exit()
        if obj.tag == inkex.addNS('path','svg') or obj.tag == inkex.addNS('g','svg'):
            if trafo.tag == inkex.addNS('path','svg'):
                #distil trafo into four node points
                mat = simpletransform.composeParents(trafo, [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0]])
                trafo = cubicsuperpath.parsePath(trafo.get('d'))
                if len(trafo[0]) < 4:
                    inkex.errormsg(_("This extension requires that the second selected path be four nodes long."))
                    exit()
                simpletransform.applyTransformToPath(mat, trafo)
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
                    if bsubprocess:
                        p = Popen('inkscape --query-%s --query-id=%s "%s"' % (query,id,file), shell=True, stdout=PIPE, stderr=PIPE)
                        rc = p.wait()
                        self.q[query] = scale*float(p.stdout.read())
                        err = p.stderr.read()
                    else:
                        f,err = os.popen3('inkscape --query-%s --query-id=%s "%s"' % (query,id,file))[1:]
                        self.q[query] = scale*float(f.read())
                        f.close()
                        err.close()

                if obj.tag == inkex.addNS("path",'svg'):
                    self.process_path(obj)
                if obj.tag == inkex.addNS("g",'svg'):
                    self.process_group(obj)
            else:
                if trafo.tag == inkex.addNS('g','svg'):
                    inkex.errormsg(_("The second selected object is a group, not a path.\nTry using the procedure Object->Ungroup."))
                else:
                    inkex.errormsg(_("The second selected object is not a path.\nTry using the procedure Path->Object to Path."))
                exit()
        else:
            inkex.errormsg(_("The first selected object is not a path.\nTry using the procedure Path->Object to Path."))
            exit()

    def process_group(self,group):
        for node in group:
            if node.tag == inkex.addNS('path','svg'):
                self.process_path(node)
            if node.tag == inkex.addNS('g','svg'):
                self.process_group(node)

    def process_path(self,path):
        mat = simpletransform.composeParents(path, [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0]])
        d = path.get('d')
        p = cubicsuperpath.parsePath(d)
        simpletransform.applyTransformToPath(mat, p)
        for subs in p:
            for csp in subs:
                csp[0] = self.trafopoint(csp[0])
                csp[1] = self.trafopoint(csp[1])
                csp[2] = self.trafopoint(csp[2])
        mat = simpletransform.invertTransform(mat)
        simpletransform.applyTransformToPath(mat, p)
        path.set('d',cubicsuperpath.formatPath(p))

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


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
