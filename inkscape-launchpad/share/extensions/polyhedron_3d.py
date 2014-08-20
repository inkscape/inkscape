#!/usr/bin/env python 
'''
Copyright (C) 2007 John Beard john.j.beard@gmail.com

##This extension draws 3d objects from a Wavefront .obj 3D file stored in a local folder
##Many settings for appearance, lighting, rotation, etc are available.

#                              ^y
#                              |
#        __--``|               |_--``|     __--
#  __--``      |         __--``|     |_--``
# |       z    |        |      |_--``|
# |       <----|--------|-----_0-----|----------------
# |            |        |_--`` |     |
# |      __--``     <-``|      |_--``
# |__--``           x   |__--``|
#   IMAGE PLANE           SCENE|
#                              |

#Vertices are given as "v" followed by three numbers (x,y,z).
#All files need a vertex list
#v  x.xxx   y.yyy   z.zzz

#Faces are given by a list of vertices
#(vertex 1 is the first in the list above, 2 the second, etc):
#f  1   2   3

#Edges are given by a list of vertices. These will be broken down
#into adjacent pairs automatically.
#l  1   2   3

#Faces are rendered according to the painter's algorithm and perhaps
#back-face culling, if selected. The parameter to sort the faces by
#is user-selectable between max, min and average z-value of the vertices

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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
'''
# standard library
import sys
import re
from math import *
# local library
import inkex
import simplestyle

inkex.localize()

# third party
try:
    from numpy import *
except:
    inkex.errormsg(_("Failed to import the numpy module. This module is required by this extension. Please install it and try again.  On a Debian-like system this can be done with the command 'sudo apt-get install python-numpy'."))
    sys.exit()

#FILE IO ROUTINES
def get_filename(self_options):
        if self_options.obj == 'from_file':
            file = self_options.spec_file
        else:
            file = self_options.obj + '.obj'
            
        return file

def objfile(name):
    import os.path
    if __name__ == '__main__':
        filename = sys.argv[0]
    else:
        filename = __file__
    path = os.path.abspath(os.path.dirname(filename))
    path = os.path.join(path, 'Poly3DObjects', name)
    return path
    
def get_obj_data(obj, name):
    infile = open(objfile(name))
    
    #regular expressions
    getname = '(.[nN]ame:\\s*)(.*)'
    floating = '([\-\+\\d*\.e]*)'   #a possibly non-integer number, with +/- and exponent.
    getvertex = '(v\\s+)'+floating+'\\s+'+floating+'\\s+'+floating
    getedgeline = '(l\\s+)(.*)'
    getfaceline = '(f\\s+)(.*)'
    getnextint = '(\\d+)([/\\d]*)(.*)'#we need to deal with 123\343\123 or 123\\456 as equivalent to 123 (we are ignoring the other options in the obj file)
    
    for line in infile:
        if line[0]=='#':                    #we have a comment line
            m = re.search(getname, line)        #check to see if this line contains a name
            if m:
                obj.name = m.group(2)           #if it does, set the property
        elif line[0] == 'v':                #we have a vertex (maybe)
            m = re.search(getvertex, line)      #check to see if this line contains a valid vertex
            if m:                               #we have a valid vertex
                obj.vtx.append( [float(m.group(2)), float(m.group(3)), float(m.group(4)) ] )
        elif line[0] == 'l':                #we have a line (maybe)
            m = re.search(getedgeline, line)    #check to see if this line begins 'l '
            if m:                               #we have a line beginning 'l '
                vtxlist = []    #buffer
                while line:
                    m2 = re.search(getnextint, line)
                    if m2:
                        vtxlist.append( int(m2.group(1)) )
                        line = m2.group(3)#remainder
                    else:
                        line = None
                if len(vtxlist) > 1:#we need at least 2 vertices to make an edge
                    for i in range (len(vtxlist)-1):#we can have more than one vertex per line - get adjacent pairs
                        obj.edg.append( ( vtxlist[i], vtxlist[i+1] ) )#get the vertex pair between that vertex and the next
        elif line[0] == 'f':                #we have a face (maybe)
            m = re.search(getfaceline, line)
            if m:                               #we have a line beginning 'f '
                vtxlist = []#buffer
                while line:
                    m2 = re.search(getnextint, line)
                    if m2:
                        vtxlist.append( int(m2.group(1)) )
                        line = m2.group(3)#remainder
                    else:
                        line = None
                if len(vtxlist) > 2:            #we need at least 3 vertices to make an edge
                    obj.fce.append(vtxlist)
    
    if obj.name == '':#no name was found, use filename, without extension (.obj)
        obj.name = name[0:-4]

#RENDERING AND SVG OUTPUT FUNCTIONS

def draw_SVG_dot((cx, cy), st, name, parent):
    style = { 'stroke': '#000000', 'stroke-width':str(st.th), 'fill': st.fill, 'stroke-opacity':st.s_opac, 'fill-opacity':st.f_opac}
    circ_attribs = {'style':simplestyle.formatStyle(style),
                    inkex.addNS('label','inkscape'):name,
                    'r':str(st.r),
                    'cx':str(cx), 'cy':str(-cy)}
    inkex.etree.SubElement(parent, inkex.addNS('circle','svg'), circ_attribs )
    
def draw_SVG_line((x1, y1),(x2, y2), st, name, parent):
    style = { 'stroke': '#000000', 'stroke-width':str(st.th), 'stroke-linecap':st.linecap}
    line_attribs = {'style':simplestyle.formatStyle(style),
                    inkex.addNS('label','inkscape'):name,
                    'd':'M '+str(x1)+','+str(-y1)+' L '+str(x2)+','+str(-y2)}
    inkex.etree.SubElement(parent, inkex.addNS('path','svg'), line_attribs )
    
def draw_SVG_poly(pts, face, st, name, parent):
    style = { 'stroke': '#000000', 'stroke-width':str(st.th), 'stroke-linejoin':st.linejoin, \
              'stroke-opacity':st.s_opac, 'fill': st.fill, 'fill-opacity':st.f_opac}   
    for i in range(len(face)):
        if i == 0:#for first point
            d = 'M'#move to
        else:
            d = d + 'L'#line to
        d = d+ str(pts[face[i]-1][0]) + ',' + str(-pts[face[i]-1][1])#add point
    d = d + 'z' #close the polygon
    
    line_attribs = {'style':simplestyle.formatStyle(style),
                    inkex.addNS('label','inkscape'):name,'d': d}
    inkex.etree.SubElement(parent, inkex.addNS('path','svg'), line_attribs )
    
def draw_edges( edge_list, pts, st, parent ):
    for edge in edge_list:#for every edge
        pt_1 = pts[ edge[0]-1 ][0:2] #the point at the start
        pt_2 = pts[ edge[1]-1 ][0:2] #the point at the end
        name = 'Edge'+str(edge[0])+'-'+str(edge[1])
        draw_SVG_line(pt_1,pt_2,st, name, parent)#plot edges
                              
def draw_faces( faces_data, pts, obj, shading, fill_col,st, parent):          
    for face in faces_data:#for every polygon that has been sorted
        if shading:
            st.fill = get_darkened_colour(fill_col, face[1]/pi)#darken proportionally to angle to lighting vector
        else:
            st.fill = get_darkened_colour(fill_col, 1)#do not darken colour
                          
        face_no = face[3]#the number of the face to draw
        draw_SVG_poly(pts, obj.fce[ face_no ], st, 'Face:'+str(face_no), parent)

def get_darkened_colour( (r,g,b), factor):
#return a hex triplet of colour, reduced in lightness proportionally to a value between 0 and 1
    return  '#' + "%02X" % floor( factor*r ) \
                + "%02X" % floor( factor*g ) \
                + "%02X" % floor( factor*b ) #make the colour string

def make_rotation_log(options):
#makes a string recording the axes and angles of each roation, so an object can be repeated
    return   options.r1_ax+str('%.2f'%options.r1_ang)+':'+\
             options.r2_ax+str('%.2f'%options.r2_ang)+':'+\
             options.r3_ax+str('%.2f'%options.r3_ang)+':'+\
             options.r1_ax+str('%.2f'%options.r4_ang)+':'+\
             options.r2_ax+str('%.2f'%options.r5_ang)+':'+\
             options.r3_ax+str('%.2f'%options.r6_ang)

#MATHEMATICAL FUNCTIONS
def get_angle( vector1, vector2 ): #returns the angle between two vectors
    return acos( dot(vector1, vector2) )

def length(vector):#return the pythagorean length of a vector
    return sqrt(dot(vector,vector))

def normalise(vector):#return the unit vector pointing in the same direction as the argument
    return vector / length(vector)

def get_normal( pts, face): #returns the normal vector for the plane passing though the first three elements of face of pts
    #n = pt[0]->pt[1] x pt[0]->pt[3]
    a = (array(pts[ face[0]-1 ]) - array(pts[ face[1]-1 ]))
    b = (array(pts[ face[0]-1 ]) - array(pts[ face[2]-1 ]))
    return cross(a,b).flatten()

def get_unit_normal(pts, face, cw_wound): #returns the unit normal for the plane passing through the first three points of face, taking account of winding
    if cw_wound:
        winding = -1 #if it is clockwise wound, reverse the vecotr direction
    else:
        winding = 1 #else leave alone
    
    return winding*normalise(get_normal(pts, face))

def rotate( matrix, angle, axis ):#choose the correct rotation matrix to use
    if   axis == 'x':
        matrix = rot_x(matrix, angle)
    elif axis == 'y':
        matrix = rot_y(matrix, angle)
    elif axis == 'z':
        matrix = rot_z(matrix, angle)
    return matrix
    
def rot_z( matrix , a):#rotate around the z-axis by a radians
    trans_mat = mat(array( [[ cos(a) , -sin(a) ,    0   ],
                            [ sin(a) ,  cos(a) ,    0   ],
                            [   0    ,    0    ,    1   ]]))
    return trans_mat*matrix

def rot_y( matrix , a):#rotate around the y-axis by a radians
    trans_mat = mat(array( [[ cos(a) ,    0    , sin(a) ],
                            [   0    ,    1    ,    0   ],
                            [-sin(a) ,    0    , cos(a) ]]))
    return trans_mat*matrix
    
def rot_x( matrix , a):#rotate around the x-axis by a radians
    trans_mat = mat(array( [[   1    ,    0    ,    0   ],
                            [   0    ,  cos(a) ,-sin(a) ],
                            [   0    ,  sin(a) , cos(a) ]]))
    return trans_mat*matrix

def get_transformed_pts( vtx_list, trans_mat):#translate the points according to the matrix
    transformed_pts = []
    for vtx in vtx_list:
        transformed_pts.append((trans_mat * mat(vtx).T).T.tolist()[0] )#transform the points at add to the list
    return transformed_pts

def get_max_z(pts, face): #returns the largest z_value of any point in the face
    max_z = pts[ face[0]-1 ][2]
    for i in range(1, len(face)):
        if pts[ face[0]-1 ][2] >= max_z:
            max_z = pts[ face[0]-1 ][2]
    return max_z
    
def get_min_z(pts, face): #returns the smallest z_value of any point in the face
    min_z = pts[ face[0]-1 ][2]
    for i in range(1, len(face)):
        if pts[ face[i]-1 ][2] <= min_z:
            min_z = pts[ face[i]-1 ][2]
    return min_z
    
def get_cent_z(pts, face): #returns the centroid z_value of any point in the face
    sum = 0
    for i in range(len(face)):
            sum += pts[ face[i]-1 ][2]
    return sum/len(face)
    
def get_z_sort_param(pts, face, method): #returns the z-sorting parameter specified by 'method' ('max', 'min', 'cent')	
    z_sort_param = ''
    if  method == 'max':
        z_sort_param  = get_max_z(pts, face)
    elif method == 'min':
        z_sort_param  = get_min_z(pts, face)
    else:
        z_sort_param  = get_cent_z(pts, face)
    return z_sort_param

#OBJ DATA MANIPULATION
def remove_duplicates(list):#removes the duplicates from a list
    list.sort()#sort the list
 
    last = list[-1]
    for i in range(len(list)-2, -1, -1):
        if last==list[i]:
            del list[i]
        else:
            last = list[i]
    return list

def make_edge_list(face_list):#make an edge vertex list from an existing face vertex list
    edge_list = []
    for i in range(len(face_list)):#for every face
        edges = len(face_list[i]) #number of edges around that face
        for j in range(edges):#for every vertex in that face
            new_edge = [face_list[i][j], face_list[i][(j+1)%edges] ]
            new_edge.sort() #put in ascending order of vertices (to ensure we spot duplicates)
            edge_list.append( new_edge )#get the vertex pair between that vertex and the next
    
    return remove_duplicates(edge_list)
    
class Style(object): #container for style information
    def __init__(self,options):
        self.th = options.th
        self.fill= '#ff0000'
        self.col = '#000000'
        self.r = 2
        self.f_opac = str(options.f_opac/100.0)
        self.s_opac = str(options.s_opac/100.0)
        self.linecap = 'round'
        self.linejoin = 'round'

class Obj(object): #a 3d object defined by the vertices and the faces (eg a polyhedron)
#edges can be generated from this information
    def __init__(self):
        self.vtx = []
        self.edg = []
        self.fce = []
        self.name=''
        
    def set_type(self, options):
        if options.type == 'face':
            if self.fce != []:
                self.type = 'face'
            else:
                inkex.errormsg(_('No face data found in specified file.'))
                inkex.errormsg(_('Try selecting "Edge Specified" in the Model File tab.\n'))
                self.type = 'error'
        else:
            if self.edg != []:
                self.type = 'edge'
            else:
                inkex.errormsg(_('No edge data found in specified file.'))
                inkex.errormsg(_('Try selecting "Face Specified" in the Model File tab.\n'))
                self.type = 'error'

class Poly_3D(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--tab",
            action="store", type="string", 
            dest="tab", default="object") 

#MODEL FILE SETTINGS
        self.OptionParser.add_option("--obj",
            action="store", type="string", 
            dest="obj", default='cube')
        self.OptionParser.add_option("--spec_file",
            action="store", type="string", 
            dest="spec_file", default='great_rhombicuboct.obj')
        self.OptionParser.add_option("--cw_wound",
            action="store", type="inkbool", 
            dest="cw_wound", default='true')
        self.OptionParser.add_option("--type",
            action="store", type="string", 
            dest="type", default='face')
#VEIW SETTINGS
        self.OptionParser.add_option("--r1_ax",
            action="store", type="string", 
            dest="r1_ax", default="X-Axis")
        self.OptionParser.add_option("--r2_ax",
            action="store", type="string", 
            dest="r2_ax", default="X-Axis")
        self.OptionParser.add_option("--r3_ax",
            action="store", type="string", 
            dest="r3_ax", default="X-Axis")
        self.OptionParser.add_option("--r4_ax",
            action="store", type="string", 
            dest="r4_ax", default="X-Axis")
        self.OptionParser.add_option("--r5_ax",
            action="store", type="string", 
            dest="r5_ax", default="X-Axis")
        self.OptionParser.add_option("--r6_ax",
            action="store", type="string", 
            dest="r6_ax", default="X-Axis")
        self.OptionParser.add_option("--r1_ang",
            action="store", type="float", 
            dest="r1_ang", default=0)
        self.OptionParser.add_option("--r2_ang",
            action="store", type="float", 
            dest="r2_ang", default=0)
        self.OptionParser.add_option("--r3_ang",
            action="store", type="float", 
            dest="r3_ang", default=0)
        self.OptionParser.add_option("--r4_ang",
            action="store", type="float", 
            dest="r4_ang", default=0)
        self.OptionParser.add_option("--r5_ang",
            action="store", type="float", 
            dest="r5_ang", default=0)
        self.OptionParser.add_option("--r6_ang",
            action="store", type="float", 
            dest="r6_ang", default=0)
        self.OptionParser.add_option("--scl",
            action="store", type="float", 
            dest="scl", default=100.0)
#STYLE SETTINGS
        self.OptionParser.add_option("--show",
            action="store", type="string", 
            dest="show", default='faces')
        self.OptionParser.add_option("--shade",
            action="store", type="inkbool", 
            dest="shade", default='true')
        self.OptionParser.add_option("--f_r",
            action="store", type="int", 
            dest="f_r", default=255)
        self.OptionParser.add_option("--f_g",
            action="store", type="int", 
            dest="f_g", default=0)
        self.OptionParser.add_option("--f_b",
            action="store", type="int", 
            dest="f_b", default=0)
        self.OptionParser.add_option("--f_opac",
            action="store", type="int", 
            dest="f_opac", default=100)
        self.OptionParser.add_option("--s_opac",
            action="store", type="int", 
            dest="s_opac", default=100)
        self.OptionParser.add_option("--th",
            action="store", type="float", 
            dest="th", default=2)
        self.OptionParser.add_option("--lv_x",
            action="store", type="float", 
            dest="lv_x", default=1)
        self.OptionParser.add_option("--lv_y",
            action="store", type="float", 
            dest="lv_y", default=1)
        self.OptionParser.add_option("--lv_z",
            action="store", type="float", 
            dest="lv_z", default=-2)
        self.OptionParser.add_option("--back",
            action="store", type="inkbool", 
            dest="back", default='false')
        self.OptionParser.add_option("--norm",
            action="store", type="inkbool", 
            dest="norm", default='true')
        self.OptionParser.add_option("--z_sort",
            action="store", type="string", 
            dest="z_sort", default='min')
            
            
    def effect(self):
        so = self.options#shorthand
        
        #INITIALISE AND LOAD DATA
        
        obj = Obj() #create the object
        file = get_filename(so)#get the file to load data from
        get_obj_data(obj, file)#load data from the obj file
        obj.set_type(so)#set the type (face or edge) as per the settings
        
        scale = self.unittouu('1px')    # convert to document units
        st = Style(so) #initialise style
        fill_col = (so.f_r, so.f_g, so.f_b) #colour tuple for the face fill
        lighting = normalise( (so.lv_x,-so.lv_y,so.lv_z) ) #unit light vector
        
        #INKSCAPE GROUP TO CONTAIN THE POLYHEDRON
        
        #Put in in the centre of the current view
        poly_transform = 'translate(' + str( self.view_center[0]) + ',' + str( self.view_center[1]) + ')'
        if scale != 1:
            poly_transform += ' scale(' + str(scale) + ')'
        #we will put all the rotations in the object name, so it can be repeated in 
        poly_name = obj.name+':'+make_rotation_log(so)
        poly_attribs = {inkex.addNS('label','inkscape'):poly_name,
                        'transform':poly_transform }
        poly = inkex.etree.SubElement(self.current_layer, 'g', poly_attribs)#the group to put everything in
        
        #TRANSFORMATION OF THE OBJECT (ROTATION, SCALE, ETC)
        
        trans_mat = mat(identity(3, float)) #init. trans matrix as identity matrix
        for i in range(1, 7):#for each rotation
            axis  = eval('so.r'+str(i)+'_ax')
            angle = eval('so.r'+str(i)+'_ang') *pi/180
            trans_mat = rotate(trans_mat, angle, axis)
        trans_mat = trans_mat*so.scl #scale by linear factor (do this only after the transforms to reduce round-off)
        
        transformed_pts = get_transformed_pts(obj.vtx, trans_mat) #the points as projected in the z-axis onto the viewplane
        
        #RENDERING OF THE OBJECT
        
        if so.show == 'vtx':
            for i in range(len(transformed_pts)):
                draw_SVG_dot([transformed_pts[i][0],transformed_pts[i][1]], st, 'Point'+str(i), poly)#plot points using transformed_pts x and y coords
        
        elif so.show == 'edg':
            if obj.type == 'face':#we must generate the edge list from the faces
                edge_list = make_edge_list(obj.fce)
            else:#we already have an edge list
                edge_list = obj.edg
                        
            draw_edges( edge_list, transformed_pts, st, poly)
                              
        elif so.show == 'fce':
            if obj.type == 'face':#we have a face list
               
                z_list = []
                
                for i in range(len(obj.fce)):
                    face = obj.fce[i] #the face we are dealing with
                    norm = get_unit_normal(transformed_pts, face, so.cw_wound) #get the normal vector to the face
                    angle = get_angle( norm, lighting )#get the angle between the normal and the lighting vector
                    z_sort_param = get_z_sort_param(transformed_pts, face, so.z_sort)
                    
                    if so.back or norm[2] > 0: # include all polygons or just the front-facing ones as needed
                        z_list.append((z_sort_param, angle, norm, i))#record the maximum z-value of the face and angle to light, along with the face ID and normal
                
                z_list.sort(lambda x, y: cmp(x[0],y[0])) #sort by ascending sort parameter of the face
                draw_faces( z_list, transformed_pts, obj, so.shade, fill_col, st, poly)

            else:#we cannot generate a list of faces from the edges without a lot of computation
                inkex.errormsg(_('Face Data Not Found. Ensure file contains face data, and check the file is imported as "Face-Specified" under the "Model File" tab.\n'))
        else:
            inkex.errormsg(_('Internal Error. No view type selected\n'))
        
if __name__ == '__main__':
    e = Poly_3D()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
