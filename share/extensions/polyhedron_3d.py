#! /usr/bin/python
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

#Faces are colored according to the Phong reflection model.

#Faces are clipped (only in Perspective projection) when vertices of 
#the face are crossing view plane.

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
import os.path
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
    
def get_obj_data(obj, name, fill):
    infile = open(objfile(name))
    fill_col = tuple(x/255 for x in fill)
    
    #regular expressions
    getname = '(.[nN]ame:\\s*)(.*)'
    material = Material(fill_col)
    for line in infile:
        if line[0]=='#':                    #we have a comment line
            m = re.search(getname, line)        #check to see if this line contains a name
            if m:
                obj.name = m.group(2)           #if it does, set the property

        else:
            tokens = line.split()				#splitting line on spaces
            if tokens:
                if tokens[0] == 'v':            #we have a vertex (maybe)
                    obj.vtx.append([float(tokens[1]),float(tokens[2]),float(tokens[3])])

                elif tokens[0] == 'l':          #we have a line (maybe)
                    vtxlist = []
                    for i in range(1,len(tokens)):
                        vtxlist.append(int(tokens[i]))

                    if len(vtxlist) > 1:        #we need at least 2 vertices to make an edge
                        obj.edg.append(vtxlist)

                elif tokens[0] == 'f':          #we have a face (maybe)
                    vtxlist = []
                    for i in range(1, len(tokens)):
                        face_vertex = tokens[i].split('/')
                        vtxlist.append(int(face_vertex[0]))

                    if len(vtxlist) > 2:        #we need at least 3 vertices to make an edge
                        obj.fce.append(vtxlist)
                        obj.material_of_faces[str(vtxlist)] = material.name

                elif tokens[0] == 'mtllib':		#we have a mtl file
                    mtl_name = os.path.join(os.path.dirname(name),tokens[1])
                    get_mtl_data(obj, mtl_name)

                elif tokens[0] == 'usemtl':		#we have to use material info
                    material = obj.materials[tokens[1]]

    
    if obj.name == '':#no name was found, use filename, without extension (.obj)
        obj.name = name[0:-4]

def get_mtl_data(obj, name):
    infile = open(name)

    count_newmtl = 0
    material = Material()					
    for line in infile:
        if line[0] == '#':					#a comment
            continue
        else:
            tokens = line.split()
            if tokens:
                if tokens[0] == 'newmtl':	#new mtl data declaration
                    if count_newmtl > 0:	#add previous one to obj
                        obj.materials[material.name] = material
                    material = Material()	#new Material object
                    material.name = tokens[1]
                    count_newmtl +=1

                elif tokens[0] == 'Ka':		#ambient reflection co-efficient
                    material.ambient = (float(tokens[1]),float(tokens[2]),float(tokens[3]))

                elif tokens[0] == 'Kd':		#diffuse reflection co-efficient
                    material.diffuse = (float(tokens[1]),float(tokens[2]),float(tokens[3]))

                elif tokens[0] == 'Ks':		#specular reflection co-efficient
                    material.specular = (float(tokens[1]),float(tokens[2]),float(tokens[3]))

                elif tokens[0] == 'Ns':		#shining co-efficient
                    material.shininess = float(tokens[1])

                elif tokens[0] == 'd':		#opacity
                    material.opacity = float(tokens[1])

                elif tokens[0] == 'illum':	#illumination model
                    material.illum_no = int(tokens[1])

    if count_newmtl > 0:					#add last one to obj
        obj.materials[material.name] = material


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

def check_color(color):		#checks if color does not cross its boundaries
    for i in range(3):
        if color[i]<0:
            color[i] = 0
        if color[i]>255:
            color[i] = 255
    return color

def get_half_vector(vector1, vector2):  #
    half_vector = add(vector1,vector2)
    return normalise(half_vector)

def get_face_color(light, material, normal):	#color of face using Phong model
    illum_no = material.illum_no
    color = (255,255,255)
    if illum_no == 1:
        ambient = multiply(light.ambient, material.ambient)
        ambient_color = check_color(multiply(color,ambient))
        diffuse = multiply(light.diffuse,material.diffuse)*dot(light.position,normal)
        diffuse_color = check_color(multiply(color,diffuse))
        color = check_color(add(ambient_color,diffuse_color))

    elif illum_no == 2:
        ambient = multiply(light.ambient, material.ambient)
        ambient_color = check_color(multiply(color,ambient))
        diffuse = multiply(light.diffuse,material.diffuse)*dot(light.position,normal)
        diffuse_color = check_color(multiply(color,diffuse))
        half = get_half_vector(light.position, normal)
        specular = multiply(light.specular,material.specular)*power(dot(normal,half),material.shininess)
        specular_color = check_color(multiply(color,specular))
        color = check_color(add(ambient_color,add(diffuse_color,specular_color)))

    else:
        diffuse = multiply(light.diffuse,material.diffuse)*dot(light.position,normal)
        color = check_color(multiply(color,diffuse))

    return get_darkened_colour(color,1)
                              
def draw_faces( faces_data, pts, faces, obj, material_name_of_faces, 
    shading,st, parent):          
    for z_list in faces_data:#for every polygon that has been sorted
        face_no = z_list[3]#the number of the face to draw
        face = faces[face_no]
        face_material = obj.materials[material_name_of_faces[str(face)]]
        if shading:
            st.fill = get_face_color(obj.light, face_material, z_list[2])
            #color faces based on angle and shininess of object
        else:
            st.fill = get_darkened_colour(multiply(face_material.diffuse, 255), 1)#do not darken colour
        st.f_opac = face_material.opacity
                          
        draw_SVG_poly(pts, face, st, 'Face:'+str(face_no), parent)

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
    if length(vector)>0:
        return vector / length(vector)
    return vector

def get_normal( pts, face): #returns the normal vector for the plane passing though the first three elements of face of pts
    #n = pt[0]->pt[1] x pt[0]->pt[3]
    v1, v2, v3 = three_diff_vertices(pts, face) #we always get a normal vector
    a = (array(v1) - array(v2))
    b = (array(v1) - array(v3))
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

def transform_perspective(vtx_list, observer):#convert vertices for perspective projection 
    transformed_vertices = []
    for vtx in vtx_list:
        vertex = vtx
        z = vertex[2]
        if z == observer:
            z += 0.1

        factor = 500/abs(observer-z)
        vertex[0] = vertex[0]*factor
        vertex[1] = vertex[1]*factor
        vertex[2] = vertex[2]*factor
        transformed_vertices.append(vertex)
    return transformed_vertices

def clip_faces(faces, pts, obs, 
    transformed_material_of_faces, material_of_faces):
	#clips polyhedron when vertices are too close to observer
	#based on Sutherland-Hodgman algorithm for polygon clipping
    screen = obs - 400 #plane for clipping polyhedron
    transformed_faces = []
    for face in faces:
        material_name = material_of_faces[str(face)] #material info of current face
        clipped_face = []
        for i in range(len(face)):
            v1 = pts[face[i]-1]
            v2 = pts[face[(i+1)%len(face)]-1]
            if (v1[2]>screen) and (v2[2]<= screen): #first vertex is out of screen
                factor = (screen-v1[2])/(v2[2]-v1[2])
                x = v1[0]+(factor*(v2[0]-v1[0]))
                y = v1[1]+(factor*(v2[1]-v1[1]))
                z = screen
                v3 = ([x,y,z])
                pts.append(v3) #add vertex to list
                clipped_face.append(len(pts)) #add clipped vertex to new face
            elif (v1[2]<=screen) and (v2[2]>screen): #second vertex is out of screen
                factor = (screen-v1[2])/(v2[2]-v1[2])
                x = v1[0]+(factor*(v2[0]-v1[0]))
                y = v1[1]+(factor*(v2[1]-v1[1]))
                z = screen
                v3 = ([x,y,z])
                pts.append(v3) 
                pts.append(v3) 
                clipped_face.append(face[i]) #add first vertex and clipped vertex to new face
                clipped_face.append(len(pts))
            elif (v1[2]<=screen) and (v2[2]<=screen): #no vertex is outside screen
                clipped_face.append(face[i])
        if len(clipped_face)>2:				#if a valid clipped face
            transformed_material_of_faces[str(clipped_face)] = material_name
            transformed_faces.append(clipped_face)
    return transformed_faces

def three_diff_vertices(pts, face): 	#returns 3 different vertices of a face for normal calculation
    v1 = pts[face[0]-1]
    index = 0
    while (index<len(face)):
        v2 = pts[face[index]-1]
        if v1 == v2:
            index = index+1
        else:
            break
    if v1 == v2:
        return False
    index = index+1
    while (index<len(face)):
        v3 = pts[face[index]-1]
        if (v1 == v3) or (v2 == v3):
            index = index+1
        else:
            break
    if (v1 == v3) or (v2 == v3):
        return False
    return v1, v2, v3

class Material:		#a material defined by properties of 3d object
    def __init__(self, diffuse = (0.8,0.8,0.8)):
        self.name = ''
        self.ambient = (0.2,0.2,0.2)
        self.diffuse = diffuse
        self.specular = (0.3,0.3,0.3)
        self.shininess = 0
        self.opacity = 1.0
        self.illum_no = 0

class Light:		#a light with some properties
    def __init__(self, position):
        self.position = position
        self.ambient = (0.2,0.2,0.2)
        self.diffuse = (0.8,0.8,0.8)
        self.specular = (0.5,0.5,0.5)
    
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
        self.material_of_faces = {}
        self.materials = {}
        self.light = None
        
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
        self.OptionParser.add_option("--projection",
            action="store", type="string", 
            dest="projection", default="Parallel")
        self.OptionParser.add_option("--observer",
            action="store", type="int", 
            dest="observer", default=100)
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
        fill_col = (so.f_r, so.f_g, so.f_b) #colour tuple for the face fill
        file = get_filename(so)#get the file to load data from
        get_obj_data(obj, file, fill_col)#load data from the obj file
        obj.set_type(so)#set the type (face or edge) as per the settings
        
        scale = self.unittouu('1px')    # convert to document units
        st = Style(so) #initialise style
        lighting = normalise( (so.lv_x,-so.lv_y,so.lv_z) ) #unit light vector
        obj.light = Light(lighting)
        fill = tuple(x/255 for x in fill_col)
        obj.materials.setdefault('',Material(fill))
        
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

        transformed_faces = obj.fce
        transformed_material_of_faces = {}

        if so.projection == 'prsp':
            transformed_pts = transform_perspective(transformed_pts, so.observer)
            transformed_faces = clip_faces(transformed_faces, 
                transformed_pts, so.observer, transformed_material_of_faces,
                obj.material_of_faces)
        else:
            transformed_material_of_faces = obj.material_of_faces

        #RENDERING OF THE OBJECT
        
        if so.show == 'vtx':
            for i in range(len(transformed_pts)):
                draw_SVG_dot([transformed_pts[i][0],transformed_pts[i][1]], st, 'Point'+str(i), poly)#plot points using transformed_pts x and y coords
        
        elif so.show == 'edg':
            if obj.type == 'face':#we must generate the edge list from the faces
                edge_list = make_edge_list(transformed_faces)
            else:#we already have an edge list
                edge_list = obj.edg
                        
            draw_edges( edge_list, transformed_pts, st, poly)
                              
        elif so.show == 'fce':
            if obj.type == 'face':#we have a face list
               
                z_list = []
                
                for i in range(len(transformed_faces)):
                    face = transformed_faces[i] #the face we are dealing with
                    if face:
                        if not three_diff_vertices(transformed_pts, face):
                            continue
                        norm = get_unit_normal(transformed_pts, face, so.cw_wound) #get the normal vector to the face
                        angle = get_angle( norm, lighting )#get the angle between the normal and the lighting vector
                        z_sort_param = get_z_sort_param(transformed_pts, face, so.z_sort)
                        
                        if so.back or norm[2] > 0: # include all polygons or just the front-facing ones as needed
                            z_list.append((z_sort_param, angle, norm, i))#record the maximum z-value of the face and angle to light, along with the face ID and normal
                
                z_list.sort(lambda x, y: cmp(x[0],y[0])) #sort by ascending sort parameter of the face
                draw_faces( z_list, transformed_pts, transformed_faces, 
                    obj, transformed_material_of_faces, so.shade, st, poly)

            else:#we cannot generate a list of faces from the edges without a lot of computation
                inkex.errormsg(_('Face Data Not Found. Ensure file contains face data, and check the file is imported as "Face-Specified" under the "Model File" tab.\n'))
        else:
            inkex.errormsg(_('Internal Error. No view type selected\n'))
        
if __name__ == '__main__':
    e = Poly_3D()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
