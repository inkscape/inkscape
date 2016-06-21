#!/usr/bin/env python
'''
Copyright (C) 2012 Juan Pablo Carbajal ajuanpi-dev@gmail.com
Copyright (C) 2005 Aaron Spike, aaron@ekips.org

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
'''
import random, math, inkex, cubicsuperpath

def randomize((x, y), rx, ry, dist):

    if dist == "Gaussian":
        r1 = random.gauss(0.0,rx)
        r2 = random.gauss(0.0,ry)
    elif dist == "Pareto":
        '''
        sign is used ot fake a double sided pareto distribution.
        for parameter value between 1 and 2 the distribution has infinite variance
        I truncate the distribution to a high value and then normalize it.
        The idea is to get spiky distributions, any distribution with long-tails is
        good (ideal would be Levy distribution).
        '''
        sign = random.uniform(-1.0,1.0)

        r1 = min(random.paretovariate(1.0), 20.0)/20.0
        r2 = min(random.paretovariate(1.0), 20.0)/20.0

        r1 = rx * math.copysign(r1, sign)
        r2 = ry * math.copysign(r2, sign)
    elif dist == "Lognorm":
        sign = random.uniform(-1.0,1.0)
        r1 = rx * math.copysign(random.lognormvariate(0.0,1.0)/3.5,sign)
        r2 = ry * math.copysign(random.lognormvariate(0.0,1.0)/3.5,sign)
    elif dist == "Uniform":
        r1 = random.uniform(-rx,rx)
        r2 = random.uniform(-ry,ry)

    x += r1
    y += r2

    return [x, y]

class JitterNodes(inkex.Effect):
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
        self.OptionParser.add_option("-d", "--dist",
                        action="store", type="string",
                        dest="dist", default="Uniform",
                        help="Choose the distribution of the displacements")
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
                            delta=randomize([0,0], self.options.radiusx, self.options.radiusy, self.options.dist)
                            csp[0][0]+=delta[0]
                            csp[0][1]+=delta[1]
                            csp[1][0]+=delta[0]
                            csp[1][1]+=delta[1]
                            csp[2][0]+=delta[0]
                            csp[2][1]+=delta[1]
                        if self.options.ctrl:
                            csp[0]=randomize(csp[0], self.options.radiusx, self.options.radiusy, self.options.dist)
                            csp[2]=randomize(csp[2], self.options.radiusx, self.options.radiusy, self.options.dist)
                node.set('d',cubicsuperpath.formatPath(p))

if __name__ == '__main__':
    e = JitterNodes()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
