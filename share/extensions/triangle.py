#!/usr/bin/env python 
'''
Copyright (C) 2007 John Beard john.j.beard@gmail.com

##This extension allows you to draw a triangle given certain information
## about side length or angles.

##Measurements of the triangle

         C(x_c,y_c)                              
        /`__                                     
       / a_c``--__                               
      /           ``--__ s_a                     
 s_b /                  ``--__                   
    /a_a                    a_b`--__             
   /--------------------------------``B(x_b, y_b)
  A(x_a,y_a)         s_b                         


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

def draw_SVG_tri( (x1, y1), (x2, y2), (x3, y3), (ox,oy), width, name, parent):
    style = { 'stroke': '#000000', 'stroke-width':str(width), 'fill': 'none' }
    tri_attribs = {'style':simplestyle.formatStyle(style),
                    inkex.addNS('label','inkscape'):name,
                    'd':'M '+str(x1+ox)+','+str(y1+oy)+
                       ' L '+str(x2+ox)+','+str(y2+oy)+
                       ' L '+str(x3+ox)+','+str(y3+oy)+
                       ' L '+str(x1+ox)+','+str(y1+oy)+' z'}
    inkex.etree.SubElement(parent, inkex.addNS('path','svg'), tri_attribs )
    
def angle_from_3_sides(a, b, c): #return the angle opposite side c
    cosx = (a*a + b*b - c*c)/(2*a*b)  #use the cosine rule
    return acos(cosx)

def third_side_from_enclosed_angle(s_a,s_b,a_c): #return the side opposite a_c
    c_squared = s_a*s_a + s_b*s_b -2*s_a*s_b*cos(a_c)
    if c_squared > 0:
        return sqrt(c_squared)
    else:
        return 0 #means we have an invalid or degenerate triangle (zero is caught at the drawing stage)

def pt_on_circ(radius, angle): #return the x,y coordinate of the polar coordinate
    x = radius * cos(angle)
    y = radius * sin(angle)
    return [x, y]

def v_add( (x1,y1),(x2,y2) ):#add an offset to coordinates
    return [x1+x2, y1+y2]

def is_valid_tri_from_sides(a,b,c):#check whether triangle with sides a,b,c is valid
    return (a+b)>c and (a+c)>b and (b+c)>a and a > 0 and b> 0 and c>0#two sides must always be greater than the third
                #no zero-length sides, no degenerate case

def draw_tri_from_3_sides(s_a, s_b, s_c, offset, width, parent): #draw a triangle from three sides (with a given offset
    if is_valid_tri_from_sides(s_a,s_b,s_c):
        a_b = angle_from_3_sides(s_a, s_c, s_b)
                
        a = (0,0)    #a is the origin
        b = v_add(a, (s_c, 0)) #point B is horizontal from the origin
        c = v_add(b, pt_on_circ(s_a, pi-a_b) ) #get point c
        c[1] = -c[1]
        
        offx = max(b[0],c[0])/2 #b or c could be the furthest right
        offy = c[1]/2 #c is the highest point
        offset = ( offset[0]-offx , offset[1]-offy ) #add the centre of the triangle to the offset
               
        draw_SVG_tri(a, b, c , offset, width, 'Triangle', parent)
    else:
        sys.stderr.write('Error:Invalid Triangle Specifications.\n')

class Grid_Polar(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--s_a",
                        action="store", type="float", 
                        dest="s_a", default=100.0,
                        help="Side Length a")
        self.OptionParser.add_option("--s_b",
                        action="store", type="float", 
                        dest="s_b", default=100.0,
                        help="Side Length b")
        self.OptionParser.add_option("--s_c",
                        action="store", type="float", 
                        dest="s_c", default=100.0,
                        help="Side Length c")
        self.OptionParser.add_option("--a_a",
                        action="store", type="float", 
                        dest="a_a", default=60.0,
                        help="Angle a")
        self.OptionParser.add_option("--a_b",
                        action="store", type="float", 
                        dest="a_b", default=30.0,
                        help="Angle b")
        self.OptionParser.add_option("--a_c",
                        action="store", type="float", 
                        dest="a_c", default=90.0,
                        help="Angle c")
        self.OptionParser.add_option("--mode",
                        action="store", type="string", 
                        dest="mode", default='3_sides',
                        help="Side Length c")
    
    def effect(self):
        
        tri = self.current_layer
        offset = computePointInNode(list(self.view_center), self.current_layer) #the offset require to centre the triangle
        self.options.s_a = self.unittouu(str(self.options.s_a) + 'px')
        self.options.s_b = self.unittouu(str(self.options.s_b) + 'px')
        self.options.s_c = self.unittouu(str(self.options.s_c) + 'px')
        stroke_width = self.unittouu('2px')
        
        if self.options.mode == '3_sides':
            s_a = self.options.s_a
            s_b = self.options.s_b
            s_c = self.options.s_c
            draw_tri_from_3_sides(s_a, s_b, s_c, offset, stroke_width, tri)
        
        elif self.options.mode == 's_ab_a_c':
            s_a = self.options.s_a
            s_b = self.options.s_b
            a_c = self.options.a_c*pi/180 #in rad
            
            s_c = third_side_from_enclosed_angle(s_a,s_b,a_c)
            draw_tri_from_3_sides(s_a, s_b, s_c, offset, stroke_width, tri)
        
        elif self.options.mode == 's_ab_a_a':
            s_a = self.options.s_a
            s_b = self.options.s_b
            a_a = self.options.a_a*pi/180 #in rad
            
            if (a_a < pi/2.0) and (s_a < s_b) and (s_a > s_b*sin(a_a) ): #this is an ambigous case
                ambiguous=True#we will give both answers
            else:
                ambiguous=False
            
            sin_a_b =  s_b*sin(a_a)/s_a
            
            if (sin_a_b <= 1) and (sin_a_b >= -1):#check the solution is possible
                a_b = asin(sin_a_b) #acute solution
                a_c = pi - a_a - a_b
                error=False
            else:
                sys.stderr.write('Error:Invalid Triangle Specifications.\n')#signal an error
                error=True
            
            if not(error) and (a_b < pi) and (a_c < pi): #check that the solution is valid, if so draw acute solution
                s_c = third_side_from_enclosed_angle(s_a,s_b,a_c)
                draw_tri_from_3_sides(s_a, s_b, s_c, offset, stroke_width, tri)
            
            if not(error) and ((a_b > pi) or (a_c > pi) or ambiguous):#we want the obtuse solution
                a_b = pi - a_b
                a_c = pi - a_a - a_b
                s_c = third_side_from_enclosed_angle(s_a,s_b,a_c)
                draw_tri_from_3_sides(s_a, s_b, s_c, offset, stroke_width, tri)
        
        elif self.options.mode == 's_a_a_ab':
            s_a = self.options.s_a
            a_a = self.options.a_a*pi/180 #in rad
            a_b = self.options.a_b*pi/180 #in rad
            
            a_c = pi - a_a - a_b
            s_b = s_a*sin(a_b)/sin(a_a)
            s_c = s_a*sin(a_c)/sin(a_a)
            
            draw_tri_from_3_sides(s_a, s_b, s_c, offset, stroke_width, tri)
        
        elif self.options.mode == 's_c_a_ab':
            s_c = self.options.s_c
            a_a = self.options.a_a*pi/180 #in rad
            a_b = self.options.a_b*pi/180 #in rad
            
            a_c = pi - a_a - a_b
            s_a = s_c*sin(a_a)/sin(a_c)
            s_b = s_c*sin(a_b)/sin(a_c)
            
            draw_tri_from_3_sides(s_a, s_b, s_c, offset, stroke_width, tri)

if __name__ == '__main__':
    e = Grid_Polar()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
