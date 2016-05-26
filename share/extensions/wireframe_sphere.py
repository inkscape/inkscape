#!/usr/bin/env python 
# -*- coding: UTF-8 -*-
'''
Copyright (C) 2009 John Beard john.j.beard@gmail.com

######DESCRIPTION######

This extension renders a wireframe sphere constructed from lines of latitude
and lines of longitude.

The number of lines of latitude and longitude is independently variable. Lines 
of latitude and longtude are in separate subgroups. The whole figure is also in
its own group.

The whole sphere can be tilted towards or away from the veiwer by a given 
number of degrees. If the whole sphere is then rotated normally in Inkscape,
any position can be acheived.

There is an option to hide the lines at the back of the sphere, as if the 
sphere were opaque.
    #FIXME: Lines of latitude only have an approximation of the function needed
            to hide the back portion. If you can derive the proper equation,
            please add it in.
            Line of longitude have the exact method already.
            Workaround: Use the Inkscape ellipse tool to edit the start and end
            points of the lines of latitude to end at the horizon circle.
            
           
#TODO:  Add support for odd numbers of lines of longitude. This means breaking
        the line at the poles, and having two half ellipses for each line.
        The angles at which the ellipse arcs pass the poles are not constant and
        need to be derived before this can be implemented.
#TODO:  Add support for prolate and oblate spheroids

######LICENCE#######
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

######VERSION HISTORY#####
    Ver.       Date                       Notes
    
    0.10    2009-10-25  First version. Basic spheres supported.
                        Hidden lines of latitude still not properly calculated.
                        Prolate and oblate spheroids not considered.
'''
# standard library
from math import *
# local library
import inkex
import simplestyle
from simpletransform import computePointInNode


#SVG OUTPUT FUNCTIONS ================================================
def draw_SVG_ellipse((rx, ry), (cx, cy), width, parent, start_end=(0,2*pi),transform='' ):

    style = {   'stroke'        : '#000000',
                'stroke-width'  : str(width),
                'fill'          : 'none'            }
    circ_attribs = {'style':simplestyle.formatStyle(style),
        inkex.addNS('cx','sodipodi')        :str(cx),
        inkex.addNS('cy','sodipodi')        :str(cy),
        inkex.addNS('rx','sodipodi')        :str(rx),
        inkex.addNS('ry','sodipodi')        :str(ry),
        inkex.addNS('start','sodipodi')     :str(start_end[0]),
        inkex.addNS('end','sodipodi')       :str(start_end[1]),
        inkex.addNS('open','sodipodi')      :'true',    #all ellipse sectors we will draw are open
        inkex.addNS('type','sodipodi')      :'arc',
        'transform'                         :transform
        
            }
    circ = inkex.etree.SubElement(parent, inkex.addNS('path','svg'), circ_attribs )
    
class Wireframe_Sphere(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        
        #PARSE OPTIONS
        self.OptionParser.add_option("--num_lat",
            action="store", type="int",
            dest="NUM_LAT", default=19)
        self.OptionParser.add_option("--num_long",
            action="store", type="int",
            dest="NUM_LONG", default=24)
        self.OptionParser.add_option("--radius",
            action="store", type="float", 
            dest="RADIUS", default=100.0)
        self.OptionParser.add_option("--tilt",
            action="store", type="float",
            dest="TILT", default=35.0)
        self.OptionParser.add_option("--rotation",
            action="store", type="float",
            dest="ROT_OFFSET", default=4)
        self.OptionParser.add_option("--hide_back",
            action="store", type="inkbool", 
            dest="HIDE_BACK", default=False)
            
    def effect(self):
        
        so = self.options
        
        #PARAMETER PROCESSING
        
        if so.NUM_LONG % 2 != 0: #lines of longitude are odd : abort
            inkex.errormsg(_('Please enter an even number of lines of longitude.'))
        else:
            if so.TILT < 0:            # if the tilt is backwards
                flip = ' scale(1, -1)' # apply a vertical flip to the whole sphere
            else:
                flip = '' #no flip

            so.RADIUS     = self.unittouu(str(so.RADIUS) + 'px')
            so.TILT       =  abs(so.TILT)*(pi/180)  #Convert to radians
            so.ROT_OFFSET = so.ROT_OFFSET*(pi/180)  #Convert to radians
            stroke_width  = self.unittouu('1px')
            
            EPSILON = 0.001 #add a tiny value to the ellipse radii, so that if we get a zero radius, the ellipse still shows up as a line

            #INKSCAPE GROUP TO CONTAIN EVERYTHING
            
            centre = tuple(computePointInNode(list(self.view_center), self.current_layer))   #Put in in the centre of the current view
            grp_transform = 'translate' + str( centre ) + flip
            grp_name = 'WireframeSphere'
            grp_attribs = {inkex.addNS('label','inkscape'):grp_name,
                           'transform':grp_transform }
            grp = inkex.etree.SubElement(self.current_layer, 'g', grp_attribs)#the group to put everything in
            
            #LINES OF LONGITUDE
            
            if so.NUM_LONG > 0:      #only process longitudes if we actually want some
                
                #GROUP FOR THE LINES OF LONGITUDE
                grp_name = 'Lines of Longitude'
                grp_attribs = {inkex.addNS('label','inkscape'):grp_name}
                grp_long = inkex.etree.SubElement(grp, 'g', grp_attribs)
                
                delta_long = 360.0/so.NUM_LONG      #angle between neighbouring lines of longitude in degrees
                
                for i in range(0,so.NUM_LONG/2):
                    long_angle = so.ROT_OFFSET + (i*delta_long)*(pi/180.0); #The longitude of this particular line in radians
                    if long_angle > pi:
                        long_angle -= 2*pi
                    width      = so.RADIUS * cos(long_angle)
                    height     = so.RADIUS * sin(long_angle) * sin(so.TILT)       #the rise is scaled by the sine of the tilt
                    # length     = sqrt(width*width+height*height)  #by pythagorean theorem
                    # inverse    = sin(acos(length/so.RADIUS))
                    inverse    = abs(sin(long_angle)) * cos(so.TILT)
                    
                    minorRad   = so.RADIUS * inverse
                    minorRad=minorRad + EPSILON
                    
                    #calculate the rotation of the ellipse to get it to pass through the pole (in degrees)
                    rotation = atan(height/width)*(180.0/pi)
                    transform = "rotate("+str(rotation)+')' #generate the transform string
                    #the rotation will be applied about the group centre (the centre of the sphere)
                    
                    # remove the hidden side of the ellipses if required
                    # this is always exactly half the ellipse, but we need to find out which half
                    start_end = (0, 2*pi)   #Default start and end angles -> full ellipse
                    if so.HIDE_BACK:
                        if long_angle <= pi/2:           #cut out the half ellispse that is hidden
                            start_end = (pi/2, 3*pi/2)
                        else:
                            start_end = (3*pi/2, pi/2)
                    
                    #finally, draw the line of longitude
                    #the centre is always at the centre of the sphere
                    draw_SVG_ellipse( ( minorRad, so.RADIUS ), (0,0), stroke_width, grp_long , start_end,transform)
                
            # LINES OF LATITUDE
            if so.NUM_LAT > 0:
            
                #GROUP FOR THE LINES OF LATITUDE
                grp_name = 'Lines of Latitude'
                grp_attribs = {inkex.addNS('label','inkscape'):grp_name}
                grp_lat = inkex.etree.SubElement(grp, 'g', grp_attribs)
                
                
                so.NUM_LAT = so.NUM_LAT + 1     #Account for the fact that we loop over N-1 elements
                delta_lat = 180.0/so.NUM_LAT    #Angle between the line of latitude (subtended at the centre)
                
                for i in range(1,so.NUM_LAT):
                    lat_angle=((delta_lat*i)*(pi/180))            #The angle of this line of latitude (from a pole)
                    
                    majorRad=so.RADIUS*sin(lat_angle)                 #The width of the LoLat (no change due to projection)
                    minorRad=so.RADIUS*sin(lat_angle) * sin(so.TILT)     #The projected height of the line of latitude
                    minorRad=minorRad + EPSILON
                    
                    cy=so.RADIUS*cos(lat_angle) * cos(so.TILT) #The projected y position of the LoLat
                    cx=0                                    #The x position is just the center of the sphere
                    
                    if so.HIDE_BACK:
                        if lat_angle > so.TILT:                     #this LoLat is partially or fully visible
                            if lat_angle > pi-so.TILT:               #this LoLat is fully visible
                                draw_SVG_ellipse((majorRad, minorRad), (cx,cy), stroke_width, grp_lat)
                            else: #this LoLat is partially visible
                                proportion = -(acos( tan(lat_angle - pi/2)/tan(pi/2 - so.TILT)) )/pi + 1
                                start_end = ( pi/2 - proportion*pi, pi/2 + proportion*pi ) #make the start and end angles (mirror image around pi/2)
                                draw_SVG_ellipse((majorRad, minorRad), (cx,cy), stroke_width, grp_lat, start_end)
                            
                    else: #just draw the full lines of latitude
                        draw_SVG_ellipse((majorRad, minorRad), (cx,cy), stroke_width, grp_lat)
            
        
            #THE HORIZON CIRCLE
            draw_SVG_ellipse((so.RADIUS, so.RADIUS), (0,0), stroke_width, grp) #circle, centred on the sphere centre
            
if __name__ == '__main__':
    e = Wireframe_Sphere()
    e.affect()

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
