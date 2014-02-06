#!/usr/bin/env python 
'''
Copyright (C) 2007 John Beard john.j.beard@gmail.com

##This extension allows you to draw various triangle constructions
##It requires a path to be selected
##It will use the first three nodes of this path

## Dimensions of a triangle__
#
#        /`__
#       / a_c``--__
#      /           ``--__ s_a
# s_b /                  ``--__
#    /a_a                    a_b`--__  
#   /--------------------------------``B
#  A              s_b

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
# standard library
import sys
from math import *
# local library
import inkex
import simplestyle
import simplepath

inkex.localize()

#DRAWING ROUTINES

#draw an SVG triangle given in trilinar coords
def draw_SVG_circle(rad, centre, params, style, name, parent):#draw an SVG circle with a given radius as trilinear coordinates
    if rad == 0: #we want a dot
        r = style.d_rad #get the dot width from the style
        circ_style = { 'stroke':style.d_col, 'stroke-width':str(style.d_th), 'fill':style.d_fill }
    else:
        r = rad #use given value
        circ_style = { 'stroke':style.c_col, 'stroke-width':str(style.c_th), 'fill':style.c_fill }

    cx,cy = get_cartesian_pt(centre, params)
    circ_attribs = {'style':simplestyle.formatStyle(circ_style),
                    inkex.addNS('label','inkscape'):name,
                    'cx':str(cx), 'cy':str(cy), 
                    'r':str(r)}
    inkex.etree.SubElement(parent, inkex.addNS('circle','svg'), circ_attribs )

#draw an SVG triangle given in trilinar coords
def draw_SVG_tri(vert_mat, params, style, name, parent):
    p1,p2,p3 = get_cartesian_tri(vert_mat, params) #get the vertex matrix in cartesian points
    tri_style   = { 'stroke': style.l_col, 'stroke-width':str(style.l_th), 'fill': style.l_fill }
    tri_attribs = {'style':simplestyle.formatStyle(tri_style),
                    inkex.addNS('label','inkscape'):name,
                    'd':'M '+str(p1[0])+','+str(p1[1])+
                       ' L '+str(p2[0])+','+str(p2[1])+
                       ' L '+str(p3[0])+','+str(p3[1])+
                       ' L '+str(p1[0])+','+str(p1[1])+' z'}
    inkex.etree.SubElement(parent, inkex.addNS('path','svg'), tri_attribs )

#draw an SVG line segment between the given (raw) points
def draw_SVG_line( (x1, y1), (x2, y2), style, name, parent):
    line_style   = { 'stroke': style.l_col, 'stroke-width':str(style.l_th), 'fill': style.l_fill }
    line_attribs = {'style':simplestyle.formatStyle(line_style),
                    inkex.addNS('label','inkscape'):name,
                    'd':'M '+str(x1)+','+str(y1)+' L '+str(x2)+','+str(y2)}
    inkex.etree.SubElement(parent, inkex.addNS('path','svg'), line_attribs )

#lines from each vertex to a corresponding point in trilinears
def draw_vertex_lines( vert_mat, params, width, name, parent):
    for i in range(3):
        oppositepoint = get_cartesian_pt( vert_mat[i], params)
        draw_SVG_line(params[3][-i%3], oppositepoint, width, name+':'+str(i), parent)
        
#MATHEMATICAL ROUTINES

def distance( (x0,y0),(x1,y1)):#find the pythagorean distance
    return sqrt( (x0-x1)*(x0-x1) + (y0-y1)*(y0-y1) )

def vector_from_to( (x0,y0),(x1,y1) ):#get the vector from (x0,y0) to (x1,y1)
    return (x1-x0, y1-y0)

def get_cartesian_pt( t, p):#get the cartesian coordinates from a trilinear set
    denom = p[0][0]*t[0] + p[0][1]*t[1] + p[0][2]*t[2]
    c1 = p[0][1]*t[1]/denom
    c2 = p[0][2]*t[2]/denom
    return ( c1*p[2][1][0]+c2*p[2][0][0], c1*p[2][1][1]+c2*p[2][0][1] )

def get_cartesian_tri( ((t11,t12,t13),(t21,t22,t23),(t31,t32,t33)), params):#get the cartesian points from a trilinear vertex matrix
    p1=get_cartesian_pt( (t11,t12,t13), params )
    p2=get_cartesian_pt( (t21,t22,t23), params )
    p3=get_cartesian_pt( (t31,t32,t33), params )
    return (p1,p2,p3)

def angle_from_3_sides(a, b, c): #return the angle opposite side c
    cosx = (a*a + b*b - c*c)/(2*a*b)  #use the cosine rule
    return acos(cosx)
    
def translate_string(string, os): #translates s_a, a_a, etc to params[x][y], with cyclic offset
    string = string.replace('s_a', 'params[0]['+str((os+0)%3)+']') #replace with ref. to the relvant values,
    string = string.replace('s_b', 'params[0]['+str((os+1)%3)+']') #cycled by i
    string = string.replace('s_c', 'params[0]['+str((os+2)%3)+']')
    string = string.replace('a_a', 'params[1]['+str((os+0)%3)+']')
    string = string.replace('a_b', 'params[1]['+str((os+1)%3)+']')
    string = string.replace('a_c', 'params[1]['+str((os+2)%3)+']')
    string = string.replace('area','params[4][0]')
    string = string.replace('semiperim','params[4][1]')
    return string

def pt_from_tcf( tcf , params):#returns a trilinear triplet from a triangle centre function
    trilin_pts=[]#will hold the final points
    for i in range(3):
        temp = tcf #read in the tcf
        temp = translate_string(temp, i)
        func = eval('lambda params: ' + temp.strip('"')) #the function leading to the trilinar element
        trilin_pts.append(func(params))#evaluate the function for the first trilinear element
    return trilin_pts
    
#SVG DATA PROCESSING
    
def get_n_points_from_path( node, n):#returns a list of first n points (x,y) in an SVG path-representing node

    p = simplepath.parsePath(node.get('d')) #parse the path
    
    xi = [] #temporary storage for x and y (will combine at end)
    yi = []
    
    for cmd,params in p:                    #a parsed path is made up of (cmd, params) pairs
        defs = simplepath.pathdefs[cmd]
        for i in range(defs[1]):
            if   defs[3][i] == 'x' and len(xi) < n:#only collect the first three
                xi.append(params[i])
            elif defs[3][i] == 'y' and len(yi) < n:#only collect the first three
                yi.append(params[i])

    if len(xi) == n and len(yi) == n:
        points = [] # returned pairs of points
        for i in range(n):
            xi[i] = Draw_From_Triangle.unittouu(e, str(xi[i]) + 'px')
            yi[i] = Draw_From_Triangle.unittouu(e, str(yi[i]) + 'px')
            points.append( [ xi[i], yi[i] ] )
    else:
        #inkex.errormsg(_('Error: Not enough nodes to gather coordinates.')) #fail silently and exit, rather than invoke an error console
        return [] #return a blank
        
    return points
    
#EXTRA MATHS FUNCTIONS
def sec(x):#secant(x)
    if x == pi/2 or x==-pi/2 or x == 3*pi/2 or x == -3*pi/2: #sec(x) is undefined
        return 100000000000
    else:
        return 1/cos(x)

def csc(x):#cosecant(x)
    if x == 0 or x==pi or x==2*pi or x==-2*pi: #csc(x) is undefined
        return 100000000000
    else:
        return 1/sin(x)

def cot(x):#cotangent(x)
    if x == 0 or x==pi or x==2*pi or x==-2*pi: #cot(x) is undefined
        return 100000000000
    else:
        return 1/tan(x)
        
def report_properties( params ):#report to the Inkscape console using errormsg
    inkex.errormsg(_("Side Length 'a' (px): " + str( params[0][0] ) ))
    inkex.errormsg(_("Side Length 'b' (px): " + str( params[0][1] ) ))
    inkex.errormsg(_("Side Length 'c' (px): " + str( params[0][2] ) ))
    inkex.errormsg(_("Angle 'A' (radians): " + str( params[1][0] ) ))
    inkex.errormsg(_("Angle 'B' (radians): " + str( params[1][1] ) ))
    inkex.errormsg(_("Angle 'C' (radians): " + str( params[1][2] ) ))
    inkex.errormsg(_("Semiperimeter (px): " + str( params[4][1] ) ))
    inkex.errormsg(_("Area (px^2): " + str( params[4][0] ) ))
    return
    

class Style(object): #container for style information
    def __init__(self, options):
        #dot markers
        self.d_rad = 4 #dot marker radius
        self.d_th  = Draw_From_Triangle.unittouu(e, '2px') #stroke width
        self.d_fill= '#aaaaaa' #fill colour
        self.d_col = '#000000' #stroke colour

        #lines
        self.l_th  = Draw_From_Triangle.unittouu(e, '2px')
        self.l_fill= 'none'
        self.l_col = '#000000'
        
        #circles
        self.c_th  = Draw_From_Triangle.unittouu(e, '2px')
        self.c_fill= 'none'
        self.c_col = '#000000'

class Draw_From_Triangle(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--tab",
                        action="store", type="string", 
                        dest="tab", default="sampling",
                        help="The selected UI-tab when OK was pressed") 
#PRESET POINT OPTIONS
        self.OptionParser.add_option("--circumcircle",
                        action="store", type="inkbool", 
                        dest="do_circumcircle", default=False)
        self.OptionParser.add_option("--circumcentre",
                        action="store", type="inkbool", 
                        dest="do_circumcentre", default=False)
        self.OptionParser.add_option("--incircle",
                        action="store", type="inkbool", 
                        dest="do_incircle", default=False)
        self.OptionParser.add_option("--incentre",
                        action="store", type="inkbool", 
                        dest="do_incentre", default=False)
        self.OptionParser.add_option("--contact_tri",
                        action="store", type="inkbool", 
                        dest="do_contact_tri", default=False)
        self.OptionParser.add_option("--excircles",
                        action="store", type="inkbool", 
                        dest="do_excircles", default=False)
        self.OptionParser.add_option("--excentres",
                        action="store", type="inkbool", 
                        dest="do_excentres", default=False)
        self.OptionParser.add_option("--extouch_tri",
                        action="store", type="inkbool", 
                        dest="do_extouch_tri", default=False)
        self.OptionParser.add_option("--excentral_tri",
                        action="store", type="inkbool", 
                        dest="do_excentral_tri", default=False)
        self.OptionParser.add_option("--orthocentre",
                        action="store", type="inkbool", 
                        dest="do_orthocentre", default=False)
        self.OptionParser.add_option("--orthic_tri",
                        action="store", type="inkbool", 
                        dest="do_orthic_tri", default=False)
        self.OptionParser.add_option("--altitudes",
                        action="store", type="inkbool", 
                        dest="do_altitudes", default=False)
        self.OptionParser.add_option("--anglebisectors",
                        action="store", type="inkbool", 
                        dest="do_anglebisectors", default=False)
        self.OptionParser.add_option("--centroid",
                        action="store", type="inkbool", 
                        dest="do_centroid", default=False)
        self.OptionParser.add_option("--ninepointcentre",
                        action="store", type="inkbool", 
                        dest="do_ninepointcentre", default=False)
        self.OptionParser.add_option("--ninepointcircle",
                        action="store", type="inkbool", 
                        dest="do_ninepointcircle", default=False)
        self.OptionParser.add_option("--symmedians",
                        action="store", type="inkbool", 
                        dest="do_symmedians", default=False)
        self.OptionParser.add_option("--sym_point",
                        action="store", type="inkbool", 
                        dest="do_sym_pt", default=False)
        self.OptionParser.add_option("--sym_tri",
                        action="store", type="inkbool", 
                        dest="do_sym_tri", default=False)
        self.OptionParser.add_option("--gergonne_pt",
                        action="store", type="inkbool", 
                        dest="do_gergonne_pt", default=False)
        self.OptionParser.add_option("--nagel_pt",
                        action="store", type="inkbool", 
                        dest="do_nagel_pt", default=False)
#CUSTOM POINT OPTIONS
        self.OptionParser.add_option("--mode",
                        action="store", type="string", 
                        dest="mode", default='trilin')
        self.OptionParser.add_option("--cust_str",
                        action="store", type="string", 
                        dest="cust_str", default='s_a')
        self.OptionParser.add_option("--cust_pt",
                        action="store", type="inkbool", 
                        dest="do_cust_pt", default=False)
        self.OptionParser.add_option("--cust_radius",
                        action="store", type="inkbool", 
                        dest="do_cust_radius", default=False)
        self.OptionParser.add_option("--radius",
                        action="store", type="string", 
                        dest="radius", default='s_a')
        self.OptionParser.add_option("--isogonal_conj",
                        action="store", type="inkbool", 
                        dest="do_isogonal_conj", default=False)
        self.OptionParser.add_option("--isotomic_conj",
                        action="store", type="inkbool", 
                        dest="do_isotomic_conj", default=False)
        self.OptionParser.add_option("--report",
                        action="store", type="inkbool", 
                        dest="report", default=False)


    def effect(self):
        
        so = self.options #shorthand
        
        pts = [] #initialise in case nothing is selected and following loop is not executed
        for id, node in self.selected.iteritems():
            if node.tag == inkex.addNS('path','svg'):
                pts = get_n_points_from_path( node, 3 ) #find the (x,y) coordinates of the first 3 points of the path


        if len(pts) == 3: #if we have right number of nodes, else skip and end program
            st = Style(so)#style for dots, lines and circles

            #CREATE A GROUP TO HOLD ALL GENERATED ELEMENTS IN
            #Hold relative to point A (pt[0])
            group_translation = 'translate(' + str( pts[0][0] ) + ','+ str( pts[0][1] ) + ')'
            group_attribs = {inkex.addNS('label','inkscape'):'TriangleElements',
                  'transform':group_translation }
            layer = inkex.etree.SubElement(self.current_layer, 'g', group_attribs)
            
            #GET METRICS OF THE TRIANGLE
            #vertices in the local coordinates (set pt[0] to be the origin)
            vtx = [[0,0],
                   [pts[1][0]-pts[0][0],pts[1][1]-pts[0][1]],
                   [pts[2][0]-pts[0][0],pts[2][1]-pts[0][1]]]
            
            s_a = distance(vtx[1],vtx[2])#get the scalar side lengths
            s_b = distance(vtx[0],vtx[1])
            s_c = distance(vtx[0],vtx[2])
            sides=(s_a,s_b,s_c)#side list for passing to functions easily and for indexing
            
            a_a = angle_from_3_sides(s_b, s_c, s_a)#angles in radians
            a_b = angle_from_3_sides(s_a, s_c, s_b)
            a_c = angle_from_3_sides(s_a, s_b, s_c)
            angles=(a_a,a_b,a_c)
            
            ab  = vector_from_to(vtx[0], vtx[1]) #vector from a to b
            ac  = vector_from_to(vtx[0], vtx[2]) #vector from a to c
            bc  = vector_from_to(vtx[1], vtx[2]) #vector from b to c
            vecs= (ab,ac) # vectors for finding cartesian point from trilinears

            semiperim = (s_a+s_b+s_c)/2.0 #semiperimeter
            area      = sqrt( semiperim*(semiperim-s_a)*(semiperim-s_b)*(semiperim-s_c) ) #area of the triangle by heron's formula
            uvals = (area, semiperim) #useful values
                    
            params = (sides, angles, vecs, vtx, uvals) #all useful triangle parameters in one object
            
            if so.report:
                report_properties( params )

            #BEGIN DRAWING
            if so.do_circumcentre or so.do_circumcircle:
                r  = s_a*s_b*s_c/(4*area)
                pt = (cos(a_a),cos(a_b),cos(a_c))
                if so.do_circumcentre:
                    draw_SVG_circle(0, pt, params, st, 'Circumcentre', layer)
                if so.do_circumcircle:
                    draw_SVG_circle(r, pt, params, st, 'Circumcircle', layer)
            
            if so.do_incentre or so.do_incircle:
                pt = [1,1,1]
                if so.do_incentre:
                    draw_SVG_circle(0, pt, params, st, 'Incentre', layer)
                if so.do_incircle:
                    r  = area/semiperim
                    draw_SVG_circle(r, pt, params, st, 'Incircle', layer)
            
            if so.do_contact_tri:
                t1 = s_b*s_c/(-s_a+s_b+s_c)
                t2 = s_a*s_c/( s_a-s_b+s_c)
                t3 = s_a*s_b/( s_a+s_b-s_c)
                v_mat = ( (0,t2,t3),(t1,0,t3),(t1,t2,0))
                draw_SVG_tri(v_mat, params, st,'ContactTriangle',layer)
                
            if so.do_extouch_tri:
                t1 = (-s_a+s_b+s_c)/s_a
                t2 = ( s_a-s_b+s_c)/s_b
                t3 = ( s_a+s_b-s_c)/s_c
                v_mat = ( (0,t2,t3),(t1,0,t3),(t1,t2,0))
                draw_SVG_tri(v_mat, params, st,'ExtouchTriangle',layer)
                          
            if so.do_orthocentre:
                pt = pt_from_tcf('cos(a_b)*cos(a_c)', params)
                draw_SVG_circle(0, pt, params, st, 'Orthocentre', layer)
            
            if so.do_orthic_tri:
                v_mat = [[0,sec(a_b),sec(a_c)],[sec(a_a),0,sec(a_c)],[sec(a_a),sec(a_b),0]]
                draw_SVG_tri(v_mat, params, st,'OrthicTriangle',layer)
            
            if so.do_centroid:
                pt = [1/s_a,1/s_b,1/s_c]
                draw_SVG_circle(0, pt, params, st, 'Centroid', layer)
            
            if so.do_ninepointcentre or so.do_ninepointcircle:
                pt = [cos(a_b-a_c),cos(a_c-a_a),cos(a_a-a_b)]
                if so.do_ninepointcentre:
                    draw_SVG_circle(0, pt, params, st, 'NinePointCentre', layer)
                if so.do_ninepointcircle:
                    r    = s_a*s_b*s_c/(8*area)
                    draw_SVG_circle(r, pt, params, st, 'NinePointCircle', layer)
            
            if so.do_altitudes:
                v_mat  = [[0,sec(a_b),sec(a_c)],[sec(a_a),0,sec(a_c)],[sec(a_a),sec(a_b),0]]
                draw_vertex_lines( v_mat, params, st, 'Altitude', layer)
            
            if so.do_anglebisectors:
                v_mat  = ((0,1,1),(1,0,1),(1,1,0))
                draw_vertex_lines(v_mat,params, st, 'AngleBisectors', layer)
            
            if so.do_excircles or so.do_excentres or so.do_excentral_tri:
                v_mat = ((-1,1,1),(1,-1,1),(1,1,-1))
                if so.do_excentral_tri:
                    draw_SVG_tri(v_mat, params, st,'ExcentralTriangle',layer)
                for i in range(3):
                    if so.do_excircles:
                        r = area/(semiperim-sides[i])
                        draw_SVG_circle(r, v_mat[i], params, st, 'Excircle:'+str(i), layer)
                    if so.do_excentres:
                        draw_SVG_circle(0, v_mat[i], params, st, 'Excentre:'+str(i), layer)
            
            if so.do_sym_tri or so.do_symmedians:
                v_mat = ((0,s_b,s_c), (s_a, 0, s_c), (s_a, s_b, 0))
                if so.do_sym_tri:
                    draw_SVG_tri(v_mat, params, st,'SymmedialTriangle',layer)
                if so.do_symmedians:
                    draw_vertex_lines(v_mat,params, st, 'Symmedian', layer)
            
            if so.do_sym_pt:
                pt = (s_a,s_b,s_c)
                draw_SVG_circle(0, pt, params, st, 'SymmmedianPoint', layer)
            
            if so.do_gergonne_pt:
                pt = pt_from_tcf('1/(s_a*(s_b+s_c-s_a))', params)
                draw_SVG_circle(0, pt, params, st, 'GergonnePoint', layer)
            
            if so.do_nagel_pt:
                pt = pt_from_tcf('(s_b+s_c-s_a)/s_a', params)
                draw_SVG_circle(0, pt, params, st, 'NagelPoint', layer)
            
            if so.do_cust_pt or so.do_cust_radius or so.do_isogonal_conj or so.do_isotomic_conj:
                pt = []#where we will store the point in trilinears
                if so.mode == 'trilin':#if we are receiving from trilinears
                    for i in range(3):
                        strings = so.cust_str.split(':')#get split string
                        strings[i] = translate_string(strings[i],0)
                        func = eval('lambda params: ' + strings[i].strip('"')) #the function leading to the trilinar element
                        pt.append(func(params)) #evaluate the function for the trilinear element
                else:#we need a triangle function
                    string = so.cust_str #don't need to translate, as the pt_from_tcf function does that for us
                    pt = pt_from_tcf(string, params)#get the point from the tcf directly
                    
                if so.do_cust_pt:#draw the point
                    draw_SVG_circle(0, pt, params, st, 'CustomTrilinearPoint', layer)
                if so.do_cust_radius:#draw the circle with given radius
                    strings = translate_string(so.radius,0)
                    func = eval('lambda params: ' + strings.strip('"')) #the function leading to the radius
                    r = func(params)
                    draw_SVG_circle(r, pt, params, st, 'CustomTrilinearCircle', layer)
                if so.do_isogonal_conj:
                    isogonal=[0,0,0]
                    for i in range (3):
                        isogonal[i] = 1/pt[i]
                    draw_SVG_circle(0, isogonal, params, st, 'CustomIsogonalConjugate', layer)
                if so.do_isotomic_conj:
                    isotomic=[0,0,0]
                    for i in range (3):
                        isotomic[i] = 1/( params[0][i]*params[0][i]*pt[i] )
                    draw_SVG_circle(0, isotomic, params, st, 'CustomIsotomicConjugate', layer)


if __name__ == '__main__':   #pragma: no cover
    e = Draw_From_Triangle()
    e.affect()
