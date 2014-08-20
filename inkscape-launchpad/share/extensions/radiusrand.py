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
'''
import random, math, inkex, cubicsuperpath

def randomize((x, y), rx, ry, norm):
    if norm:
        r = abs(random.normalvariate(0.0,0.5*max(rx, ry)))
    else:
        r = random.uniform(0.0,max(rx, ry))
    a = random.uniform(0.0,2*math.pi)
    x += math.cos(a)*rx
    y += math.sin(a)*ry
    return [x, y]

class RadiusRandomize(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--title")
        self.OptionParser.add_option("-x", "--radiusx",
                        action="store", type="float", 
                        dest="radiusx", default=10.0,
                        help="Randomly move nodes and handles within this radius, X")
        self.OptionParser.add_option("-y", "--radiusy",
                        action="store", type="float", 
                        dest="radiusy", default=10.0,
                        help="Randomly move nodes and handles within this radius, Y")
        self.OptionParser.add_option("-c", "--ctrl",
                        action="store", type="inkbool", 
                        dest="ctrl", default=True,
                        help="Randomize control points")
        self.OptionParser.add_option("-e", "--end",
                        action="store", type="inkbool", 
                        dest="end", default=True,
                        help="Randomize nodes")
        self.OptionParser.add_option("-n", "--norm",
                        action="store", type="inkbool", 
                        dest="norm", default=True,
                        help="Use normal distribution")
        self.OptionParser.add_option("--tab",
                        action="store", type="string",
                        dest="tab",
                        help="The selected UI-tab when OK was pressed")

    def effect(self):
        for id, node in self.selected.iteritems():
            if node.tag == inkex.addNS('path','svg'):
                d = node.get('d')
                p = cubicsuperpath.parsePath(d)
                for subpath in p:
                    for csp in subpath:
                        if self.options.end:
                            delta=randomize([0,0], self.options.radiusx, self.options.radiusy, self.options.norm)
                            csp[0][0]+=delta[0] 
                            csp[0][1]+=delta[1] 
                            csp[1][0]+=delta[0] 
                            csp[1][1]+=delta[1] 
                            csp[2][0]+=delta[0] 
                            csp[2][1]+=delta[1] 
                        if self.options.ctrl:
                            csp[0]=randomize(csp[0], self.options.radiusx, self.options.radiusy, self.options.norm)
                            csp[2]=randomize(csp[2], self.options.radiusx, self.options.radiusy, self.options.norm)
                node.set('d',cubicsuperpath.formatPath(p))

if __name__ == '__main__':
    e = RadiusRandomize()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
