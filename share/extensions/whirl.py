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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
'''
import math, inkex, cubicsuperpath
from simpletransform import computePointInNode

class Whirl(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-t", "--whirl",
                        action="store", type="float", 
                        dest="whirl", default=1.0,
                        help="amount of whirl")
        self.OptionParser.add_option("-r", "--rotation",
                        action="store", type="inkbool", 
                        dest="rotation", default=True,
                        help="direction of rotation")
    def effect(self):
        view_center = computePointInNode(list(self.view_center), self.current_layer)
        for id, node in self.selected.iteritems():
            rotation = -1
            if self.options.rotation == True:
                rotation = 1
            whirl = self.options.whirl / 1000
            if node.tag == inkex.addNS('path','svg'):
                d = node.get('d')
                p = cubicsuperpath.parsePath(d)
                for sub in p:
                    for csp in sub:
                        for point in csp:
                            point[0] -= view_center[0]
                            point[1] -= view_center[1]
                            dist = math.sqrt((point[0] ** 2) + (point[1] ** 2))
                            if dist != 0:
                                a = rotation * dist * whirl
                                theta = math.atan2(point[1], point[0]) + a
                                point[0] = (dist * math.cos(theta))
                                point[1] = (dist * math.sin(theta))
                            point[0] += view_center[0]
                            point[1] += view_center[1]
                node.set('d',cubicsuperpath.formatPath(p))

if __name__ == '__main__':
    e = Whirl()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
