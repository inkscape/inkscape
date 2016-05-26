#!/usr/bin/env python
"""
Copyright (C) 2010 Alvin Penner, penner@vaxxine.com

- Voronoi Diagram algorithm and C code by Steven Fortune, 1987, http://ect.bell-labs.com/who/sjf/
- Python translation to file voronoi.py by Bill Simons, 2005, http://www.oxfish.com/

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
import random
# local library
import inkex
import simplestyle, simpletransform
import voronoi

try:
    from subprocess import Popen, PIPE
except:
    inkex.errormsg(_("Failed to import the subprocess module. Please report this as a bug at: https://bugs.launchpad.net/inkscape."))
    inkex.errormsg(_("Python version is: ") + str(inkex.sys.version_info))
    exit()

def clip_line(x1, y1, x2, y2, w, h):
    if x1 < 0 and x2 < 0:
        return [0, 0, 0, 0]
    if x1 > w and x2 > w:
        return [0, 0, 0, 0]
    if x1 < 0:
        y1 = (y1*x2 - y2*x1)/(x2 - x1)
        x1 = 0
    if x2 < 0:
        y2 = (y1*x2 - y2*x1)/(x2 - x1)
        x2 = 0
    if x1 > w:
        y1 = y1 + (w - x1)*(y2 - y1)/(x2 - x1)
        x1 = w
    if x2 > w:
        y2 = y1 + (w - x1)*(y2 - y1)/(x2 - x1)
        x2 = w
    if y1 < 0 and y2 < 0:
        return [0, 0, 0, 0]
    if y1 > h and y2 > h:
        return [0, 0, 0, 0]
    if x1 == x2 and y1 == y2:
        return [0, 0, 0, 0]
    if y1 < 0:
        x1 = (x1*y2 - x2*y1)/(y2 - y1)
        y1 = 0
    if y2 < 0:
        x2 = (x1*y2 - x2*y1)/(y2 - y1)
        y2 = 0
    if y1 > h:
        x1 = x1 + (h - y1)*(x2 - x1)/(y2 - y1)
        y1 = h
    if y2 > h:
        x2 = x1 + (h - y1)*(x2 - x1)/(y2 - y1)
        y2 = h
    return [x1, y1, x2, y2]

class Pattern(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--size",
                        action="store", type="int", 
                        dest="size", default=10,
                        help="Average size of cell (px)")
        self.OptionParser.add_option("--border",
                        action="store", type="int", 
                        dest="border", default=0,
                        help="Size of Border (px)")
        self.OptionParser.add_option("--tab",
                        action="store", type="string",
                        dest="tab",
                        help="The selected UI-tab when OK was pressed")

    def effect(self):
        if not self.options.ids:
            inkex.errormsg(_("Please select an object"))
            exit()
        scale = self.unittouu('1px')            # convert to document units
        self.options.size *= scale
        self.options.border *= scale
        q = {'x':0,'y':0,'width':0,'height':0}  # query the bounding box of ids[0]
        for query in q.keys():
            p = Popen('inkscape --query-%s --query-id=%s "%s"' % (query, self.options.ids[0], self.args[-1]), shell=True, stdout=PIPE, stderr=PIPE)
            rc = p.wait()
            q[query] = scale*float(p.stdout.read())
        mat = simpletransform.composeParents(self.selected[self.options.ids[0]], [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0]])
        defs = self.xpathSingle('/svg:svg//svg:defs')
        pattern = inkex.etree.SubElement(defs ,inkex.addNS('pattern','svg'))
        pattern.set('id', 'Voronoi' + str(random.randint(1, 9999)))
        pattern.set('width', str(q['width']))
        pattern.set('height', str(q['height']))
        pattern.set('patternTransform', 'translate(%s,%s)' % (q['x'] - mat[0][2], q['y'] - mat[1][2]))
        pattern.set('patternUnits', 'userSpaceOnUse')

        # generate random pattern of points
        c = voronoi.Context()
        pts = []
        b = float(self.options.border)          # width of border
        for i in range(int(q['width']*q['height']/self.options.size/self.options.size)):
            x = random.random()*q['width']
            y = random.random()*q['height']
            if b > 0:                           # duplicate border area
                pts.append(voronoi.Site(x, y))
                if x < b:
                    pts.append(voronoi.Site(x + q['width'], y))
                    if y < b:
                        pts.append(voronoi.Site(x + q['width'], y + q['height']))
                    if y > q['height'] - b:
                        pts.append(voronoi.Site(x + q['width'], y - q['height']))
                if x > q['width'] - b:
                    pts.append(voronoi.Site(x - q['width'], y))
                    if y < b:
                        pts.append(voronoi.Site(x - q['width'], y + q['height']))
                    if y > q['height'] - b:
                        pts.append(voronoi.Site(x - q['width'], y - q['height']))
                if y < b:
                    pts.append(voronoi.Site(x, y + q['height']))
                if y > q['height'] - b:
                    pts.append(voronoi.Site(x, y - q['height']))
            elif x > -b and y > -b and x < q['width'] + b and y < q['height'] + b:
                pts.append(voronoi.Site(x, y))  # leave border area blank
            # dot = inkex.etree.SubElement(pattern, inkex.addNS('rect','svg'))
            # dot.set('x', str(x-1))
            # dot.set('y', str(y-1))
            # dot.set('width', '2')
            # dot.set('height', '2')
        if len(pts) < 3:
            inkex.errormsg("Please choose a larger object, or smaller cell size")
            exit()

        # plot Voronoi diagram
        sl = voronoi.SiteList(pts)
        voronoi.voronoi(sl, c)
        path = ""
        for edge in c.edges:
            if edge[1] >= 0 and edge[2] >= 0:       # two vertices
                [x1, y1, x2, y2] = clip_line(c.vertices[edge[1]][0], c.vertices[edge[1]][1], c.vertices[edge[2]][0], c.vertices[edge[2]][1], q['width'], q['height'])
            elif edge[1] >= 0:                      # only one vertex
                if c.lines[edge[0]][1] == 0:        # vertical line
                    xtemp = c.lines[edge[0]][2]/c.lines[edge[0]][0]
                    if c.vertices[edge[1]][1] > q['height']/2:
                        ytemp = q['height']
                    else:
                        ytemp = 0
                else:
                    xtemp = q['width']
                    ytemp = (c.lines[edge[0]][2] - q['width']*c.lines[edge[0]][0])/c.lines[edge[0]][1]
                [x1, y1, x2, y2] = clip_line(c.vertices[edge[1]][0], c.vertices[edge[1]][1], xtemp, ytemp, q['width'], q['height'])
            elif edge[2] >= 0:                      # only one vertex
                if c.lines[edge[0]][1] == 0:        # vertical line
                    xtemp = c.lines[edge[0]][2]/c.lines[edge[0]][0]
                    if c.vertices[edge[2]][1] > q['height']/2:
                        ytemp = q['height']
                    else:
                        ytemp = 0
                else:
                    xtemp = 0
                    ytemp = c.lines[edge[0]][2]/c.lines[edge[0]][1]
                [x1, y1, x2, y2] = clip_line(xtemp, ytemp, c.vertices[edge[2]][0], c.vertices[edge[2]][1], q['width'], q['height'])
            if x1 or x2 or y1 or y2:
                path += 'M %.3f,%.3f %.3f,%.3f ' % (x1, y1, x2, y2)

        patternstyle = {'stroke': '#000000', 'stroke-width': str(scale)}
        attribs = {'d': path, 'style': simplestyle.formatStyle(patternstyle)}
        inkex.etree.SubElement(pattern, inkex.addNS('path', 'svg'), attribs)

        # link selected object to pattern
        obj = self.selected[self.options.ids[0]]
        style = {}
        if obj.attrib.has_key('style'):
            style = simplestyle.parseStyle(obj.attrib['style'])
        style['fill'] = 'url(#%s)' % pattern.get('id')
        obj.attrib['style'] = simplestyle.formatStyle(style)
        if obj.tag == inkex.addNS('g', 'svg'):
            for node in obj:
                style = {}
                if node.attrib.has_key('style'):
                    style = simplestyle.parseStyle(node.attrib['style'])
                style['fill'] = 'url(#%s)' % pattern.get('id')
                node.attrib['style'] = simplestyle.formatStyle(style)

if __name__ == '__main__':
    e = Pattern()
    e.affect()

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
