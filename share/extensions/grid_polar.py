#!/usr/bin/env python 
'''
Copyright (C) 2007 John Beard john.j.beard@gmail.com

##This extension allows you to draw a polar grid in Inkscape.
##There is a wide range of options including subdivision and labels.

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

import inkex
import simplestyle, sys
from math import *
from simpletransform import computePointInNode

def draw_SVG_circle(r, cx, cy, width, fill, name, parent):
    style = { 'stroke': '#000000', 'stroke-width':str(width), 'fill': fill }
    circ_attribs = {'style':simplestyle.formatStyle(style),
                    'cx':str(cx), 'cy':str(cy), 
                    'r':str(r),
                    inkex.addNS('label','inkscape'):name}
    circle = inkex.etree.SubElement(parent, inkex.addNS('circle','svg'), circ_attribs )

def draw_SVG_line(x1, y1, x2, y2, width, name, parent):
    style = { 'stroke': '#000000', 'stroke-width':str(width), 'fill': 'none' }
    line_attribs = {'style':simplestyle.formatStyle(style),
                    inkex.addNS('label','inkscape'):name,
                    'd':'M '+str(x1)+','+str(y1)+' L '+str(x2)+','+str(y2)}
    inkex.etree.SubElement(parent, inkex.addNS('path','svg'), line_attribs )
    
def draw_SVG_label_centred(x, y, string, font_size, name, parent):
    style = {'text-align': 'center', 'vertical-align': 'top',
             'text-anchor': 'middle', 'font-size': str(font_size)+'px',
             'fill-opacity': '1.0', 'stroke': 'none',
             'font-weight': 'normal', 'font-style': 'normal', 'fill': '#000000'}
    label_attribs = {'style':simplestyle.formatStyle(style),
                     inkex.addNS('label','inkscape'):name,
                     'x':str(x), 'y':str(y)}
    label = inkex.etree.SubElement(parent, inkex.addNS('text','svg'), label_attribs)
    label.text = string

class Grid_Polar(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--r_divs",
                        action="store", type="int", 
                        dest="r_divs", default=5,
                        help="Circular Divisions")
        self.OptionParser.add_option("--dr",
                        action="store", type="float", 
                        dest="dr", default=50,
                        help="Circular Division Spacing")
        self.OptionParser.add_option("--r_subdivs",
                        action="store", type="int", 
                        dest="r_subdivs", default=3,
                        help="Circular Subdivisions per Major division")
        self.OptionParser.add_option("--r_log",
                        action="store", type="inkbool", 
                        dest="r_log", default=False,
                        help="Logarithmic subdivisions if true")
        self.OptionParser.add_option("--r_divs_th",
                        action="store", type="float", 
                        dest="r_divs_th", default=2,
                        help="Major Circular Division Line thickness")
        self.OptionParser.add_option("--r_subdivs_th",
                        action="store", type="float", 
                        dest="r_subdivs_th", default=1,
                        help="Minor Circular Division Line thickness")
        self.OptionParser.add_option("--a_divs",
                        action="store", type="int", 
                        dest="a_divs", default=24,
                        help="Angle Divisions")
        self.OptionParser.add_option("--a_divs_cent",
                        action="store", type="int", 
                        dest="a_divs_cent", default=4,
                        help="Angle Divisions at Centre")
        self.OptionParser.add_option("--a_subdivs",
                        action="store", type="int", 
                        dest="a_subdivs", default=1,
                        help="Angcular Subdivisions per Major division")
        self.OptionParser.add_option("--a_subdivs_cent",
                        action="store", type="int", 
                        dest="a_subdivs_cent", default=1,
                        help="Angular Subdivisions end 'n' major circular divisions before the centre")
        self.OptionParser.add_option("--a_divs_th",
                        action="store", type="float", 
                        dest="a_divs_th", default=2,
                        help="Major Angular Division Line thickness")
        self.OptionParser.add_option("--a_subdivs_th",
                        action="store", type="float", 
                        dest="a_subdivs_th", default=1,
                        help="Minor Angular Division Line thickness")
        self.OptionParser.add_option("--c_dot_dia",
                        action="store", type="float", 
                        dest="c_dot_dia", default=5.0,
                        help="Diameter of Centre Dot")
        self.OptionParser.add_option("--a_labels",
                        action="store", type="string", 
                        dest="a_labels", default='deg',
                        help="The kind of labels to apply")
        self.OptionParser.add_option("--a_label_size",
                        action="store", type="int", 
                        dest="a_label_size", default=18,
                        help="The nominal pixel size of the circumferential labels")
        self.OptionParser.add_option("--a_label_outset",
                        action="store", type="float", 
                        dest="a_label_outset", default=24,
                        help="The radial outset of the circumferential labels")

    def effect(self):

        self.options.dr = self.unittouu(str(self.options.dr) + 'px')
        self.options.r_divs_th = self.unittouu(str(self.options.r_divs_th) + 'px')
        self.options.r_subdivs_th = self.unittouu(str(self.options.r_subdivs_th) + 'px')
        self.options.a_divs_th = self.unittouu(str(self.options.a_divs_th) + 'px')
        self.options.a_subdivs_th = self.unittouu(str(self.options.a_subdivs_th) + 'px')
        self.options.c_dot_dia = self.unittouu(str(self.options.c_dot_dia) + 'px')
        self.options.a_label_size = self.unittouu(str(self.options.a_label_size) + 'px')
        self.options.a_label_outset = self.unittouu(str(self.options.a_label_outset) + 'px')

        # Embed grid in group
        #Put in in the centre of the current view
        view_center = computePointInNode(list(self.view_center), self.current_layer)
        t = 'translate(' + str( view_center[0] ) + ',' + str( view_center[1] ) + ')'
        g_attribs = {inkex.addNS('label','inkscape'):'Grid_Polar:R' +
                                 str( self.options.r_divs )+':A'+str( self.options.a_divs ),
                     'transform':t }
        grid = inkex.etree.SubElement(self.current_layer, 'g', g_attribs)

        dr = self.options.dr                        #Distance between neighbouring circles
        dtheta = 2 * pi / self.options.a_divs_cent  #Angular change between adjacent radial lines at centre
        rmax = self.options.r_divs * dr
        
        #Create SVG circles
        for i in range(1, self.options.r_divs+1):
            draw_SVG_circle(i*dr, 0, 0, #major div circles
                            self.options.r_divs_th, 'none',
                            'MajorDivCircle'+str(i)+':R'+str(i*dr), grid)
            
            if self.options.r_log: #logarithmic subdivisions
                for j in range (2, self.options.r_subdivs):
                    draw_SVG_circle(i*dr-(1-log(j, self.options.r_subdivs))*dr, #minor div circles
                                    0, 0, self.options.r_subdivs_th, 'none',
                                    'MinorDivCircle'+str(i)+':Log'+str(j), grid)
            else: #linear subdivs
                for j in range (1, self.options.r_subdivs):
                    draw_SVG_circle(i*dr-j*dr/self.options.r_subdivs, #minor div circles
                                    0, 0, self.options.r_subdivs_th, 'none',
                                    'MinorDivCircle'+str(i)+':R'+str(i*dr), grid)
        
        if self.options.a_divs == self.options.a_divs_cent: #the lines can go from the centre to the edge
            for i in range(0, self.options.a_divs):
                draw_SVG_line(0, 0, rmax*sin(i*dtheta), rmax*cos(i*dtheta), 
                              self.options.a_divs_th, 'RadialGridline'+str(i), grid)
        
        else: #we need separate lines
            for i in range(0, self.options.a_divs_cent): #lines that go to the first circle
                draw_SVG_line(0, 0, dr*sin(i*dtheta), dr*cos(i*dtheta), 
                              self.options.a_divs_th, 'RadialGridline'+str(i), grid)
        
            dtheta = 2 * pi / self.options.a_divs #work out the angle change for outer lines
            
            for i in range(0, self.options.a_divs): #lines that go from there to the edge
                draw_SVG_line(  dr*sin(i*dtheta+pi/2.0),   dr*cos(i*dtheta+pi/2.0), 
                              rmax*sin(i*dtheta+pi/2.0), rmax*cos(i*dtheta+pi/2.0), 
                              self.options.a_divs_th, 'RadialGridline'+str(i), grid)
        
        if self.options.a_subdivs > 1: #draw angular subdivs
            for i in range(0, self.options.a_divs): #for each major divison
                for j in range(1, self.options.a_subdivs): #draw the subdivisions
                    angle = i*dtheta-j*dtheta/self.options.a_subdivs+pi/2.0 # the angle of the subdivion line
                    draw_SVG_line(dr*self.options.a_subdivs_cent*sin(angle),
                                  dr*self.options.a_subdivs_cent*cos(angle), 
                                  rmax*sin(angle), rmax*cos(angle), 
                                  self.options.a_subdivs_th, 'RadialMinorGridline'+str(i), grid)
        
        if self.options.c_dot_dia <> 0: #if a non-zero diameter, draw the centre dot
            draw_SVG_circle(self.options.c_dot_dia /2.0,
                            0, 0, 0, '#000000', 'CentreDot', grid)
        
        if self.options.a_labels == 'deg':
            label_radius = rmax+self.options.a_label_outset  #radius of label centres
            label_size = self.options.a_label_size
            numeral_size = 0.73*label_size #numerals appear to be 0.73 the height of the nominal pixel size of the font in "Sans"
            
            for i in range(0, self.options.a_divs):#self.options.a_divs): #radial line labels
                draw_SVG_label_centred(sin(i*dtheta+pi/2.0)*label_radius,        #0 at the RHS, mathematical style
                                       cos(i*dtheta+pi/2.0)*label_radius+ numeral_size/2.0, #centre the text vertically 
                                       str(i*360/self.options.a_divs), 
                                       label_size, 'Label'+str(i), grid)

if __name__ == '__main__':
    e = Grid_Polar()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
