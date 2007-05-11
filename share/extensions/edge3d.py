#!/usr/bin/env python 
'''
Copyright (C) 2007 Terry Brown, terry_n_brown@yahoo.com

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
import inkex, simplepath, sys
from math import degrees, atan2

class Edge3d(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        opts = [('-a', '--angle', 'float', 'angle', 45.0,
                 'angle of illumination, clockwise, 45 = upper right'),
                ('-d', '--stddev', 'float', 'stddev', 5.0,
                 'stdDeviation for Gaussian Blur'),
                ('-H', '--blurheight', 'float', 'blurheight', 2.0,
                 'height for Gaussian Blur'),
                ('-W', '--blurwidth', 'float', 'blurwidth', 2.0,
                 'width for Gaussian Blur'),
                ('-s', '--shades', 'int', 'shades', 2,
                 'shades, 2 = black and white, 3 = black, grey, white, etc.'),
                ('-b', '--bw', 'inkbool', 'bw', False,
                 'black and white, create only the fully black and white wedges'),
                ('-p', '--thick', 'float', 'thick', 10.,
                 'stroke-width for path pieces'),
                ]
        for o in opts:
            self.OptionParser.add_option(o[0], o[1], action="store", type=o[2],
                                         dest=o[3], default=o[4], help=o[5])

    def angleBetween(self, start, end, angle):
        """Return true if angle (degrees, clockwise, 0 = up/north) is between
           angles start and end"""
        def f(x):
            """Maybe add 360 to x"""
            if x < 0: return x + 360.
            return x
        # rotate all inputs by start, => start = 0
        a = f(f(angle) - f(start))
        e = f(f(end) - f(start))
        return a < e

    def effectX(self):
        # size of a wedge for shade i, wedges come in pairs
        delta = 360. / self.options.shades / 2.

        for shade in range(0, self.options.shades):
            if self.options.bw and shade > 0 and shade < self.options.shades-1:
                continue
            self.start = [self.options.angle - delta * (shade+1)]
            self.end = [self.options.angle - delta * (shade)]
            self.start.append( self.options.angle + delta * (shade) )
            self.end.append( self.options.angle + delta * (shade+1) )
            self.makeShade(float(shade)/float(self.options.shades-1))

    def effect(self):
        """Check each internode to see if it's in one of the wedges
           for the current shade.  shade is a floating point 0-1 white-black"""
        # size of a wedge for shade i, wedges come in pairs
        delta = 360. / self.options.shades / 2.
        for id, node in self.selected.iteritems():
            if node.tagName == 'path':
                d = node.attributes.getNamedItem('d')
                p = simplepath.parsePath(d.value)
                g = None
                for shade in range(0, self.options.shades):
                    if (self.options.bw and shade > 0 and
                        shade < self.options.shades - 1):
                        continue
                    self.start = [self.options.angle - delta * (shade+1)]
                    self.end = [self.options.angle - delta * (shade)]
                    self.start.append( self.options.angle + delta * (shade) )
                    self.end.append( self.options.angle + delta * (shade+1) )
                    level=float(shade)/float(self.options.shades-1)
                    last = []
                    result = []
                    for cmd,params in p:
                        if cmd == 'Z':
                            last = []
                            continue
                        if last:
                            a = degrees(atan2(params[-2:][0] - last[0],
                                              params[-2:][1] - last[1]))
                            if (self.angleBetween(self.start[0], self.end[0], a) or
                                self.angleBetween(self.start[1], self.end[1], a)):
                                result.append(('M', last))
                                result.append((cmd, params))
                        last = params[-2:]
                    if result:
                        if not g:
                            g = self.getGroup(node)
                        nn = g.appendChild(node.cloneNode(True))
                        d2 = nn.attributes.getNamedItem('d')
                        d2.value = simplepath.formatPath(result)
    
                        a = nn.ownerDocument.createAttribute('style')
                        col = 255 - int(255. * level)
                        a.value = 'fill:none;stroke:#%02x%02x%02x;stroke-opacity:1;stroke-width:10;%s' % ((col,)*3 + (self.filtId,))
                        nn.attributes.setNamedItem(a)
 
    def setAttr(self, node, name, value):
        attr = node.ownerDocument.createAttribute(name)
        attr.value = value
        node.attributes.setNamedItem(attr)
        
    def getGroup(self, node):
        defs = node.ownerDocument.getElementsByTagName('defs')
        if defs:
            defs = defs[0]
        if defs:
            # make a clipped group, clip with clone of original, clipped group
            # include original and group of paths
            clip = defs.appendChild(node.ownerDocument.createElement('clipPath'))
            clip.appendChild(node.cloneNode(True))
            clipId = self.uniqueId('clipPath')
            self.setAttr(clip, 'id', clipId)
            clipG = node.parentNode.appendChild(node.ownerDocument.createElement('g'))
            clipG.appendChild(node)
            g = clipG.appendChild(node.ownerDocument.createElement('g'))
            self.setAttr(clipG, 'clip-path', 'url(#'+clipId+')')
            # make a blur filter reference by the style of each path
            filt = defs.appendChild(node.ownerDocument.createElement('filter'))
            filtId = self.uniqueId('filter')
            for k, v in [('id', filtId), ('height', str(self.options.blurheight)),
                         ('width', str(self.options.blurwidth)),
                         ('x', '-0.5'), ('y', '-0.5')]:
                self.setAttr(filt, k, v)
            fe = filt.appendChild(node.ownerDocument.createElement('feGaussianBlur'))
            self.setAttr(fe, 'stdDeviation', str(self.options.stddev))
            self.filtId = 'filter:url(#%s);' % filtId
        else:
            # can't find defs, just group paths
            g = node.parentNode.appendChild(node.ownerDocument.createElement('g'))

        return g

e = Edge3d()
e.affect()
