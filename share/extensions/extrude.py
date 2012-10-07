#!/usr/bin/env python 
'''
Copyright (C) 2007

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
# local library
import inkex
import simplepath
import simpletransform
import cubicsuperpath

inkex.localize()

class Extrude(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        opts = [('-m', '--mode', 'string', 'mode', 'Lines',
                 'Join paths with lines or polygons'),
                ]
        for o in opts:
            self.OptionParser.add_option(o[0], o[1], action="store", type=o[2],
                                         dest=o[3], default=o[4], help=o[5])

    def effect(self):
        paths = []
        for id, node in self.selected.iteritems():
            if node.tag == '{http://www.w3.org/2000/svg}path':
                paths.append(node)
        if len(paths) < 2:
            inkex.errormsg(_('Need at least 2 paths selected'))
            return

        pts = [cubicsuperpath.parsePath(paths[i].get('d'))
               for i in range(len(paths))]

        for i in range(len(paths)):
            if 'transform' in paths[i].keys():
                trans = paths[i].get('transform')
                trans = simpletransform.parseTransform(trans)
                simpletransform.applyTransformToPath(trans, pts[i])

        for n1 in range(0, len(paths)):
            for n2 in range(n1 + 1, len(paths)):
                verts = []
                for i in range(0, min(map(len, pts))):
                    comp = []
                    for j in range(0, min(len(pts[n1][i]), len(pts[n2][i]))):
                        comp.append([pts[n1][i][j][1][-2:], pts[n2][i][j][1][-2:]])
                    verts.append(comp)

                if self.options.mode.lower() == 'lines':
                    line = []
                    for comp in verts:
                        for n,v in enumerate(comp):
                            line += [('M', v[0])]
                            line += [('L', v[1])]
                    ele = inkex.etree.Element('{http://www.w3.org/2000/svg}path')
                    paths[0].xpath('..')[0].append(ele)
                    ele.set('d', simplepath.formatPath(line))
                    ele.set('style', 'fill:none;stroke:#000000;stroke-opacity:1;stroke-width:1;')
                elif self.options.mode.lower() == 'polygons':
                    g = inkex.etree.Element('{http://www.w3.org/2000/svg}g')
                    g.set('style', 'fill:#000000;stroke:#000000;fill-opacity:0.3;stroke-width:2;stroke-opacity:0.6;')   
                    paths[0].xpath('..')[0].append(g)
                    for comp in verts:
                        for n,v in enumerate(comp):
                            nn = n+1
                            if nn == len(comp): nn = 0
                            line = []
                            line += [('M', comp[n][0])]
                            line += [('L', comp[n][1])]
                            line += [('L', comp[nn][1])]
                            line += [('L', comp[nn][0])]
                            line += [('L', comp[n][0])]
                            ele = inkex.etree.Element('{http://www.w3.org/2000/svg}path')
                            g.append(ele)
                            ele.set('d', simplepath.formatPath(line))


if __name__ == '__main__':   #pragma: no cover
    e = Extrude()
    e.affect()
