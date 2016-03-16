#!/usr/bin/env python
'''
Copyright (C) 2007 Aaron Spike  (aaron @ ekips.org)
Copyright (C) 2007 Tavmjong Bah (tavmjong @ free.fr)

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
import simplestyle, sys
from simpletransform import computePointInNode
from math import *
import string

def involute_intersect_angle(Rb, R):
    Rb, R = float(Rb), float(R)
    return (sqrt(R**2 - Rb**2) / (Rb)) - (acos(Rb / R))

def point_on_circle(radius, angle):
    x = radius * cos(angle)
    y = radius * sin(angle)
    return (x, y)

def points_to_svgd(p):
    f = p[0]
    p = p[1:]
    svgd = 'M%.5f,%.5f' % f
    for x in p:
        svgd += ' L%.5f,%.5f' % x
    svgd += 'z'
    return svgd

class Gears(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-t", "--teeth",
                        action="store", type="int",
                        dest="teeth", default=24,
                        help="Number of teeth")
        self.OptionParser.add_option("-p", "--pitch",
                        action="store", type="float",
                        dest="pitch", default=20.0,
                        help="Circular Pitch (length of arc from one tooth to next)")
        self.OptionParser.add_option("-a", "--angle",
                        action="store", type="float",
                        dest="angle", default=20.0,
                        help="Pressure Angle (common values: 14.5, 20, 25 degrees)")
        self.OptionParser.add_option("-c", "--centerdiameter",
                        action="store", type="float",
                        dest="centerdiameter", default=10.0,
                        help="Diameter of central hole - 0.0 for no hole")
        self.OptionParser.add_option("-u", "--unit",
                        action="store", type="string",
                        dest="unit", default="px",
                        help="unit of measure for circular pitch and center diameter")
    def effect(self):

        teeth = self.options.teeth
        pitch = self.unittouu( str(self.options.pitch) + self.options.unit)
        angle = self.options.angle  # Angle of tangent to tooth at circular pitch wrt radial line.
        centerdiameter = self.unittouu( str(self.options.centerdiameter) + self.options.unit)

        # print >>sys.stderr, "Teeth: %s\n"        % teeth

        two_pi = 2.0 * pi

        # Pitch (circular pitch): Length of the arc from one tooth to the next)
        # Pitch diameter: Diameter of pitch circle.
        pitch_diameter = float( teeth ) * pitch / pi
        pitch_radius   = pitch_diameter / 2.0

        # Base Circle
        base_diameter = pitch_diameter * cos( radians( angle ) )
        base_radius   = base_diameter / 2.0

        # Diametrial pitch: Number of teeth per unit length.
        pitch_diametrial = float( teeth )/ pitch_diameter

        # Addendum: Radial distance from pitch circle to outside circle.
        addendum = 1.0 / pitch_diametrial

        # Outer Circle
        outer_radius = pitch_radius + addendum
        outer_diameter = outer_radius * 2.0

        # Tooth thickness: Tooth width along pitch circle.
        tooth  = ( pi * pitch_diameter ) / ( 2.0 * float( teeth ) )

        # Undercut?
        undercut = (2.0 / ( sin( radians( angle ) ) ** 2))
        needs_undercut = teeth < undercut


        # Clearance: Radial distance between top of tooth on one gear to bottom of gap on another.
        clearance = 0.0

        # Dedendum: Radial distance from pitch circle to root diameter.
        dedendum = addendum + clearance

        # Root diameter: Diameter of bottom of tooth spaces. 
        root_radius =  pitch_radius - dedendum
        root_diameter = root_radius * 2.0

        half_thick_angle = two_pi / (4.0 * float( teeth ) )
        pitch_to_base_angle  = involute_intersect_angle( base_radius, pitch_radius )
        pitch_to_outer_angle = involute_intersect_angle( base_radius, outer_radius ) - pitch_to_base_angle

        centers = [(x * two_pi / float( teeth) ) for x in range( teeth ) ]

        points = []

        for c in centers:

            # Angles
            pitch1 = c - half_thick_angle
            base1  = pitch1 - pitch_to_base_angle
            outer1 = pitch1 + pitch_to_outer_angle

            pitch2 = c + half_thick_angle
            base2  = pitch2 + pitch_to_base_angle
            outer2 = pitch2 - pitch_to_outer_angle

            # Points
            b1 = point_on_circle( base_radius,  base1  )
            p1 = point_on_circle( pitch_radius, pitch1 )
            o1 = point_on_circle( outer_radius, outer1 )

            b2 = point_on_circle( base_radius,  base2  )
            p2 = point_on_circle( pitch_radius, pitch2 )
            o2 = point_on_circle( outer_radius, outer2 )

            if root_radius > base_radius:
                pitch_to_root_angle = pitch_to_base_angle - involute_intersect_angle(base_radius, root_radius )
                root1 = pitch1 - pitch_to_root_angle
                root2 = pitch2 + pitch_to_root_angle
                r1 = point_on_circle(root_radius, root1)
                r2 = point_on_circle(root_radius, root2)
                p_tmp = [r1,p1,o1,o2,p2,r2]
            else:
                r1 = point_on_circle(root_radius, base1)
                r2 = point_on_circle(root_radius, base2)
                p_tmp = [r1,b1,p1,o1,o2,p2,b2,r2]

            points.extend( p_tmp )

        path = points_to_svgd( points )

        # Embed gear in group to make animation easier:
        #  Translate group, Rotate path.
        view_center = computePointInNode(list(self.view_center), self.current_layer)
        t = 'translate(' + str( view_center[0] ) + ',' + str( view_center[1] ) + ')'
        g_attribs = {inkex.addNS('label','inkscape'):'Gear' + str( teeth ),
                     'transform':t }
        g = inkex.etree.SubElement(self.current_layer, 'g', g_attribs)

        # Create SVG Path for gear
        style = { 'stroke': '#000000', 'fill': 'none', 'stroke-width': str(self.unittouu('1px')) }
        gear_attribs = {'style':simplestyle.formatStyle(style), 'd':path}
        gear = inkex.etree.SubElement(g, inkex.addNS('path','svg'), gear_attribs )
        if(centerdiameter > 0.0):
            center_attribs = {'style':simplestyle.formatStyle(style), 
                inkex.addNS('cx','sodipodi')        :'0.0',
                inkex.addNS('cy','sodipodi')        :'0.0',
                inkex.addNS('rx','sodipodi')        :str(centerdiameter/2),
                inkex.addNS('ry','sodipodi')        :str(centerdiameter/2),
                inkex.addNS('type','sodipodi')      :'arc'
            }
            center = inkex.etree.SubElement(g, inkex.addNS('path','svg'), center_attribs )

if __name__ == '__main__':
    e = Gears()
    e.affect()



# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
