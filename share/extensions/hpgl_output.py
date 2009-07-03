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
import inkex, cubicsuperpath, simplepath, simplestyle, cspsubdiv

class MyEffect(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-f", "--flatness",
                        action="store", type="float", 
                        dest="flat", default=0.2,
                        help="Minimum flatness of the subdivided curves")
        self.OptionParser.add_option("-m", "--mirror",
                        action="store", type="inkbool", 
                        dest="mirror", default="FALSE",
                        help="Mirror Y-Axis")
        self.OptionParser.add_option("-x", "--xOrigin",
                        action="store", type="float", 
                        dest="xOrigin", default=0.0,
                        help="X Origin (pixels)")
        self.OptionParser.add_option("-y", "--yOrigin",
                        action="store", type="float", 
                        dest="yOrigin", default=0.0,
                        help="Y Origin (pixels)")
        self.OptionParser.add_option("-p", "--plotInvisibleLayers",
                        action="store", type="inkbool", 
                        dest="plotInvisibleLayers", default="FALSE",
                        help="Plot invisible layers")

        self.hpgl = ['IN;SP1;']
    def output(self):
        print ''.join(self.hpgl)
    def effect(self):
        x0 = self.options.xOrigin
        y0 = self.options.yOrigin
        scale = 1016.0/90
        mirror = 1.0
        if self.options.mirror:
            mirror = -1.0
            if self.document.getroot().get('height'):
                y0 -= float(self.document.getroot().get('height'))
        i = 0
        layerPath = '//svg:g[@inkscape:groupmode="layer"]'
        for layer in self.document.getroot().xpath(layerPath, namespaces=inkex.NSS):
            i += 1
            style = layer.get('style')
            if style:
                style = simplestyle.parseStyle(style)
                if style['display']=='none':
                    if not self.options.plotInvisibleLayers:
                        continue
            nodePath = ('//svg:g[@inkscape:groupmode="layer"][%d]/descendant::svg:path') % i
            for node in self.document.getroot().xpath(nodePath, namespaces=inkex.NSS):
                d = node.get('d')
                if len(simplepath.parsePath(d)):
                    p = cubicsuperpath.parsePath(d)
                    cspsubdiv.cspsubdiv(p, self.options.flat)
                    for sp in p:
                        first = True
                        for csp in sp:
                            cmd = 'PD'
                            if first:
                                cmd = 'PU'
                            first = False
                            self.hpgl.append('%s%d,%d;' % (cmd,(csp[1][0] - x0)*scale,(csp[1][1]*mirror - y0)*scale))

if __name__ == '__main__':   #pragma: no cover
    e = MyEffect()
    e.affect()

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 encoding=utf-8 textwidth=99
