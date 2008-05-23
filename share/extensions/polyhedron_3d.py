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
#into adjacent pairs. Each edge can connect only two vertices
#l  1   2   3

#Faces are rendered according to the painter's algorithm and perhaps
#back-face culling, if selected. The parameter to sort the faces by
#is user-selectable

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

import inkex
import simplestyle, sys, simplepath, re
from math import *
try:
    from numpy import *
except:
    inkex.debug("Failed to import the numpy module. This module is required by this extension. Please install them and try again.  On a Debian-like system this can be done with the command, sudo apt-get install python-numpy.")
    sys.exit()

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
    floating = '([\-\+\\d*\.e]*)'
    getvertex = '(v\\s+)'+floating+'\\s+'+floating+'\\s+'+floating
    getedgeline = '(l\\s+)(.*)'
    getfaceline = '(f\\s+)(.*)'
    getnextint = '(\\d+)([/\\d]*)(.*)'#we need to deal with 133\343\123 or 123\\456 as one item
    
    obj.vtx = []
    obj.edg = []
    obj.fce = []
    obj.name=''
    
    for line in infile:
        if line[0]=='#': #we have a comment line
            m = re.search(getname, line)
            if m:
                obj.name = m.group(2)
        elif line[0:1] == 'v': #we have a vertex (maybe)
            m = re.search(getvertex, line)
            if m: #we have a valid vertex
                obj.vtx.append( [float(m.group(2)), float(m.group(3)), float(m.group(4)) ] )
        elif line[0:1] == 'l':#we have a line (maybe)
            m = re.search(getedgeline, line)
            if m:#we have a line beginning 'l '
                vtxlist = []#buffer
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
        elif line[0:1] == 'f':#we have a face (maybe)
            m = re.search(getfaceline, line)
            if m:#we have a line beginning 'l '
                vtxlist = []#buffer
                while line:
                    m2 = re.search(getnextint, line)
                    if m2:
                        vtxlist.append( int(m2.group(1)) )
                        line = m2.group(3)#remainder
                    else:
                        line = None
                if len(vtxlist) > 2:#we need at least 3 vertices to make an edge
                    obj.fce.append(vtxlist)
    
    if obj.name == '':#no name was found, use filename, without extension
        obj.name = name[0:-4]

def draw_SVG_dot((cx, cy), st, name, parent):
    style = { 'stroke': '#000000', 'stroke-width':str(st.th), 'fill': st.fill, 'stroke-opacity':st.s_opac, 'fill-opacity':st.f_opac}
    circ_attribs = {'style':simplestyle.formatStyle(style),
                    inkex.addNS('label','inkscape'):name,
                    'r':str(st.r),
                    'cx':str(cx), 'cy':str(-cy)}
    inkex.etree.SubElement(parent, inkex.addNS('circle','svg'), circ_attribs )
    
def draw_SVG_line((x1, y1),(x2, y2), st, name, parent):
    #sys.stderr.write(str(p1))
    style = { 'stroke': '#000000', 'stroke-width':str(st.th)}
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
    
def get_normal( pts, face): #returns the normal vector for the plane passing though the first three elements of face of pts
    #n = pt[0]->pt[1] x pt[0]->pt[3]
    a = (array(pts[ face[0]-1 ]) - array(pts[ face[1]-1 ]))
    b = (array(pts[ face[0]-1 ]) - array(pts[ face[2]-1 ]))
    return cross(a,b).flatten()
    
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
    
def length(vector):#return the pythagorean length of a vector
    return sqrt(dot(vector,vector))
    
def rot_z( matrix , a):
    trans_mat = mat(array( [[ cos(a) , -sin(a) ,    0   ],
                            [ sin(a) ,  cos(a) ,    0   ],
                            [   0    ,    0    ,    1   ]]))
    return trans_mat*matrix

def rot_y( matrix , a):
    trans_mat = mat(array( [[ cos(a) ,    0    , sin(a) ],
                            [   0    ,    1    ,    0   ],
                            [-sin(a) ,    0    , cos(a) ]]))
    return trans_mat*matrix
    
def rot_x( matrix , a):
    trans_mat = mat(array( [[   1    ,    0    ,    0   ],
                            [   0    ,  cos(a) ,-sin(a) ],
                            [   0    ,  sin(a) , cos(a) ]]))
    return trans_mat*matrix
    
def make_edge_list(face_list):#make an edge vertex list from an existing face vertex list
    edge_list = []
    for i in range(len(face_list)):#for every face
        edges = len(face_list[i]) #number of edges around that face
        for j in range(edges):#for every vertex in that face
            edge_list.append( [face_list[i][j], face_list[i][(j+1)%edges] ] )#get the vertex pair between that vertex and the next
 
    for i in range(len(edge_list)):
        edge_list[i].sort()#sort the entries of the entries
    edge_list.sort()#sort the list
 
    last = edge_list[-1] #delete duplicate entries
    for i in range(len(edge_list)-2, -1, -1):
        if last==edge_list[i]:
            del edge_list[i]
        else:
            last=edge_list[i]
    return edge_list
    
class Style(object): #container for style information
    def __init__(self):
        None

class Obj(object): #a 3d object defined by the vertices and the faces (eg a polyhedron)
#edges can be generated from this information
    def __init__(self):
        None

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
            dest="r1_ax", default=0)
        self.OptionParser.add_option("--r2_ax",
            action="store", type="string", 
            dest="r2_ax", default=0)
        self.OptionParser.add_option("--r3_ax",
            action="store", type="string", 
            dest="r3_ax", default=0)
        self.OptionParser.add_option("--r4_ax",
            action="store", type="string", 
            dest="r4_ax", default=0)
        self.OptionParser.add_option("--r5_ax",
            action="store", type="string", 
            dest="r5_ax", default=0)
        self.OptionParser.add_option("--r6_ax",
            action="store", type="string", 
            dest="r6_ax", default=0)
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
        so = self.options
        
        st = Style()
        st.th = so.th
        st.fill= '#ff0000'
        st.col = '#000000'
        st.r = 2
        st.f_opac = str(so.f_opac/100.0)
        st.s_opac = str(so.s_opac/100.0)
        st.linecap = 'round'
        st.linejoin = 'round'
        
        file = ''
        if   so.obj == 'cube':
            file = 'cube.obj'
        elif so.obj == 't_cube':
            file = 'trunc_cube.obj'
        elif so.obj == 'sn_cube':
            file = 'snub_cube.obj'
        elif so.obj == 'cuboct':
            file = 'cuboct.obj'
        elif so.obj == 'tet':
            file = 'tet.obj'
        elif so.obj == 't_tet':
            file = 'trunc_tet.obj'
        elif so.obj == 'oct':
            file = 'oct.obj'
        elif so.obj == 't_oct':
            file = 'trunc_oct.obj'
        elif so.obj == 'icos':
            file = 'icos.obj'
        elif so.obj == 't_icos':
            file = 'trunc_icos.obj'
        elif so.obj == 's_t_icos':
            file = 'small_triam_icos.obj'
        elif so.obj == 'g_s_dodec':
            file = 'great_stel_dodec.obj'
        elif so.obj == 'dodec':
            file = 'dodec.obj'
        elif so.obj == 'sn_dodec':
            file = 'snub_dodec.obj'
        elif so.obj == 'g_dodec':
            file = 'great_dodec.obj'
        elif so.obj == 't_dodec':
            file = 'trunc_dodec.obj'
        elif so.obj == 'from_file':
            file = so.spec_file
            
        obj = Obj() #create the object
        get_obj_data(obj, file)
        
        obj.type=''
        if so.type == 'face':
            if len(obj.fce) > 0:
                obj.type = 'face'
            else:
                sys.stderr.write('No face data found in specified file\n')
                obj.type = 'error'
        else:
            if len(obj.edg) > 0:
                obj.type = 'edge'
            else:
                sys.stderr.write('No edge data found in specified file\n')
                obj.type = 'error'
        
        trans_mat = mat(identity(3, float)) #init. trans matrix as identity matrix
        
        #perform rotations
        for i in range(1, 7):#for each rotation
            axis  = eval('so.r'+str(i)+'_ax')
            angle = eval('so.r'+str(i)+'_ang') *pi/180
            if   axis == 'x':
                trans_mat = rot_x(trans_mat, angle)
            elif axis == 'y':
                trans_mat = rot_y(trans_mat, angle)
            elif axis == 'z':
                trans_mat = rot_z(trans_mat, angle)

        # Embed points in group
        #Put in in the centre of the current view
        t = 'translate(' + str( self.view_center[0]) + ',' + str( self.view_center[1]) + ')'
        #we will put all the rotations in the object name, so it can be repeated in future
        proj_attribs = {inkex.addNS('label','inkscape'):obj.name+':'+so.r1_ax+str('%.2f'%so.r1_ang)+':'+
                                                                     so.r2_ax+str('%.2f'%so.r2_ang)+':'+
                                                                     so.r3_ax+str('%.2f'%so.r3_ang)+':'+
                                                                     so.r1_ax+str('%.2f'%so.r4_ang)+':'+
                                                                     so.r2_ax+str('%.2f'%so.r5_ang)+':'+
                                                                     so.r3_ax+str('%.2f'%so.r6_ang),
                        'transform':t }
        proj = inkex.etree.SubElement(self.current_layer, 'g', proj_attribs)#the group to put everything in
        
        vp_pts=[] #the points as projected in the z-axis onto the viewplane
        
        for i in range(len(obj.vtx)):
            vp_pts.append((so.scl* (trans_mat * mat(obj.vtx[i]).T)).T.tolist()[0] )#transform the points at add to vp_pts
        
        lighting = [so.lv_x,-so.lv_y,so.lv_z] #direction of light vector
        lighting = lighting/length(lighting) #normalise
        
        if so.show == 'vtx':
            for i in range(len(vp_pts)):
                draw_SVG_dot([vp_pts[i][0],vp_pts[i][1]], st, 'Point'+str(i), proj)#plot points
        
        elif so.show == 'edg':
            if obj.type == 'face':#we must generate the edge list
                edge_list = make_edge_list(obj.fce)
            else:#we already have an edge list
                edge_list = obj.edg
                        
            for i in range(len(edge_list)):#for every edge
                pt_1 = vp_pts[ edge_list[i][0]-1 ] #the point at the start
                pt_2 = vp_pts[ edge_list[i][1]-1 ] #the point at the end
                
                draw_SVG_line((pt_1[0], pt_1[1]),
                              (pt_2[0], pt_2[1]),
                              st, 'Edge', proj)#plot edges
                              
        elif so.show == 'fce':
            if obj.type == 'face':#we have a face list
                
                if so.cw_wound: rev = -1 #if cw wound, reverse normals
                else: rev = 1
                
                z_list = []
                
                for i in range(len(obj.fce)):
                    norm = get_normal(vp_pts, obj.fce[i])#get the normal to the face
                    norm = rev*norm / length(norm)#normalise and reverse if needed
                    angle = acos( dot(norm, lighting) )#get the angle between the normal and the lighting vector
                    
                    
                    if   so.z_sort =='max':
                        z_sort_param  = get_max_z(vp_pts, obj.fce[i])
                    elif so.z_sort == 'min':
                        z_sort_param  = get_min_z(vp_pts, obj.fce[i])
                    else:
                        z_sort_param  = get_cent_z(vp_pts, obj.fce[i])
                    
                    if so.norm:#if a log of normals is required
                        if i == 0:
                            sys.stderr.write('Normal Vectors for each face are: \n\n')
                        sys.stderr.write('Face '+str(i)+': ' + str(norm) + '\n')
                    
                    if so.back: # draw all polygons
                        z_list.append((z_sort_param, angle, norm, i) )
                    elif norm[2] > 0:#ignore backwards-facing faces (back face cull)
                        z_list.append((z_sort_param, angle, norm, i) ) #record the maximum z-value of the face and angle to light, along with the face ID and normal
                
                z_list.sort(lambda x, y: cmp(x[0],y[0])) #sort by ascending sort parameter of the face
                
                for i in range(len(z_list)):#for every polygon that has been sorted
                    if so.shade:
                        st.fill = '#' + "%02X" % floor( z_list[i][1]*so.f_r/pi ) \
                                      + "%02X" % floor( z_list[i][1]*so.f_g/pi ) \
                                      + "%02X" % floor( z_list[i][1]*so.f_b/pi ) #make the colour string
                    else:
                        st.fill = '#' + '%02X' % so.f_r + '%02X' % so.f_g + '%02X' % so.f_b #opaque
                                      
                    face_no = z_list[i][3]#the number of the face to draw
                    draw_SVG_poly(vp_pts, obj.fce[ face_no ], st, 'Face:'+str(face_no), proj)
            else:
                sys.stderr.write('Face Data Not Found. Ensure file contains face data, and check the file is imported as "Face-Specifed" under the "Model File" tab.\n')
        else:
            sys.stderr.write('Internal Error. No view type selected\n')
        
if __name__ == '__main__':
    e = Poly_3D()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 encoding=utf-8 textwidth=99
