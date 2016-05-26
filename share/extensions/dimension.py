#!/usr/bin/env python 
'''
dimension.py
An Inkscape effect for adding CAD style dimensions to selected objects
in a drawing.

It uses the selection's bounding box, so if the bounding box has empty
space in the x- or y-direction (such as with some stars) the results
will look strange.  Strokes might also overlap the edge of the 
bounding box.

The dimension arrows aren't measured: use the "Visualize Path/Measure
Path" effect to add measurements.

This code contains snippets from existing effects in the Inkscape
extensions library, and marker data from markers.svg.

Copyright (C) 2007 Peter Lewerin, peter.lewerin@tele2.se

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

# standard library
import sys
try:
    from subprocess import Popen, PIPE
    bsubprocess = True
except:
    bsubprocess = False
# local library
import inkex
import pathmodifier
from simpletransform import *


class Dimension(pathmodifier.PathModifier):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-x", "--xoffset",
                        action="store", type="float", 
                        dest="xoffset", default=100.0,
                        help="x offset of the vertical dimension arrow")    
        self.OptionParser.add_option("-y", "--yoffset",
                        action="store", type="float", 
                        dest="yoffset", default=100.0,
                        help="y offset of the horizontal dimension arrow")    
        self.OptionParser.add_option("-t", "--type",
                        action="store", type="string", 
                        dest="type", default="geometric",
                        help="Bounding box type")

    def addMarker(self, name, rotate):
        defs = self.xpathSingle('/svg:svg//svg:defs')
        if defs == None:
            defs = inkex.etree.SubElement(self.document.getroot(),inkex.addNS('defs','svg'))
        marker = inkex.etree.SubElement(defs ,inkex.addNS('marker','svg'))
        marker.set('id', name)
        marker.set('orient', 'auto')
        marker.set('refX', '0.0')
        marker.set('refY', '0.0')
        marker.set('style', 'overflow:visible')
        marker.set(inkex.addNS('stockid','inkscape'), name)

        arrow = inkex.etree.Element("path")
        arrow.set('d', 'M 0.0,0.0 L 5.0,-5.0 L -12.5,0.0 L 5.0,5.0 L 0.0,0.0 z ')
        if rotate:
            arrow.set('transform', 'scale(0.8) rotate(180) translate(12.5,0)')
        else:
            arrow.set('transform', 'scale(0.8) translate(12.5,0)')
        arrow.set('style', 'fill-rule:evenodd;stroke:#000000;stroke-width:1.0pt;marker-start:none')
        marker.append(arrow)

    def dimHLine(self, y, xlat):
        line = inkex.etree.Element("path")
        x1 = self.bbox[0] - xlat[0] * self.xoffset
        x2 = self.bbox[1]
        y = y - xlat[1] * self.yoffset
        line.set('d', 'M %f %f H %f' % (x1, y, x2))
        return line

    def dimVLine(self, x, xlat):
        line = inkex.etree.Element("path")
        x = x - xlat[0] * self.xoffset
        y1 = self.bbox[2] - xlat[1] * self.yoffset
        y2 = self.bbox[3]
        line.set('d', 'M %f %f V %f' % (x, y1, y2))
        return line

    def effect(self):
        scale = self.unittouu('1px')    # convert to document units
        self.xoffset = scale*self.options.xoffset
        self.yoffset = scale*self.options.yoffset

        # query inkscape about the bounding box
        if len(self.options.ids) == 0:
            inkex.errormsg(_("Please select an object."))
            exit()
        if self.options.type == "geometric":
            self.bbox = computeBBox(self.selected.values())
        else:
            q = {'x':0,'y':0,'width':0,'height':0}
            file = self.args[-1]
            id = self.options.ids[0]
            for query in q.keys():
                if bsubprocess:
                    p = Popen('inkscape --query-%s --query-id=%s "%s"' % (query,id,file), shell=True, stdout=PIPE, stderr=PIPE)
                    rc = p.wait()
                    q[query] = scale*float(p.stdout.read())
                    err = p.stderr.read()
                else:
                    f,err = os.popen3('inkscape --query-%s --query-id=%s "%s"' % (query,id,file))[1:]
                    q[query] = scale*float(f.read())
                    f.close()
                    err.close()
            self.bbox = (q['x'], q['x']+q['width'], q['y'], q['y']+q['height'])

        # Avoid ugly failure on rects and texts.
        try:
            testing_the_water = self.bbox[0]
        except TypeError:
            inkex.errormsg(_('Unable to process this object.  Try changing it into a path first.'))
            exit()

        layer = self.current_layer

        self.addMarker('Arrow1Lstart', False)
        self.addMarker('Arrow1Lend',  True)

        group = inkex.etree.SubElement(layer, 'g')
        # group = inkex.etree.Element("g")
        group.set('fill', 'none')
        group.set('stroke', 'black')

        line = self.dimHLine(self.bbox[2], [0, 1])
        line.set('marker-start', 'url(#Arrow1Lstart)')
        line.set('marker-end', 'url(#Arrow1Lend)')
        line.set('stroke-width', str(scale))
        group.append(line)

        line = self.dimVLine(self.bbox[0], [0, 2])
        line.set('stroke-width', str(0.5*scale))
        group.append(line)
        
        line = self.dimVLine(self.bbox[1], [0, 2])
        line.set('stroke-width', str(0.5*scale))
        group.append(line)
        
        line = self.dimVLine(self.bbox[0], [1, 0])
        line.set('marker-start', 'url(#Arrow1Lstart)')
        line.set('marker-end', 'url(#Arrow1Lend)')
        line.set('stroke-width', str(scale))
        group.append(line)
        
        line = self.dimHLine(self.bbox[2], [2, 0])
        line.set('stroke-width', str(0.5*scale))
        group.append(line)

        line = self.dimHLine(self.bbox[3], [2, 0])
        line.set('stroke-width', str(0.5*scale))
        group.append(line)

        for id, node in self.selected.iteritems():
            group.append(node)
        
        layer.append(group)

if __name__ == '__main__':
    e = Dimension()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
