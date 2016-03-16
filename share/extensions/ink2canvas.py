#!/usr/bin/env python
'''
Copyright (C) 2011 Karlisson Bezerra <contact@hacktoon.com>

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

import inkex
from ink2canvas.canvas import Canvas
import ink2canvas.svg as svg

log = inkex.debug  #alias to debug method


class Ink2Canvas(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.canvas = None

    def output(self):
        import sys
        sys.stdout.write(self.canvas.output())

    def get_tag_name(self, node):
        # remove namespace part from "{http://www.w3.org/2000/svg}elem"
        return node.tag.split("}")[1]

    def get_gradient_defs(self, elem):
        url_id = elem.get_gradient_href()
        # get the gradient element
        gradient = self.xpathSingle("//*[@id='%s']" % url_id)
        # get the color stops
        url_stops = gradient.get(inkex.addNS("href", "xlink"))
        gstops = self.xpathSingle("//svg:linearGradient[@id='%s']" % url_stops[1:])
        colors = []
        for stop in gstops:
            colors.append(stop.get("style"))
        if gradient.get("r"):
            return svg.RadialGradientDef(gradient, colors)
        else:
            return svg.LinearGradientDef(gradient, colors)

    def get_clip_defs(self, elem):
        if elem.has_clip():
            pass
        return

    def walk_tree(self, root):
        for node in root:
            if node.tag is inkex.etree.Comment:
                continue
            tag = self.get_tag_name(node)
            class_name = tag.capitalize()
            if not hasattr(svg, class_name):
                continue
            gradient = None
            clip = None
            # creates a instance of 'elem'
            # similar to 'elem = Rect(tag, node, ctx)'
            elem = getattr(svg, class_name)(tag, node, self.canvas)
            if elem.has_gradient():
                gradient = self.get_gradient_defs(elem)
            elem.start(gradient)
            elem.draw()
            self.walk_tree(node)
            elem.end()

    def effect(self):
        """Applies the effect"""
        svg_root = self.document.getroot()
        width = self.unittouu(svg_root.get("width"))
        height = self.unittouu(svg_root.get("height"))
        self.canvas = Canvas(self, width, height)
        self.walk_tree(svg_root)


if __name__ == "__main__":
    ink = Ink2Canvas()
    ink.affect()
