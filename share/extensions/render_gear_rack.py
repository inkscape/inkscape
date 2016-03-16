#!/usr/bin/env python
'''
Copyright (C) 2013 Brett Graham (hahahaha @ hahaha.org)

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
import simplestyle
from simpletransform import computePointInNode
from math import *


def involute_intersect_angle(Rb, R):
    Rb, R = float(Rb), float(R)
    return (sqrt(R**2 - Rb**2) / (Rb)) - (acos(Rb / R))


def point_on_circle(radius, angle):
    x = radius * cos(angle)
    y = radius * sin(angle)
    return (x, y)


def points_to_svgd(p):
    """
    p: list of 2 tuples (x, y coordinates)
    """
    f = p[0]
    p = p[1:]
    svgd = 'M%.3f,%.3f' % f
    for x in p:
        svgd += 'L%.3f,%.3f' % x
    return svgd


class RackGear(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option(
            "-l", "--length",
            action="store", type="float",
            dest="length", default=100.,
            help="Rack Length")
        self.OptionParser.add_option(
            "-s", "--spacing",
            action="store", type="float",
            dest="spacing", default=10.,
            help="Tooth Spacing")
        self.OptionParser.add_option(
            "-a", "--angle",
            action="store", type="float",
            dest="angle", default=20.,
            help="Contact Angle")

    def effect(self):
        length = self.unittouu(str(self.options.length) + 'px')
        spacing = self.unittouu(str(self.options.spacing) + 'px')
        angle = radians(self.options.angle)

        # generate points: list of (x, y) pairs
        points = []
        x = 0
        tas = tan(angle) * spacing
        while x < length:
            # move along path, generating the next 'tooth'
            points.append((x, 0))
            points.append((x + tas, spacing))
            points.append((x + spacing, spacing))
            points.append((x + spacing + tas, 0))
            x += spacing * 2.

        path = points_to_svgd(points)

        # Embed gear in group to make animation easier:
        #  Translate group, Rotate path.
        view_center = computePointInNode(list(self.view_center), self.current_layer)
        t = 'translate(' + str(view_center[0]) + ',' + \
            str(view_center[1]) + ')'
        g_attribs = {
            inkex.addNS('label', 'inkscape'): 'RackGear' + str(length),
            'transform': t}
        g = inkex.etree.SubElement(self.current_layer, 'g', g_attribs)

        # Create SVG Path for gear
        style = {'stroke': '#000000', 'fill': 'none', 'stroke-width': str(self.unittouu('1px'))}
        gear_attribs = {
            'style': simplestyle.formatStyle(style),
            'd': path}
        gear = inkex.etree.SubElement(
            g, inkex.addNS('path', 'svg'), gear_attribs)

if __name__ == '__main__':
    e = RackGear()
    e.affect()
