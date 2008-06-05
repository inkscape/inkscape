#!/usr/bin/env python 
'''
Copyright (C) 2008 Aaron Spike, aaron@ekips.org

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
import inkex, cubicsuperpath, simplepath, cspsubdiv

class MyEffect(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-f", "--flatness",
                        action="store", type="float", 
                        dest="flat", default=10.0,
                        help="Minimum flatness of the subdivided curves")
        self.hpgl = ['IN;SP1;']
    def output(self):
        print ''.join(self.hpgl)
    def effect(self):
        path = '//svg:path'
        for node in self.document.getroot().xpath(path, namespaces=inkex.NSS):
            d = node.get('d')
            p = cubicsuperpath.parsePath(d)
            cspsubdiv.cspsubdiv(p, self.options.flat)
            for sp in p:
                first = True
                for csp in sp:
                    cmd = 'PD'
                    if first:
                        cmd = 'PU'
                    first = False
                    self.hpgl.append('%s%s,%s;' % (cmd,csp[1][0],csp[1][1]))

e = MyEffect()
e.affect()