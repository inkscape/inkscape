#!/usr/bin/env python 
"""
Comments starting "#LT" or "#CLT" are by Chris Lusby Taylor who rewrote the engraving function in 2011.
History of CLT changes to engraving and other functions it uses:
9 May 2011 Changed test of tool diameter to square it
10 May Note that there are many unused functions, including:
	  bound_to_bound_distance, csp_curvature_radius_at_t,
        csp_special_points, csplength, rebuild_csp, csp_slope,
        csp_simple_bound_to_point_distance, csp_bound_to_point_distance,
        bez_at_t, bez_to_point_distance, bez_normalized_slope, matrix_mul, transpose
       Fixed csp_point_inside_bound() to work if x outside bounds
20 May Now encoding the bisectors of angles.
23 May Using r/cos(a) instead of normalised normals for bisectors of angles.
23 May Note that Z values generated for engraving are in pixels, not mm.
       Removed the biarc curves - straight lines are better.
24 May Changed Bezier slope calculation to be less sensitive to tiny differences in points.
       Added use of self.options.engraving_newton_iterations to control accuracy
25 May Big restructure and new recursive function.
	   Changed the way I treat corners - I now find if the centre of a proposed circle is
                within the area bounded by the line being tested and the two angle bisectors at
		    its ends. See get_radius_to_line().
29 May Eliminating redundant points. If A,B,C colinear, drop B
30 May Eliminating redundant lines in divided Beziers. Changed subdivision of lines
  7Jun Try to show engraving in 3D
 8 Jun Displaying in stereo 3D.
       Fixed a bug in bisect - it could go wrong due to rounding errors if
			1+x1.x2+y1.y2<0 which should never happen. BTW, I spotted a non-normalised normal
			returned by csp_normalized_normal. Need to check for that.
 9 Jun Corrected spelling of 'definition' but still match previous 'defention' and 	  'defenition' if found in file
	 Changed get_tool to find 1.6.04 tools or new tools with corrected spelling
10 Jun Put 3D into a separate layer called 3D, created unless it already exists
       Changed csp_normalized_slope to reject lines shorter than 1e-9.
10 Jun Changed all dimensions seen by user to be mm/inch, not pixels. This includes
	  tool diameter, maximum engraving distance, tool shape and all Z values.
12 Jun ver 208 Now scales correctly if orientation points moved or stretched.
12 Jun ver 209. Now detect if engraving toolshape not a function of radius
                Graphics now indicate Gcode toolpath, limited by min(tool diameter/2,max-dist)
TODO Change line division to be recursive, depending on what line is touched. See line_divide


engraving() functions (c) 2011 Chris Lusby Taylor, clusbytaylor@enterprise.net
Copyright (C) 2009 Nick Drobchenko, nick@cnc-club.ru
based on gcode.py (C) 2007 hugomatic... 
based on addnodes.py (C) 2005,2007 Aaron Spike, aaron@ekips.org
based on dots.py (C) 2005 Aaron Spike, aaron@ekips.org
based on interp.py (C) 2005 Aaron Spike, aaron@ekips.org
based on bezmisc.py (C) 2005 Aaron Spike, aaron@ekips.org
based on cubicsuperpath.py (C) 2005 Aaron Spike, aaron@ekips.org

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
"""

###
###		Gcodetools v 1.7
###

gcodetools_current_version = "1.7"

# standard library
import os
import math
import bezmisc
import re
import copy
import sys
import time
import cmath
import numpy
import codecs
import random
# local library
import inkex
import simplestyle
import simplepath
import cubicsuperpath
import simpletransform
import bezmisc

inkex.localize()
 
### Check if inkex has errormsg (0.46 version does not have one.) Could be removed later.
if "errormsg" not in dir(inkex):
	inkex.errormsg = lambda msg: sys.stderr.write((unicode(msg) + "\n").encode("UTF-8"))


def bezierslopeatt(((bx0,by0),(bx1,by1),(bx2,by2),(bx3,by3)),t):
	ax,ay,bx,by,cx,cy,x0,y0=bezmisc.bezierparameterize(((bx0,by0),(bx1,by1),(bx2,by2),(bx3,by3)))
	dx=3*ax*(t**2)+2*bx*t+cx
	dy=3*ay*(t**2)+2*by*t+cy
	if dx==dy==0 : 
		dx = 6*ax*t+2*bx
		dy = 6*ay*t+2*by
		if dx==dy==0 : 
			dx = 6*ax
			dy = 6*ay
			if dx==dy==0 : 
				print_("Slope error x = %s*t^3+%s*t^2+%s*t+%s, y = %s*t^3+%s*t^2+%s*t+%s,  t = %s, dx==dy==0" % (ax,bx,cx,dx,ay,by,cy,dy,t))
				print_(((bx0,by0),(bx1,by1),(bx2,by2),(bx3,by3)))
				dx, dy = 1, 1
				
	return dx,dy
bezmisc.bezierslopeatt = bezierslopeatt


def ireplace(self,old,new,count=0):
	pattern = re.compile(re.escape(old),re.I)
	return re.sub(pattern,new,self,count) 

def isset(variable):
	# VARIABLE NAME SHOULD BE A STRING! Like isset("foobar")
	return variable in locals() or variable in globals()
	
	
################################################################################
###
###		Styles and additional parameters
###
################################################################################

math.pi2 = math.pi*2
straight_tolerance = 0.0001
straight_distance_tolerance = 0.0001
engraving_tolerance = 0.0001
loft_lengths_tolerance = 0.0000001

EMC_TOLERANCE_EQUAL = 0.00001

options = {}
defaults = {
'header': """%
(Header)
(Generated by gcodetools from Inkscape.)
(Using default header. To add your own header create file "header" in the output dir.)
M3
(Header end.)
""",
'footer': """
(Footer)
M5
G00 X0.0000 Y0.0000
M2
(Using default footer. To add your own footer create file "footer" in the output dir.)
(end)
%"""
}

intersection_recursion_depth = 10
intersection_tolerance = 0.00001

styles = {
		"in_out_path_style" : simplestyle.formatStyle({ 'stroke': '#0072a7', 'fill': 'none', 'stroke-width':'1', 'marker-mid':'url(#InOutPathMarker)' }),
		
		"loft_style" : {
				'main curve':	simplestyle.formatStyle({ 'stroke': '#88f', 'fill': 'none', 'stroke-width':'1', 'marker-end':'url(#Arrow2Mend)' }),
			},
		"biarc_style" : {
				'biarc0':	simplestyle.formatStyle({ 'stroke': '#88f', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'1' }),
				'biarc1':	simplestyle.formatStyle({ 'stroke': '#8f8', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'1' }),
				'line':		simplestyle.formatStyle({ 'stroke': '#f88', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'1' }),
				'area':		simplestyle.formatStyle({ 'stroke': '#777', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'0.1' }),
			},
		"biarc_style_dark" : {
				'biarc0':	simplestyle.formatStyle({ 'stroke': '#33a', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'1' }),
				'biarc1':	simplestyle.formatStyle({ 'stroke': '#3a3', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'1' }),
				'line':		simplestyle.formatStyle({ 'stroke': '#a33', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'1' }),
				'area':		simplestyle.formatStyle({ 'stroke': '#222', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'0.3' }),
			},
		"biarc_style_dark_area" : {
				'biarc0':	simplestyle.formatStyle({ 'stroke': '#33a', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'0.1' }),
				'biarc1':	simplestyle.formatStyle({ 'stroke': '#3a3', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'0.1' }),
				'line':		simplestyle.formatStyle({ 'stroke': '#a33', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'0.1' }),
				'area':		simplestyle.formatStyle({ 'stroke': '#222', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'0.3' }),
			},
		"biarc_style_i"  : {
				'biarc0':	simplestyle.formatStyle({ 'stroke': '#880', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'1' }),
				'biarc1':	simplestyle.formatStyle({ 'stroke': '#808', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'1' }),
				'line':		simplestyle.formatStyle({ 'stroke': '#088', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'1' }),
				'area':		simplestyle.formatStyle({ 'stroke': '#999', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'0.3' }),
			},
		"biarc_style_dark_i" : {
				'biarc0':	simplestyle.formatStyle({ 'stroke': '#dd5', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'1' }),
				'biarc1':	simplestyle.formatStyle({ 'stroke': '#d5d', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'1' }),
				'line':		simplestyle.formatStyle({ 'stroke': '#5dd', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'1' }),
				'area':		simplestyle.formatStyle({ 'stroke': '#aaa', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'0.3' }),
			},
		"biarc_style_lathe_feed" : {
				'biarc0':	simplestyle.formatStyle({ 'stroke': '#07f', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'.4' }),
				'biarc1':	simplestyle.formatStyle({ 'stroke': '#0f7', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'.4' }),
				'line':		simplestyle.formatStyle({ 'stroke': '#f44', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'.4' }),
				'area':		simplestyle.formatStyle({ 'stroke': '#aaa', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'0.3' }),
			},
		"biarc_style_lathe_passing feed" : {
				'biarc0':	simplestyle.formatStyle({ 'stroke': '#07f', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'.4' }),
				'biarc1':	simplestyle.formatStyle({ 'stroke': '#0f7', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'.4' }),
				'line':		simplestyle.formatStyle({ 'stroke': '#f44', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'.4' }),
				'area':		simplestyle.formatStyle({ 'stroke': '#aaa', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'0.3' }),
			},
		"biarc_style_lathe_fine feed" : {
				'biarc0':	simplestyle.formatStyle({ 'stroke': '#7f0', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'.4' }),
				'biarc1':	simplestyle.formatStyle({ 'stroke': '#f70', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'.4' }),
				'line':		simplestyle.formatStyle({ 'stroke': '#744', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'.4' }),
				'area':		simplestyle.formatStyle({ 'stroke': '#aaa', 'fill': 'none', "marker-end":"url(#DrawCurveMarker)", 'stroke-width':'0.3' }),
			},
		"area artefact": 		simplestyle.formatStyle({ 'stroke': '#ff0000', 'fill': '#ffff00', 'stroke-width':'1' }),
		"area artefact arrow":	simplestyle.formatStyle({ 'stroke': '#ff0000', 'fill': '#ffff00', 'stroke-width':'1' }),
		"dxf_points":		 	simplestyle.formatStyle({ "stroke": "#ff0000", "fill": "#ff0000"}),
		
	}



################################################################################
###		Gcode additional functions
################################################################################

def gcode_comment_str(s, replace_new_line = False):
	if replace_new_line :
		s = re.sub(r"[\n\r]+", ".", s)
	res = ""
	if s[-1] == "\n" : s = s[:-1]
	for a in s.split("\n") :
		if a != "" : 
			res +=  "(" + re.sub(r"[\(\)\\\n\r]", ".", a) + ")\n"
		else : 	
			res +=  "\n"
	return res	


################################################################################
###		Cubic Super Path additional functions
################################################################################


def csp_from_polyline(line) :
	return [ [ [point[:] for k in range(3) ] for point in subline ]  for subline in line ]
	
def csp_remove_zerro_segments(csp, tolerance = 1e-7):	
	res = []
	for subpath in csp:
		if len(subpath) > 0 :
			res.append([subpath[0]])
			for sp1,sp2 in zip(subpath,subpath[1:]) :
				if point_to_point_d2(sp1[1],sp2[1])<=tolerance and point_to_point_d2(sp1[2],sp2[1])<=tolerance and point_to_point_d2(sp1[1],sp2[0])<=tolerance :
					res[-1][-1][2] = sp2[2]
				else :
					res[-1].append(sp2)
	return res
			

	


def point_inside_csp(p,csp, on_the_path = True) :
	# we'll do the raytracing and see how many intersections are there on the ray's way. 
	# if number of intersections is even then point is outside.
	# ray will be x=p.x and y=>p.y
	# you can assing any value to on_the_path, by dfault if point is on the path 
	# function will return thai it's inside the path. 
	x,y = p
	ray_intersections_count = 0
	for subpath in csp :
		
		for i in range(1, len(subpath)) :
			sp1, sp2 = subpath[i-1], subpath[i]
			ax,ay,bx,by,cx,cy,dx,dy = csp_parameterize(sp1,sp2)
		 	if  ax==0 and bx==0 and cx==0 and dx==x : 
		 		#we've got a special case here
		 		b = csp_true_bounds( [[sp1,sp2]])
		 		if  b[1][1]<=y<=b[3][1] :
		 			# points is on the path 
			 		return on_the_path
			 	else :
			 		# we can skip this segment because it wont influence the answer.
			 		pass	
		 	else: 
	 			for t in csp_line_intersection([x,y],[x,y+5],sp1,sp2) :
	 				if t == 0 or t == 1 :
	 					#we've got another special case here
		 				x1,y1 = csp_at_t(sp1,sp2,t)
		 				if y1==y : 
		 					# the point is on the path 
		 					return on_the_path
	 					# if t == 0 we sould have considered this case previously. 
	 					if t == 1 :
							# we have to check the next segmant if it is on the same side of the ray
							st_d = csp_normalized_slope(sp1,sp2,1)[0]
							if st_d == 0 : st_d = csp_normalized_slope(sp1,sp2,0.99)[0]
							
							for j in range(1, len(subpath)+1):
								if (i+j) % len(subpath) == 0  : continue # skip the closing segment 
								sp11,sp22 = subpath[(i-1+j) % len(subpath)], subpath[(i+j) % len(subpath)]
								ax1,ay1,bx1,by1,cx1,cy1,dx1,dy1 = csp_parameterize(sp1,sp2)
		 						if  ax1==0 and bx1==0 and cx1==0 and dx1==x : continue # this segment parallel to the ray, so skip it 
								en_d = csp_normalized_slope(sp11,sp22,0)[0]
								if en_d == 0 : en_d = csp_normalized_slope(sp11,sp22,0.01)[0]
								if st_d*en_d <=0 : 
			 						ray_intersections_count += 1
			 						break 
	 				else :	
		 				x1,y1 = csp_at_t(sp1,sp2,t)
		 				if y1==y : 
		 					# the point is on the path 
		 					return on_the_path
		 				else :
		 					if y1>y and 3*ax*t**2 + 2*bx*t + cx !=0 : # if it's 0 the path only touches the ray
		 						ray_intersections_count += 1 	
	return ray_intersections_count%2 == 1 				

def csp_close_all_subpaths(csp, tolerance = 0.000001):
	for i in range(len(csp)):
		if point_to_point_d2(csp[i][0][1] , csp[i][-1][1])> tolerance**2 :
			csp[i][-1][2] = csp[i][-1][1][:] 
			csp[i] += [ [csp[i][0][1][:] for j in range(3)] ]
		else: 
			if csp[i][0][1] != csp[i][-1][1] : 
				csp[i][-1][1] = csp[i][0][1][:]
	return csp

def csp_simple_bound(csp):
	minx,miny,maxx,maxy = None,None,None,None
	for subpath in csp:
		for sp in subpath : 
			for p in sp:
				minx = min(minx,p[0]) if minx!=None else p[0]
				miny = min(miny,p[1]) if miny!=None else p[1]
				maxx = max(maxx,p[0]) if maxx!=None else p[0]
				maxy = max(maxy,p[1]) if maxy!=None else p[1]
	return minx,miny,maxx,maxy		


def csp_segment_to_bez(sp1,sp2) :
	return sp1[1:]+sp2[:2]


def bound_to_bound_distance(sp1,sp2,sp3,sp4) :
	min_dist = 1e100
	max_dist = 0
	points1 = csp_segment_to_bez(sp1,sp2)
	points2 = csp_segment_to_bez(sp3,sp4)
	for i in range(4) :
		for j in range(4) :
			min_, max_ = line_to_line_min_max_distance_2(points1[i-1], points1[i], points2[j-1], points2[j])
			min_dist = min(min_dist,min_)
			max_dist = max(max_dist,max_)
			print_("bound_to_bound", min_dist, max_dist)
	return min_dist, max_dist
	
def csp_to_point_distance(csp, p, dist_bounds = [0,1e100], tolerance=.01) :
	min_dist = [1e100,0,0,0]
	for j in range(len(csp)) :
		for i in range(1,len(csp[j])) :
			d = csp_seg_to_point_distance(csp[j][i-1],csp[j][i],p,sample_points = 5, tolerance = .01)
			if d[0] < dist_bounds[0] : 
#				draw_pointer( list(csp_at_t(subpath[dist[2]-1],subpath[dist[2]],dist[3]))
#					+list(csp_at_t(csp[dist[4]][dist[5]-1],csp[dist[4]][dist[5]],dist[6])),"red","line", comment = math.sqrt(dist[0]))
				return [d[0],j,i,d[1]]
			else : 
				if d[0] < min_dist[0] : min_dist = [d[0],j,i,d[1]]
	return min_dist
			
def csp_seg_to_point_distance(sp1,sp2,p,sample_points = 5, tolerance = .01) :
	ax,ay,bx,by,cx,cy,dx,dy = csp_parameterize(sp1,sp2)
	dx, dy = dx-p[0], dy-p[1]
	if sample_points < 2 : sample_points = 2
	d = min( [(p[0]-sp1[1][0])**2 + (p[1]-sp1[1][1])**2,0.], [(p[0]-sp2[1][0])**2 + (p[1]-sp2[1][1])**2,1.]	)	
	for k in range(sample_points) :
		t = float(k)/(sample_points-1)
		i = 0
		while i==0 or abs(f)>0.000001 and i<20 : 
			t2,t3 = t**2,t**3
			f = (ax*t3+bx*t2+cx*t+dx)*(3*ax*t2+2*bx*t+cx) + (ay*t3+by*t2+cy*t+dy)*(3*ay*t2+2*by*t+cy)
			df = (6*ax*t+2*bx)*(ax*t3+bx*t2+cx*t+dx) + (3*ax*t2+2*bx*t+cx)**2 + (6*ay*t+2*by)*(ay*t3+by*t2+cy*t+dy) + (3*ay*t2+2*by*t+cy)**2
			if df!=0 :
				t = t - f/df
			else :	
				break
			i += 1	
		if 0<=t<=1 : 
			p1 = csp_at_t(sp1,sp2,t)
			d1 = (p1[0]-p[0])**2 + (p1[1]-p[1])**2
			if d1 < d[0] :
				d = [d1,t]
	return d	


def csp_seg_to_csp_seg_distance(sp1,sp2,sp3,sp4, dist_bounds = [0,1e100], sample_points = 5, tolerance=.01) : 
	# check the ending points first
	dist =	csp_seg_to_point_distance(sp1,sp2,sp3[1],sample_points, tolerance)
	dist += [0.]
	if dist[0] <= dist_bounds[0] : return dist
	d = csp_seg_to_point_distance(sp1,sp2,sp4[1],sample_points, tolerance)
	if d[0]<dist[0] :
		dist = d+[1.]
		if dist[0] <= dist_bounds[0] : return dist
	d =	csp_seg_to_point_distance(sp3,sp4,sp1[1],sample_points, tolerance)
	if d[0]<dist[0] :
		dist = [d[0],0.,d[1]]
		if dist[0] <= dist_bounds[0] : return dist
	d =	csp_seg_to_point_distance(sp3,sp4,sp2[1],sample_points, tolerance)
	if d[0]<dist[0] :
		dist = [d[0],1.,d[1]]
		if dist[0] <= dist_bounds[0] : return dist
	sample_points -= 2
	if sample_points < 1 : sample_points = 1
	ax1,ay1,bx1,by1,cx1,cy1,dx1,dy1 = csp_parameterize(sp1,sp2)
	ax2,ay2,bx2,by2,cx2,cy2,dx2,dy2 = csp_parameterize(sp3,sp4)
	#	try to find closes points using Newtons method
	for k in range(sample_points) :
		for j in range(sample_points) : 
			t1,t2 = float(k+1)/(sample_points+1), float(j)/(sample_points+1)
			t12, t13, t22, t23 = t1*t1, t1*t1*t1, t2*t2, t2*t2*t2
			i = 0
			F1, F2, F = [0,0], [[0,0],[0,0]], 1e100
			x,y   = ax1*t13+bx1*t12+cx1*t1+dx1 - (ax2*t23+bx2*t22+cx2*t2+dx2), ay1*t13+by1*t12+cy1*t1+dy1 - (ay2*t23+by2*t22+cy2*t2+dy2)			
			while i<2 or abs(F-Flast)>tolerance and i<30 :
				#draw_pointer(csp_at_t(sp1,sp2,t1))
				f1x = 3*ax1*t12+2*bx1*t1+cx1
				f1y = 3*ay1*t12+2*by1*t1+cy1
				f2x = 3*ax2*t22+2*bx2*t2+cx2
				f2y = 3*ay2*t22+2*by2*t2+cy2
				F1[0] = 2*f1x*x +  2*f1y*y
				F1[1] = -2*f2x*x -  2*f2y*y
				F2[0][0] =  2*(6*ax1*t1+2*bx1)*x + 2*f1x*f1x + 2*(6*ay1*t1+2*by1)*y +2*f1y*f1y
				F2[0][1] = -2*f1x*f2x - 2*f1y*f2y
				F2[1][0] = -2*f2x*f1x - 2*f2y*f1y 
				F2[1][1] = -2*(6*ax2*t2+2*bx2)*x + 2*f2x*f2x - 2*(6*ay2*t2+2*by2)*y + 2*f2y*f2y
				F2 = inv_2x2(F2)
				if F2!=None :
					t1 -= ( F2[0][0]*F1[0] + F2[0][1]*F1[1] )
					t2 -= ( F2[1][0]*F1[0] + F2[1][1]*F1[1] )
					t12, t13, t22, t23 = t1*t1, t1*t1*t1, t2*t2, t2*t2*t2
					x,y   = ax1*t13+bx1*t12+cx1*t1+dx1 - (ax2*t23+bx2*t22+cx2*t2+dx2), ay1*t13+by1*t12+cy1*t1+dy1 - (ay2*t23+by2*t22+cy2*t2+dy2)
					Flast = F
					F = x*x+y*y
				else : 
					break
				i += 1
			if F < dist[0] and 0<=t1<=1 and 0<=t2<=1:
				dist = [F,t1,t2]
				if dist[0] <= dist_bounds[0] : 
					return dist
	return dist			


def csp_to_csp_distance(csp1,csp2, dist_bounds = [0,1e100], tolerance=.01) : 
	dist = [1e100,0,0,0,0,0,0]
	for i1 in range(len(csp1)) : 
		for j1 in range(1,len(csp1[i1])) :
			for i2 in range(len(csp2)) :
				for j2 in range(1,len(csp2[i2])) :
					d = csp_seg_bound_to_csp_seg_bound_max_min_distance(csp1[i1][j1-1],csp1[i1][j1],csp2[i2][j2-1],csp2[i2][j2])
					if d[0] >= dist_bounds[1] : continue
					if  d[1] < dist_bounds[0] : return [d[1],i1,j1,1,i2,j2,1]
					d = csp_seg_to_csp_seg_distance(csp1[i1][j1-1],csp1[i1][j1],csp2[i2][j2-1],csp2[i2][j2], dist_bounds, tolerance=tolerance)
					if d[0] < dist[0] :
						dist = [d[0], i1,j1,d[1], i2,j2,d[2]]
					if dist[0] <= dist_bounds[0] :	
						return dist
			if dist[0] >= dist_bounds[1] :	
				return dist
	return dist
#	draw_pointer( list(csp_at_t(csp1[dist[1]][dist[2]-1],csp1[dist[1]][dist[2]],dist[3]))
#				+ list(csp_at_t(csp2[dist[4]][dist[5]-1],csp2[dist[4]][dist[5]],dist[6])), "#507","line")
					
					
def csp_split(sp1,sp2,t=.5) :
	[x1,y1],[x2,y2],[x3,y3],[x4,y4] = sp1[1], sp1[2], sp2[0], sp2[1] 
	x12 = x1+(x2-x1)*t
	y12 = y1+(y2-y1)*t
	x23 = x2+(x3-x2)*t
	y23 = y2+(y3-y2)*t
	x34 = x3+(x4-x3)*t
	y34 = y3+(y4-y3)*t
	x1223 = x12+(x23-x12)*t
	y1223 = y12+(y23-y12)*t
	x2334 = x23+(x34-x23)*t
	y2334 = y23+(y34-y23)*t
	x = x1223+(x2334-x1223)*t
	y = y1223+(y2334-y1223)*t
	return [sp1[0],sp1[1],[x12,y12]], [[x1223,y1223],[x,y],[x2334,y2334]], [[x34,y34],sp2[1],sp2[2]]
	
	
def csp_true_bounds(csp) :
	# Finds minx,miny,maxx,maxy of the csp and return their (x,y,i,j,t) 
	minx = [float("inf"), 0, 0, 0]																																																																																																								
	maxx = [float("-inf"), 0, 0, 0]
	miny = [float("inf"), 0, 0, 0]
	maxy = [float("-inf"), 0, 0, 0]
	for i in range(len(csp)):
		for j in range(1,len(csp[i])):
			ax,ay,bx,by,cx,cy,x0,y0 = bezmisc.bezierparameterize((csp[i][j-1][1],csp[i][j-1][2],csp[i][j][0],csp[i][j][1]))
			roots = cubic_solver(0, 3*ax, 2*bx, cx)	 + [0,1]
			for root in roots :
				if type(root) is complex and abs(root.imag)<1e-10:
					root = root.real
				if type(root) is not complex and 0<=root<=1:
					y = ay*(root**3)+by*(root**2)+cy*root+y0  
					x = ax*(root**3)+bx*(root**2)+cx*root+x0  
					maxx = max([x,y,i,j,root],maxx)
					minx = min([x,y,i,j,root],minx)

			roots = cubic_solver(0, 3*ay, 2*by, cy)	 + [0,1]
			for root in roots :
				if type(root) is complex and root.imag==0:
					root = root.real
				if type(root) is not complex and 0<=root<=1:
					y = ay*(root**3)+by*(root**2)+cy*root+y0  
					x = ax*(root**3)+bx*(root**2)+cx*root+x0  
					maxy = max([y,x,i,j,root],maxy)
					miny = min([y,x,i,j,root],miny)
	maxy[0],maxy[1] = maxy[1],maxy[0]
	miny[0],miny[1] = miny[1],miny[0]

	return minx,miny,maxx,maxy


############################################################################
### csp_segments_intersection(sp1,sp2,sp3,sp4)
###
### Returns array containig all intersections between two segmets of cubic 
### super path. Results are [ta,tb], or [ta0, ta1, tb0, tb1, "Overlap"] 
### where ta, tb are values of t for the intersection point.
############################################################################
def csp_segments_intersection(sp1,sp2,sp3,sp4) :
	a, b = csp_segment_to_bez(sp1,sp2), csp_segment_to_bez(sp3,sp4)

	def polish_intersection(a,b,ta,tb, tolerance = intersection_tolerance) :
		ax,ay,bx,by,cx,cy,dx,dy			= bezmisc.bezierparameterize(a)
		ax1,ay1,bx1,by1,cx1,cy1,dx1,dy1	= bezmisc.bezierparameterize(b)
		i = 0
		F, F1 =  [.0,.0], [[.0,.0],[.0,.0]]
		while i==0 or (abs(F[0])**2+abs(F[1])**2 > tolerance and i<10):
			ta3, ta2, tb3, tb2 = ta**3, ta**2, tb**3, tb**2
			F[0] = ax*ta3+bx*ta2+cx*ta+dx-ax1*tb3-bx1*tb2-cx1*tb-dx1
			F[1] = ay*ta3+by*ta2+cy*ta+dy-ay1*tb3-by1*tb2-cy1*tb-dy1
			F1[0][0] =  3*ax *ta2 + 2*bx *ta + cx
			F1[0][1] = -3*ax1*tb2 - 2*bx1*tb - cx1
			F1[1][0] =  3*ay *ta2 + 2*by *ta + cy
			F1[1][1] = -3*ay1*tb2 - 2*by1*tb - cy1	
			det = F1[0][0]*F1[1][1] - F1[0][1]*F1[1][0]
			if det!=0 :
				F1 = [	[ F1[1][1]/det, -F1[0][1]/det],	[-F1[1][0]/det,  F1[0][0]/det] ]
				ta = ta - ( F1[0][0]*F[0] + F1[0][1]*F[1] )
				tb = tb - ( F1[1][0]*F[0] + F1[1][1]*F[1] )
			else: break	
			i += 1

		return ta, tb 			


	def recursion(a,b, ta0,ta1,tb0,tb1, depth_a,depth_b) :
		global bezier_intersection_recursive_result
		if a==b : 
			bezier_intersection_recursive_result += [[ta0,tb0,ta1,tb1,"Overlap"]]
			return 
		tam, tbm = (ta0+ta1)/2, (tb0+tb1)/2
		if depth_a>0 and depth_b>0 : 
			a1,a2 = bez_split(a,0.5)
			b1,b2 = bez_split(b,0.5)
			if bez_bounds_intersect(a1,b1) : recursion(a1,b1, ta0,tam,tb0,tbm, depth_a-1,depth_b-1) 
			if bez_bounds_intersect(a2,b1) : recursion(a2,b1, tam,ta1,tb0,tbm, depth_a-1,depth_b-1) 
			if bez_bounds_intersect(a1,b2) : recursion(a1,b2, ta0,tam,tbm,tb1, depth_a-1,depth_b-1) 
			if bez_bounds_intersect(a2,b2) : recursion(a2,b2, tam,ta1,tbm,tb1, depth_a-1,depth_b-1) 
		elif depth_a>0  : 
			a1,a2 = bez_split(a,0.5)
			if bez_bounds_intersect(a1,b) : recursion(a1,b, ta0,tam,tb0,tb1, depth_a-1,depth_b) 
			if bez_bounds_intersect(a2,b) : recursion(a2,b, tam,ta1,tb0,tb1, depth_a-1,depth_b) 
		elif depth_b>0  : 
			b1,b2 = bez_split(b,0.5)
			if bez_bounds_intersect(a,b1) : recursion(a,b1, ta0,ta1,tb0,tbm, depth_a,depth_b-1) 
			if bez_bounds_intersect(a,b2) : recursion(a,b2, ta0,ta1,tbm,tb1, depth_a,depth_b-1) 
		else : # Both segments have been subdevided enougth. Let's get some intersections :).
			intersection, t1, t2 =  straight_segments_intersection([a[0]]+[a[3]],[b[0]]+[b[3]])
			if intersection :
				if intersection == "Overlap" :
					t1 = ( max(0,min(1,t1[0]))+max(0,min(1,t1[1])) )/2
					t2 = ( max(0,min(1,t2[0]))+max(0,min(1,t2[1])) )/2
				bezier_intersection_recursive_result += [[ta0+t1*(ta1-ta0),tb0+t2*(tb1-tb0)]]

	global bezier_intersection_recursive_result
	bezier_intersection_recursive_result = []
	recursion(a,b,0.,1.,0.,1.,intersection_recursion_depth,intersection_recursion_depth)
	intersections = bezier_intersection_recursive_result
	for i in range(len(intersections)) :			
		if len(intersections[i])<5 or intersections[i][4] != "Overlap" :
			intersections[i] = polish_intersection(a,b,intersections[i][0],intersections[i][1])
	return intersections


def csp_segments_true_intersection(sp1,sp2,sp3,sp4) :
	intersections = csp_segments_intersection(sp1,sp2,sp3,sp4)
	res = []
	for intersection in intersections :
		if  (
				(len(intersection)==5 and intersection[4] == "Overlap" and (0<=intersection[0]<=1 or 0<=intersection[1]<=1) and (0<=intersection[2]<=1 or 0<=intersection[3]<=1) ) 
			 or ( 0<=intersection[0]<=1 and 0<=intersection[1]<=1 )
			) :
			res += [intersection]
	return res


def csp_get_t_at_curvature(sp1,sp2,c, sample_points = 16):
	# returns a list containning [t1,t2,t3,...,tn],  0<=ti<=1...
	if sample_points < 2 : sample_points = 2
	tolerance = .0000000001
	res = []
	ax,ay,bx,by,cx,cy,dx,dy = csp_parameterize(sp1,sp2)
	for k in range(sample_points) :
		t = float(k)/(sample_points-1)
		i, F = 0, 1e100
		while i<2 or abs(F)>tolerance and i<17 :
			try : # some numerical calculation could exceed the limits 
				t2 = t*t
				#slopes...
				f1x = 3*ax*t2+2*bx*t+cx
				f1y = 3*ay*t2+2*by*t+cy
				f2x = 6*ax*t+2*bx
				f2y = 6*ay*t+2*by
				f3x = 6*ax
				f3y = 6*ay
				d = (f1x**2+f1y**2)**1.5
				F1 = (
						 (	(f1x*f3y-f3x*f1y)*d - (f1x*f2y-f2x*f1y)*3.*(f2x*f1x+f2y*f1y)*((f1x**2+f1y**2)**.5) )	/ 
				 				((f1x**2+f1y**2)**3)
					 )
				F = (f1x*f2y-f1y*f2x)/d - c
				t -= F/F1
			except:
				break
			i += 1
		if 0<=t<=1 and F<=tolerance:
			if len(res) == 0 :
				res.append(t)
			for i in res : 
				if abs(t-i)<=0.001 :
					break
			if not abs(t-i)<=0.001 :
				res.append(t)
	return res	
		
	
def csp_max_curvature(sp1,sp2):
	ax,ay,bx,by,cx,cy,dx,dy = csp_parameterize(sp1,sp2)
	tolerance = .0001
	F = 0.
	i = 0
	while i<2 or F-Flast<tolerance and i<10 :
		t = .5
		f1x = 3*ax*t**2 + 2*bx*t + cx
		f1y = 3*ay*t**2 + 2*by*t + cy
		f2x = 6*ax*t + 2*bx
		f2y = 6*ay*t + 2*by
		f3x = 6*ax
		f3y = 6*ay
		d = pow(f1x**2+f1y**2,1.5)
		if d != 0 :
			Flast = F
			F = (f1x*f2y-f1y*f2x)/d
			F1 = 	(
						 (	d*(f1x*f3y-f3x*f1y) - (f1x*f2y-f2x*f1y)*3.*(f2x*f1x+f2y*f1y)*pow(f1x**2+f1y**2,.5) )	/ 
				 				(f1x**2+f1y**2)**3
					)
			i+=1	
			if F1!=0:
				t -= F/F1
			else:
				break
		else: break
	return t			
	

def csp_curvature_at_t(sp1,sp2,t, depth = 3) :
	ax,ay,bx,by,cx,cy,dx,dy = bezmisc.bezierparameterize(csp_segment_to_bez(sp1,sp2))
	
	#curvature = (x'y''-y'x'') / (x'^2+y'^2)^1.5 
	
	f1x = 3*ax*t**2 + 2*bx*t + cx
	f1y = 3*ay*t**2 + 2*by*t + cy
	f2x = 6*ax*t + 2*bx
	f2y = 6*ay*t + 2*by
	d = (f1x**2+f1y**2)**1.5
	if d != 0 :
		return (f1x*f2y-f1y*f2x)/d
	else :
		t1 = f1x*f2y-f1y*f2x
		if t1 > 0 : return 1e100
		if t1 < 0 : return -1e100
		# Use the Lapitals rule to solve 0/0 problem for 2 times...
		t1 = 2*(bx*ay-ax*by)*t+(ay*cx-ax*cy)
		if t1 > 0 : return 1e100
		if t1 < 0 : return -1e100
		t1 = bx*ay-ax*by
		if t1 > 0 : return 1e100
		if t1 < 0 : return -1e100
		if depth>0 :
			# little hack ;^) hope it wont influence anything...
			return csp_curvature_at_t(sp1,sp2,t*1.004, depth-1)
		return 1e100

	 	
def csp_curvature_radius_at_t(sp1,sp2,t) :
	c = csp_curvature_at_t(sp1,sp2,t)
	if c == 0 : return 1e100 
	else: return 1/c


def csp_special_points(sp1,sp2) :
	# special points = curvature == 0
	ax,ay,bx,by,cx,cy,dx,dy = bezmisc.bezierparameterize((sp1[1],sp1[2],sp2[0],sp2[1]))	
	a = 3*ax*by-3*ay*bx
	b = 3*ax*cy-3*cx*ay
	c = bx*cy-cx*by
	roots = cubic_solver(0, a, b, c)	
	res = []
	for i in roots :
		if type(i) is complex and i.imag==0:
			i = i.real
		if type(i) is not complex and 0<=i<=1:
			res.append(i)
	return res

	
def csp_subpath_ccw(subpath):
	# Remove all zerro length segments
	s = 0
	#subpath = subpath[:]
	if (P(subpath[-1][1])-P(subpath[0][1])).l2() > 1e-10 :
		subpath[-1][2] = subpath[-1][1]
		subpath[0][0] = subpath[0][1]
		subpath += [ [subpath[0][1],subpath[0][1],subpath[0][1]]  ]
	pl = subpath[-1][2]
	for sp1 in subpath:
		for p in sp1 :
			s += (p[0]-pl[0])*(p[1]+pl[1])
			pl = p
	return s<0


def csp_at_t(sp1,sp2,t):
	ax,bx,cx,dx = sp1[1][0], sp1[2][0], sp2[0][0], sp2[1][0]
	ay,by,cy,dy = sp1[1][1], sp1[2][1], sp2[0][1], sp2[1][1]

	x1, y1 = ax+(bx-ax)*t, ay+(by-ay)*t	
	x2, y2 = bx+(cx-bx)*t, by+(cy-by)*t	
	x3, y3 = cx+(dx-cx)*t, cy+(dy-cy)*t	
	
	x4,y4 = x1+(x2-x1)*t, y1+(y2-y1)*t 
	x5,y5 = x2+(x3-x2)*t, y2+(y3-y2)*t 
	
	x,y = x4+(x5-x4)*t, y4+(y5-y4)*t 
	return [x,y]

def csp_at_length(sp1,sp2,l=0.5, tolerance = 0.01):
	bez = (sp1[1][:],sp1[2][:],sp2[0][:],sp2[1][:])
	t = bezmisc.beziertatlength(bez, l, tolerance)
	return csp_at_t(sp1,sp2,t)
	

def csp_splitatlength(sp1, sp2, l = 0.5, tolerance = 0.01):
	bez = (sp1[1][:],sp1[2][:],sp2[0][:],sp2[1][:])
	t = bezmisc.beziertatlength(bez, l, tolerance)
	return csp_split(sp1, sp2, t)	

	
def cspseglength(sp1,sp2, tolerance = 0.01):
	bez = (sp1[1][:],sp1[2][:],sp2[0][:],sp2[1][:])
	return bezmisc.bezierlength(bez, tolerance)	


def csplength(csp):
	total = 0
	lengths = []
	for sp in csp:
		for i in xrange(1,len(sp)):
			l = cspseglength(sp[i-1],sp[i])
			lengths.append(l)
			total += l			
	return lengths, total


def csp_segments(csp):
	l, seg = 0, [0]
	for sp in csp:
		for i in xrange(1,len(sp)):
			l += cspseglength(sp[i-1],sp[i])
			seg += [ l ] 

	if l>0 :
		seg = [seg[i]/l for i in xrange(len(seg))]
	return seg,l


def rebuild_csp (csp, segs, s=None):
	# rebuild_csp() adds to csp control points making it's segments looks like segs
	if s==None : s, l = csp_segments(csp)
	
	if len(s)>len(segs) : return None
	segs = segs[:]
	segs.sort()
	for i in xrange(len(s)):
		d = None
		for j in xrange(len(segs)):
			d = min( [abs(s[i]-segs[j]),j], d) if d!=None else [abs(s[i]-segs[j]),j]
		del segs[d[1]]
	for i in xrange(len(segs)):
		for j in xrange(0,len(s)):
			if segs[i]<s[j] : break
		if s[j]-s[j-1] != 0 :
			t = (segs[i] - s[j-1])/(s[j]-s[j-1])
			sp1,sp2,sp3 = csp_split(csp[j-1],csp[j], t)
			csp = csp[:j-1] + [sp1,sp2,sp3] + csp[j+1:]
			s = s[:j] + [ s[j-1]*(1-t)+s[j]*t   ] + s[j:]
	return csp, s


def csp_slope(sp1,sp2,t):
	bez = (sp1[1][:],sp1[2][:],sp2[0][:],sp2[1][:])
	return bezmisc.bezierslopeatt(bez,t)


def csp_line_intersection(l1,l2,sp1,sp2):
	dd=l1[0]
	cc=l2[0]-l1[0]
	bb=l1[1]
	aa=l2[1]-l1[1]
	if aa==cc==0 : return []
	if aa:
		coef1=cc/aa
		coef2=1
	else:
		coef1=1
		coef2=aa/cc
	bez = (sp1[1][:],sp1[2][:],sp2[0][:],sp2[1][:])
	ax,ay,bx,by,cx,cy,x0,y0=bezmisc.bezierparameterize(bez)
	a=coef1*ay-coef2*ax
	b=coef1*by-coef2*bx
	c=coef1*cy-coef2*cx
	d=coef1*(y0-bb)-coef2*(x0-dd)
	roots = cubic_solver(a,b,c,d)
	retval = []
	for i in roots :
		if type(i) is complex and abs(i.imag)<1e-7:
			i = i.real
		if type(i) is not complex and -1e-10<=i<=1.+1e-10:
			retval.append(i)
	return retval


def csp_split_by_two_points(sp1,sp2,t1,t2) :
	if t1>t2 : t1, t2 = t2, t1
	if t1 == t2 : 
		sp1,sp2,sp3 =  csp_split(sp1,sp2,t)
		return [sp1,sp2,sp2,sp3]
	elif t1 <= 1e-10 and t2 >= 1.-1e-10 :
		return [sp1,sp1,sp2,sp2]
	elif t1 <= 1e-10:	
		sp1,sp2,sp3 = csp_split(sp1,sp2,t2)
		return [sp1,sp1,sp2,sp3]
	elif t2 >= 1.-1e-10 : 
		sp1,sp2,sp3 = csp_split(sp1,sp2,t1)
		return [sp1,sp2,sp3,sp3]
	else:
		sp1,sp2,sp3 = csp_split(sp1,sp2,t1)
		sp2,sp3,sp4 = csp_split(sp2,sp3,(t2-t1)/(1-t1) )
		return [sp1,sp2,sp3,sp4]

def csp_seg_split(sp1,sp2, points):
	# points is float=t or list [t1, t2, ..., tn]
	if type(points) is float :
		points = [points]
	points.sort()
	res = [sp1,sp2]
	last_t = 0
	for t in points: 
		if 1e-10<t<1.-1e-10 :
			sp3,sp4,sp5 = csp_split(res[-2],res[-1], (t-last_t)/(1-last_t))
			last_t = t
			res[-2:] = [sp3,sp4,sp5]
	return res		


def csp_subpath_split_by_points(subpath, points) :
	# points are [[i,t]...] where i-segment's number
	points.sort()
	points = [[1,0.]] + points + [[len(subpath)-1,1.]]
	parts = []
	for int1,int2 in zip(points,points[1:]) : 
		if int1==int2 :
			continue
		if int1[1] == 1. :
			int1[0] += 1
			int1[1] = 0.
		if int1==int2 :
			continue
		if int2[1] == 0. :
			int2[0] -= 1
			int2[1] = 1.
		if int1[0] == 0 and int2[0]==len(subpath)-1:# and small(int1[1]) and small(int2[1]-1) :
			continue
		if int1[0]==int2[0] :	# same segment
			sp = csp_split_by_two_points(subpath[int1[0]-1],subpath[int1[0]],int1[1], int2[1])
			if sp[1]!=sp[2] :
				parts += [   [sp[1],sp[2]]     ]
		else :
			sp5,sp1,sp2 = csp_split(subpath[int1[0]-1],subpath[int1[0]],int1[1])
			sp3,sp4,sp5 = csp_split(subpath[int2[0]-1],subpath[int2[0]],int2[1])
			if int1[0]==int2[0]-1 :
				parts += [	[sp1, [sp2[0],sp2[1],sp3[2]], sp4]  ]
			else :
				parts += [  [sp1,sp2]+subpath[int1[0]+1:int2[0]-1]+[sp3,sp4]  ]
	return parts


def arc_from_s_r_n_l(s,r,n,l) :
	if abs(n[0]**2+n[1]**2 - 1) > 1e-10 : n = normalize(n)
	return arc_from_c_s_l([s[0]+n[0]*r, s[1]+n[1]*r],s,l)


def arc_from_c_s_l(c,s,l) :
	r = point_to_point_d(c,s)
	if r == 0 : return []
	alpha = l/r 
	cos_, sin_ = math.cos(alpha), math.sin(alpha)
	e = [ c[0] + (s[0]-c[0])*cos_ - (s[1]-c[1])*sin_, c[1] + (s[0]-c[0])*sin_ + (s[1]-c[1])*cos_]
	n = [c[0]-s[0],c[1]-s[1]]
	slope = rotate_cw(n) if l>0 else rotate_ccw(n)
	return csp_from_arc(s, e, c, r, slope)


def csp_from_arc(start, end, center, r, slope_st) : 
	# Creates csp that approximise specified arc
	r = abs(r)
	alpha = (atan2(end[0]-center[0],end[1]-center[1]) - atan2(start[0]-center[0],start[1]-center[1])) % math.pi2

	sectors = int(abs(alpha)*2/math.pi)+1
	alpha_start = atan2(start[0]-center[0],start[1]-center[1])
	cos_,sin_ = math.cos(alpha_start), math.sin(alpha_start)
	k = (4.*math.tan(alpha/sectors/4.)/3.)
	if dot(slope_st , [- sin_*k*r, cos_*k*r]) < 0 :
		if alpha>0 : alpha -= math.pi2
		else: alpha += math.pi2
	if abs(alpha*r)<0.001 : 
		return []

	sectors = int(abs(alpha)*2/math.pi)+1
	k = (4.*math.tan(alpha/sectors/4.)/3.)
	result = []
	for i in range(sectors+1) :
		cos_,sin_ = math.cos(alpha_start + alpha*i/sectors), math.sin(alpha_start + alpha*i/sectors)
		sp = [ [], [center[0] + cos_*r, center[1] + sin_*r], [] ]
		sp[0] = [sp[1][0] + sin_*k*r, sp[1][1] - cos_*k*r ]
		sp[2] = [sp[1][0] - sin_*k*r, sp[1][1] + cos_*k*r ]
		result += [sp]
	result[0][0] = result[0][1][:]
	result[-1][2] = result[-1][1]
		
	return result


def point_to_arc_distance(p, arc):
	###		Distance calculattion from point to arc
	P0,P2,c,a = arc
	dist = None
	p = P(p)
	r = (P0-c).mag()
	if r>0 :
		i = c + (p-c).unit()*r
		alpha = ((i-c).angle() - (P0-c).angle())
		if a*alpha<0: 
			if alpha>0:	alpha = alpha-math.pi2
			else: alpha = math.pi2+alpha
		if between(alpha,0,a) or min(abs(alpha),abs(alpha-a))<straight_tolerance : 
			return (p-i).mag(), [i.x, i.y]
		else : 
			d1, d2 = (p-P0).mag(), (p-P2).mag()
			if d1<d2 : 
				return (d1, [P0.x,P0.y])
			else :
				return (d2, [P2.x,P2.y])


def csp_to_arc_distance(sp1,sp2, arc1, arc2, tolerance = 0.01 ): # arc = [start,end,center,alpha]
	n, i = 10, 0
	d, d1, dl = (0,(0,0)), (0,(0,0)), 0
	while i<1 or (abs(d1[0]-dl[0])>tolerance and i<4):
		i += 1
		dl = d1*1	
		for j in range(n+1):
			t = float(j)/n
			p = csp_at_t(sp1,sp2,t) 
			d = min(point_to_arc_distance(p,arc1), point_to_arc_distance(p,arc2))
			d1 = max(d1,d)
		n=n*2
	return d1[0]


def csp_simple_bound_to_point_distance(p, csp):
	minx,miny,maxx,maxy = None,None,None,None
	for subpath in csp:
		for sp in subpath:
			for p_ in sp:
				minx = min(minx,p_[0]) if minx!=None else p_[0]
				miny = min(miny,p_[1]) if miny!=None else p_[1]
				maxx = max(maxx,p_[0]) if maxx!=None else p_[0]
				maxy = max(maxy,p_[1]) if maxy!=None else p_[1]
	return math.sqrt(max(minx-p[0],p[0]-maxx,0)**2+max(miny-p[1],p[1]-maxy,0)**2)


def csp_point_inside_bound(sp1, sp2, p):
	bez = [sp1[1],sp1[2],sp2[0],sp2[1]]
	x,y = p
	c = 0
	#CLT added test of x in range
	xmin=1e100
	xmax=-1e100
	for i in range(4):
		[x0,y0], [x1,y1] = bez[i-1], bez[i]
		xmin=min(xmin,x0)
		xmax=max(xmax,x0)
		if x0-x1!=0 and (y-y0)*(x1-x0)>=(x-x0)*(y1-y0) and x>min(x0,x1) and x<=max(x0,x1) :
			c +=1
	return xmin<=x<=xmax and c%2==0	


def csp_bound_to_point_distance(sp1, sp2, p):
	if csp_point_inside_bound(sp1, sp2, p) :
		return 0.
	bez = csp_segment_to_bez(sp1,sp2)
	min_dist = 1e100
	for i in range(0,4):
		d = point_to_line_segment_distance_2(p, bez[i-1],bez[i])
		if d <= min_dist : min_dist = d
	return min_dist 


def line_line_intersect(p1,p2,p3,p4) : # Return only true intersection. 
	if (p1[0]==p2[0] and p1[1]==p2[1]) or (p3[0]==p4[0] and p3[1]==p4[1]) : return False
	x = (p2[0]-p1[0])*(p4[1]-p3[1]) - (p2[1]-p1[1])*(p4[0]-p3[0])
	if x==0 : # Lines are parallel
		if (p3[0]-p1[0])*(p2[1]-p1[1]) == (p3[1]-p1[1])*(p2[0]-p1[0]) :
			if p3[0]!=p4[0] :
				t11 = (p1[0]-p3[0])/(p4[0]-p3[0]) 
				t12 = (p2[0]-p3[0])/(p4[0]-p3[0]) 
				t21 = (p3[0]-p1[0])/(p2[0]-p1[0])
				t22 = (p4[0]-p1[0])/(p2[0]-p1[0])
			else:
				t11 = (p1[1]-p3[1])/(p4[1]-p3[1]) 
				t12 = (p2[1]-p3[1])/(p4[1]-p3[1]) 
				t21 = (p3[1]-p1[1])/(p2[1]-p1[1])
				t22 = (p4[1]-p1[1])/(p2[1]-p1[1])
			return ("Overlap" if (0<=t11<=1 or 0<=t12<=1) and (0<=t21<=1 or  0<=t22<=1) else False)
		else: return False	
	else :
		return (
					0<=((p4[0]-p3[0])*(p1[1]-p3[1]) - (p4[1]-p3[1])*(p1[0]-p3[0]))/x<=1 and
					0<=((p2[0]-p1[0])*(p1[1]-p3[1]) - (p2[1]-p1[1])*(p1[0]-p3[0]))/x<=1 )
					
					
def line_line_intersection_points(p1,p2,p3,p4) : # Return only points [ (x,y) ] 
	if (p1[0]==p2[0] and p1[1]==p2[1]) or (p3[0]==p4[0] and p3[1]==p4[1]) : return []
	x = (p2[0]-p1[0])*(p4[1]-p3[1]) - (p2[1]-p1[1])*(p4[0]-p3[0])
	if x==0 : # Lines are parallel
		if (p3[0]-p1[0])*(p2[1]-p1[1]) == (p3[1]-p1[1])*(p2[0]-p1[0]) :
			if p3[0]!=p4[0] :
				t11 = (p1[0]-p3[0])/(p4[0]-p3[0]) 
				t12 = (p2[0]-p3[0])/(p4[0]-p3[0]) 
				t21 = (p3[0]-p1[0])/(p2[0]-p1[0])
				t22 = (p4[0]-p1[0])/(p2[0]-p1[0])
			else:
				t11 = (p1[1]-p3[1])/(p4[1]-p3[1]) 
				t12 = (p2[1]-p3[1])/(p4[1]-p3[1]) 
				t21 = (p3[1]-p1[1])/(p2[1]-p1[1])
				t22 = (p4[1]-p1[1])/(p2[1]-p1[1])
			res = [] 
			if (0<=t11<=1 or 0<=t12<=1) and (0<=t21<=1 or  0<=t22<=1) :
				if 0<=t11<=1 : res += [p1]	
				if 0<=t12<=1 : res += [p2]	
				if 0<=t21<=1 : res += [p3]	
				if 0<=t22<=1 : res += [p4]	
			return res
		else: return []
	else :
		t1 = ((p4[0]-p3[0])*(p1[1]-p3[1]) - (p4[1]-p3[1])*(p1[0]-p3[0]))/x
		t2 = ((p2[0]-p1[0])*(p1[1]-p3[1]) - (p2[1]-p1[1])*(p1[0]-p3[0]))/x
		if 0<=t1<=1 and 0<=t2<=1 : return [ [p1[0]*(1-t1)+p2[0]*t1, p1[1]*(1-t1)+p2[1]*t1] ]
		else : return []					


def point_to_point_d2(a,b):
	return (a[0]-b[0])**2 + (a[1]-b[1])**2


def point_to_point_d(a,b):
	return math.sqrt((a[0]-b[0])**2 + (a[1]-b[1])**2)


def point_to_line_segment_distance_2(p1, p2,p3) :
	# p1 - point, p2,p3 - line segment
	#draw_pointer(p1)
	w0 = [p1[0]-p2[0], p1[1]-p2[1]]
	v = [p3[0]-p2[0], p3[1]-p2[1]]
	c1 = w0[0]*v[0] + w0[1]*v[1]
	if c1 <= 0 :
		return w0[0]*w0[0]+w0[1]*w0[1]
	c2 = v[0]*v[0] + v[1]*v[1]
	if c2 <= c1 :
		return  (p1[0]-p3[0])**2 + (p1[1]-p3[1])**2
	return (p1[0]- p2[0]-v[0]*c1/c2)**2 + (p1[1]- p2[1]-v[1]*c1/c2)


def line_to_line_distance_2(p1,p2,p3,p4):
	if line_line_intersect(p1,p2,p3,p4) : return 0
	return min(
			point_to_line_segment_distance_2(p1,p3,p4),
			point_to_line_segment_distance_2(p2,p3,p4),
			point_to_line_segment_distance_2(p3,p1,p2),
			point_to_line_segment_distance_2(p4,p1,p2))


def csp_seg_bound_to_csp_seg_bound_max_min_distance(sp1,sp2,sp3,sp4) :
	bez1 = csp_segment_to_bez(sp1,sp2)
	bez2 = csp_segment_to_bez(sp3,sp4) 
	min_dist = 1e100
	max_dist = 0.
	for i in range(4) :
		if csp_point_inside_bound(sp1, sp2, bez2[i]) or csp_point_inside_bound(sp3, sp4, bez1[i]) :
			min_dist = 0.
			break	
	for i in range(4) : 
		for j in range(4) : 
			d = line_to_line_distance_2(bez1[i-1],bez1[i],bez2[j-1],bez2[j])
			if d < min_dist : min_dist = d
			d = (bez2[j][0]-bez1[i][0])**2 + (bez2[j][1]-bez1[i][1])**2 
			if max_dist < d  : max_dist = d
	return min_dist, max_dist


def csp_reverse(csp) :
	for i in range(len(csp)) :
	 	n = []
	 	for j in csp[i] :
			n = [  [j[2][:],j[1][:],j[0][:]]  ] + n
	 	csp[i] = n[:]
	return csp			


def csp_normalized_slope(sp1,sp2,t) :
	ax,ay,bx,by,cx,cy,dx,dy=bezmisc.bezierparameterize((sp1[1][:],sp1[2][:],sp2[0][:],sp2[1][:]))
	if sp1[1]==sp2[1]==sp1[2]==sp2[0] : return [1.,0.]
	f1x = 3*ax*t*t+2*bx*t+cx
	f1y = 3*ay*t*t+2*by*t+cy
	if abs(f1x*f1x+f1y*f1y) > 1e-9 : #LT changed this from 1e-20, which caused problems
		l = math.sqrt(f1x*f1x+f1y*f1y)
		return [f1x/l, f1y/l]

	if t == 0 :
		f1x = sp2[0][0]-sp1[1][0]
		f1y = sp2[0][1]-sp1[1][1]
		if abs(f1x*f1x+f1y*f1y) > 1e-9 : #LT changed this from 1e-20, which caused problems
			l = math.sqrt(f1x*f1x+f1y*f1y)
			return [f1x/l, f1y/l]
		else :
			f1x = sp2[1][0]-sp1[1][0]
			f1y = sp2[1][1]-sp1[1][1]
			if f1x*f1x+f1y*f1y != 0 :
				l = math.sqrt(f1x*f1x+f1y*f1y)
				return [f1x/l, f1y/l]
	elif t == 1 :
		f1x = sp2[1][0]-sp1[2][0]
		f1y = sp2[1][1]-sp1[2][1]
		if abs(f1x*f1x+f1y*f1y) > 1e-9 :
			l = math.sqrt(f1x*f1x+f1y*f1y)
			return [f1x/l, f1y/l]
		else :
			f1x = sp2[1][0]-sp1[1][0]
			f1y = sp2[1][1]-sp1[1][1]
			if f1x*f1x+f1y*f1y != 0 :
				l = math.sqrt(f1x*f1x+f1y*f1y)
				return [f1x/l, f1y/l]
	else :
		return [1.,0.]

				
def csp_normalized_normal(sp1,sp2,t) :
	nx,ny = csp_normalized_slope(sp1,sp2,t)
	return [-ny, nx]


def csp_parameterize(sp1,sp2):
	return bezmisc.bezierparameterize(csp_segment_to_bez(sp1,sp2))


def csp_concat_subpaths(*s):
	
	def concat(s1,s2) :
		if s1 == [] : return s2
		if s2 == [] : return s1
		if (s1[-1][1][0]-s2[0][1][0])**2 + (s1[-1][1][1]-s2[0][1][1])**2 > 0.00001 :
	 		return s1[:-1]+[ [s1[-1][0],s1[-1][1],s1[-1][1]],  [s2[0][1],s2[0][1],s2[0][2]] ] + s2[1:]		
	 	else :
	 		return s1[:-1]+[ [s1[-1][0],s2[0][1],s2[0][2]] ] + s2[1:]		
	 		
	if len(s) == 0 : return []
	if len(s) ==1 : return s[0]
	result = s[0]
	for s1 in s[1:]:
		result = concat(result,s1)
	return result

def csp_subpaths_end_to_start_distance2(s1,s2):
	return (s1[-1][1][0]-s2[0][1][0])**2 + (s1[-1][1][1]-s2[0][1][1])**2


def csp_clip_by_line(csp,l1,l2) :
	result = []
	for i in range(len(csp)):
		s = csp[i]
		intersections = []
		for j in range(1,len(s)) :
			intersections += [  [j,int_] for int_ in csp_line_intersection(l1,l2,s[j-1],s[j])]
		splitted_s = csp_subpath_split_by_points(s, intersections)
		for s in splitted_s[:] :
			clip = False
			for p in csp_true_bounds([s]) :
				if (l1[1]-l2[1])*p[0] + (l2[0]-l1[0])*p[1] + (l1[0]*l2[1]-l2[0]*l1[1])<-0.01 : 
					clip = True
					break
			if clip :
				splitted_s.remove(s)
		result += splitted_s
	return result


def csp_subpath_line_to(subpath, points, prepend = False) :
	# Appends subpath with line or polyline.
	if len(points)>0 :
		if not prepend : 
			if len(subpath)>0:
				subpath[-1][2] = subpath[-1][1][:]
			if type(points[0]) == type([1,1]) :
				for p in points :
					subpath += [ [p[:],p[:],p[:]] ]
			else: 
				subpath += [ [points,points,points] ]
		else : 		
			if len(subpath)>0:
				subpath[0][0] = subpath[0][1][:]
			if type(points[0]) == type([1,1]) :
				for p in points :
					subpath = [ [p[:],p[:],p[:]] ] + subpath
			else: 
				subpath = [ [points,points,points] ] + subpath
	return subpath

	
def csp_join_subpaths(csp) :
	result = csp[:]
	done_smf = True
	joined_result = []
	while done_smf :
		done_smf = False
		while len(result)>0:
			s1 = result[-1][:]
			del(result[-1])
			j = 0
			joined_smf = False
			while j<len(joined_result) :
				if csp_subpaths_end_to_start_distance2(joined_result[j],s1) <0.000001 :
					joined_result[j] = csp_concat_subpaths(joined_result[j],s1)
					done_smf = True
					joined_smf = True
					break				
				if csp_subpaths_end_to_start_distance2(s1,joined_result[j]) <0.000001 :
					joined_result[j] = csp_concat_subpaths(s1,joined_result[j])
					done_smf = True
					joined_smf = True
					break				
				j += 1
			if not joined_smf : joined_result += [s1[:]]
		if done_smf : 
			result = joined_result[:]
			joined_result = []
	return joined_result


def triangle_cross(a,b,c):
	return (a[0]-b[0])*(c[1]-b[1]) - (c[0]-b[0])*(a[1]-b[1])
	

def csp_segment_convex_hull(sp1,sp2):
	a,b,c,d = sp1[1][:], sp1[2][:], sp2[0][:], sp2[1][:]
	
	abc = triangle_cross(a,b,c)
	abd = triangle_cross(a,b,d)
	bcd = triangle_cross(b,c,d)
	cad = triangle_cross(c,a,d)
	if abc == 0 and abd == 0 : return [min(a,b,c,d), max(a,b,c,d)]
	if abc == 0 : return [d, min(a,b,c), max(a,b,c)]
	if abd == 0 : return [c, min(a,b,d), max(a,b,d)]
	if bcd == 0 : return [a, min(b,c,d), max(b,c,d)]
	if cad == 0 : return [b, min(c,a,d), max(c,a,d)]
	
	m1, m2, m3  =  abc*abd>0, abc*bcd>0, abc*cad>0
	if m1 and m2 and m3 : return [a,b,c]
	if     m1 and     m2 and not m3 : return [a,b,c,d]
	if     m1 and not m2 and     m3 : return [a,b,d,c]
	if not m1 and     m2 and     m3 : return [a,d,b,c]
	if m1 and not (m2 and m3) : return [a,b,d]
	if not (m1 and m2) and m3 : return [c,a,d]
	if not (m1 and m3) and m2 : return [b,c,d]
	
	raise ValueError, "csp_segment_convex_hull happend something that shouldnot happen!"	

	
################################################################################
###		Bezier additional functions
################################################################################

def bez_bounds_intersect(bez1, bez2) :
	return bounds_intersect(bez_bound(bez2), bez_bound(bez1))


def bez_bound(bez) :
	return [
				min(bez[0][0], bez[1][0], bez[2][0], bez[3][0]),
				min(bez[0][1], bez[1][1], bez[2][1], bez[3][1]),
				max(bez[0][0], bez[1][0], bez[2][0], bez[3][0]),
				max(bez[0][1], bez[1][1], bez[2][1], bez[3][1]),
			]


def bounds_intersect(a, b) :
	return not ( (a[0]>b[2]) or (b[0]>a[2]) or (a[1]>b[3]) or (b[1]>a[3]) )


def tpoint((x1,y1),(x2,y2),t):
	return [x1+t*(x2-x1),y1+t*(y2-y1)]


def bez_to_csp_segment(bez) :
	return [bez[0],bez[0],bez[1]], [bez[2],bez[3],bez[3]]


def bez_split(a,t=0.5) :
	 a1 = tpoint(a[0],a[1],t)
	 at = tpoint(a[1],a[2],t)
	 b2 = tpoint(a[2],a[3],t)
	 a2 = tpoint(a1,at,t)
	 b1 = tpoint(b2,at,t)
	 a3 = tpoint(a2,b1,t)
	 return [a[0],a1,a2,a3], [a3,b1,b2,a[3]]

	
def bez_at_t(bez,t) :
	return csp_at_t([bez[0],bez[0],bez[1]],[bez[2],bez[3],bez[3]],t)


def bez_to_point_distance(bez,p,needed_dist=[0.,1e100]):
	# returns [d^2,t]
	return csp_seg_to_point_distance(bez_to_csp_segment(bez),p,needed_dist)


def bez_normalized_slope(bez,t):
	return csp_normalized_slope([bez[0],bez[0],bez[1]], [bez[2],bez[3],bez[3]],t)	

################################################################################
###	Some vector functions
################################################################################
	
def normalize((x,y)) :
	l = math.sqrt(x**2+y**2)
	if l == 0 : return [0.,0.]
	else : 		return [x/l, y/l]


def cross(a,b) :
	return a[1] * b[0] - a[0] * b[1]


def dot(a,b) :
	return a[0] * b[0] + a[1] * b[1]


def rotate_ccw(d) :
	return [-d[1],d[0]]

def rotate_cw(d) :
	return [d[1],-d[0]]


def vectors_ccw(a,b):
	return a[0]*b[1]-b[0]*a[1] < 0

def vector_add(a,b) :
	return [a[0]+b[0],a[1]+b[1]]

def vector_mul(a,b) :
	return [a[0]*b,a[1]*b]


def vector_from_to_length(a,b):
	return math.sqrt((a[0]-b[0])*(a[0]-b[0]) + (a[1]-b[1])*(a[1]-b[1]))

################################################################################
###	Common functions
################################################################################

def matrix_mul(a,b) :
	return [ [ sum([a[i][k]*b[k][j] for k in range(len(a[0])) ])   for j in range(len(b[0]))]   for i in range(len(a))] 
	try :
		return [ [ sum([a[i][k]*b[k][j] for k in range(len(a[0])) ])   for j in range(len(b[0]))]   for i in range(len(a))] 
	except :
		return None


def transpose(a) :
	try :
		return [ [ a[i][j] for i in range(len(a)) ] for j in range(len(a[0])) ]
	except :
		return None


def det_3x3(a):
	return  float(
		a[0][0]*a[1][1]*a[2][2] + a[0][1]*a[1][2]*a[2][0] + a[1][0]*a[2][1]*a[0][2]
		- a[0][2]*a[1][1]*a[2][0] - a[0][0]*a[2][1]*a[1][2] - a[0][1]*a[2][2]*a[1][0]
		)


def inv_3x3(a): # invert matrix 3x3
	det = det_3x3(a)
	if det==0: return None
	return	[  
		[  (a[1][1]*a[2][2] - a[2][1]*a[1][2])/det,  -(a[0][1]*a[2][2] - a[2][1]*a[0][2])/det,  (a[0][1]*a[1][2] - a[1][1]*a[0][2])/det ], 
		[ -(a[1][0]*a[2][2] - a[2][0]*a[1][2])/det,   (a[0][0]*a[2][2] - a[2][0]*a[0][2])/det, -(a[0][0]*a[1][2] - a[1][0]*a[0][2])/det ], 
		[  (a[1][0]*a[2][1] - a[2][0]*a[1][1])/det,  -(a[0][0]*a[2][1] - a[2][0]*a[0][1])/det,  (a[0][0]*a[1][1] - a[1][0]*a[0][1])/det ]
	]


def inv_2x2(a): # invert matrix 2x2
	det = a[0][0]*a[1][1] - a[1][0]*a[0][1]
	if det==0: return None
	return [
			[a[1][1]/det, -a[0][1]/det],
			[-a[1][0]/det, a[0][0]/det]
			]


def small(a) :
	global small_tolerance
	return abs(a)<small_tolerance

					
def atan2(*arg):	
	if len(arg)==1 and ( type(arg[0]) == type([0.,0.]) or type(arg[0])==type((0.,0.)) ) :
		return (math.pi/2 - math.atan2(arg[0][0], arg[0][1]) ) % math.pi2
	elif len(arg)==2 :
		
		return (math.pi/2 - math.atan2(arg[0],arg[1]) ) % math.pi2
	else :
		raise ValueError, "Bad argumets for atan! (%s)" % arg  

def get_text(node) :
	value = None
	if node.text!=None : value = value +"\n" + node.text if value != None else node.text
	for k in node :
		if k.tag == inkex.addNS('tspan','svg'):
			if k.text!=None : value = value +"\n" + k.text if value != None else k.text
	return value



def draw_text(text,x,y, group = None, style = None, font_size = 10, gcodetools_tag = None) :
	if style == None : 
		style = "font-family:DejaVu Sans;font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-family:DejaVu Sans;fill:#000000;fill-opacity:1;stroke:none;"
	style += "font-size:%fpx;"%font_size
	attributes = {			'x':	str(x),
							inkex.addNS("space","xml"):"preserve",
							'y':	str(y),
							'style' : style
						}
	if gcodetools_tag!=None : 
		attributes["gcodetools"] = str(gcodetools_tag)
 
	if group == None:
		group = options.doc_root

	t = inkex.etree.SubElement(	group, inkex.addNS('text','svg'), attributes)
	text = str(text).split("\n")
	for s in text :
		span = inkex.etree.SubElement( t, inkex.addNS('tspan','svg'), 
						{
							'x':	str(x),
							'y':	str(y),
							inkex.addNS("role","sodipodi"):"line",
						})					
		y += font_size
		span.text = str(s)

def draw_csp(csp, stroke = "#f00", fill = "none", comment = "", width = 0.354, group = None, style = None, gcodetools_tag = None) :
	if style == None : 
		style = "fill:%s;fill-opacity:1;stroke:%s;stroke-width:%s"%(fill,stroke,width)
	attributes = {			'd':	cubicsuperpath.formatPath(csp),
							'style' : style
				}
	if comment != '':
		attributes['comment'] = comment
	if group == None :
		group = options.doc_root
		
	return inkex.etree.SubElement( group, inkex.addNS('path','svg'), attributes) 
	
def draw_pointer(x,color = "#f00", figure = "cross", group = None, comment = "", fill=None, width = .1, size = 10., text = None, font_size=None, pointer_type=None, attrib = None) :
	size = size/2
	if attrib == None : attrib = {}
	if pointer_type == None: 
		pointer_type = "Pointer"
	attrib["gcodetools"] = pointer_type
	if group == None:
		group = options.self.current_layer
	if text != None	:
		if font_size == None : font_size = 7
		group = inkex.etree.SubElement( group, inkex.addNS('g','svg'), {"gcodetools": pointer_type+" group"} )		
		draw_text(text,x[0]+size*2.2,x[1]-size, group = group, font_size = font_size) 
	if figure  == "line" :
		s = ""
		for i in range(1,len(x)/2) :
			s+= " %s, %s " %(x[i*2],x[i*2+1])
		attrib.update({"d": "M %s,%s L %s"%(x[0],x[1],s), "style":"fill:none;stroke:%s;stroke-width:%f;"%(color,width),"comment":str(comment)})			
		inkex.etree.SubElement( group, inkex.addNS('path','svg'), attrib)
	elif figure == "arrow" :
		if fill == None : fill = "#12b3ff"
		fill_opacity = "0.8" 
		d = "m %s,%s " % (x[0],x[1]) + re.sub("([0-9\-.e]+)",(lambda match: str(float(match.group(1))*size*2.)), "0.88464,-0.40404 c -0.0987,-0.0162 -0.186549,-0.0589 -0.26147,-0.1173 l 0.357342,-0.35625 c 0.04631,-0.039 0.0031,-0.13174 -0.05665,-0.12164 -0.0029,-1.4e-4 -0.0058,-1.4e-4 -0.0087,0 l -2.2e-5,2e-5 c -0.01189,0.004 -0.02257,0.0119 -0.0305,0.0217 l -0.357342,0.35625 c -0.05818,-0.0743 -0.102813,-0.16338 -0.117662,-0.26067 l -0.409636,0.88193 z")
		attrib.update({"d": d, "style":"fill:%s;stroke:none;fill-opacity:%s;"%(fill,fill_opacity),"comment":str(comment)})			
		inkex.etree.SubElement( group, inkex.addNS('path','svg'), attrib)
	else :
		attrib.update({"d": "m %s,%s l %f,%f %f,%f %f,%f %f,%f , %f,%f"%(x[0],x[1], size,size, -2*size,-2*size, size,size, size,-size, -2*size,2*size ), "style":"fill:none;stroke:%s;stroke-width:%f;"%(color,width),"comment":str(comment)})
		inkex.etree.SubElement( group, inkex.addNS('path','svg'), attrib)


def straight_segments_intersection(a,b, true_intersection = True) : # (True intersection means check ta and tb are in [0,1])
 	ax,bx,cx,dx, ay,by,cy,dy = a[0][0],a[1][0],b[0][0],b[1][0], a[0][1],a[1][1],b[0][1],b[1][1] 
	if (ax==bx and ay==by) or (cx==dx and cy==dy) : return False, 0, 0
	if (bx-ax)*(dy-cy)-(by-ay)*(dx-cx)==0 :	# Lines are parallel
		ta = (ax-cx)/(dx-cx) if cx!=dx else (ay-cy)/(dy-cy)
		tb = (bx-cx)/(dx-cx) if cx!=dx else (by-cy)/(dy-cy)
		tc = (cx-ax)/(bx-ax) if ax!=bx else (cy-ay)/(by-ay)
		td = (dx-ax)/(bx-ax) if ax!=bx else (dy-ay)/(by-ay)
		return ("Overlap" if 0<=ta<=1 or 0<=tb<=1 or  0<=tc<=1 or  0<=td<=1 or not true_intersection else False), (ta,tb), (tc,td)
	else :
		ta = ( (ay-cy)*(dx-cx)-(ax-cx)*(dy-cy) ) / ( (bx-ax)*(dy-cy)-(by-ay)*(dx-cx) )
		tb = ( ax-cx+ta*(bx-ax) ) / (dx-cx) if dx!=cx else ( ay-cy+ta*(by-ay) ) / (dy-cy)
		return (0<=ta<=1 and 0<=tb<=1 or not true_intersection), ta, tb
	
	

def isnan(x): return type(x) is float and x != x

def isinf(x): inf = 1e5000; return x == inf or x == -inf

def between(c,x,y):
		return x-straight_tolerance<=c<=y+straight_tolerance or y-straight_tolerance<=c<=x+straight_tolerance

def cubic_solver_real(a,b,c,d):
	# returns only real roots of a cubic equation.
	roots = cubic_solver(a,b,c,d)
	res = []
	for root in roots :
		if type(root) is complex :	
			if -1e-10<root.imag<1e-10 :
				res.append(root.real)
		else :
			res.append(root)
	return res 
	
	
def cubic_solver(a,b,c,d):	
	if a!=0:
		#	Monics formula see http://en.wikipedia.org/wiki/Cubic_function#Monic_formula_of_roots
		a,b,c = (b/a, c/a, d/a)
		m = 2*a**3 - 9*a*b + 27*c
		k = a**2 - 3*b
		n = m**2 - 4*k**3
		w1 = -.5 + .5*cmath.sqrt(3)*1j
		w2 = -.5 - .5*cmath.sqrt(3)*1j
		if n>=0 :
			t = m+math.sqrt(n)
			m1 = pow(t/2,1./3) if t>=0 else -pow(-t/2,1./3)
			t = m-math.sqrt(n)
			n1 = pow(t/2,1./3) if t>=0 else -pow(-t/2,1./3)
		else :
			m1 = pow(complex((m+cmath.sqrt(n))/2),1./3)
			n1 = pow(complex((m-cmath.sqrt(n))/2),1./3)
		x1 = -1./3 * (a + m1 + n1)
		x2 = -1./3 * (a + w1*m1 + w2*n1)
		x3 = -1./3 * (a + w2*m1 + w1*n1)
		return [x1,x2,x3]
	elif b!=0:
		det = c**2-4*b*d
		if det>0 :
			return [(-c+math.sqrt(det))/(2*b),(-c-math.sqrt(det))/(2*b)]
		elif d == 0 :
			return [-c/(b*b)] 	
		else :
			return [(-c+cmath.sqrt(det))/(2*b),(-c-cmath.sqrt(det))/(2*b)]
	elif c!=0 :
		return [-d/c]
	else : return []


################################################################################
###		print_ prints any arguments into specified log file
################################################################################

def print_(*arg):
	f = open(options.log_filename,"a")
	for s in arg :
		s = str(unicode(s).encode('unicode_escape'))+" "
		f.write( s )
	f.write("\n")
	f.close()


################################################################################
###		Point (x,y) operations
################################################################################
class P:
	def __init__(self, x, y=None):
		if not y==None:
			self.x, self.y = float(x), float(y)
		else:
			self.x, self.y = float(x[0]), float(x[1])
	def __add__(self, other): return P(self.x + other.x, self.y + other.y)
	def __sub__(self, other): return P(self.x - other.x, self.y - other.y)
	def __neg__(self): return P(-self.x, -self.y)
	def __mul__(self, other):
		if isinstance(other, P):
			return self.x * other.x + self.y * other.y
		return P(self.x * other, self.y * other)
	__rmul__ = __mul__
	def __div__(self, other): return P(self.x / other, self.y / other)
	def mag(self): return math.hypot(self.x, self.y)
	def unit(self):
		h = self.mag()
		if h: return self / h
		else: return P(0,0)
	def dot(self, other): return self.x * other.x + self.y * other.y
	def rot(self, theta):
		c = math.cos(theta)
		s = math.sin(theta)
		return P(self.x * c - self.y * s,  self.x * s + self.y * c)
	def angle(self): return math.atan2(self.y, self.x)
	def __repr__(self): return '%f,%f' % (self.x, self.y)
	def pr(self): return "%.2f,%.2f" % (self.x, self.y)
	def to_list(self): return [self.x, self.y]	
	def ccw(self): return P(-self.y,self.x)
	def l2(self): return self.x*self.x + self.y*self.y


class Arc():
	def __init__(self,st,end,c,a):
		self.st = P(st)
		self.end = P(end)
		self.c = P(c)
		self.r = (P(st)-P(c)).mag()
		self.a = ( (self.st-self.c).angle() - (self.end-self.c).angle() ) % math.pi2
		if a<0 : self.a -= math.pi2

	def offset(self, r):
		if self.a>0 :
			r += self.r
		else :
			r = self.r - r
		
		if self.r != 0 :
			self.st = self.c + (self.st-self.c)*r/self.r
			self.end = self.c + (self.end-self.c)*r/self.r
			self.r = r
	
	def length(self):
		return abs(self.a*self.r)
	

	def draw(self, group, style, layer, transform, num = 0, reverse_angle = 1):
		st = P(gcodetools.transform(self.st.to_list(), layer, True))
		c = P(gcodetools.transform(self.c.to_list(), layer, True))
		a = self.a * reverse_angle
		r = (st-c)
		a_st = (math.atan2(r.x,-r.y) - math.pi/2) % (math.pi*2)
		r = r.mag()
		if a<0:
			a_end = a_st+a
			style = style['biarc%s'%(num%2)]
		else: 
			a_end = a_st
			a_st = a_st+a
			style = style['biarc%s_r'%(num%2)]
		
		attr = {
				'style': style,
				 inkex.addNS('cx','sodipodi'):		str(c.x),
				 inkex.addNS('cy','sodipodi'):		str(c.y),
				 inkex.addNS('rx','sodipodi'):		str(r),
				 inkex.addNS('ry','sodipodi'):		str(r),
				 inkex.addNS('start','sodipodi'):	str(a_st),
				 inkex.addNS('end','sodipodi'):		str(a_end),
				 inkex.addNS('open','sodipodi'):	'true',
				 inkex.addNS('type','sodipodi'):	'arc',
				 "gcodetools": "Preview",
				}	
		if transform != [] :
			attr["transform"] = transform	
		inkex.etree.SubElement(	group, inkex.addNS('path','svg'), attr)
		
	def intersect(self,b) :
		return []


class Line():
	def __init__(self,st,end):
		if st.__class__ == P :
			st = st.to_list()
		if end.__class__ == P :
			end = end.to_list()
		self.st = P(st)
		self.end = P(end)
		self.l = self.length() 
		if self.l != 0 :
			self.n = ((self.end-self.st)/self.l).ccw()
		else: 
			self.n = [0,1]
				
	def offset(self, r):
		self.st -= self.n*r
		self.end -= self.n*r
		
	def l2(self): return (self.st-self.end).l2()
	def length(self): return (self.st-self.end).mag()
	
	def draw(self, group, style, layer, transform, num = 0, reverse_angle = 1):
		st = gcodetools.transform(self.st.to_list(), layer, True)
		end = gcodetools.transform(self.end.to_list(), layer, True)


		attr = {	'style': style['line'],
					'd':'M %s,%s L %s,%s' % (st[0],st[1],end[0],end[1]),
					"gcodetools": "Preview",
				}
		if transform != [] :
			attr["transform"] = transform		
		inkex.etree.SubElement(	group, inkex.addNS('path','svg'),  attr	)
	
	def intersect(self,b) :
		if b.__class__ == Line :
			if self.l < 10e-8 or b.l < 10e-8 : return []
			v1 = self.end - self.st
			v2 = b.end - b.st
			x = v1.x*v2.y - v2.x*v1.y 
			if x == 0 :
				# lines are parallel
				res = []

				if (self.st.x-b.st.x)*v1.y - (self.st.y-b.st.y)*v1.x  == 0:
					# lines are the same
					if v1.x != 0 :
						if 0<=(self.st.x-b.st.x)/v2.x<=1 :  res.append(self.st)
						if 0<=(self.end.x-b.st.x)/v2.x<=1 :  res.append(self.end)
						if 0<=(b.st.x-self.st.x)/v1.x<=1 :  res.append(b.st)
						if 0<=(b.end.x-b.st.x)/v1.x<=1 :  res.append(b.end)
					else :
						if 0<=(self.st.y-b.st.y)/v2.y<=1 :  res.append(self.st)
						if 0<=(self.end.y-b.st.y)/v2.y<=1 :  res.append(self.end)
						if 0<=(b.st.y-self.st.y)/v1.y<=1 :  res.append(b.st)
						if 0<=(b.end.y-b.st.y)/v1.y<=1 :  res.append(b.end)
				return res
			else :
				t1 = ( -v1.x*(b.end.y-self.end.y) + v1.y*(b.end.x-self.end.x) ) / x
				t2 = ( -v1.y*(self.st.x-b.st.x) + v1.x*(self.st.y-b.st.y) ) / x

				gcodetools.error((x,t1,t2), "warning")
				if 0<=t1<=1 and 0<=t2<=1 : return [ self.st+v1*t1 ]
				else : return []					
		else: return []	
			
			
	
	
class Biarc:
	def __init__(self, items=None):
		if items == None :
			self.items = []
		else: 	
			self.items = items
			
	def l(self) : 
		return sum([i.length() for i in items])
	
	def close(self) :
		for subitems in self.items:
			if (subitems[0].st-subitems[-1].end).l2()>10e-16 :
				subitems.append(Line(subitems[-1].end,subitems[0].st))
	
	def offset(self,r) :
		# offset each element
		self.close()
		for subitems in self.items :
			for item in subitems :
				item.offset(r)
		self.connect(r)
		
	def connect(self, r) :				
		for subitems in self.items :
			for a,b in zip(subitems, subitems[1:]) :
				i = a.intersect(b)
				for p in i : 
					draw_pointer(p.to_list())
				
						
				
						
	def clip_offset(self):
		pass	
		
	def draw(self, layer, group=None, style=styles["biarc_style"]):
		global gcodetools
		gcodetools.set_markers()

		for i in [0,1]:
			style['biarc%s_r'%i] = simplestyle.parseStyle(style['biarc%s'%i])
			style['biarc%s_r'%i]["marker-start"] = "url(#DrawCurveMarker_r)"
			del(style['biarc%s_r'%i]["marker-end"])
			style['biarc%s_r'%i] = simplestyle.formatStyle(style['biarc%s_r'%i])
		
		if group==None:
			if "preview_groups" not in dir(options.self) :
				gcodetools.preview_groups = { layer: inkex.etree.SubElement( gcodetools.layers[min(1,len(gcodetools.layers)-1)], inkex.addNS('g','svg'), {"gcodetools": "Preview group"} ) }
			elif layer not in gcodetools.preview_groups :
				gcodetools.preview_groups[layer] = inkex.etree.SubElement( gcodetools.layers[min(1,len(gcodetools.layers)-1)], inkex.addNS('g','svg'), {"gcodetools": "Preview group"} )
			group = gcodetools.preview_groups[layer]
		
		transform = gcodetools.get_transforms(group)
		if transform != [] : 
			transform = gcodetools.reverse_transform(transform)
			transform = simpletransform.formatTransform(transform)

		a,b,c = [0.,0.], [1.,0.], [0.,1.]
		k = (b[0]-a[0])*(c[1]-a[1])-(c[0]-a[0])*(b[1]-a[1])
		a,b,c = gcodetools.transform(a, layer, True), gcodetools.transform(b, layer, True), gcodetools.transform(c, layer, True)
		if ((b[0]-a[0])*(c[1]-a[1])-(c[0]-a[0])*(b[1]-a[1]))*k > 0 : reverse_angle = -1
		else : reverse_angle = 1 


		num = 0
		for subitems in self.items :
			for item in subitems :
				num += 1
				#if num>1 : break
				item.draw(group, style, layer, transform, num, reverse_angle)

	def from_old_style(self, curve) :
		#Crve defenitnion [start point, type = {'arc','line','move','end'}, arc center, arc angle, end point, [zstart, zend]]		
		self.items = []
		for sp in curve:
			print_(sp)
			if sp[1] == 'move':				
				self.items.append([])
			if sp[1] == 'arc':
				self.items[-1].append(Arc(sp[0],sp[4],sp[2],sp[3]))
			if sp[1] == 'line':
				self.items[-1].append(Line(sp[0],sp[4]))
		
			
		
			
	
################################################################################
###
### Offset function 
###
### This function offsets given cubic super path.  
### It's based on src/livarot/PathOutline.cpp from Inkscape's source code.
###
###
################################################################################
def csp_offset(csp, r) :
	offset_tolerance = 0.05
	offset_subdivision_depth = 10
	time_ = time.time()
	time_start  = time_
	print_("Offset start at %s"% time_)
	print_("Offset radius %s"% r)
	
	
	def csp_offset_segment(sp1,sp2,r) :
		result = []
		t = csp_get_t_at_curvature(sp1,sp2,1/r)
		if len(t) == 0 : t =[0.,1.]
		t.sort()
		if t[0]>.00000001 : t = [0.]+t 
		if t[-1]<.99999999 : t.append(1.) 
		for st,end in zip(t,t[1:]) :	
			c = csp_curvature_at_t(sp1,sp2,(st+end)/2)
			sp = csp_split_by_two_points(sp1,sp2,st,end)
			if sp[1]!=sp[2]:
				if (c>1/r and r<0 or c<1/r and r>0) :
					offset = offset_segment_recursion(sp[1],sp[2],r, offset_subdivision_depth, offset_tolerance) 
				else : # This part will be clipped for sure... TODO Optimize it...
					offset = offset_segment_recursion(sp[1],sp[2],r, offset_subdivision_depth, offset_tolerance) 
					
				if result==[] :
					result = offset[:]
				else: 
					if csp_subpaths_end_to_start_distance2(result,offset)<0.0001 :
						result = csp_concat_subpaths(result,offset)
					else:
						
						intersection = csp_get_subapths_last_first_intersection(result,offset)
						if intersection != [] :
							i,t1,j,t2 = intersection
							sp1_,sp2_,sp3_ = csp_split(result[i-1],result[i],t1)
							result = result[:i-1] + [ sp1_, sp2_ ]
							sp1_,sp2_,sp3_ = csp_split(offset[j-1],offset[j],t2)
							result = csp_concat_subpaths( result, [sp2_,sp3_] + offset[j+1:] )
						else : 
							pass # ???						
							#raise ValueError, "Offset curvature clipping error"
		#draw_csp([result])							
		return result					
	
						
	def create_offset_segment(sp1,sp2,r) :
		# See	Gernot Hoffmann "Bezier Curves"  p.34 -> 7.1 Bezier Offset Curves
		p0,p1,p2,p3 = P(sp1[1]),P(sp1[2]),P(sp2[0]),P(sp2[1])
		s0,s1,s3 = p1-p0,p2-p1,p3-p2
		n0 = s0.ccw().unit() if s0.l2()!=0 else P(csp_normalized_normal(sp1,sp2,0))
		n3 = s3.ccw().unit() if s3.l2()!=0 else P(csp_normalized_normal(sp1,sp2,1))
		n1 = s1.ccw().unit() if s1.l2()!=0 else (n0.unit()+n3.unit()).unit()

		q0,q3 = p0+r*n0, p3+r*n3
		c = csp_curvature_at_t(sp1,sp2,0)
		q1 = q0 + (p1-p0)*(1- (r*c if abs(c)<100 else 0) )
		c = csp_curvature_at_t(sp1,sp2,1)
		q2 = q3 + (p2-p3)*(1- (r*c if abs(c)<100 else 0) ) 

		
		return [[q0.to_list(), q0.to_list(), q1.to_list()],[q2.to_list(), q3.to_list(), q3.to_list()]]
	
	
	def csp_get_subapths_last_first_intersection(s1,s2):
		_break = False
		for i in range(1,len(s1)) :
			sp11, sp12 = s1[-i-1], s1[-i]
			for j in range(1,len(s2)) :
				sp21,sp22 = s2[j-1], s2[j] 
				intersection = csp_segments_true_intersection(sp11,sp12,sp21,sp22)
				if intersection != [] :
					_break = True
					break 
			if _break:break
		if _break :
			intersection = max(intersection)
			return [len(s1)-i,intersection[0], j,intersection[1]]
		else : 
			return []
	
		
	def csp_join_offsets(prev,next,sp1,sp2,sp1_l,sp2_l,r):
		if len(next)>1 :
			if (P(prev[-1][1])-P(next[0][1])).l2()<0.001 : 
				return prev,[],next
			intersection = csp_get_subapths_last_first_intersection(prev,next)
			if intersection != [] :
				i,t1,j,t2 = intersection
				sp1_,sp2_,sp3_ = csp_split(prev[i-1],prev[i],t1)
				sp3_,sp4_,sp5_ = csp_split(next[j-1], next[j],t2)
				return prev[:i-1] + [ sp1_, sp2_ ], [], [sp4_,sp5_] + next[j+1:] 
			
		# Offsets do not intersect... will add an arc...
		start = (P(csp_at_t(sp1_l,sp2_l,1.)) + r*P(csp_normalized_normal(sp1_l,sp2_l,1.))).to_list()
		end   = (P(csp_at_t(sp1,sp2,0.)) + r*P(csp_normalized_normal(sp1,sp2,0.))).to_list()
		arc = csp_from_arc(start, end, sp1[1], r, csp_normalized_slope(sp1_l,sp2_l,1.) )
		if arc == [] :
			return prev,[],next
		else:
			# Clip prev by arc
			if csp_subpaths_end_to_start_distance2(prev,arc)>0.00001 :
				intersection = csp_get_subapths_last_first_intersection(prev,arc)
				if intersection != [] :
					i,t1,j,t2 = intersection
					sp1_,sp2_,sp3_ = csp_split(prev[i-1],prev[i],t1)
					sp3_,sp4_,sp5_ = csp_split(arc[j-1],arc[j],t2)
					prev = prev[:i-1] + [ sp1_, sp2_ ]
					arc = [sp4_,sp5_] + arc[j+1:] 
				#else : raise ValueError, "Offset curvature clipping error"
			# Clip next by arc
			if next == [] :
				return prev,[],arc
			if csp_subpaths_end_to_start_distance2(arc,next)>0.00001 :
				intersection = csp_get_subapths_last_first_intersection(arc,next)
				if intersection != [] :
					i,t1,j,t2 = intersection
					sp1_,sp2_,sp3_ = csp_split(arc[i-1],arc[i],t1)
					sp3_,sp4_,sp5_ = csp_split(next[j-1],next[j],t2)
					arc = arc[:i-1] + [ sp1_, sp2_ ]
					next = [sp4_,sp5_] + next[j+1:] 
				#else : raise ValueError, "Offset curvature clipping error"

			return prev,arc,next
		
		
	def offset_segment_recursion(sp1,sp2,r, depth, tolerance) :
		sp1_r,sp2_r = create_offset_segment(sp1,sp2,r)
		err = max(
				csp_seg_to_point_distance(sp1_r,sp2_r, (P(csp_at_t(sp1,sp2,.25)) + P(csp_normalized_normal(sp1,sp2,.25))*r).to_list())[0], 
				csp_seg_to_point_distance(sp1_r,sp2_r, (P(csp_at_t(sp1,sp2,.50)) + P(csp_normalized_normal(sp1,sp2,.50))*r).to_list())[0],
				csp_seg_to_point_distance(sp1_r,sp2_r, (P(csp_at_t(sp1,sp2,.75)) + P(csp_normalized_normal(sp1,sp2,.75))*r).to_list())[0],
				)

		if  err>tolerance**2 and depth>0:
			#print_(csp_seg_to_point_distance(sp1_r,sp2_r, (P(csp_at_t(sp1,sp2,.25)) + P(csp_normalized_normal(sp1,sp2,.25))*r).to_list())[0], tolerance)
			if depth > offset_subdivision_depth-2 :
				t = csp_max_curvature(sp1,sp2)
				t = max(.1,min(.9 ,t))
			else :
				t = .5
			sp3,sp4,sp5 = csp_split(sp1,sp2,t)
			r1 = offset_segment_recursion(sp3,sp4,r, depth-1, tolerance)
			r2 = offset_segment_recursion(sp4,sp5,r, depth-1, tolerance)
			return r1[:-1]+ [[r1[-1][0],r1[-1][1],r2[0][2]]] + r2[1:]
		else :
			#draw_csp([[sp1_r,sp2_r]])
			#draw_pointer(sp1[1]+sp1_r[1], "#057", "line")
			#draw_pointer(sp2[1]+sp2_r[1], "#705", "line")
			return [sp1_r,sp2_r]
		

	############################################################################
	# Some small definitions
	############################################################################
	csp_len = len(csp)

	############################################################################
	# Prepare the path
	############################################################################
	# Remove all small segments (segment length < 0.001)

	for i in xrange(len(csp)) :
		for j in xrange(len(csp[i])) :
			sp = csp[i][j]
			if (P(sp[1])-P(sp[0])).mag() < 0.001 :
				csp[i][j][0] = sp[1]
			if (P(sp[2])-P(sp[0])).mag() < 0.001 :
				csp[i][j][2] = sp[1]
	for i in xrange(len(csp)) :
		for j in xrange(1,len(csp[i])) :
			if cspseglength(csp[i][j-1], csp[i][j])<0.001 : 
				csp[i] = csp[i][:j] + csp[i][j+1:]
		if cspseglength(csp[i][-1],csp[i][0])>0.001 : 
			csp[i][-1][2] = csp[i][-1][1]
			csp[i]+= [ [csp[i][0][1],csp[i][0][1],csp[i][0][1]] ]
	
	# TODO Get rid of self intersections.

	original_csp = csp[:]
	# Clip segments which has curvature>1/r. Because their offset will be selfintersecting and very nasty.
					
	print_("Offset prepared the path in %s"%(time.time()-time_))
	print_("Path length = %s"% sum([len(i)for i in csp] ) )
	time_ = time.time()

	############################################################################
	# Offset
	############################################################################
	# Create offsets for all segments in the path. And join them together inside each subpath. 		
	unclipped_offset = [[] for i in xrange(csp_len)]
	offsets_original = [[] for i in xrange(csp_len)]
	join_points = [[] for i in xrange(csp_len)]
	intersection = [[] for i in xrange(csp_len)]
	for i in xrange(csp_len) :
		subpath = csp[i]
		subpath_offset = []
		last_offset_len = 0
		for sp1,sp2 in zip(subpath, subpath[1:]) : 
			segment_offset = csp_offset_segment(sp1,sp2,r)
			if subpath_offset == [] :
				subpath_offset = segment_offset
				
				prev_l = len(subpath_offset)
			else : 
				prev, arc, next = csp_join_offsets(subpath_offset[-prev_l:],segment_offset,sp1,sp2,sp1_l,sp2_l,r)
				#draw_csp([prev],"Blue")
				#draw_csp([arc],"Magenta")
				subpath_offset = csp_concat_subpaths(subpath_offset[:-prev_l+1],prev,arc,next)
				prev_l = len(next)				
			sp1_l, sp2_l = sp1[:], sp2[:]
						
		# Join last and first offsets togother to close the curve
		
		prev, arc, next = csp_join_offsets(subpath_offset[-prev_l:], subpath_offset[:2], subpath[0], subpath[1], sp1_l,sp2_l, r)
		subpath_offset[:2] = next[:]
		subpath_offset = csp_concat_subpaths(subpath_offset[:-prev_l+1],prev,arc)
		#draw_csp([prev],"Blue")
		#draw_csp([arc],"Red")
		#draw_csp([next],"Red")

		# Collect subpath's offset and save it to unclipped offset list. 	
		unclipped_offset[i] = subpath_offset[:]	

		#for k,t in intersection[i]:
		#	draw_pointer(csp_at_t(subpath_offset[k-1], subpath_offset[k], t))
			
	#inkex.etree.SubElement( options.doc_root, inkex.addNS('path','svg'), {"d": cubicsuperpath.formatPath(unclipped_offset), "style":"fill:none;stroke:#0f0;"} )	
	print_("Offsetted path in %s"%(time.time()-time_))
	time_ = time.time()
	
	#for i in range(len(unclipped_offset)):
	#	draw_csp([unclipped_offset[i]], color = ["Green","Red","Blue"][i%3], width = .1)
	#return []
	############################################################################
	# Now to the clipping. 
	############################################################################
	# First of all find all intersection's between all segments of all offseted subpaths, including self intersections.

	#TODO define offset tolerance here
	global small_tolerance
	small_tolerance = 0.01
	summ = 0
	summ1 = 0 	 
	for subpath_i in xrange(csp_len) :
		for subpath_j in xrange(subpath_i,csp_len) :
			subpath = unclipped_offset[subpath_i]
			subpath1 = unclipped_offset[subpath_j]
			for i in xrange(1,len(subpath)) :
				# If subpath_i==subpath_j we are looking for self intersections, so 
				# we'll need search intersections only for xrange(i,len(subpath1))
				for j in ( xrange(i,len(subpath1)) if subpath_i==subpath_j else xrange(len(subpath1))) :
					if subpath_i==subpath_j and j==i :
						# Find self intersections of a segment
						sp1,sp2,sp3 = csp_split(subpath[i-1],subpath[i],.5)
						intersections = csp_segments_intersection(sp1,sp2,sp2,sp3)
						summ +=1 
						for t in intersections :
							summ1 += 1
							if not ( small(t[0]-1) and small(t[1]) ) and 0<=t[0]<=1 and 0<=t[1]<=1 :
								intersection[subpath_i] += [ [i,t[0]/2],[j,t[1]/2+.5] ]
					else :
						intersections = csp_segments_intersection(subpath[i-1],subpath[i],subpath1[j-1],subpath1[j])
						summ +=1 
						for t in intersections :
							summ1 += 1
							#TODO tolerance dependence to cpsp_length(t)
							if len(t) == 2 and 0<=t[0]<=1 and 0<=t[1]<=1 and not (
									subpath_i==subpath_j and (
									(j-i-1) % (len(subpath)-1) == 0 and small(t[0]-1) and small(t[1]) or 
									(i-j-1) % (len(subpath)-1) == 0 and small(t[1]-1) and small(t[0]) )  ) :
								intersection[subpath_i] += [ [i,t[0]] ]
								intersection[subpath_j] += [ [j,t[1]] ]	
								#draw_pointer(csp_at_t(subpath[i-1],subpath[i],t[0]),"#f00")
								#print_(t)
								#print_(i,j)
							elif len(t)==5 and t[4]=="Overlap":	
								intersection[subpath_i] += [ [i,t[0]], [i,t[1]] ]
								intersection[subpath_j] += [ [j,t[1]], [j,t[3]] ]

	print_("Intersections found in %s"%(time.time()-time_))
	print_("Examined %s segments"%(summ))
	print_("found %s intersections"%(summ1))
	time_ = time.time()
								
	########################################################################
	# Split unclipped offset by intersection points into splitted_offset
	########################################################################
	splitted_offset = []
	for i in xrange(csp_len) :
		subpath = unclipped_offset[i]
		if len(intersection[i]) > 0 :
			parts = csp_subpath_split_by_points(subpath, intersection[i])
			# Close	parts list to close path (The first and the last parts are joined together)		
			if  [1,0.] not in intersection[i] : 
				parts[0][0][0] = parts[-1][-1][0]
				parts[0] = csp_concat_subpaths(parts[-1], parts[0])
				splitted_offset += parts[:-1]
			else: 
				splitted_offset += parts[:]
		else :
			splitted_offset += [subpath[:]]
	
	#for i in range(len(splitted_offset)):
	#	draw_csp([splitted_offset[i]], color = ["Green","Red","Blue"][i%3])
	print_("Splitted in %s"%(time.time()-time_))
	time_ = time.time()

	
	########################################################################
	# Clipping
	########################################################################		
	result = []
	for subpath_i in range(len(splitted_offset)):
		clip = False
		s1 = splitted_offset[subpath_i]
		for subpath_j in range(len(splitted_offset)):
			s2 = splitted_offset[subpath_j]
			if (P(s1[0][1])-P(s2[-1][1])).l2()<0.0001 and ( (subpath_i+1) % len(splitted_offset) != subpath_j ): 
				if dot(csp_normalized_normal(s2[-2],s2[-1],1.),csp_normalized_slope(s1[0],s1[1],0.))*r<-0.0001 :
					clip = True
					break
			if (P(s2[0][1])-P(s1[-1][1])).l2()<0.0001 and ( (subpath_j+1) % len(splitted_offset) != subpath_i ):
				if dot(csp_normalized_normal(s2[0],s2[1],0.),csp_normalized_slope(s1[-2],s1[-1],1.))*r>0.0001 :
					clip = True
					break
			
		if not clip :
			result += [s1[:]]
		elif options.offset_draw_clippend_path :
			draw_csp([s1],color="Red",width=.1)
			draw_pointer( csp_at_t(s2[-2],s2[-1],1.)+
				(P(csp_at_t(s2[-2],s2[-1],1.))+ P(csp_normalized_normal(s2[-2],s2[-1],1.))*10).to_list(),"Green", "line"  )
			draw_pointer( csp_at_t(s1[0],s1[1],0.)+
				(P(csp_at_t(s1[0],s1[1],0.))+ P(csp_normalized_slope(s1[0],s1[1],0.))*10).to_list(),"Red", "line"  )
				
	# Now join all together and check closure and orientation of result
	joined_result = csp_join_subpaths(result)
	# Check if each subpath from joined_result is closed
	#draw_csp(joined_result,color="Green",width=1)

	
	for s in joined_result[:] :
		if csp_subpaths_end_to_start_distance2(s,s) > 0.001 :
			# Remove open parts
			if options.offset_draw_clippend_path:
				draw_csp([s],color="Orange",width=1)
				draw_pointer(s[0][1], comment= csp_subpaths_end_to_start_distance2(s,s))
				draw_pointer(s[-1][1], comment = csp_subpaths_end_to_start_distance2(s,s))
			joined_result.remove(s)
		else : 			
			# Remove small parts
			minx,miny,maxx,maxy = csp_true_bounds([s])
			if (minx[0]-maxx[0])**2 + (miny[1]-maxy[1])**2 < 0.1 :
				joined_result.remove(s)
	print_("Clipped and joined path in %s"%(time.time()-time_))
	time_ = time.time()
			
	########################################################################
	# Now to the Dummy cliping: remove parts from splitted offset if their 
	# centers are  closer to the original path than offset radius. 
	########################################################################		
	
	r1,r2 = ( (0.99*r)**2, (1.01*r)**2 ) if abs(r*.01)<1 else  ((abs(r)-1)**2, (abs(r)+1)**2)
	for s in joined_result[:]:
		dist = csp_to_point_distance(original_csp, s[int(len(s)/2)][1], dist_bounds = [r1,r2], tolerance = .000001)
		if not r1 < dist[0] < r2 : 
			joined_result.remove(s)
			if options.offset_draw_clippend_path:
				draw_csp([s], comment = math.sqrt(dist[0]))
				draw_pointer(csp_at_t(csp[dist[1]][dist[2]-1],csp[dist[1]][dist[2]],dist[3])+s[int(len(s)/2)][1],"blue", "line", comment = [math.sqrt(dist[0]),i,j,sp]  )

	print_("-----------------------------")
	print_("Total offset time %s"%(time.time()-time_start))
	print_()
	return joined_result
	
	
		


################################################################################
###
###		Biarc function
###
###		Calculates biarc approximation of cubic super path segment
###		splits segment if needed or approximates it with straight line
###
################################################################################
def biarc(sp1, sp2, z1, z2, depth=0):
	def biarc_split(sp1,sp2, z1, z2, depth): 
		if depth<options.biarc_max_split_depth:
			sp1,sp2,sp3 = csp_split(sp1,sp2)
			l1, l2 = cspseglength(sp1,sp2), cspseglength(sp2,sp3)
			if l1+l2 == 0 : zm = z1
			else : zm = z1+(z2-z1)*l1/(l1+l2)
			return biarc(sp1,sp2,z1,zm,depth+1)+biarc(sp2,sp3,zm,z2,depth+1)
		else: return [ [sp1[1],'line', 0, 0, sp2[1], [z1,z2]] ]

	P0, P4 = P(sp1[1]), P(sp2[1])
	TS, TE, v = (P(sp1[2])-P0), -(P(sp2[0])-P4), P0 - P4
	tsa, tea, va = TS.angle(), TE.angle(), v.angle()
	if TE.mag()<straight_distance_tolerance and TS.mag()<straight_distance_tolerance:	
		# Both tangents are zerro - line straight
		return [ [sp1[1],'line', 0, 0, sp2[1], [z1,z2]] ]
	if TE.mag() < straight_distance_tolerance:
		TE = -(TS+v).unit()
		r = TS.mag()/v.mag()*2
	elif TS.mag() < straight_distance_tolerance:
		TS = -(TE+v).unit()
		r = 1/( TE.mag()/v.mag()*2 )
	else:	
		r=TS.mag()/TE.mag()
	TS, TE = TS.unit(), TE.unit()
	tang_are_parallel = ((tsa-tea)%math.pi<straight_tolerance or math.pi-(tsa-tea)%math.pi<straight_tolerance )
	if ( tang_are_parallel  and 
				((v.mag()<straight_distance_tolerance or TE.mag()<straight_distance_tolerance or TS.mag()<straight_distance_tolerance) or
					1-abs(TS*v/(TS.mag()*v.mag()))<straight_tolerance)	):
				# Both tangents are parallel and start and end are the same - line straight
				# or one of tangents still smaller then tollerance

				# Both tangents and v are parallel - line straight
		return [ [sp1[1],'line', 0, 0, sp2[1], [z1,z2]] ]

	c,b,a = v*v, 2*v*(r*TS+TE), 2*r*(TS*TE-1)
	if v.mag()==0:
		return biarc_split(sp1, sp2, z1, z2, depth)
	asmall, bsmall, csmall = abs(a)<10**-10,abs(b)<10**-10,abs(c)<10**-10 
	if 		asmall and b!=0:	beta = -c/b
	elif 	csmall and a!=0:	beta = -b/a 
	elif not asmall:	 
		discr = b*b-4*a*c
		if discr < 0:	raise ValueError, (a,b,c,discr)
		disq = discr**.5
		beta1 = (-b - disq) / 2 / a
		beta2 = (-b + disq) / 2 / a
		if beta1*beta2 > 0 :	raise ValueError, (a,b,c,disq,beta1,beta2)
		beta = max(beta1, beta2)
	elif	asmall and bsmall:	
		return biarc_split(sp1, sp2, z1, z2, depth)
	alpha = beta * r
	ab = alpha + beta 
	P1 = P0 + alpha * TS
	P3 = P4 - beta * TE
	P2 = (beta / ab)  * P1 + (alpha / ab) * P3


	def calculate_arc_params(P0,P1,P2):
		D = (P0+P2)/2
		if (D-P1).mag()==0: return None, None
		R = D - ( (D-P0).mag()**2/(D-P1).mag() )*(P1-D).unit()
		p0a, p1a, p2a = (P0-R).angle()%(2*math.pi), (P1-R).angle()%(2*math.pi), (P2-R).angle()%(2*math.pi)
		alpha =  (p2a - p0a) % (2*math.pi)					
		if (p0a<p2a and  (p1a<p0a or p2a<p1a))	or	(p2a<p1a<p0a) : 
			alpha = -2*math.pi+alpha 
		if abs(R.x)>1000000 or abs(R.y)>1000000  or (R-P0).mag<options.min_arc_radius**2 :
			return None, None
		else :	
			return  R, alpha
	R1,a1 = calculate_arc_params(P0,P1,P2)
	R2,a2 = calculate_arc_params(P2,P3,P4)
	if R1==None or R2==None or (R1-P0).mag()<straight_tolerance or (R2-P2).mag()<straight_tolerance	: return [ [sp1[1],'line', 0, 0, sp2[1], [z1,z2]] ]
	
	d = csp_to_arc_distance(sp1,sp2, [P0,P2,R1,a1],[P2,P4,R2,a2])
	if d > options.biarc_tolerance and depth<options.biarc_max_split_depth	 : return biarc_split(sp1, sp2, z1, z2, depth)
	else:
		if R2.mag()*a2 == 0 : zm = z2
		else : zm  = z1 + (z2-z1)*(abs(R1.mag()*a1))/(abs(R2.mag()*a2)+abs(R1.mag()*a1)) 

		l = (P0-P2).l2()
		if  l < EMC_TOLERANCE_EQUAL**2 or l<EMC_TOLERANCE_EQUAL**2 * R1.l2() /100 :
			# arc should be straight otherwise it could be threated as full circle
			arc1 = [ sp1[1], 'line', 0, 0, [P2.x,P2.y], [z1,zm] ] 
		else :
			arc1 = [ sp1[1], 'arc', [R1.x,R1.y], a1, [P2.x,P2.y], [z1,zm] ] 

		l = (P4-P2).l2()
		if  l < EMC_TOLERANCE_EQUAL**2 or l<EMC_TOLERANCE_EQUAL**2 * R2.l2() /100 :
			# arc should be straight otherwise it could be threated as full circle
			arc2 = [ [P2.x,P2.y], 'line', 0, 0, [P4.x,P4.y], [zm,z2] ] 
		else :
			arc2 = [ [P2.x,P2.y], 'arc', [R2.x,R2.y], a2, [P4.x,P4.y], [zm,z2] ]
		
		return [ arc1, arc2 ]


def biarc_curve_segment_length(seg):
	if seg[1] == "arc" :
		return math.sqrt((seg[0][0]-seg[2][0])**2+(seg[0][1]-seg[2][1])**2)*seg[3]
	elif seg[1] == "line" :	
		return math.sqrt((seg[0][0]-seg[4][0])**2+(seg[0][1]-seg[4][1])**2)
	else: 
		return 0	


def biarc_curve_clip_at_l(curve, l, clip_type = "strict") :
	# get first subcurve and ceck it's length  
	subcurve, subcurve_l, moved = [], 0, False
	for seg in curve:
		if seg[1] == "move" and moved or seg[1] == "end" :	
			break
		if seg[1] == "move" : moved = True 
		subcurve_l += biarc_curve_segment_length(seg)
		if seg[1] == "arc" or seg[1] == "line" : 
			subcurve += [seg]

	if subcurve_l < l and clip_type == "strict" : return []	
	lc = 0
	if (subcurve[-1][4][0]-subcurve[0][0][0])**2 + (subcurve[-1][4][1]-subcurve[0][0][1])**2 < 10**-7 : subcurve_closed = True
	i = 0
	reverse = False
	while lc<l :
		seg = subcurve[i]
		if reverse :  
			if seg[1] == "line" :
				seg = [seg[4], "line", 0 , 0, seg[0], seg[5]] # Hmmm... Do we have to swap seg[5][0] and seg[5][1] (zstart and zend) or not?
			elif seg[1] == "arc" :
				seg = [seg[4], "arc", seg[2] , -seg[3], seg[0], seg[5]] # Hmmm... Do we have to swap seg[5][0] and seg[5][1] (zstart and zend) or not?
		ls = biarc_curve_segment_length(seg)
		if ls != 0 :
			if l-lc>ls :
				res += [seg]
			else :
				if seg[1] == "arc" :
					r  = math.sqrt((seg[0][0]-seg[2][0])**2+(seg[0][1]-seg[2][1])**2)
					x,y = seg[0][0]-seg[2][0], seg[0][1]-seg[2][1]
					a = seg[3]/ls*(l-lc)
					x,y = x*math.cos(a) - y*math.sin(a),  x*math.sin(a) + y*math.cos(a)
					x,y = x+seg[2][0], y+seg[2][1]
					res += [[ seg[0], "arc",  seg[2], a, [x,y], [seg[5][0],seg[5][1]/ls*(l-lc)]  ]]
				if seg[1] == "line" :
					res += [[ seg[0], "line",  0, 0, [(seg[4][0]-seg[0][0])/ls*(l-lc),(seg[4][1]-seg[0][1])/ls*(l-lc)], [seg[5][0],seg[5][1]/ls*(l-lc)]  ]]
		i += 1 
		if i >= len(subcurve) and not subcurve_closed: 
			reverse = not reverse
		i = i%len(subcurve)
	return res	

	
	
class Postprocessor():
	def __init__(self, error_function_handler):	
		self.error = error_function_handler 
		self.functions = {
					"remap"		: self.remap,
					"remapi"	: self.remapi ,
					"scale"		: self.scale,
					"move"		: self.move,
					"flip"		: self.flip_axis,
					"flip_axis"	: self.flip_axis,
					"round"		: self.round_coordinates,
					"parameterize"	: self.parameterize,
					"regex"			: self.re_sub_on_gcode_lines
					}
	
			
	def process(self,command):
		command = re.sub(r"\\\\",":#:#:slash:#:#:",command)
		command = re.sub(r"\\;",":#:#:semicolon:#:#:",command)
		command = command.split(";")
		for s in command: 
			s = re.sub(":#:#:slash:#:#:","\\\\",s)
			s = re.sub(":#:#:semicolon:#:#:","\\;",s)
			s = s.strip()
			if s!="" :
				self.parse_command(s)		
			
	
	def parse_command(self,command):
		r = re.match(r"([A-Za-z0-9_]+)\s*\(\s*(.*)\)",command)
		if not r:
			self.error("Parse error while postprocessing.\n(Command: '%s')"%(command), "error")
		function, parameters = r.group(1).lower(),r.group(2)
		if function in self.functions :
			print_("Postprocessor: executing function %s(%s)"%(function,parameters))
			self.functions[function](parameters)
		else : 
			self.error("Unrecognized function '%s' while postprocessing.\n(Command: '%s')"%(function,command), "error")
	
	
	def re_sub_on_gcode_lines(self, parameters):
		gcode = self.gcode.split("\n")
		self.gcode = ""
		try :
			for line in gcode :
				self.gcode += eval( "re.sub(%s,line)"%parameters) +"\n"

		except Exception as ex :	
			self.error("Bad parameters for regexp. They should be as re.sub pattern and replacement parameters! For example: r\"G0(\d)\", r\"G\\1\" \n(Parameters: '%s')\n %s"%(parameters, ex), "error")				
		
	
	def remapi(self,parameters):
		self.remap(parameters, case_sensitive = True)
	
	
	def remap(self,parameters, case_sensitive = False):
		# remap parameters should be like "x->y,y->x"
		parameters = parameters.replace("\,",":#:#:coma:#:#:")
		parameters = parameters.split(",")
		pattern, remap = [], []
		for s in parameters:
			s = s.replace(":#:#:coma:#:#:","\,")
			r = re.match("""\s*(\'|\")(.*)\\1\s*->\s*(\'|\")(.*)\\3\s*""",s)
			if not r :
				self.error("Bad parameters for remap.\n(Parameters: '%s')"%(parameters), "error")
			pattern +=[r.group(2)]	
			remap +=[r.group(4)]	
		
		
		
		for i in range(len(pattern)) :
			if case_sensitive :
				self.gcode = ireplace(self.gcode, pattern[i], ":#:#:remap_pattern%s:#:#:"%i )
			else :
				self.gcode = self.gcode.replace(pattern[i], ":#:#:remap_pattern%s:#:#:"%i)
			
		for i in range(len(remap)) :
			self.gcode = self.gcode.replace(":#:#:remap_pattern%s:#:#:"%i, remap[i])
	
	
	def transform(self, move, scale):
		axis = ["xi","yj","zk","a"]
		flip = scale[0]*scale[1]*scale[2] < 0 
		gcode = ""
		warned = []
		r_scale = scale[0]
		plane = "g17"
		for s in self.gcode.split("\n"):
			# get plane selection: 
			s_wo_comments = re.sub(r"\([^\)]*\)","",s)
			r = re.search(r"(?i)(G17|G18|G19)", s_wo_comments)
			if r :
				plane = r.group(1).lower()
				if plane == "g17" : r_scale = scale[0] # plane XY -> scale x
				if plane == "g18" : r_scale = scale[0] # plane XZ -> scale x
				if plane == "g19" : r_scale = scale[1] # plane YZ -> scale y
			# Raise warning if scale factors are not the game for G02 and G03	
			if plane not in warned:
				r = re.search(r"(?i)(G02|G03)", s_wo_comments)
				if r :
					if plane == "g17" and scale[0]!=scale[1]: self.error("Post-processor: Scale factors for X and Y axis are not the same. G02 and G03 codes will be corrupted.","warning") 
					if plane == "g18" and scale[0]!=scale[2]: self.error("Post-processor: Scale factors for X and Z axis are not the same. G02 and G03 codes will be corrupted.","warning") 
					if plane == "g19" and scale[1]!=scale[2]: self.error("Post-processor: Scale factors for Y and Z axis are not the same. G02 and G03 codes will be corrupted.","warning") 
					warned += [plane]
			# Transform		
			for i in range(len(axis)) :
				if move[i] != 0 or scale[i] != 1:
					for a in axis[i] :
						r = re.search(r"(?i)("+a+r")\s*(-?)\s*(\d*\.?\d*)", s)
						if r and r.group(3)!="":
							s = re.sub(r"(?i)("+a+r")\s*(-?)\s*(\d*\.?\d*)", r"\1 %f"%(float(r.group(2)+r.group(3))*scale[i]+(move[i] if a not in ["i","j","k"] else 0) ), s)
			#scale radius R
			if r_scale != 1 :
				r = re.search(r"(?i)(r)\s*(-?\s*(\d*\.?\d*))", s)
				if r and r.group(3)!="":
					try:
						s = re.sub(r"(?i)(r)\s*(-?)\s*(\d*\.?\d*)", r"\1 %f"%( float(r.group(2)+r.group(3))*r_scale ), s)
					except:
						pass	

			gcode += s + "\n"
			
		self.gcode = gcode
		if flip : 
			self.remapi("'G02'->'G03', 'G03'->'G02'")


	def parameterize(self,parameters) :
		planes = []
		feeds = {}
		coords = []
		gcode = ""
		coords_def = {"x":"x","y":"y","z":"z","i":"x","j":"y","k":"z","a":"a"}
		for s in self.gcode.split("\n"):
			s_wo_comments = re.sub(r"\([^\)]*\)","",s)
			# get Planes
			r = re.search(r"(?i)(G17|G18|G19)", s_wo_comments)
			if r :
				plane = r.group(1).lower()
				if plane not in planes : 
					planes += [plane]
			# get Feeds
			r = re.search(r"(?i)(F)\s*(-?)\s*(\d*\.?\d*)", s_wo_comments)
			if r :
				feed  = float (r.group(2)+r.group(3))
				if feed not in feeds :
					feeds[feed] = "#"+str(len(feeds)+20)
					
			#Coordinates
			for c in "xyzijka" :
				r = re.search(r"(?i)("+c+r")\s*(-?)\s*(\d*\.?\d*)", s_wo_comments)
				if r :
					c = coords_def[r.group(1).lower()]
					if c not in coords :
						coords += [c]
		# Add offset parametrization
		offset = {"x":"#6","y":"#7","z":"#8","a":"#9"}
		for c in coords:
			gcode += "%s  = 0 (%s axis offset)\n" %  (offset[c],c.upper())
			
		# Add scale parametrization
		if planes == [] : planes = ["g17"]
		if len(planes)>1 :  # have G02 and G03 in several planes scale_x = scale_y = scale_z required
			gcode += "#10 = 1 (Scale factor)\n"
			scale = {"x":"#10","i":"#10","y":"#10","j":"#10","z":"#10","k":"#10","r":"#10"}
		else :
			gcode += "#10 = 1 (%s Scale factor)\n" % ({"g17":"XY","g18":"XZ","g19":"YZ"}[planes[0]])
			gcode += "#11 = 1 (%s Scale factor)\n" % ({"g17":"Z","g18":"Y","g19":"X"}[planes[0]])
			scale = {"x":"#10","i":"#10","y":"#10","j":"#10","z":"#10","k":"#10","r":"#10"}
			if "g17" in planes :
				scale["z"] = "#11"				
				scale["k"] = "#11"				
			if "g18" in planes :
				scale["y"] = "#11"				
				scale["j"] = "#11"				
			if "g19" in planes :
				scale["x"] = "#11"				
				scale["i"] = "#11"				
		# Add a scale 
		if "a" in coords:
			gcode += "#12  = 1 (A axis scale)\n" 
			scale["a"] = "#12"
		
		# Add feed parametrization 
		for f in feeds :
			gcode += "%s = %f (Feed definition)\n" % (feeds[f],f)

		# Parameterize Gcode		
		for s in self.gcode.split("\n"):
			#feed replace :
			r = re.search(r"(?i)(F)\s*(-?)\s*(\d*\.?\d*)", s)
			if r and len(r.group(3))>0:
				s = re.sub(r"(?i)(F)\s*(-?)\s*(\d*\.?\d*)", "F [%s]"%feeds[float(r.group(2)+r.group(3))], s)
			#Coords XYZA replace
			for c in "xyza" :
				r = re.search(r"(?i)(("+c+r")\s*(-?)\s*(\d*\.?\d*))", s)
				if r and len(r.group(4))>0:
					s = re.sub(r"(?i)("+c+r")\s*((-?)\s*(\d*\.?\d*))", r"\1[\2*%s+%s]"%(scale[c],offset[c]), s)

			#Coords IJKR replace
			for c in "ijkr" :
				r = re.search(r"(?i)(("+c+r")\s*(-?)\s*(\d*\.?\d*))", s)
				if r and len(r.group(4))>0:
					s = re.sub(r"(?i)("+c+r")\s*((-?)\s*(\d*\.?\d*))", r"\1[\2*%s]"%scale[c], s)

			gcode += s + "\n"
	
		self.gcode = gcode	

	
	def round_coordinates(self,parameters) :
		try: 
			round_ = int(parameters)
		except :	
			self.error("Bad parameters for round. Round should be an integer! \n(Parameters: '%s')"%(parameters), "error")		
		gcode = ""
		for s in self.gcode.split("\n"):
			for a in "xyzijkaf" :
				r = re.search(r"(?i)("+a+r")\s*(-?\s*(\d*\.?\d*))", s)
				if r : 
					
					if r.group(2)!="":
						s = re.sub(
									r"(?i)("+a+r")\s*(-?)\s*(\d*\.?\d*)", 
									(r"\1 %0."+str(round_)+"f" if round_>0 else r"\1 %d")%round(float(r.group(2)),round_),
									s)
			gcode += s + "\n"
		self.gcode = gcode
	

	def scale(self, parameters):
		parameters = parameters.split(",")
		scale = [1.,1.,1.,1.]
		try :
			for i in range(len(parameters)) :
				if float(parameters[i])==0 : 
					self.error("Bad parameters for scale. Scale should not be 0 at any axis! \n(Parameters: '%s')"%(parameters), "error")		
				scale[i] = float(parameters[i])
		except :
			self.error("Bad parameters for scale.\n(Parameters: '%s')"%(parameters), "error")
		self.transform([0,0,0,0],scale)		

	
	def move(self, parameters):
		parameters = parameters.split(",")
		move = [0.,0.,0.,0.]
		try :
			for i in range(len(parameters)) :
				move[i] = float(parameters[i])
		except :
			self.error("Bad parameters for move.\n(Parameters: '%s')"%(parameters), "error")
		self.transform(move,[1.,1.,1.,1.])		


	def flip_axis(self, parameters):
		parameters = parameters.lower()
		axis = {"x":1.,"y":1.,"z":1.,"a":1.}	
		for p in parameters: 
			if p in [","," ","	","\r","'",'"'] : continue
			if p not in ["x","y","z","a"] : 
				self.error("Bad parameters for flip_axis. Parameter should be string consists of 'xyza' \n(Parameters: '%s')"%(parameters), "error")
			axis[p] = -axis[p]
		self.scale("%f,%f,%f,%f"%(axis["x"],axis["y"],axis["z"],axis["a"]))	

	
			
################################################################################
###		Polygon class
################################################################################
class Polygon:
	def __init__(self, polygon=None):
		self.polygon = [] if polygon==None else polygon[:]
	
	
	def move(self, x, y) :
		for i in range(len(self.polygon)) :
			for j in range(len(self.polygon[i])) :
				self.polygon[i][j][0] += x
				self.polygon[i][j][1] += y
	
	
	def bounds(self) : 
		minx,miny,maxx,maxy = 1e400, 1e400, -1e400, -1e400
		for poly in self.polygon :
			for p in poly :
				if minx > p[0] : minx = p[0]
				if miny > p[1] : miny = p[1]
				if maxx < p[0] : maxx = p[0]
				if maxy < p[1] : maxy = p[1]
		return minx*1,miny*1,maxx*1,maxy*1		
	
	
	def width(self):
		b = self.bounds()
		return b[2]-b[0]
	
	
	def rotate_(self,sin,cos) :
		self.polygon = [
				[
					[point[0]*cos - point[1]*sin,point[0]*sin + point[1]*cos] for point in subpoly
				]			
				for subpoly in self.polygon
			]

	
	def rotate(self, a):
		cos, sin = math.cos(a), math.sin(a)
		self.rotate_(sin,cos)
	
			
	def drop_into_direction(self, direction, surface) :
		# Polygon is a list of simple polygons
		# Surface is a polygon + line y = 0 
		# Direction is [dx,dy]  
		if len(self.polygon) == 0 or len(self.polygon[0])==0 : return
		if direction[0]**2 + direction[1]**2 <1e-10 : return
		direction = normalize(direction)
		sin,cos = direction[0], -direction[1]
		self.rotate_(-sin,cos)
		surface.rotate_(-sin,cos)
		self.drop_down(surface, zerro_plane = False)
		self.rotate_(sin,cos)
		surface.rotate_(sin,cos)
		
			
	def centroid(self):
		centroids = []
		sa = 0
		for poly in self.polygon:
			cx,cy,a = 0,0,0
			for i in range(len(poly)):
				[x1,y1],[x2,y2] = poly[i-1],poly[i]
				cx += (x1+x2)*(x1*y2-x2*y1)
				cy += (y1+y2)*(x1*y2-x2*y1)
				a  += (x1*y2-x2*y1)
			a *= 3.
			if abs(a)>0 :
				cx /= a
				cy /= a
				sa += abs(a)
				centroids += [ [cx,cy,a] ]
		if sa == 0 : return	[0.,0.]
		cx,cy = 0.,0.
		for c in centroids :
			cx += c[0]*c[2]
			cy += c[1]*c[2]
		cx /= sa
		cy /= sa
		return [cx,cy]

			
	def drop_down(self, surface, zerro_plane = True) :
		# Polygon is a list of simple polygons
		# Surface is a polygon + line y = 0 
		# Down means min y (0,-1)  
		if len(self.polygon) == 0 or len(self.polygon[0])==0 : return
		# Get surface top point
		top = surface.bounds()[3]
		if zerro_plane : top = max(0, top)
		# Get polygon bottom point
		bottom = self.bounds()[1]
		self.move(0, top - bottom + 10)		
		# Now get shortest distance from surface to polygon in positive x=0 direction
		# Such distance = min(distance(vertex, edge)...)  where edge from surface and 
		# vertex from polygon and vice versa...
		dist = 1e300
		for poly in surface.polygon :
			for i in range(len(poly)) :
				for poly1 in self.polygon :
					for i1 in range(len(poly1)) :
						st,end = poly[i-1], poly[i] 
						vertex = poly1[i1]
						if st[0]<=vertex[0]<= end[0] or end[0]<=vertex[0]<=st[0] :
							if st[0]==end[0] : d = min(vertex[1]-st[1],vertex[1]-end[1])
							else : d = vertex[1] - st[1] - (end[1]-st[1])*(vertex[0]-st[0])/(end[0]-st[0]) 
							if dist > d  : dist = d
						# and vice versa just change the sign because vertex now under the edge
						st,end = poly1[i1-1], poly1[i1] 
						vertex = poly[i]
						if st[0]<=vertex[0]<=end[0] or end[0]<=vertex[0]<=st[0] :
							if st[0]==end[0] : d = min(- vertex[1]+st[1],-vertex[1]+end[1])
							else : d =  - vertex[1] + st[1] + (end[1]-st[1])*(vertex[0]-st[0])/(end[0]-st[0]) 
							if dist > d  : dist = d
		
		if zerro_plane and dist > 10 + top : dist = 10 + top 
		#print_(dist, top, bottom)
		#self.draw()
		self.move(0, -dist)		
		
					
	def draw(self,color="#075",width=.1, group = None) :
		csp = [csp_subpath_line_to([],poly+[poly[0]]) for poly in self.polygon]
		draw_csp( csp, color=color,width=width, group = group)
			
			
	
	def add(self, add) :
		if type(add) == type([]) :
			self.polygon += add[:]
		else :	
			self.polygon += add.polygon[:]

	
	def point_inside(self,p) :
		inside = False
		for poly in self.polygon :
			for i in range(len(poly)):
				st,end = poly[i-1], poly[i]
				if p==st or p==end : return True # point is a vertex = point is on the edge
				if st[0]>end[0] : st, end = end, st # This will be needed to check that edge if open only at rigth end
				c = (p[1]-st[1])*(end[0]-st[0])-(end[1]-st[1])*(p[0]-st[0])
				#print_(c)
				if st[0]<=p[0]<end[0] : 
					if c<0 : 
						inside = not inside
					elif c == 0 : return True # point is on the edge
				elif st[0]==end[0]==p[0] and (st[1]<=p[1]<=end[1] or end[1]<=p[1]<=st[1]) : # point is on the edge
					return True
		return inside			

	
	def hull(self) :
		# Add vertices at all self intersection points. 
		hull = []
		for i1 in range(len(self.polygon)):
			poly1 = self.polygon[i1]
			poly_ = []
			for j1 in range(len(poly1)):
				s, e = poly1[j1-1],poly1[j1]
				poly_ += [s]
				
				# Check self intersections 
				for j2 in range(j1+1,len(poly1)):
					s1, e1 = poly1[j2-1],poly1[j2]
					int_ = line_line_intersection_points(s,e,s1,e1)
					for p in int_ :
						if point_to_point_d2(p,s)>0.000001 and point_to_point_d2(p,e)>0.000001 : 
							poly_ += [p]
				# Check self intersections with other polys  
				for i2 in range(len(self.polygon)):
					if i1==i2 : continue
					poly2 = self.polygon[i2]
					for j2 in range(len(poly2)):
						s1, e1 = poly2[j2-1],poly2[j2]
						int_ = line_line_intersection_points(s,e,s1,e1)
						for p in int_ :
							if point_to_point_d2(p,s)>0.000001 and point_to_point_d2(p,e)>0.000001 : 
								poly_ += [p]
			hull += [poly_]
		# Create the dictionary containing all edges in both directions
		edges = {}
		for poly in self.polygon :
			for i in range(len(poly)):
				s,e = tuple(poly[i-1]), tuple(poly[i])
				if (point_to_point_d2(e,s)<0.000001) : continue
				break_s, break_e = False, False
				for p in edges :
					if point_to_point_d2(p,s)<0.000001 : 
						break_s = True
						s = p
					if point_to_point_d2(p,e)<0.000001 : 
						break_e = True
						e = p
					if break_s and break_e : break
				l = point_to_point_d(s,e)
				if not break_s and not break_e : 
					edges[s] = [ [s,e,l] ]
					edges[e] = [ [e,s,l] ]
					#draw_pointer(s+e,"red","line")
					#draw_pointer(s+e,"red","line")
				else : 
					if e in edges :	
						for edge in edges[e] :	
							if point_to_point_d2(edge[1],s)<0.000001 :
								break
						if point_to_point_d2(edge[1],s)>0.000001 :
							edges[e] += [ [e,s,l] ]
							#draw_pointer(s+e,"red","line")
							
					else : 
						edges[e] = [ [e,s,l] ]
						#draw_pointer(s+e,"green","line")
					if s in edges :	
						for edge in edges[s] :	
							if  point_to_point_d2(edge[1],e)<0.000001 :
								break
						if point_to_point_d2(edge[1],e)>0.000001 :
							edges[s] += [ [s,e, l] ]
							#draw_pointer(s+e,"red","line")
					else : 
						edges[s] = [ [s,e,l] ]
						#draw_pointer(s+e,"green","line")

		
		def angle_quadrant(sin,cos):
			# quadrants are (0,pi/2], (pi/2,pi], (pi,3*pi/2], (3*pi/2, 2*pi], i.e. 0 is in the 4-th quadrant
			if sin>0 and cos>=0 : return 1
			if sin>=0 and cos<0 : return 2
			if sin<0 and cos<=0 : return 3
			if sin<=0 and cos>0 : return 4
			
		
		def angle_is_less(sin,cos,sin1,cos1):
			# 0 = 2*pi is the largest angle
			if [sin1, cos1] == [0,1] : return True
			if [sin, cos] == [0,1] : return False
			if angle_quadrant(sin,cos)>angle_quadrant(sin1,cos1) : 
				return False
			if angle_quadrant(sin,cos)<angle_quadrant(sin1,cos1) : 
				return True
			if sin>=0 and cos>0 : return sin<sin1
			if sin>0 and cos<=0 : return sin>sin1
			if sin<=0 and cos<0 : return sin>sin1
			if sin<0 and cos>=0 : return sin<sin1

			
		def get_closes_edge_by_angle(edges, last):
			# Last edge is normalized vector of the last edge.
			min_angle = [0,1]
			next = last
			last_edge = [(last[0][0]-last[1][0])/last[2], (last[0][1]-last[1][1])/last[2]]
			for p in edges:
				#draw_pointer(list(p[0])+[p[0][0]+last_edge[0]*40,p[0][1]+last_edge[1]*40], "Red", "line", width=1)
				#print_("len(edges)=",len(edges))
				cur = [(p[1][0]-p[0][0])/p[2],(p[1][1]-p[0][1])/p[2]]
				cos, sin = dot(cur,last_edge),  cross(cur,last_edge)
				#draw_pointer(list(p[0])+[p[0][0]+cur[0]*40,p[0][1]+cur[1]*40], "Orange", "line", width=1, comment = [sin,cos])
				#print_("cos, sin=",cos,sin)
				#print_("min_angle_before=",min_angle)

				if 	angle_is_less(sin,cos,min_angle[0],min_angle[1]) : 
					min_angle = [sin,cos]
					next = p
				#print_("min_angle=",min_angle)

			return next		
			
		# Join edges together into new polygon cutting the vertexes inside new polygon
		self.polygon = []
		len_edges = sum([len(edges[p]) for p in edges]) 
		loops = 0 
		
		while len(edges)>0 :
			poly = []
			if loops > len_edges  : raise ValueError, "Hull error"
			loops+=1
			# Find left most vertex.
			start = (1e100,1)
			for edge in edges : 
				start = min(start, min(edges[edge])) 
			last = [(start[0][0]-1,start[0][1]),start[0],1]
			first_run = True
			loops1 = 0
			while (last[1]!=start[0] or first_run) : 	
				first_run = False
				if loops1 > len_edges  : raise ValueError, "Hull error"
				loops1 += 1
				next = get_closes_edge_by_angle(edges[last[1]],last)
				#draw_pointer(next[0]+next[1],"Green","line", comment=i, width= 1)
				#print_(next[0],"-",next[1]) 

				last = next
				poly += [ list(last[0]) ]				
			self.polygon += [ poly ]
			# Remove all edges that are intersects new poly (any vertex inside new poly)
			poly_ = Polygon([poly])
			for p in edges.keys()[:] : 
				if poly_.point_inside(list(p)) : del edges[p]
		self.draw(color="Green", width=1)	


class Arangement_Genetic:
	# gene = [fittness, order, rotation, xposition]
	# spieces = [gene]*shapes count
	# population = [spieces]
	def __init__(self, polygons, material_width):
		self.population = []
		self.genes_count = len(polygons)
		self.polygons = polygons
		self.width = material_width
		self.mutation_factor = 0.1
		self.order_mutate_factor = 1.
		self.move_mutate_factor = 1.

	
	def add_random_species(self,count):
		for i in range(count):
			specimen = []
			order = range(self.genes_count)
			random.shuffle(order)
			for j in order:
				specimen += [ [j, random.random(), random.random()] ]
			self.population += [ [None,specimen] ]

	
	def species_distance2(self,sp1,sp2) :
		# retun distance, each component is normalized
		s = 0
		for j in range(self.genes_count) :
			s += ((sp1[j][0]-sp2[j][0])/self.genes_count)**2 + (( sp1[j][1]-sp2[j][1]))**2 + ((sp1[j][2]-sp2[j][2]))**2
		return s

	
	def similarity(self,sp1,top) :
		# Define similarity as a simple distance between two points in len(gene)*len(spiece) -th dimentions
		# for sp2 in top_spieces sum(|sp1-sp2|)/top_count
		sim = 0
		for sp2 in top : 
			sim += math.sqrt(species_distance2(sp1,sp2[1]))
		return sim/len(top)
		
	
	def leave_top_species(self,count):
		self.population.sort()
		res = [  copy.deepcopy(self.population[0]) ]
		del self.population[0]
		for i in range(count-1) :
			t = []
			for j in range(20) : 
				i1 = random.randint(0,len(self.population)-1) 
				t += [ [self.population[i1][0],i1] ] 
			t.sort()
			res += [  copy.deepcopy(self.population[t[0][1]]) ]
			del self.population[t[0][1]]
		self.population = res		
		#del self.population[0]
		#for c in range(count-1) :
		#	rank = []
		#	for i in range(len(self.population)) :	
		#		sim = self.similarity(self.population[i][1],res)
		#		rank += [ [self.population[i][0] / sim if sim>0 else 1e100,i] ]
		#	rank.sort()
		#	res += [  copy.deepcopy(self.population[rank[0][1]]) ]
		#	print_(rank[0],self.population[rank[0][1]][0])
		#	print_(res[-1])
		#	del self.population[rank[0][1]]
			
		self.population = res
			
			
	def populate_species(self,count, parent_count):
		self.population.sort()
		self.inc = 0
		for c in range(count):
			parent1 = random.randint(0,parent_count-1)
			parent2 = random.randint(0,parent_count-1)
			if parent1==parent2 : parent2 = (parent2+1) % parent_count
			parent1, parent2 = self.population[parent1][1], self.population[parent2][1]
			i1,i2 = 0, 0
			genes_order = []
			specimen = [ [0,0.,0.] for i in range(self.genes_count) ]
			
			self.incest_mutation_multiplyer = 1.
			self.incest_mutation_count_multiplyer = 1.

			if self.species_distance2(parent1, parent2) <= .01/self.genes_count :
				# OMG it's a incest :O!!!
				# Damn you bastards!
				self.inc +=1
				self.incest_mutation_multiplyer = 2. 
				self.incest_mutation_count_multiplyer = 2. 
			else :
				pass
#				if random.random()<.01 : print_(self.species_distance2(parent1, parent2))	
			start_gene = random.randint(0,self.genes_count)
			end_gene = (max(1,random.randint(0,self.genes_count),int(self.genes_count/4))+start_gene) % self.genes_count
			if end_gene<start_gene : 
				end_gene, start_gene = start_gene, end_gene
				parent1, parent2 = parent2, parent1
			for i in range(start_gene,end_gene) : 
				#rotation_mutate_param = random.random()/100
				#xposition_mutate_param = random.random()/100
				tr = 1. #- rotation_mutate_param
				tp = 1. #- xposition_mutate_param
				specimen[i] = [parent1[i][0], parent1[i][1]*tr+parent2[i][1]*(1-tr),parent1[i][2]*tp+parent2[i][2]*(1-tp)]
				genes_order += [ parent1[i][0] ]

			for i in range(0,start_gene)+range(end_gene,self.genes_count) : 
				tr = 0. #rotation_mutate_param
				tp = 0. #xposition_mutate_param
				j = i 
				while parent2[j][0] in genes_order :
					j = (j+1)%self.genes_count
				specimen[i] = [parent2[j][0], parent1[i][1]*tr+parent2[i][1]*(1-tr),parent1[i][2]*tp+parent2[i][2]*(1-tp)]
				genes_order += [ parent2[j][0] ]						
				

			for i in range(random.randint(self.mutation_genes_count[0],self.mutation_genes_count[0]*self.incest_mutation_count_multiplyer )) :
				if random.random() < self.order_mutate_factor * self.incest_mutation_multiplyer : 
					i1,i2 = random.randint(0,self.genes_count-1),random.randint(0,self.genes_count-1)
					specimen[i1][0], specimen[i2][0] = specimen[i2][0], specimen[i1][0]
				if random.random() < self.move_mutation_factor * self.incest_mutation_multiplyer: 
					i1 = random.randint(0,self.genes_count-1)
					specimen[i1][1] =  (specimen[i1][1]+random.random()*math.pi2*self.move_mutation_multiplier)%1.
					specimen[i1][2] =  (specimen[i1][2]+random.random()*self.move_mutation_multiplier)%1.
			self.population += [ [None,specimen] ]	

	
	def test_spiece_drop_down(self,spiece) : 
		surface = Polygon()
		for p in spiece :
			time_ = time.time()
			poly = Polygon(copy.deepcopy(self.polygons[p[0]].polygon))
			poly.rotate(p[1]*math.pi2)
			w = poly.width()
			left = poly.bounds()[0]
			poly.move( -left + (self.width-w)*p[2],0)
			poly.drop_down(surface)
			surface.add(poly)
		return surface

	
	def test(self,test_function): 
		time_ = time.time()
		for i in range(len(self.population)) :
			if self.population[i][0] == None :
				surface = test_function(self.population[i][1])
				b = surface.bounds()
				self.population[i][0] = (b[3]-b[1])*(b[2]-b[0])
		self.population.sort() 				

	def test_spiece_centroid(self,spiece) : 
		poly = Polygon(	self.polygons[spiece[0][0]].polygon[:])
		poly.rotate(spiece[0][1]*math.pi2)
		surface  = Polygon(poly.polygon)
		for p in spiece[1:] :
			poly = Polygon(self.polygons[p[0]].polygon[:])
			c = surface.centroid()
			surface.move(-c[0],-c[1])
			c1 = poly.centroid()
			poly.move(-c1[0],-c1[1])
			poly.rotate(p[1]*math.pi2+p[2]*math.pi2)
			surface.rotate(p[2]*math.pi2)
			poly.drop_down(surface)
			surface.add(poly)
			surface.rotate(-p[2]*math.pi2)
		return surface
		
				
	def test_inline(self) : 
		###
		###	Fast test function using weave's from scipy inline function
		###
		try :
			converters is None 
		except :	
			try:
				from scipy import weave
				from scipy.weave import converters
			except:
				options.self.error("For this function Scipy is needed. See http://www.cnc-club.ru/gcodetools for details.","error")		
	
		# Prepare vars 
		poly_, subpoly_, points_ = [], [], []
		for  poly in self.polygons :
			p = poly.polygon
			poly_ += [len(subpoly_), len(subpoly_)+len(p)*2]
			for subpoly in p :
				subpoly_ += [len(points_), len(points_)+len(subpoly)*2+2]
				for point in subpoly :
					points_ += point
				points_ += subpoly[0] # Close subpolygon

		test_ = []
		population_ = []
		for spiece in self.population:
			test_.append( spiece[0] if spiece[0] != None else -1)
			for sp in spiece[1]:
				population_ += sp
			
		lp_, ls_, l_, lt_ = len(poly_), len(subpoly_), len(points_), len(test_)
		 
		f = open('inline_test.c', 'r')
		code = f.read()
		f.close()
		
		f = open('inline_test_functions.c', 'r')
		functions = f.read()
		f.close()
		
		stdout_ = sys.stdout
		s = ''		
		sys.stdout = s 

		test = weave.inline(
							code,
							['points_','subpoly_','poly_', 'lp_', 'ls_', 'l_', 'lt_','test_', 'population_'],
							compiler='gcc',
							support_code = functions,
							)
		if s!='' : options.self.error(s,"warning")		
		sys.stdout = stdout_
		
		for i in range(len(test_)):
			self.population[i][0] = test_[i]
		
		
		
		
		#surface.draw()

		
################################################################################
###
###		Gcodetools class
###
################################################################################

class Gcodetools(inkex.Effect):

	def export_gcode(self,gcode, no_headers = False) :
		if self.options.postprocessor != ""  or self.options.postprocessor_custom != "" :
			postprocessor = Postprocessor(self.error)
			postprocessor.gcode = gcode
			if self.options.postprocessor != "" :
				postprocessor.process(self.options.postprocessor)
			if self.options.postprocessor_custom != "" :
				postprocessor.process(self.options.postprocessor_custom)

		if not no_headers :
			postprocessor.gcode = self.header + postprocessor.gcode + self.footer

		f = open(self.options.directory+self.options.file, "w")	
		f.write(postprocessor.gcode)
		f.close()							


################################################################################
###		In/out paths:
###		TODO move it to the bottom
################################################################################
	def plasma_prepare_path(self) :
	
		def add_arc(sp1,sp2,end = False,l=10.,r=10.) :
			if not end :
				n = csp_normalized_normal(sp1,sp2,0.)
				return csp_reverse([arc_from_s_r_n_l(sp1[1],r,n,-l)])[0]
			else: 
				n = csp_normalized_normal(sp1,sp2,1.)
				return arc_from_s_r_n_l(sp2[1],r,n,l)
				
		def add_normal(sp1,sp2,end = False,l=10.,r=10.) :
			# r is needed only for be compatible with add_arc
			if not end : 
				n = csp_normalized_normal(sp1,sp2,0.)
				p = [n[0]*l+sp1[1][0],n[1]*l+sp1[1][1]]
				return csp_subpath_line_to([], [p,sp1[1]])
			else: 
				n = csp_normalized_normal(sp1,sp2,1.)
				p = [n[0]*l+sp2[1][0],n[1]*l+sp2[1][1]]
				return csp_subpath_line_to([], [sp2[1],p])
				
		def add_tangent(sp1,sp2,end = False,l=10.,r=10.) :
			# r is needed only for be compatible with add_arc
			if not end : 
				n = csp_normalized_slope(sp1,sp2,0.)
				p = [-n[0]*l+sp1[1][0],-n[1]*l+sp1[1][1]]
				return csp_subpath_line_to([], [p,sp1[1]])
			else: 
				n = csp_normalized_slope(sp1,sp2,1.)
				p = [n[0]*l+sp2[1][0],n[1]*l+sp2[1][1]]
				return csp_subpath_line_to([], [sp2[1],p])
	
		if not self.options.in_out_path and not self.options.plasma_prepare_corners and self.options.in_out_path_do_not_add_reference_point: 
			self.error("Warning! Extenstion is not said to do anything! Enable one of Create in-out paths or Prepare corners checkboxes or disable Do not add in-out referense point!")
			return

		# Add in-out-reference point if there is no one yet.		
		if ( (len(self.in_out_reference_points)==0 and self.options.in_out_path
			or not self.options.in_out_path and not self.options.plasma_prepare_corners )
			 and not self.options.in_out_path_do_not_add_reference_point) :
					self.options.orientation_points_count = "in-out reference point"		
					self.orientation()
			
		if self.options.in_out_path or self.options.plasma_prepare_corners:
			self.set_markers()
			add_func = {"Round":add_arc, "Perpendicular": add_normal, "Tangent": add_tangent}[self.options.in_out_path_type]
			if self.options.in_out_path_type == "Round" and self.options.in_out_path_len > self.options.in_out_path_radius*3/2*math.pi :
				self.error("In-out len is to big for in-out radius will cropp it to be r*3/2*pi!", "warning") 
			
			if self.selected_paths == {} and self.options.auto_select_paths:
				self.selected_paths = self.paths
				self.error(_("No paths are selected! Trying to work on all available paths."),"warning")

			if self.selected_paths == {}:
				self.error(_("Nothing is selected. Please select something."),"warning")
			a = self.options.plasma_prepare_corners_tolerance
			corner_tolerance = cross([1.,0.], [math.cos(a),math.sin(a)])

			for layer in self.layers :
				if layer in self.selected_paths :
					max_dist =	self.transform_scalar(self.options.in_out_path_point_max_dist, layer, reverse=True)
					l = 		self.transform_scalar(self.options.in_out_path_len, layer, reverse=True)
					plasma_l = 	self.transform_scalar(self.options.plasma_prepare_corners_distance, layer, reverse=True)
					r = 		self.transform_scalar(self.options.in_out_path_radius, layer, reverse=True)
					l = min(l,r*3/2*math.pi)

					for path in self.selected_paths[layer]:
						csp = self.apply_transforms( path, cubicsuperpath.parsePath(path.get("d")) )
						csp = csp_remove_zerro_segments(csp)
						res = []

						for subpath in csp :
						# Find closes point to in-out reference point
						# If subpath is open skip this step 
							if self.options.in_out_path :
								# split and reverse path for further add in-out points
								if point_to_point_d2(subpath[0][1], subpath[-1][1]) < 1.e-10 :
									d = [1e100,1,1,1.]
									for p in self.in_out_reference_points : 
										d1 = csp_to_point_distance([subpath], p, dist_bounds = [0,max_dist], tolerance=.01)
										if d1[0] < d[0] : 
											d = d1[:]
											p_ = p
									if d[0] < max_dist**2 :
										# Lets find is there any angles near this point to put in-out path in 
										# the angle if it's possible 
										# remove last node to make iterations easier
										subpath[0][0] = subpath[-1][0]
										del subpath[-1]
										max_cross = [-1e100, None]
										for j in range(len(subpath)) :
											sp1,sp2,sp3 = subpath[j-2],subpath[j-1],subpath[j]
											if point_to_point_d2(sp2[1],p_)<max_dist**2:
												s1,s2 = csp_normalized_slope(sp1,sp2,1.), csp_normalized_slope(sp2,sp3,0.)
												max_cross = max(max_cross,[cross(s1,s2),j-1])
										# return back last point		
										subpath.append(subpath[0])
										if max_cross[1] !=None  and max_cross[0]>corner_tolerance :
											# there's an angle near the point
											j = max_cross[1]
											if j<0 : j -= 1
											if j!=0 :
												subpath	= csp_concat_subpaths(subpath[j:],subpath[:j+1])
										else :
											# have to cut path's segment
											d,i,j,t = d
											sp1,sp2,sp3 = csp_split(subpath[j-1],subpath[j],t)
											subpath = csp_concat_subpaths([sp2,sp3], subpath[j:], subpath[:j], [sp1,sp2]) 							

							if self.options.plasma_prepare_corners :
								# prepare corners
								# find corners and add some nodes
								# corner at path's start/end is ignored
								res_ = [subpath[0]]
								for sp2, sp3 in zip(subpath[1:],subpath[2:]) :
									sp1 = res_[-1]
									s1,s2 = csp_normalized_slope(sp1,sp2,1.), csp_normalized_slope(sp2,sp3,0.)
									if cross(s1,s2) > corner_tolerance :
										# got a corner to process
										S1,S2 = P(s1),P(s2)
										N = (S1-S2).unit()*plasma_l
										SP2= P(sp2[1])
										P1 = (SP2 + N)
										res_ += [
													[sp2[0],sp2[1],  (SP2+S1*plasma_l).to_list() ],
													[ (P1-N.ccw()/2 ).to_list(), P1.to_list(), (P1+N.ccw()/2).to_list()], 
													[(SP2-S2*plasma_l).to_list(), sp2[1],sp2[2]]
												]
									else: 
										res_ += [sp2]
								res_ += [sp3]
								subpath = res_
							if self.options.in_out_path :
								# finally add let's add in-out paths... 
								subpath = csp_concat_subpaths(
													add_func(subpath[0],subpath[1],False,l,r),
													subpath,
													add_func(subpath[-2],subpath[-1],True,l,r)
													)

							
							res += [ subpath ] 
							
							
						if self.options.in_out_path_replace_original_path :
							path.set("d", cubicsuperpath.formatPath( self.apply_transforms(path,res,True) ))
						else:	
							draw_csp(res, width=1, style=styles["in_out_path_style"] )
		
################################################################################
###		Arrangement: arranges paths by givven params
###		TODO move it to the bottom
################################################################################
	def arrangement(self) :
		paths = self.selected_paths
		surface = Polygon()
		polygons = []
		time_ = time.time()
		print_("Arrangement start at %s"%(time_))
		original_paths = []
		for layer in self.layers :
			if layer in paths :
				for path in paths[layer] :
					csp = cubicsuperpath.parsePath(path.get("d"))
					polygon = Polygon()
					for subpath in csp :
						for sp1, sp2 in zip(subpath,subpath[1:]) :
							polygon.add([csp_segment_convex_hull(sp1,sp2)])
					#print_("Redused edges count from", sum([len(poly) for poly in polygon.polygon ]) )
					polygon.hull()
					original_paths += [path]
					polygons += [polygon]
					
		print_("Paths hull computed in %s sec."%(time.time()-time_))
		print_("Got %s polygons having average %s edges each."% ( len(polygons), float(sum([ sum([len(poly) for poly in polygon.polygon]) for polygon in polygons ])) / len(polygons) ) )
		time_ = time.time()
		
#		material_width = self.options.arrangement_material_width
#		population = Arangement_Genetic(polygons, material_width)
#		population.add_random_species(1)
#		population.test_population_centroid()
##		return
		material_width = self.options.arrangement_material_width
		population = Arangement_Genetic(polygons, material_width)
		
		
		print_("Genetic algorithm start at %s"%(time_))
		start_time = time.time()
		time_ = time.time()
		
	

		population.add_random_species(50)
		#population.test(population.test_spiece_centroid)
		print_("Initial population done in %s"%(time.time()-time_))
		time_ = time.time()
		pop = copy.deepcopy(population)
		population_count = self.options.arrangement_population_count
		last_champ = -1
		champions_count = 0
		
		
		
		
		for i in range(population_count):
			population.leave_top_species(20)
			population.move_mutation_multiplier = random.random()/2
			
			population.order_mutation_factor = .2
			population.move_mutation_factor = 1.
			population.mutation_genes_count = [1,2]
			population.populate_species(250, 20)
			print_("Populate done at %s"%(time.time()-time_))			
			"""
			randomize = i%100 < 40
			if 	i%100 < 40 : 
				population.add_random_species(250)
			if  40<= i%100 < 100 : 
				population.mutation_genes_count = [1,max(2,int(population.genes_count/4))]  #[1,max(2,int(population.genes_count/2))] if 40<=i%100<60 else [1,max(2,int(population.genes_count/10))]
				population.move_mutation_multiplier = 1. if 40<=i%100<80 else .1
				population.move_mutation_factor = (-(i%100)/30+10/3) if 50<=i%100<100 else .5
				population.order_mutation_factor = 1./(i%100-79) if 80<=i%100<100 else 1.
				population.populate_species(250, 10)
			"""
			if self.options.arrangement_inline_test :
				population.test_inline()
			else:		
				population.test(population.test_spiece_centroid)
				
			print_("Test done at %s"%(time.time()-time_))
			draw_new_champ = False
			print_()

			
			if population.population[0][0]!= last_champ : 
				draw_new_champ = True
				improve = last_champ-population.population[0][0]
				last_champ = population.population[0][0]*1

			
			print_("Cicle %s done in %s"%(i,time.time()-time_))
			time_ = time.time()
			print_("%s incests been found"%population.inc)
			print_()

			if i == 0  or i == population_count-1 or draw_new_champ :
				colors = ["blue"]
				
				surface = population.test_spiece_centroid(population.population[0][1])
				b = surface.bounds()
				x,y = 400* (champions_count%10), 700*int(champions_count/10)
				surface.move(x-b[0],y-b[1])
				surface.draw(width=2, color=colors[0])
				draw_text("Step = %s\nSquare = %f\nSquare improvement = %f\nTime from start = %f"%(i,(b[2]-b[0])*(b[3]-b[1]),improve,time.time()-start_time),x,y-50)
				champions_count += 1
				"""
				spiece = population.population[0][1]
				poly = Polygon(copy.deepcopy(population.polygons[spiece[0][0]].polygon))
				poly.rotate(spiece[0][2]*math.pi2)
				surface  = Polygon(poly.polygon)
				poly.draw(width = 2, color= "Violet")
				for p in spiece[1:] :
					poly = Polygon(copy.deepcopy(population.polygons[p[0]].polygon))
					poly.rotate(p[2]*math.pi2)
					direction = [math.cos(p[1]*math.pi2), -math.sin(p[1]*math.pi2)]
					normalize(direction)
					c = surface.centroid()
					c1 = poly.centroid()
					poly.move(c[0]-c1[0]-direction[0]*400,c[1]-c1[1]-direction[1]*400)
					c = surface.centroid()
					c1 = poly.centroid()
					poly.draw(width = 5, color= "Violet")
					draw_pointer(c+c1,"Green","line")
					direction = normalize(direction)
					
					
					sin,cos = direction[0], direction[1]
					poly.rotate_(-sin,cos)
					surface.rotate_(-sin,cos)
#					poly.draw(color = "Violet",width=4)					
					surface.draw(color = "Orange",width=4)					
					poly.rotate_(sin,cos)
					surface.rotate_(sin,cos)


					poly.drop_into_direction(direction,surface)
					surface.add(poly)
				
				"""
		# Now we'll need apply transforms to original paths
		
		
	def __init__(self):
		inkex.Effect.__init__(self)
		self.OptionParser.add_option("-d", "--directory",					action="store", type="string", 		dest="directory", default="/home/",					help="Directory for gcode file")
		self.OptionParser.add_option("-f", "--filename",					action="store", type="string", 		dest="file", default="-1.0",						help="File name")			
		self.OptionParser.add_option("",   "--add-numeric-suffix-to-filename", action="store", type="inkbool",	dest="add_numeric_suffix_to_filename", default=True,help="Add numeric suffix to filename")			
		self.OptionParser.add_option("",   "--Zscale",						action="store", type="float", 		dest="Zscale", default="1.0",						help="Scale factor Z")				
		self.OptionParser.add_option("",   "--Zoffset",						action="store", type="float", 		dest="Zoffset", default="0.0",						help="Offset along Z")
		self.OptionParser.add_option("-s", "--Zsafe",						action="store", type="float", 		dest="Zsafe", default="0.5",						help="Z above all obstacles")
		self.OptionParser.add_option("-z", "--Zsurface",					action="store", type="float", 		dest="Zsurface", default="0.0",						help="Z of the surface")
		self.OptionParser.add_option("-c", "--Zdepth",						action="store", type="float", 		dest="Zdepth", default="-0.125",					help="Z depth of cut")
		self.OptionParser.add_option("",   "--Zstep",						action="store", type="float", 		dest="Zstep", default="-0.125",						help="Z step of cutting")		
		self.OptionParser.add_option("-p", "--feed",						action="store", type="float", 		dest="feed", default="4.0",							help="Feed rate in unit/min")

		self.OptionParser.add_option("",   "--biarc-tolerance",				action="store", type="float", 		dest="biarc_tolerance", default="1",				help="Tolerance used when calculating biarc interpolation.")				
		self.OptionParser.add_option("",   "--biarc-max-split-depth",		action="store", type="int", 		dest="biarc_max_split_depth", default="4",			help="Defines maximum depth of splitting while approximating using biarcs.")				
		self.OptionParser.add_option("",   "--path-to-gcode-order",			action="store", type="string", 		dest="path_to_gcode_order", default="path by path",	help="Defines cutting order path by path or layer by layer.")				
		self.OptionParser.add_option("",   "--path-to-gcode-depth-function",action="store", type="string", 		dest="path_to_gcode_depth_function", default="zd",	help="Path to gcode depth function.")				
		self.OptionParser.add_option("",   "--path-to-gcode-sort-paths",	action="store", type="inkbool",		dest="path_to_gcode_sort_paths", default=True,		help="Sort paths to reduse rapid distance.")		
		self.OptionParser.add_option("",   "--comment-gcode",				action="store", type="string", 		dest="comment_gcode", default="",					help="Comment Gcode")				
		self.OptionParser.add_option("",   "--comment-gcode-from-properties",action="store", type="inkbool", 	dest="comment_gcode_from_properties", default=False,help="Get additional comments from Object Properties")				



		self.OptionParser.add_option("",   "--tool-diameter",				action="store", type="float", 		dest="tool_diameter", default="3",					help="Tool diameter used for area cutting")		
		self.OptionParser.add_option("",   "--max-area-curves",				action="store", type="int", 		dest="max_area_curves", default="100",				help="Maximum area curves for each area")
		self.OptionParser.add_option("",   "--area-inkscape-radius",		action="store", type="float", 		dest="area_inkscape_radius", default="0",			help="Area curves overlaping (depends on tool diameter [0,0.9])")
		self.OptionParser.add_option("",   "--area-tool-overlap",			action="store", type="float", 		dest="area_tool_overlap", default="-10",			help="Radius for preparing curves using inkscape")
		self.OptionParser.add_option("",   "--unit",						action="store", type="string", 		dest="unit", default="G21 (All units in mm)",		help="Units")
		self.OptionParser.add_option("",   "--active-tab",					action="store", type="string", 		dest="active_tab", default="",						help="Defines which tab is active")

		self.OptionParser.add_option("",   "--area-fill-angle",				action="store", type="float", 		dest="area_fill_angle", default="0",					help="Fill area with lines heading this angle")
		self.OptionParser.add_option("",   "--area-fill-shift",				action="store", type="float", 		dest="area_fill_shift", default="0",					help="Shift the lines by tool d * shift")
		self.OptionParser.add_option("",   "--area-fill-method",			action="store", type="string", 		dest="area_fill_method", default="zig-zag",					help="Filling method either zig-zag or spiral")

		self.OptionParser.add_option("",   "--area-find-artefacts-diameter",action="store", type="float", 		dest="area_find_artefacts_diameter", default="1",					help="Artefacts seeking radius")
		self.OptionParser.add_option("",   "--area-find-artefacts-action",	action="store", type="string",	 	dest="area_find_artefacts_action", default="mark with an arrow",	help="Artefacts action type")

		self.OptionParser.add_option("",   "--auto_select_paths",			action="store", type="inkbool",		dest="auto_select_paths", default=True,				help="Select all paths if nothing is selected.")		

		self.OptionParser.add_option("",   "--loft-distances",				action="store", type="string", 		dest="loft_distances", default="10",				help="Distances between paths.")
		self.OptionParser.add_option("",   "--loft-direction",				action="store", type="string", 		dest="loft_direction", default="crosswise",			help="Direction of loft's interpolation.")
		self.OptionParser.add_option("",   "--loft-interpolation-degree",	action="store", type="float",		dest="loft_interpolation_degree", default="2",		help="Which interpolation use to loft the paths smooth interpolation or staright.")

		self.OptionParser.add_option("",   "--min-arc-radius",				action="store", type="float", 		dest="min_arc_radius", default=".1",				help="All arc having radius less than minimum will be considered as straight line")		

		self.OptionParser.add_option("",   "--engraving-sharp-angle-tollerance",action="store", type="float",	dest="engraving_sharp_angle_tollerance", default="150",		help="All angles thar are less than engraving-sharp-angle-tollerance will be thought sharp")		
		self.OptionParser.add_option("",   "--engraving-max-dist",			action="store", type="float", 		dest="engraving_max_dist", default="10",					help="Distanse from original path where engraving is not needed (usualy it's cutting tool diameter)")		
		self.OptionParser.add_option("",   "--engraving-newton-iterations", action="store", type="int", 		dest="engraving_newton_iterations", default="4",			help="Number of sample points used to calculate distance")		
		self.OptionParser.add_option("",   "--engraving-draw-calculation-paths",action="store", type="inkbool",	dest="engraving_draw_calculation_paths", default=False,		help="Draw additional graphics to debug engraving path")		
		self.OptionParser.add_option("",   "--engraving-cutter-shape-function",action="store", type="string", 	dest="engraving_cutter_shape_function", default="w",		help="Cutter shape function z(w). Ex. cone: w. ")

		self.OptionParser.add_option("",   "--lathe-width",					action="store", type="float", 		dest="lathe_width", default=10.,							help="Lathe width")
		self.OptionParser.add_option("",   "--lathe-fine-cut-width",		action="store", type="float", 		dest="lathe_fine_cut_width", default=1.,					help="Fine cut width")
		self.OptionParser.add_option("",   "--lathe-fine-cut-count",		action="store", type="int", 		dest="lathe_fine_cut_count", default=1.,					help="Fine cut count")
		self.OptionParser.add_option("",   "--lathe-create-fine-cut-using",	action="store", type="string",		dest="lathe_create_fine_cut_using", default="Move path",			help="Create fine cut using")
		self.OptionParser.add_option("",   "--lathe-x-axis-remap",			action="store", type="string", 		dest="lathe_x_axis_remap", default="X",						help="Lathe X axis remap")
		self.OptionParser.add_option("",   "--lathe-z-axis-remap",			action="store", type="string", 		dest="lathe_z_axis_remap", default="Z",						help="Lathe Z axis remap")

		self.OptionParser.add_option("",   "--lathe-rectangular-cutter-width",action="store", type="float", 	dest="lathe_rectangular_cutter_width", default="4",		help="Rectangular cutter width")

		self.OptionParser.add_option("",   "--create-log",					action="store", type="inkbool", 	dest="log_create_log", default=False,				help="Create log files")
		self.OptionParser.add_option("",   "--log-filename",				action="store", type="string", 		dest="log_filename", default='',					help="Create log files")

		self.OptionParser.add_option("",   "--orientation-points-count",	action="store", type="string", 		dest="orientation_points_count", default="2",			help="Orientation points count")
		self.OptionParser.add_option("",   "--tools-library-type",			action="store", type="string", 		dest="tools_library_type", default='cylinder cutter',	help="Create tools definition")

		self.OptionParser.add_option("",   "--dxfpoints-action",			action="store", type="string", 		dest="dxfpoints_action", default='replace',			help="dxfpoint sign toggle")
																										  
		self.OptionParser.add_option("",   "--help-language",				action="store", type="string", 		dest="help_language", default='http://www.cnc-club.ru/forum/viewtopic.php?f=33&t=35',	help="Open help page in webbrowser.")

		self.OptionParser.add_option("",   "--offset-radius",				action="store", type="float", 		dest="offset_radius", default=10.,		help="Offset radius")
		self.OptionParser.add_option("",   "--offset-step",					action="store", type="float", 		dest="offset_step", default=10.,		help="Offset step")
		self.OptionParser.add_option("",   "--offset-draw-clippend-path",	action="store", type="inkbool",		dest="offset_draw_clippend_path", default=False,		help="Draw clipped path")		
		self.OptionParser.add_option("",   "--offset-just-get-distance",	action="store", type="inkbool",		dest="offset_just_get_distance", default=False,		help="Don't do offset just get distance")		
	
		self.OptionParser.add_option("",   "--arrangement-material-width",	action="store", type="float",		dest="arrangement_material_width", default=500,		help="Materials width for arrangement")		
		self.OptionParser.add_option("",   "--arrangement-population-count",action="store", type="int",			dest="arrangement_population_count", default=100,	help="Genetic algorithm populations count")		
		self.OptionParser.add_option("",   "--arrangement-inline-test",		action="store", type="inkbool", 	dest="arrangement_inline_test", default=False,	help="Use C-inline test (some additional packets will be needed)")


		self.OptionParser.add_option("",   "--postprocessor",				action="store", type="string", 		dest="postprocessor", default='',			help="Postprocessor command.")
		self.OptionParser.add_option("",   "--postprocessor-custom",		action="store", type="string", 		dest="postprocessor_custom", default='',	help="Postprocessor custom command.")
	
		self.OptionParser.add_option("",   "--graffiti-max-seg-length",		action="store", type="float", 		dest="graffiti_max_seg_length", default=1.,	help="Graffiti maximum segment length.")
		self.OptionParser.add_option("",   "--graffiti-min-radius",			action="store", type="float", 		dest="graffiti_min_radius", default=10.,	help="Graffiti minimal connector's radius.")
		self.OptionParser.add_option("",   "--graffiti-start-pos",			action="store", type="string", 		dest="graffiti_start_pos", default="(0;0)",	help="Graffiti Start position (x;y).")
		self.OptionParser.add_option("",   "--graffiti-create-linearization-preview",	action="store", type="inkbool", 	dest="graffiti_create_linearization_preview", default=True,	help="Graffiti create linearization preview.")
		self.OptionParser.add_option("",   "--graffiti-create-preview",		action="store", type="inkbool", 	dest="graffiti_create_preview", default=True,	help="Graffiti create preview.")
		self.OptionParser.add_option("",   "--graffiti-preview-size",		action="store", type="int", 		dest="graffiti_preview_size", default=800,	help="Graffiti preview's size.")
		self.OptionParser.add_option("",   "--graffiti-preview-emmit",		action="store", type="int", 		dest="graffiti_preview_emmit", default=800,	help="Preview's paint emmit (pts/s).")


		self.OptionParser.add_option("",   "--in-out-path",					action="store", type="inkbool", 	dest="in_out_path",	default=True,			help="Create in-out paths")
		self.OptionParser.add_option("",   "--in-out-path-do-not-add-reference-point",	action="store", type="inkbool", dest="in_out_path_do_not_add_reference_point", default=False,	help="Just add reference in-out point")
		self.OptionParser.add_option("",   "--in-out-path-point-max-dist",	action="store", type="float", 		dest="in_out_path_point_max_dist", default=10.,	help="In-out path max distance to reference point")
		self.OptionParser.add_option("",   "--in-out-path-type",			action="store", type="string", 		dest="in_out_path_type", default="Round",	help="In-out path type")
		self.OptionParser.add_option("",   "--in-out-path-len",				action="store", type="float", 		dest="in_out_path_len", default=10.,		help="In-out path length")
		self.OptionParser.add_option("",   "--in-out-path-replace-original-path",action="store", type="inkbool", dest="in_out_path_replace_original_path", default=False,	help="Replace original path")
		self.OptionParser.add_option("",   "--in-out-path-radius",			action="store", type="float", 		dest="in_out_path_radius", default=10.,		help="In-out path radius for round path")

		self.OptionParser.add_option("",   "--plasma-prepare-corners",		action="store", type="inkbool",		dest="plasma_prepare_corners", default=True,	help="Prepare corners")
		self.OptionParser.add_option("",   "--plasma-prepare-corners-distance", action="store", type="float",	dest="plasma_prepare_corners_distance", default=10.,help="Stepout distance for corners")
		self.OptionParser.add_option("",   "--plasma-prepare-corners-tolerance", action="store", type="float",	dest="plasma_prepare_corners_tolerance", default=10.,help="Maximum angle for corner (0-180 deg)")

		self.default_tool = {
					"name": "Default tool",
					"id": "default tool",
					"diameter":10.,
					"shape": "10",
					"penetration angle":90.,
					"penetration feed":100.,
					"depth step":1.,
					"feed":400.,
					"in trajectotry":"",
					"out trajectotry":"",
					"gcode before path":"",
					"gcode after path":"",
					"sog":"",
					"spinlde rpm":"",
					"CW or CCW":"",
					"tool change gcode":" ",
					"4th axis meaning": " ",
					"4th axis scale": 1.,
					"4th axis offset": 0.,
					"passing feed":"800",					
					"fine feed":"800",					
				}			
		self.tools_field_order = [
					'name',
					'id',
					'diameter',
					'feed',
					'shape',
					'penetration angle',
					'penetration feed',
					"passing feed",
					'depth step',
					"in trajectotry",
					"out trajectotry",
					"gcode before path",
					"gcode after path",
					"sog",
					"spinlde rpm",
					"CW or CCW",
					"tool change gcode",
				]


	def parse_curve(self, p, layer, w = None, f = None):
			c = []
			if len(p)==0 : 
				return []
			p = self.transform_csp(p, layer)
			

			### Sort to reduce Rapid distance	
			k = range(1,len(p))
			keys = [0]
			while len(k)>0:
				end = p[keys[-1]][-1][1]
				dist = None
				for i in range(len(k)):
					start = p[k[i]][0][1]
					dist = max(   ( -( ( end[0]-start[0])**2+(end[1]-start[1])**2 ) ,i)	,   dist )
				keys += [k[dist[1]]]
				del k[dist[1]]
			for k in keys:
				subpath = p[k]
				c += [ [	[subpath[0][1][0],subpath[0][1][1]]   , 'move', 0, 0] ]
				for i in range(1,len(subpath)):
					sp1 = [  [subpath[i-1][j][0], subpath[i-1][j][1]] for j in range(3)]
					sp2 = [  [subpath[i  ][j][0], subpath[i  ][j][1]] for j in range(3)]
					c += biarc(sp1,sp2,0,0) if w==None else biarc(sp1,sp2,-f(w[k][i-1]),-f(w[k][i]))
#					l1 = biarc(sp1,sp2,0,0) if w==None else biarc(sp1,sp2,-f(w[k][i-1]),-f(w[k][i]))
#					print_((-f(w[k][i-1]),-f(w[k][i]), [i1[5] for i1 in l1]) )
				c += [ [ [subpath[-1][1][0],subpath[-1][1][1]]  ,'end',0,0] ]
			return c


################################################################################
### 	Draw csp 
################################################################################

	def draw_csp(self, csp, layer=None, group=None, fill='none', stroke='#178ade', width=0.354, style=None):
		if layer!=None :
			csp = self.transform_csp(csp,layer,reverse=True)
		if group==None and layer==None:
			group = self.document.getroot()
		elif group==None and layer!=None :
			group = layer
		csp = self.apply_transforms(group,csp, reverse=True)
		if style!=None :
			return draw_csp(csp, group=group, style=style)
		else :
			return draw_csp(csp, group=group, fill=fill, stroke=stroke, width=width)
				



	def draw_curve(self, curve, layer, group=None, style=styles["biarc_style"]):
		self.set_markers()

		for i in [0,1]:
			style['biarc%s_r'%i] = simplestyle.parseStyle(style['biarc%s'%i])
			style['biarc%s_r'%i]["marker-start"] = "url(#DrawCurveMarker_r)"
			del(style['biarc%s_r'%i]["marker-end"])
			style['biarc%s_r'%i] = simplestyle.formatStyle(style['biarc%s_r'%i])
		
		if group==None:
			if "preview_groups" not in dir(self) :
				self.preview_groups = { layer: inkex.etree.SubElement( self.layers[min(1,len(self.layers)-1)], inkex.addNS('g','svg'), {"gcodetools": "Preview group"} ) }
			elif layer not in self.preview_groups :
				self.preview_groups[layer] = inkex.etree.SubElement( self.layers[min(1,len(self.layers)-1)], inkex.addNS('g','svg'), {"gcodetools": "Preview group"} )
			group = self.preview_groups[layer]

		s, arcn = '', 0
		
		transform = self.get_transforms(group)
		if transform != [] : 
			transform = self.reverse_transform(transform)
			transform = simpletransform.formatTransform(transform)
		
		a,b,c = [0.,0.], [1.,0.], [0.,1.]
		k = (b[0]-a[0])*(c[1]-a[1])-(c[0]-a[0])*(b[1]-a[1])
		a,b,c = self.transform(a, layer, True), self.transform(b, layer, True), self.transform(c, layer, True)
		if ((b[0]-a[0])*(c[1]-a[1])-(c[0]-a[0])*(b[1]-a[1]))*k > 0 : reverse_angle = 1
		else : reverse_angle = -1 
		for sk in curve:
			si = sk[:]
			si[0], si[2] = self.transform(si[0], layer, True), (self.transform(si[2], layer, True) if type(si[2])==type([]) and len(si[2])==2 else si[2])
			
			if s!='':
				if s[1] == 'line':
					attr = {	'style': style['line'],
								'd':'M %s,%s L %s,%s' % (s[0][0], s[0][1], si[0][0], si[0][1]),
								"gcodetools": "Preview",
							}
					if transform != [] :
						attr["transform"] = transform		
					inkex.etree.SubElement(	group, inkex.addNS('path','svg'),  attr	)
				elif s[1] == 'arc':
					arcn += 1
					sp = s[0]
					c = s[2]
					s[3] = s[3]*reverse_angle
						
					a =  ( (P(si[0])-P(c)).angle() - (P(s[0])-P(c)).angle() )%math.pi2 #s[3]
					if s[3]*a<0: 
							if a>0:	a = a-math.pi2
							else: a = math.pi2+a
					r = math.sqrt( (sp[0]-c[0])**2 + (sp[1]-c[1])**2 )
					a_st = ( math.atan2(sp[0]-c[0],- (sp[1]-c[1])) - math.pi/2 ) % (math.pi*2)
					st = style['biarc%s' % (arcn%2)][:]
					if a>0:
						a_end = a_st+a
						st = style['biarc%s'%(arcn%2)]
					else: 
						a_end = a_st*1
						a_st = a_st+a
						st = style['biarc%s_r'%(arcn%2)]
					
					attr = {
							'style': st,
							 inkex.addNS('cx','sodipodi'):		str(c[0]),
							 inkex.addNS('cy','sodipodi'):		str(c[1]),
							 inkex.addNS('rx','sodipodi'):		str(r),
							 inkex.addNS('ry','sodipodi'):		str(r),
							 inkex.addNS('start','sodipodi'):	str(a_st),
							 inkex.addNS('end','sodipodi'):		str(a_end),
							 inkex.addNS('open','sodipodi'):	'true',
							 inkex.addNS('type','sodipodi'):	'arc',
							 "gcodetools": "Preview",
							}	
						
					if transform != [] :
						attr["transform"] = transform	
					inkex.etree.SubElement(	group, inkex.addNS('path','svg'), attr)
			s = si
	

	def check_dir(self):
		if self.options.directory[-1] not in ["/","\\"]:
			if "\\" in self.options.directory :
				self.options.directory += "\\"
			else :
				self.options.directory += "/"
		print_("Checking directory: '%s'"%self.options.directory)
		if (os.path.isdir(self.options.directory)):
			if (os.path.isfile(self.options.directory+'header')):
				f = open(self.options.directory+'header', 'r')
				self.header = f.read()
				f.close()
			else:
				self.header = defaults['header']
			if (os.path.isfile(self.options.directory+'footer')):
				f = open(self.options.directory+'footer','r')
				self.footer = f.read()
				f.close()
			else:
				self.footer = defaults['footer']
			self.header += self.options.unit + "\n" 
		else: 
			self.error(_("Directory does not exist! Please specify existing directory at Preferences tab!"),"error")
			return False

		if self.options.add_numeric_suffix_to_filename :
			dir_list = os.listdir(self.options.directory)
			if "." in self.options.file : 
				r = re.match(r"^(.*)(\..*)$",self.options.file)
				ext = r.group(2)
				name = r.group(1)
			else: 	
				ext = ""
				name = self.options.file
			max_n = 0
			for s in dir_list :
				r = re.match(r"^%s_0*(\d+)%s$"%(re.escape(name),re.escape(ext) ), s)
				if r :
					max_n = max(max_n,int(r.group(1)))
			filename = name + "_" + ( "0"*(4-len(str(max_n+1))) + str(max_n+1) ) + ext
			self.options.file = filename

		if self.options.directory[-1] not in ["/","\\"]:
			if "\\" in self.options.directory :
				self.options.directory += "\\"
			else :
				self.options.directory += "/"

		try: 	
			f = open(self.options.directory+self.options.file, "w")	
			f.close()							
		except:
			self.error(_("Can not write to specified file!\n%s"%(self.options.directory+self.options.file)),"error")
			return False
		return True
			


################################################################################
###
###		Generate Gcode
###		Generates Gcode on given curve.
###
###		Curve definition [start point, type = {'arc','line','move','end'}, arc center, arc angle, end point, [zstart, zend]]		
###
################################################################################
	def generate_gcode(self, curve, layer, depth):
		Zauto_scale = self.Zauto_scale[layer]
		tool = self.tools[layer][0]
		g = ""

		def c(c):
			c = [c[i] if i<len(c) else None for i in range(6)]
			if c[5] == 0 : c[5]=None
			s,s1 = [" X", " Y", " Z", " I", " J", " K"], ["","","","","",""]
			m,a = [1,1,self.options.Zscale*Zauto_scale,1,1,self.options.Zscale*Zauto_scale], [0,0,self.options.Zoffset,0,0,0]
			r = ''	
			for i in range(6):
				if c[i]!=None:
					r += s[i] + ("%f" % (c[i]*m[i]+a[i])) + s1[i]
			return r

		def calculate_angle(a, current_a):
			return  min(					
						[abs(a-current_a%math.pi2+math.pi2), a+current_a-current_a%math.pi2+math.pi2],
						[abs(a-current_a%math.pi2-math.pi2), a+current_a-current_a%math.pi2-math.pi2],
						[abs(a-current_a%math.pi2),			 a+current_a-current_a%math.pi2])[1]
		if len(curve)==0 : return ""	
				
		try :
			self.last_used_tool == None
		except :
			self.last_used_tool = None
		print_("working on curve")
		print_(curve)
		
		if tool != self.last_used_tool :
			g += ( "(Change tool to %s)\n" % re.sub("\"'\(\)\\\\"," ",tool["name"]) ) + tool["tool change gcode"] + "\n"

		lg, zs, f =  'G00', self.options.Zsafe, " F%f"%tool['feed'] 
		current_a = 0
		go_to_safe_distance = "G00" + c([None,None,zs]) + "\n" 
		penetration_feed = " F%s"%tool['penetration feed'] 
		for i in range(1,len(curve)):
		#	Creating Gcode for curve between s=curve[i-1] and si=curve[i] start at s[0] end at s[4]=si[0]
			s, si = curve[i-1], curve[i]
			feed = f if lg not in ['G01','G02','G03'] else ''
			if s[1]	== 'move':
				g += go_to_safe_distance + "G00" + c(si[0]) + "\n" + tool['gcode before path'] + "\n"
				lg = 'G00'
			elif s[1] == 'end':
				g += go_to_safe_distance + tool['gcode after path'] + "\n"
				lg = 'G00'
			elif s[1] == 'line':
				if tool['4th axis meaning'] == "tangent knife" : 
					a = atan2(si[0][0]-s[0][0],si[0][1]-s[0][1])
					a = calculate_angle(a, current_a)
					g+="G01 A%s\n" % (a*tool['4th axis scale']+tool['4th axis offset'])
					current_a = a
				if lg=="G00": g += "G01" + c([None,None,s[5][0]+depth]) + penetration_feed +"(Penetrate)\n"	
				g += "G01" +c(si[0]+[s[5][1]+depth]) + feed + "\n"
				lg = 'G01'
			elif s[1] == 'arc':
				r = [(s[2][0]-s[0][0]), (s[2][1]-s[0][1])]
				if tool['4th axis meaning'] == "tangent knife" : 
					if s[3]<0 : # CW
						a1 = atan2(s[2][1]-s[0][1],-s[2][0]+s[0][0]) + math.pi 
					else: #CCW
						a1 = atan2(-s[2][1]+s[0][1],s[2][0]-s[0][0]) + math.pi
					a = calculate_angle(a1, current_a)
					g+="G01 A%s\n" % (a*tool['4th axis scale']+tool['4th axis offset'])
					current_a = a
					axis4 = " A%s"%((current_a+s[3])*tool['4th axis scale']+tool['4th axis offset'])
					current_a = current_a+s[3]
				else : axis4 = ""
				if lg=="G00": g += "G01" + c([None,None,s[5][0]+depth]) + penetration_feed + "(Penetrate)\n"				
				if (r[0]**2 + r[1]**2)>self.options.min_arc_radius**2:
					r1, r2 = (P(s[0])-P(s[2])), (P(si[0])-P(s[2]))
					if abs(r1.mag()-r2.mag()) < 0.001 :
						g += ("G02" if s[3]<0 else "G03") + c(si[0]+[ s[5][1]+depth, (s[2][0]-s[0][0]),(s[2][1]-s[0][1])  ]) + feed + axis4 + "\n"
					else:
						r = (r1.mag()+r2.mag())/2
						g += ("G02" if s[3]<0 else "G03") + c(si[0]+[s[5][1]+depth]) + " R%f" % (r) + feed  + axis4 + "\n"
					lg = 'G02'
				else:
					if tool['4th axis meaning'] == "tangent knife" : 
						a = atan2(si[0][0]-s[0][0],si[0][1]-s[0][1]) + math.pi
						a = calculate_angle(a, current_a)
						g+="G01 A%s\n" % (a*tool['4th axis scale']+tool['4th axis offset'])
						current_a = a
					g += "G01" +c(si[0]+[s[5][1]+depth]) + feed + "\n"
					lg = 'G01'
		if si[1] == 'end':
			g += go_to_safe_distance + tool['gcode after path'] + "\n"
		return g


	def get_transforms(self,g):
		root = self.document.getroot()
		trans = []
		while (g!=root):
			if 'transform' in g.keys():
				t = g.get('transform')
				t = simpletransform.parseTransform(t)
				trans = simpletransform.composeTransform(t,trans) if trans != [] else t
				print_(trans)
			g=g.getparent()
		return trans 
	
	def reverse_transform(self,transform):
		trans = numpy.array(transform + [[0,0,1]])
		if numpy.linalg.det(trans)!=0 :
			trans = numpy.linalg.inv(trans).tolist()[:2]		
			return trans
		else :
		 return transform
		

	def apply_transforms(self,g,csp, reverse=False):
		trans = self.get_transforms(g)
		if trans != []:
			if not reverse :
				simpletransform.applyTransformToPath(trans, csp)
			else :
				simpletransform.applyTransformToPath(self.reverse_transform(trans), csp)
		return csp
		
		

	def transform_scalar(self,x,layer,reverse=False):
		return self.transform([x,0],layer,reverse)[0] - self.transform([0,0],layer,reverse)[0]

	def transform(self,source_point, layer, reverse=False):
		if layer not in self.transform_matrix:
			for i in range(self.layers.index(layer),-1,-1):
				if self.layers[i] in self.orientation_points : 
					break
			if self.layers[i] not in self.orientation_points :
				self.error(_("Orientation points for '%s' layer have not been found! Please add orientation points using Orientation tab!") % layer.get(inkex.addNS('label','inkscape')),"no_orientation_points")
			elif self.layers[i] in self.transform_matrix :
				self.transform_matrix[layer] = self.transform_matrix[self.layers[i]]
				self.Zcoordinates[layer] = self.Zcoordinates[self.layers[i]]
			else :
				orientation_layer = self.layers[i]
				if len(self.orientation_points[orientation_layer])>1 : 
					self.error(_("There are more than one orientation point groups in '%s' layer") % orientation_layer.get(inkex.addNS('label','inkscape')),"more_than_one_orientation_point_groups")
				points = self.orientation_points[orientation_layer][0]
				if len(points)==2:
					points += [ [ [(points[1][0][1]-points[0][0][1])+points[0][0][0], -(points[1][0][0]-points[0][0][0])+points[0][0][1]], [-(points[1][1][1]-points[0][1][1])+points[0][1][0], points[1][1][0]-points[0][1][0]+points[0][1][1]] ] ]
				if len(points)==3:
					print_("Layer '%s' Orientation points: " % orientation_layer.get(inkex.addNS('label','inkscape')))
					for point in points:
						print_(point)
					#	Zcoordinates definition taken from Orientatnion point 1 and 2 
					self.Zcoordinates[layer] = [max(points[0][1][2],points[1][1][2]), min(points[0][1][2],points[1][1][2])]
					matrix = numpy.array([
								[points[0][0][0], points[0][0][1], 1, 0, 0, 0, 0, 0, 0],
								[0, 0, 0, points[0][0][0], points[0][0][1], 1, 0, 0, 0],
								[0, 0, 0, 0, 0, 0, points[0][0][0], points[0][0][1], 1],
								[points[1][0][0], points[1][0][1], 1, 0, 0, 0, 0, 0, 0],
								[0, 0, 0, points[1][0][0], points[1][0][1], 1, 0, 0, 0],
								[0, 0, 0, 0, 0, 0, points[1][0][0], points[1][0][1], 1],
								[points[2][0][0], points[2][0][1], 1, 0, 0, 0, 0, 0, 0],
								[0, 0, 0, points[2][0][0], points[2][0][1], 1, 0, 0, 0],
								[0, 0, 0, 0, 0, 0, points[2][0][0], points[2][0][1], 1]
							])
								
					if numpy.linalg.det(matrix)!=0 :
						m = numpy.linalg.solve(matrix,
							numpy.array(
								[[points[0][1][0]], [points[0][1][1]], [1], [points[1][1][0]], [points[1][1][1]], [1], [points[2][1][0]], [points[2][1][1]], [1]]	
										)
							).tolist()
						self.transform_matrix[layer] = [[m[j*3+i][0] for i in range(3)] for j in range(3)]
					
					else :
						self.error(_("Orientation points are wrong! (if there are two orientation points they should not be the same. If there are three orientation points they should not be in a straight line.)"),"wrong_orientation_points")
				else :
					self.error(_("Orientation points are wrong! (if there are two orientation points they should not be the same. If there are three orientation points they should not be in a straight line.)"),"wrong_orientation_points")

			self.transform_matrix_reverse[layer] = numpy.linalg.inv(self.transform_matrix[layer]).tolist()		
			print_("\n Layer '%s' transformation matrixes:" % layer.get(inkex.addNS('label','inkscape')) )
			print_(self.transform_matrix)
			print_(self.transform_matrix_reverse)

			###self.Zauto_scale[layer]  = math.sqrt( (self.transform_matrix[layer][0][0]**2 + self.transform_matrix[layer][1][1]**2)/2 )
			### Zautoscale is absolete
			self.Zauto_scale[layer] = 1
			print_("Z automatic scale = %s (computed according orientation points)" % self.Zauto_scale[layer])

		x,y = source_point[0], source_point[1]
		if not reverse :
			t = self.transform_matrix[layer]
		else :
			t = self.transform_matrix_reverse[layer]
		return [t[0][0]*x+t[0][1]*y+t[0][2], t[1][0]*x+t[1][1]*y+t[1][2]]


	def transform_csp(self, csp_, layer, reverse = False):
		csp = [  [ [csp_[i][j][0][:],csp_[i][j][1][:],csp_[i][j][2][:]]  for j in range(len(csp_[i])) ]   for i in range(len(csp_)) ]
		for i in xrange(len(csp)):
			for j in xrange(len(csp[i])): 
				for k in xrange(len(csp[i][j])): 
					csp[i][j][k] = self.transform(csp[i][j][k],layer, reverse)
		return csp
	
		
################################################################################
###		Errors handling function, notes are just printed into Logfile, 
###		warnings are printed into log file and warning message is displayed but
###		extension continues working, errors causes log and execution is halted
###		Notes, warnings adn errors could be assigned to space or comma or dot 
###		sepparated strings (case is ignoreg).
################################################################################
	def error(self, s, type_= "Warning"):
		notes = "Note "
		warnings = """
						Warning tools_warning
						orientation_warning
						bad_orientation_points_in_some_layers
						more_than_one_orientation_point_groups
						more_than_one_tool
						orientation_have_not_been_defined
						tool_have_not_been_defined
						selection_does_not_contain_paths
						selection_does_not_contain_paths_will_take_all
						selection_is_empty_will_comupe_drawing
						selection_contains_objects_that_are_not_paths
						Continue
						"""
		errors = """
						Error 	
						wrong_orientation_points	
						area_tools_diameter_error
						no_tool_error
						active_layer_already_has_tool
						active_layer_already_has_orientation_points
					"""
		s = str(s)
		if type_.lower() in re.split("[\s\n,\.]+", errors.lower()) :
			print_(s)
			inkex.errormsg(s+"\n")		
			sys.exit()
		elif type_.lower() in re.split("[\s\n,\.]+", warnings.lower()) :
			print_(s)
			inkex.errormsg(s+"\n")		
		elif type_.lower() in re.split("[\s\n,\.]+", notes.lower()) :
			print_(s)
		else :
			print_(s)
			inkex.errormsg(s)		
			sys.exit()
	

################################################################################
###		Set markers
################################################################################
	def set_markers(self) :
		self.get_defs()
		# Add marker to defs if it doesnot exists
		if "CheckToolsAndOPMarker" not in self.defs : 
			defs = inkex.etree.SubElement( self.document.getroot(), inkex.addNS("defs","svg"))
			marker = inkex.etree.SubElement( defs, inkex.addNS("marker","svg"), {"id":"CheckToolsAndOPMarker","orient":"auto","refX":"-4","refY":"-1.687441","style":"overflow:visible"})
			inkex.etree.SubElement( marker, inkex.addNS("path","svg"), 
	
					{	"d":"	m -4.588864,-1.687441 0.0,0.0 L -9.177728,0.0 c 0.73311,-0.996261 0.728882,-2.359329 0.0,-3.374882",
						"style": "fill:#000044; fill-rule:evenodd;stroke:none;"	}
				)

		if "DrawCurveMarker" not in self.defs : 
			defs = inkex.etree.SubElement( self.document.getroot(), inkex.addNS("defs","svg"))
			marker = inkex.etree.SubElement( defs, inkex.addNS("marker","svg"), {"id":"DrawCurveMarker","orient":"auto","refX":"-4","refY":"-1.687441","style":"overflow:visible"})
			inkex.etree.SubElement( marker, inkex.addNS("path","svg"), 
					{	"d":"m -4.588864,-1.687441 0.0,0.0 L -9.177728,0.0 c 0.73311,-0.996261 0.728882,-2.359329 0.0,-3.374882",
						"style": "fill:#000044; fill-rule:evenodd;stroke:none;"	}
				)

		if "DrawCurveMarker_r" not in self.defs : 
			defs = inkex.etree.SubElement( self.document.getroot(), inkex.addNS("defs","svg"))
			marker = inkex.etree.SubElement( defs, inkex.addNS("marker","svg"), {"id":"DrawCurveMarker_r","orient":"auto","refX":"4","refY":"-1.687441","style":"overflow:visible"})
			inkex.etree.SubElement( marker, inkex.addNS("path","svg"), 
					{	"d":"m 4.588864,-1.687441 0.0,0.0 L 9.177728,0.0 c -0.73311,-0.996261 -0.728882,-2.359329 0.0,-3.374882",
						"style": "fill:#000044; fill-rule:evenodd;stroke:none;"	}
				)

		if "InOutPathMarker" not in self.defs : 
			defs = inkex.etree.SubElement( self.document.getroot(), inkex.addNS("defs","svg"))
			marker = inkex.etree.SubElement( defs, inkex.addNS("marker","svg"), {"id":"InOutPathMarker","orient":"auto","refX":"-4","refY":"-1.687441","style":"overflow:visible"})
			inkex.etree.SubElement( marker, inkex.addNS("path","svg"), 
					{	"d":"m -4.588864,-1.687441 0.0,0.0 L -9.177728,0.0 c 0.73311,-0.996261 0.728882,-2.359329 0.0,-3.374882",
						"style": "fill:#0072a7; fill-rule:evenodd;stroke:none;"	}
				)


	
################################################################################
###		Get defs from svg
################################################################################
	def get_defs(self) :
		self.defs = {}
		def recursive(g) :
			for i in g:
				if i.tag == inkex.addNS("defs","svg") : 
					for j in i: 
						self.defs[j.get("id")] = i
				if i.tag ==inkex.addNS("g",'svg') :
					recursive(i)
		recursive(self.document.getroot())


################################################################################
###
###		Get Gcodetools info from the svg
###
################################################################################
	def get_info(self):
		self.selected_paths = {}
		self.paths = {}		
		self.tools = {}
		self.orientation_points = {}
		self.graffiti_reference_points = {}
		self.layers = [self.document.getroot()]
		self.Zcoordinates = {}
		self.transform_matrix = {}
		self.transform_matrix_reverse = {}
		self.Zauto_scale = {}
		self.in_out_reference_points = []
		self.my3Dlayer = None

		def recursive_search(g, layer, selected=False):
			items = g.getchildren()
			items.reverse()
			for i in items:
				if selected:
					self.selected[i.get("id")] = i
				if i.tag == inkex.addNS("g",'svg') and i.get(inkex.addNS('groupmode','inkscape')) == 'layer':
					if i.get(inkex.addNS('label','inkscape')) == '3D' :
						self.my3Dlayer=i
					else :
						self.layers += [i]
						recursive_search(i,i)

				elif i.get('gcodetools') == "Gcodetools orientation group" :
					points = self.get_orientation_points(i)
					if points != None :
						self.orientation_points[layer] = self.orientation_points[layer]+[points[:]] if layer in self.orientation_points else [points[:]]
						print_("Found orientation points in '%s' layer: %s" % (layer.get(inkex.addNS('label','inkscape')), points))
					else :
						self.error(_("Warning! Found bad orientation points in '%s' layer. Resulting Gcode could be corrupt!") % layer.get(inkex.addNS('label','inkscape')), "bad_orientation_points_in_some_layers") 

				#Need to recognise old files ver 1.6.04 and earlier
				elif i.get("gcodetools") == "Gcodetools tool definition" or i.get("gcodetools") == "Gcodetools tool defenition"  :
					tool = self.get_tool(i)
					self.tools[layer] = self.tools[layer] + [tool.copy()] if layer in self.tools else [tool.copy()]
					print_("Found tool in '%s' layer: %s" % (layer.get(inkex.addNS('label','inkscape')), tool))

				elif i.get("gcodetools") == "Gcodetools graffiti reference point" :
					point = self.get_graffiti_reference_points(i)
					if point != [] :
						self.graffiti_reference_points[layer] = self.graffiti_reference_points[layer]+[point[:]] if layer in self.graffiti_reference_points else [point]
					else :
						self.error(_("Warning! Found bad graffiti reference point in '%s' layer. Resulting Gcode could be corrupt!") % layer.get(inkex.addNS('label','inkscape')), "bad_orientation_points_in_some_layers") 
				
				elif i.tag == inkex.addNS('path','svg'):
					if "gcodetools"  not in i.keys() :
						self.paths[layer] = self.paths[layer] + [i] if layer in self.paths else [i]  
						if i.get("id") in self.selected :
							self.selected_paths[layer] = self.selected_paths[layer] + [i] if layer in self.selected_paths else [i]  

				elif i.get("gcodetools") == "In-out reference point group" :
					items_ = i.getchildren()
					items_.reverse()
					for j in items_ :
						if j.get("gcodetools") == "In-out reference point" :
							self.in_out_reference_points.append( self.apply_transforms(j,cubicsuperpath.parsePath(j.get("d")))[0][0][1] )


				elif i.tag == inkex.addNS("g",'svg'):
					recursive_search(i,layer, (i.get("id") in self.selected) )

				elif i.get("id") in self.selected :
# xgettext:no-pango-format
					self.error(_("This extension works with Paths and Dynamic Offsets and groups of them only! All other objects will be ignored!\nSolution 1: press Path->Object to path or Shift+Ctrl+C.\nSolution 2: Path->Dynamic offset or Ctrl+J.\nSolution 3: export all contours to PostScript level 2 (File->Save As->.ps) and File->Import this file."),"selection_contains_objects_that_are_not_paths")
				
					
		recursive_search(self.document.getroot(),self.document.getroot())

		if len(self.layers) == 1 : 
			self.error(_("Document has no layers! Add at least one layer using layers panel (Ctrl+Shift+L)"),"Error")
		root = self.document.getroot() 

		if  root in self.selected_paths or root in self.paths :
			self.error(_("Warning! There are some paths in the root of the document, but not in any layer! Using bottom-most layer for them."), "tools_warning" )

		if  root in self.selected_paths :
			if self.layers[-1] in self.selected_paths :
				self.selected_paths[self.layers[-1]] += self.selected_paths[root][:]
			else :	
				self.selected_paths[self.layers[-1]] = self.selected_paths[root][:]
			del self.selected_paths[root]
			
		if root in self.paths :	
			if self.layers[-1] in self.paths :
				self.paths[self.layers[-1]] += self.paths[root][:]
			else :
				self.paths[self.layers[-1]] = self.paths[root][:]
			del self.paths[root]


	def get_orientation_points(self,g):
		items = g.getchildren()
		items.reverse()
		p2, p3 = [], []
		p = None
		for i in items:
			if i.tag == inkex.addNS("g",'svg') and i.get("gcodetools") == "Gcodetools orientation point (2 points)":
				p2 += [i]
			if i.tag == inkex.addNS("g",'svg') and i.get("gcodetools") == "Gcodetools orientation point (3 points)":
				p3 += [i]
		if len(p2)==2 : p=p2 
		elif len(p3)==3 : p=p3 
		if p==None : return None
		points = []
		for i in p :	
			point = [[],[]]	
			for  node in i :
				if node.get('gcodetools') == "Gcodetools orientation point arrow":
					point[0] = self.apply_transforms(node,cubicsuperpath.parsePath(node.get("d")))[0][0][1]
				if node.get('gcodetools') == "Gcodetools orientation point text":
					r = re.match(r'(?i)\s*\(\s*(-?\s*\d*(?:,|\.)*\d*)\s*;\s*(-?\s*\d*(?:,|\.)*\d*)\s*;\s*(-?\s*\d*(?:,|\.)*\d*)\s*\)\s*',get_text(node))
					point[1] = [float(r.group(1)),float(r.group(2)),float(r.group(3))]
			if point[0]!=[] and point[1]!=[]:	points += [point]
		if len(points)==len(p2)==2 or len(points)==len(p3)==3 : return points
		else : return None
	
	def get_graffiti_reference_points(self,g):
			point = [[], '']
			for node in g :
				if node.get('gcodetools') == "Gcodetools graffiti reference point arrow":
					point[0] = self.apply_transforms(node,cubicsuperpath.parsePath(node.get("d")))[0][0][1]
				if node.get('gcodetools') == "Gcodetools graffiti reference point text":
					point[1] = get_text(node)
			if point[0]!=[] and point[1]!='' : return point
			else : return []

	def get_tool(self, g):
		tool = self.default_tool.copy()
		tool["self_group"] = g 
		for i in g:
			#	Get parameters
			if i.get("gcodetools") == "Gcodetools tool background" : 
				tool["style"] = simplestyle.parseStyle(i.get("style"))
			elif i.get("gcodetools") == "Gcodetools tool parameter" :
				key = None
				value = None
				for j in i:
					#need to recognise old tools from ver 1.6.04
					if j.get("gcodetools") == "Gcodetools tool definition field name" or j.get("gcodetools") == "Gcodetools tool defention field name":
						key = get_text(j)
					if j.get("gcodetools") == "Gcodetools tool definition field value" or j.get("gcodetools") == "Gcodetools tool defention field value":
						value = get_text(j)
						if value == "(None)": value = ""
				if value == None or key == None: continue
				#print_("Found tool parameter '%s':'%s'" % (key,value))
				if key in self.default_tool.keys() :
					 try :
						tool[key] = type(self.default_tool[key])(value)
					 except :
						tool[key] = self.default_tool[key]
						self.error(_("Warning! Tool's and default tool's parameter's (%s) types are not the same ( type('%s') != type('%s') ).") % (key, value, self.default_tool[key]), "tools_warning")
				else :
					tool[key] = value
					self.error(_("Warning! Tool has parameter that default tool has not ( '%s': '%s' ).") % (key, value), "tools_warning" )
		return tool
		
		
	def set_tool(self,layer):
#		print_(("index(layer)=",self.layers.index(layer),"set_tool():layer=",layer,"self.tools=",self.tools))
#		for l in self.layers:
#			print_(("l=",l))
		for i in range(self.layers.index(layer),-1,-1):
#			print_(("processing layer",i))
			if self.layers[i] in self.tools : 
				break
		if self.layers[i] in self.tools :
			if self.layers[i] != layer : self.tools[layer] = self.tools[self.layers[i]]
			if len(self.tools[layer])>1 : self.error(_("Layer '%s' contains more than one tool!") % self.layers[i].get(inkex.addNS('label','inkscape')), "more_than_one_tool")
			return self.tools[layer]
		else :
			self.error(_("Can not find tool for '%s' layer! Please add one with Tools library tab!") % layer.get(inkex.addNS('label','inkscape')), "no_tool_error")


################################################################################
###
###		Path to Gcode
###
################################################################################
	def path_to_gcode(self) :
		from functools import partial
		def get_boundaries(points):
			minx,miny,maxx,maxy=None,None,None,None
			out=[[],[],[],[]]
			for p in points:
				if minx==p[0]:
					out[0]+=[p]
				if minx==None or p[0]<minx: 
					minx=p[0]
					out[0]=[p]

				if miny==p[1]:
					out[1]+=[p]
				if miny==None or p[1]<miny: 
					miny=p[1]
					out[1]=[p]

				if maxx==p[0]:
					out[2]+=[p]
				if maxx==None or p[0]>maxx: 
					maxx=p[0]
					out[2]=[p]

				if maxy==p[1]:
					out[3]+=[p]
				if maxy==None or p[1]>maxy: 
					maxy=p[1]
					out[3]=[p]
			return out


		def remove_duplicates(points):
			i=0		
			out=[]
			for p in points:
				for j in xrange(i,len(points)):
					if p==points[j]: points[j]=[None,None]	
				if p!=[None,None]: out+=[p]
			i+=1
			return(out)
	
	
		def get_way_len(points):
			l=0
			for i in xrange(1,len(points)):
				l+=math.sqrt((points[i][0]-points[i-1][0])**2 + (points[i][1]-points[i-1][1])**2)
			return l

	
		def sort_dxfpoints(points):
			points=remove_duplicates(points)
#			print_(get_boundaries(get_boundaries(points)[2])[1])
			ways=[
						  # l=0, d=1, r=2, u=3
			 [3,0], # ul
			 [3,2], # ur
			 [1,0], # dl
			 [1,2], # dr
			 [0,3], # lu
			 [0,1], # ld
			 [2,3], # ru
			 [2,1], # rd
			]
#			print_(("points=",points))
			minimal_way=[]
			minimal_len=None
			minimal_way_type=None
			for w in ways:
				tpoints=points[:]
				cw=[]
#				print_(("tpoints=",tpoints))
				for j in xrange(0,len(points)):
					p=get_boundaries(get_boundaries(tpoints)[w[0]])[w[1]]
#					print_(p)
					tpoints.remove(p[0])
					cw+=p
				curlen = get_way_len(cw)
				if minimal_len==None or curlen < minimal_len: 
					minimal_len=curlen
					minimal_way=cw
					minimal_way_type=w
			
			return minimal_way

		def sort_lines(lines):
			if len(lines) == 0 : return []
			lines = [ [key]+lines[key] for key in range(len(lines))]			
			keys = [0]
			end_point = lines[0][3:]
			print_("!!!",lines,"\n",end_point)
			del lines[0]
			while len(lines)>0:
				dist = [ [point_to_point_d2(end_point,lines[i][1:3]),i] for i in range(len(lines))]
				i = min(dist)[1]
				keys.append(lines[i][0])
				end_point = lines[i][3:]
				del lines[i]
			return keys
			
		def sort_curves(curves):
			lines = []
			for curve in curves:
				lines += [curve[0][0][0] + curve[-1][-1][0]]
			return sort_lines(lines)
		
		def print_dxfpoints(points):
			gcode=""
			for point in points:
				gcode +="(drilling dxfpoint)\nG00 Z%f\nG00 X%f Y%f\nG01 Z%f F%f\nG04 P%f\nG00 Z%f\n" % (self.options.Zsafe,point[0],point[1],self.Zcoordinates[layer][1],self.tools[layer][0]["penetration feed"],0.2,self.options.Zsafe) 
#			print_(("got dxfpoints array=",points))
			return gcode
		
		def get_path_properties(node, recursive=True, tags={inkex.addNS('desc','svg'):"Description",inkex.addNS('title','svg'):"Title"} ) :
			res = {}
			done = False
			root = self.document.getroot()
			while not done and node != root :
				for i in node.getchildren():
					if i.tag in tags:
						res[tags[i.tag]] = i.text
					done = True	
				node =	node.getparent()
			return res

		if self.selected_paths == {} and self.options.auto_select_paths:
			paths=self.paths
			self.error(_("No paths are selected! Trying to work on all available paths."),"warning")
		else :
			paths = self.selected_paths
		self.check_dir() 
		gcode = ""
		
		biarc_group = inkex.etree.SubElement( self.selected_paths.keys()[0] if len(self.selected_paths.keys())>0 else self.layers[0], inkex.addNS('g','svg') )
		print_(("self.layers=",self.layers))
		print_(("paths=",paths))
		colors = {}
		for layer in self.layers :
			if layer in paths :
				print_(("layer",layer))
				# transform simple path to get all var about orientation
				self.transform_csp([ [ [[0,0],[0,0],[0,0]],  [[0,0],[0,0],[0,0]] ] ], layer)
			
				self.set_tool(layer)
				curves = []
				dxfpoints = []

				try :
					depth_func = eval('lambda c,d,s: ' + self.options.path_to_gcode_depth_function.strip('"'))				
				except:
					self.error("Bad depth function! Enter correct function at Path to Gcode tab!") 

				for path in paths[layer] :
					if "d" not in path.keys() : 
						self.error(_("Warning: One or more paths do not have 'd' parameter, try to Ungroup (Ctrl+Shift+G) and Object to Path (Ctrl+Shift+C)!"),"selection_contains_objects_that_are_not_paths")
						continue					
					csp = cubicsuperpath.parsePath(path.get("d"))
					csp = self.apply_transforms(path, csp)
					id_ = path.get("id")
					
					def set_comment(match, path):
						if match.group(1) in path.keys() :
							return path.get(match.group(1))
						else: 
							return "None"
					if self.options.comment_gcode != "" :
						comment = re.sub("\[([A-Za-z_\-\:]+)\]", partial(set_comment, path=path), self.options.comment_gcode)
						comment = comment.replace(":newline:","\n") 
						comment = gcode_comment_str(comment)
					else:
						comment = ""
					if self.options.comment_gcode_from_properties :
						tags = get_path_properties(path)
						for tag in tags :
							comment += gcode_comment_str("%s: %s"%(tag,tags[tag]))

					style = simplestyle.parseStyle(path.get("style"))
					colors[id_] = simplestyle.parseColor(style['stroke'] if "stroke"  in style and style['stroke']!='none' else "#000")
					if path.get("dxfpoint") == "1":
						tmp_curve=self.transform_csp(csp, layer)
						x=tmp_curve[0][0][0][0]
						y=tmp_curve[0][0][0][1]
						print_("got dxfpoint (scaled) at (%f,%f)" % (x,y))
						dxfpoints += [[x,y]]
					else:
						
						zd,zs = self.Zcoordinates[layer][1],	self.Zcoordinates[layer][0]
						c = 1. - float(sum(colors[id_]))/255/3
						curves += 	[
										 [  
										 	[id_, depth_func(c,zd,zs), comment],
										 	[ self.parse_curve([subpath], layer) for subpath in csp  ]
										 ]
									]
#				for c in curves : 
#					print_(c)
				dxfpoints=sort_dxfpoints(dxfpoints)
				gcode+=print_dxfpoints(dxfpoints)
				
				
				for curve in curves :
					for subcurve in curve[1] :
						self.draw_curve(subcurve, layer)
					
				if self.options.path_to_gcode_order == 'subpath by subpath':
					curves_ = []
					for curve in curves :
						curves_ += [ [curve[0],[subcurve]]  for subcurve in curve[1] ]  
					curves = curves_	

					self.options.path_to_gcode_order = 'path by path'
					
				if self.options.path_to_gcode_order == 'path by path':
					if self.options.path_to_gcode_sort_paths :
						keys = sort_curves( [curve[1] for curve in curves] )
					else :
						keys = range(len(curves))
					for key in keys:
						d = curves[key][0][1]
						for step in range( 0,  int(math.ceil( abs((zs-d)/self.tools[layer][0]["depth step"] )) ) ):
							z = max(d, zs - abs(self.tools[layer][0]["depth step"]*(step+1)))
							
							gcode += gcode_comment_str("\nStart cutting path id: %s"%curves[key][0][0])
							if curves[key][0][2] != "()" :
								gcode += curves[key][0][2] # add comment
								
							for curve in curves[key][1]:
								gcode += self.generate_gcode(curve, layer, z)
								
							gcode += gcode_comment_str("End cutting path id: %s\n\n"%curves[key][0][0])
							
				else:	# pass by pass
					mind = min( [curve[0][1] for curve in curves] )	
					for step in range( 0,  int(math.ceil( abs((zs-mind)/self.tools[layer][0]["depth step"] )) ) ):
						z = zs - abs(self.tools[layer][0]["depth step"]*(step))
						curves_ = []
						for curve in curves:
							if curve[0][1]<z : 
								curves_.append(curve)

						z = zs - abs(self.tools[layer][0]["depth step"]*(step+1))
						gcode += "\n(Pass at depth %s)\n"%z

						if self.options.path_to_gcode_sort_paths :
							keys = sort_curves( [curve[1] for curve in curves_] )		
						else :
							keys = range(len(curves_))
						for key in keys:
				
							gcode += gcode_comment_str("Start cutting path id: %s"%curves[key][0][0])
							if curves[key][0][2] != "()" :
								gcode += curves[key][0][2] # add comment

							for subcurve in curves_[key][1]:
								gcode += self.generate_gcode(subcurve, layer, max(z,curves_[key][0][1]))
				
							gcode += gcode_comment_str("End cutting path id: %s\n\n"%curves[key][0][0])

							
		self.export_gcode(gcode)
	
################################################################################
###
###		dxfpoints
###
################################################################################
	def dxfpoints(self):
		if self.selected_paths == {}:
			self.error(_("Nothing is selected. Please select something to convert to drill point (dxfpoint) or clear point sign."),"warning")
		for layer in self.layers :
			if layer in self.selected_paths :
				for path in self.selected_paths[layer]:
#					print_(("processing path",path.get('d')))
					if self.options.dxfpoints_action == 'replace':
#						print_("trying to set as dxfpoint")
						
						path.set("dxfpoint","1")
						r = re.match("^\s*.\s*(\S+)",path.get("d"))
						if r!=None:
							print_(("got path=",r.group(1)))
							path.set("d","m %s 2.9375,-6.343750000001 0.8125,1.90625 6.843748640396,-6.84374864039 0,0 0.6875,0.6875 -6.84375,6.84375 1.90625,0.812500000001 z" % r.group(1))
							path.set("style",styles["dxf_points"])

					if self.options.dxfpoints_action == 'save':
						path.set("dxfpoint","1")

					if self.options.dxfpoints_action == 'clear' and path.get("dxfpoint") == "1":
						path.set("dxfpoint","0")
#						for id, node in self.selected.iteritems():
#							print_((id,node,node.attrib))


################################################################################
###
###		Artefacts
###
################################################################################
	def area_artefacts(self) :
			if self.selected_paths == {} and self.options.auto_select_paths:
				paths=self.paths
				self.error(_("No paths are selected! Trying to work on all available paths."),"warning")
			else :
				paths = self.selected_paths
			for layer in paths :
#				paths[layer].reverse() # Reverse list of paths to leave their order 
				for path in paths[layer] :
					parent = path.getparent()
					style = path.get("style") if "style" in path.keys() else ""
					if "d" not in path.keys() : 
						self.error(_("Warning: One or more paths do not have 'd' parameter, try to Ungroup (Ctrl+Shift+G) and Object to Path (Ctrl+Shift+C)!"),"selection_contains_objects_that_are_not_paths")
						continue		
					csp = cubicsuperpath.parsePath(path.get("d"))
					remove = []
					for i in range(len(csp)) :
						subpath = [ [point[:] for point in points] for points in csp[i]]
						subpath = self.apply_transforms(path,[subpath])[0]
						bounds = csp_simple_bound([subpath])
						if  (bounds[2]-bounds[0])**2+(bounds[3]-bounds[1])**2 < self.options.area_find_artefacts_diameter**2:
							if self.options.area_find_artefacts_action == "mark with an arrow" :
								arrow =  cubicsuperpath.parsePath( 'm %s,%s 2.9375,-6.343750000001 0.8125,1.90625 6.843748640396,-6.84374864039 0,0 0.6875,0.6875 -6.84375,6.84375 1.90625,0.812500000001 z' % (subpath[0][1][0],subpath[0][1][1]) )
 								arrow = self.apply_transforms(path,arrow,True)
								inkex.etree.SubElement(parent, inkex.addNS('path','svg'), 
										{
											'd': cubicsuperpath.formatPath(arrow),
											'style': styles["area artefact arrow"],
											'gcodetools': 'area artefact arrow',
										})
							elif self.options.area_find_artefacts_action == "mark with style" :
								inkex.etree.SubElement(parent, inkex.addNS('path','svg'), {'d': cubicsuperpath.formatPath(csp[i]), 'style': styles["area artefact"]})
								remove.append(i)
							elif self.options.area_find_artefacts_action == "delete" :
								remove.append(i)
								print_("Deleted artefact %s" % subpath )
					remove.reverse()
					for i in remove :
						del csp[i]
					if len(csp) == 0 :	
						parent.remove(path)
					else :
						path.set("d", cubicsuperpath.formatPath(csp))
							
			return
			

################################################################################
###
###		Calculate area curves
###
################################################################################
	def area(self) :
		if len(self.selected_paths)<=0:
			self.error(_("This extension requires at least one selected path."),"warning")
			return
		for layer in self.layers :
			if layer in self.selected_paths :
				self.set_tool(layer)
				if self.tools[layer][0]['diameter']<=0 : 
					self.error(_("Tool diameter must be > 0 but tool's diameter on '%s' layer is not!") % layer.get(inkex.addNS('label','inkscape')),"area_tools_diameter_error")
	
				for path in self.selected_paths[layer]:
					print_(("doing path",	path.get("style"), path.get("d")))

					area_group = inkex.etree.SubElement( path.getparent(), inkex.addNS('g','svg') )
				
					d = path.get('d')
					print_(d)
					if d==None:
						print_("omitting non-path")
						self.error(_("Warning: omitting non-path"),"selection_contains_objects_that_are_not_paths")
						continue
					csp = cubicsuperpath.parsePath(d)
				
					if path.get(inkex.addNS('type','sodipodi'))!="inkscape:offset":
						print_("Path %s is not an offset. Preparation started." % path.get("id"))
						# Path is not offset. Preparation will be needed.
						# Finding top most point in path (min y value)
						
						min_x,min_y,min_i,min_j,min_t = csp_true_bounds(csp)[1]
						
						# Reverse path if needed.
						if min_y!=float("-inf") :
							# Move outline subpath to the begining of csp
	 					 	subp = csp[min_i]
	 					 	del csp[min_i]
	 					 	j = min_j
	 					 	# Split by the topmost point and join again
	 					 	if min_t in [0,1]:
	 					 		if min_t == 0: j=j-1
		 					 	subp[-1][2], subp[0][0] = subp[-1][1], subp[0][1]
	 					 		subp = [ [subp[j][1], subp[j][1], subp[j][2]] ] + subp[j+1:] + subp[:j] + [ [subp[j][0], subp[j][1], subp[j][1]] ]
							else: 					 		
		 						sp1,sp2,sp3 = csp_split(subp[j-1],subp[j],min_t)
		 						subp[-1][2], subp[0][0] = subp[-1][1], subp[0][1]
								subp = [ [ sp2[1], sp2[1],sp2[2] ] ] + [sp3] + subp[j+1:] + subp[:j-1] + [sp1] + [[ sp2[0], sp2[1],sp2[1] ]] 					 	  
	 					 	csp = [subp] + csp
							# reverse path if needed
							if csp_subpath_ccw(csp[0]) :
								for i in range(len(csp)):
								 	n = []
								 	for j in csp[i]:
								 		n = [  [j[2][:],j[1][:],j[0][:]]  ] + n
								 	csp[i] = n[:]

	 					 	
						d = cubicsuperpath.formatPath(csp)
						print_(("original  d=",d))
						d = re.sub(r'(?i)(m[^mz]+)',r'\1 Z ',d)
						d = re.sub(r'(?i)\s*z\s*z\s*',r' Z ',d)
						d = re.sub(r'(?i)\s*([A-Za-z])\s*',r' \1 ',d)
						print_(("formatted d=",d))
					# scale = sqrt(Xscale**2 + Yscale**2) / sqrt(1**2 + 1**2)
					p0 = self.transform([0,0],layer)
					p1 = self.transform([0,1],layer)
					scale = (P(p0)-P(p1)).mag()
					if scale == 0 : scale = 1. 
					else : scale = 1./scale
					print_(scale)
					tool_d = self.tools[layer][0]['diameter']*scale
					r = self.options.area_inkscape_radius * scale
					sign=1 if r>0 else -1
					print_("Tool diameter = %s, r = %s" % (tool_d, r))
					
					# avoiding infinite loops
					if self.options.area_tool_overlap>0.9 : self.options.area_tool_overlap = .9
			
					for i in range(self.options.max_area_curves):
						radius = - tool_d * (i*(1-self.options.area_tool_overlap)+0.5) * sign
						if abs(radius)>abs(r): 
							radius = -r
						
						inkex.etree.SubElement(	area_group, inkex.addNS('path','svg'), 
										{
											 inkex.addNS('type','sodipodi'):	'inkscape:offset',
											 inkex.addNS('radius','inkscape'):	str(radius),
											 inkex.addNS('original','inkscape'):	d,
											'style': styles["biarc_style_i"]['area']
										})
						print_(("adding curve",area_group,d,styles["biarc_style_i"]['area']))
						if radius == -r : break 


################################################################################
###
###		Polyline to biarc
###
###		Converts Polyline to Biarc
################################################################################
	def polyline_to_biarc(self):
		
		
		
		def biarc(sm, depth=0):
			def biarc_split(sp1,sp2, z1, z2, depth): 
				if depth<options.biarc_max_split_depth:
					sp1,sp2,sp3 = csp_split(sp1,sp2)
					l1, l2 = cspseglength(sp1,sp2), cspseglength(sp2,sp3)
					if l1+l2 == 0 : zm = z1
					else : zm = z1+(z2-z1)*l1/(l1+l2)
					return biarc(sp1,sp2,z1,zm,depth+1)+biarc(sp2,sp3,zm,z2,depth+1)
				else: return [ [sp1[1],'line', 0, 0, sp2[1], [z1,z2]] ]

			P0, P4 = P(sp1[1]), P(sp2[1])
			TS, TE, v = (P(sp1[2])-P0), -(P(sp2[0])-P4), P0 - P4
			tsa, tea, va = TS.angle(), TE.angle(), v.angle()
			if TE.mag()<straight_distance_tolerance and TS.mag()<straight_distance_tolerance:	
				# Both tangents are zerro - line straight
				return [ [sp1[1],'line', 0, 0, sp2[1], [z1,z2]] ]
			if TE.mag() < straight_distance_tolerance:
				TE = -(TS+v).unit()
				r = TS.mag()/v.mag()*2
			elif TS.mag() < straight_distance_tolerance:
				TS = -(TE+v).unit()
				r = 1/( TE.mag()/v.mag()*2 )
			else:	
				r=TS.mag()/TE.mag()
			TS, TE = TS.unit(), TE.unit()
			tang_are_parallel = ((tsa-tea)%math.pi<straight_tolerance or math.pi-(tsa-tea)%math.pi<straight_tolerance )
			if ( tang_are_parallel  and 
						((v.mag()<straight_distance_tolerance or TE.mag()<straight_distance_tolerance or TS.mag()<straight_distance_tolerance) or
							1-abs(TS*v/(TS.mag()*v.mag()))<straight_tolerance)	):
						# Both tangents are parallel and start and end are the same - line straight
						# or one of tangents still smaller then tollerance

						# Both tangents and v are parallel - line straight
				return [ [sp1[1],'line', 0, 0, sp2[1], [z1,z2]] ]

			c,b,a = v*v, 2*v*(r*TS+TE), 2*r*(TS*TE-1)
			if v.mag()==0:
				return biarc_split(sp1, sp2, z1, z2, depth)
			asmall, bsmall, csmall = abs(a)<10**-10,abs(b)<10**-10,abs(c)<10**-10 
			if 		asmall and b!=0:	beta = -c/b
			elif 	csmall and a!=0:	beta = -b/a 
			elif not asmall:	 
				discr = b*b-4*a*c
				if discr < 0:	raise ValueError, (a,b,c,discr)
				disq = discr**.5
				beta1 = (-b - disq) / 2 / a
				beta2 = (-b + disq) / 2 / a
				if beta1*beta2 > 0 :	raise ValueError, (a,b,c,disq,beta1,beta2)
				beta = max(beta1, beta2)
			elif	asmall and bsmall:	
				return biarc_split(sp1, sp2, z1, z2, depth)
			alpha = beta * r
			ab = alpha + beta 
			P1 = P0 + alpha * TS
			P3 = P4 - beta * TE
			P2 = (beta / ab)  * P1 + (alpha / ab) * P3


			def calculate_arc_params(P0,P1,P2):
				D = (P0+P2)/2
				if (D-P1).mag()==0: return None, None
				R = D - ( (D-P0).mag()**2/(D-P1).mag() )*(P1-D).unit()
				p0a, p1a, p2a = (P0-R).angle()%(2*math.pi), (P1-R).angle()%(2*math.pi), (P2-R).angle()%(2*math.pi)
				alpha =  (p2a - p0a) % (2*math.pi)					
				if (p0a<p2a and  (p1a<p0a or p2a<p1a))	or	(p2a<p1a<p0a) : 
					alpha = -2*math.pi+alpha 
				if abs(R.x)>1000000 or abs(R.y)>1000000  or (R-P0).mag<options.min_arc_radius**2 :
					return None, None
				else :	
					return  R, alpha
			R1,a1 = calculate_arc_params(P0,P1,P2)
			R2,a2 = calculate_arc_params(P2,P3,P4)
			if R1==None or R2==None or (R1-P0).mag()<straight_tolerance or (R2-P2).mag()<straight_tolerance	: return [ [sp1[1],'line', 0, 0, sp2[1], [z1,z2]] ]
	
			d = csp_to_arc_distance(sp1,sp2, [P0,P2,R1,a1],[P2,P4,R2,a2])
			if d > options.biarc_tolerance and depth<options.biarc_max_split_depth	 : return biarc_split(sp1, sp2, z1, z2, depth)
			else:
				if R2.mag()*a2 == 0 : zm = z2
				else : zm  = z1 + (z2-z1)*(abs(R1.mag()*a1))/(abs(R2.mag()*a2)+abs(R1.mag()*a1)) 

				l = (P0-P2).l2()
				if  l < EMC_TOLERANCE_EQUAL**2 or l<EMC_TOLERANCE_EQUAL**2 * R1.l2() /100 :
					# arc should be straight otherwise it could be threated as full circle
					arc1 = [ sp1[1], 'line', 0, 0, [P2.x,P2.y], [z1,zm] ] 
				else :
					arc1 = [ sp1[1], 'arc', [R1.x,R1.y], a1, [P2.x,P2.y], [z1,zm] ] 

				l = (P4-P2).l2()
				if  l < EMC_TOLERANCE_EQUAL**2 or l<EMC_TOLERANCE_EQUAL**2 * R2.l2() /100 :
					# arc should be straight otherwise it could be threated as full circle
					arc2 = [ [P2.x,P2.y], 'line', 0, 0, [P4.x,P4.y], [zm,z2] ] 
				else :
					arc2 = [ [P2.x,P2.y], 'arc', [R2.x,R2.y], a2, [P4.x,P4.y], [zm,z2] ]
		
				return [ arc1, arc2 ]

		
		
		
		
		for layer in self.layers :
			if layer in self.selected_paths :
				for path in self.selected_paths[layer]:
					d = path.get('d')
					if d==None:
						print_("omitting non-path")
						self.error(_("Warning: omitting non-path"),"selection_contains_objects_that_are_not_paths")
						continue
					csp = cubicsuperpath.parsePath(d)
					csp = self.apply_transforms(path, csp)
					csp = self.transform_csp(csp, layer)
					
					# lets pretend that csp is a polyline 
					poly = [ [point[1] for point in subpath] for subpath in csp ]
					
					self.draw_csp([ [ [point,point,point] for point in subpoly] for subpoly in poly ],layer)
					
					# lets create biarcs 
					for subpoly in poly :
						# lets split polyline into different smooth parths. 
						
						if len(subpoly)>2 :
							smooth = [ [subpoly[0],subpoly[1]] ]
							for p1,p2,p3 in zip(subpoly,subpoly[1:],subpoly[2:]) :
								# normalize p1p2 and p2p3 to get angle
								s1,s2 = normalize( p1[0]-p2[0], p1[1]-p2[1]), normalize( p3[0]-p2[0], p3[1]-p2[1]) 
								if cross(s1,s2) > corner_tolerance :
									#it's an angle 
									smooth += [  [p2,p3]  ]
								else: 
									smooth[-1].append(p3)							
							for sm in smooth :
								smooth_polyline_to_biarc(sm)	

################################################################################
###
###		Area fill 
###
###		Fills area with lines
################################################################################


	def area_fill(self):
		# convert degrees into rad
		self.options.area_fill_angle = self.options.area_fill_angle * math.pi / 180
		if len(self.selected_paths)<=0:
			self.error(_("This extension requires at least one selected path."),"warning")
			return
		for layer in self.layers :
			if layer in self.selected_paths :
				self.set_tool(layer)
				if self.tools[layer][0]['diameter']<=0 : 
					self.error(_("Tool diameter must be > 0 but tool's diameter on '%s' layer is not!") % layer.get(inkex.addNS('label','inkscape')),"area_tools_diameter_error")
				tool = self.tools[layer][0]
				for path in self.selected_paths[layer]:
					lines = []
					print_(("doing path",	path.get("style"), path.get("d")))
					area_group = inkex.etree.SubElement( path.getparent(), inkex.addNS('g','svg') )
					d = path.get('d')
					if d==None:
						print_("omitting non-path")
						self.error(_("Warning: omitting non-path"),"selection_contains_objects_that_are_not_paths")
						continue
					csp = cubicsuperpath.parsePath(d)
					csp = self.apply_transforms(path, csp)
					csp = csp_close_all_subpaths(csp)
					csp = self.transform_csp(csp, layer)
					#maxx = max([x,y,i,j,root],maxx)
					
					# rotate the path to get bounds in defined direction.
					a = - self.options.area_fill_angle
					rotated_path = [   [ [ [point[0]*math.cos(a) - point[1]*math.sin(a), point[0]*math.sin(a)+point[1]*math.cos(a)]  for point in sp] for sp in subpath] for subpath in csp  ]
					bounds =  csp_true_bounds(rotated_path)
					
					# Draw the lines 
					# Get path's bounds
					b = [0.0, 0.0, 0.0, 0.0] # [minx,miny,maxx,maxy]
					for k in range(4):
						i, j, t = bounds[k][2], bounds[k][3], bounds[k][4]
						b[k] = csp_at_t(rotated_path[i][j-1],rotated_path[i][j],t)[k%2]

						
					# Zig-zag
					r = tool['diameter']*(1-self.options.area_tool_overlap)
					if r<=0 :
						self.error('Tools diameter must be greater than 0!', 'error')
						return

					lines += [ [] ]

					if self.options.area_fill_method == 'zig-zag' :
						i = b[0] - self.options.area_fill_shift*r
						top = True
						last_one = True
						while (i<b[2] or last_one) : 
							if i>=b[2] : last_one = False
							if lines[-1] == [] :
								lines[-1] += [  [i,b[3]]  ]

							if top :
								lines[-1] += [ [i,b[1]],[i+r,b[1]] ]

							else :
									lines[-1] += [ [i,b[3]], [i+r,b[3]] ]

							top = not top
							i += r
					else :
					
						w, h  = b[2]-b[0] + self.options.area_fill_shift*r , b[3]-b[1] +  self.options.area_fill_shift*r
						x,y = b[0] - self.options.area_fill_shift*r, b[1] - self.options.area_fill_shift*r
						lines[-1] += [  [x,y] ]
						stage = 0
						start = True
						while w>0 and h>0 :
							stage = (stage+1)%4
							if   stage == 0 :
								y -= h
								h -= r
							elif stage == 1:
								x += w
								if not start:
									w -= r
								start = False	
							elif stage == 2 :
								y += h
								h -= r
							elif stage == 3:
								x -= w
								w -=r
							
							lines[-1] += [ [x,y] ]

						stage = (stage+1)%4							
						if w <= 0 and h>0 :
							y = y-h if stage == 0 else y+h  
						if h <= 0  and w>0 :
							x = x-w if stage == 3 else x+w  
						lines[-1] += [ [x,y] ]
					# Rotate created paths back
					a =  self.options.area_fill_angle
					lines = [ [ [point[0]*math.cos(a) - point[1]*math.sin(a), point[0]*math.sin(a)+point[1]*math.cos(a)] for point in subpath] for subpath in lines  ]

					# get the intersection points
					
					splitted_line = [ [lines[0][0]] ]
					intersections = {}
					for l1,l2, in zip(lines[0],lines[0][1:]): 
						ints = []
						
						if l1[0]==l2[0] and l1[1]==l2[1] : continue
						for i in range(len(csp)) :
							for j in range(1,len(csp[i]))  :
								sp1,sp2 = csp[i][j-1], csp[i][j]
								roots = csp_line_intersection(l1,l2,sp1,sp2)
								for t in roots :
									p = tuple(csp_at_t(sp1,sp2,t))
									if l1[0]==l2[0] :
										t1 = (p[1]-l1[1])/(l2[1]-l1[1])
									else :
										t1 = (p[0]-l1[0])/(l2[0]-l1[0])
									if 0<=t1<=1	:
										ints += [[t1, p[0],p[1], i,j,t]]
										if p in intersections :
											intersections[p]  += [ [i,j,t] ]  
										else : 	
											intersections[p]  = [ [i,j,t] ]  
										#p = self.transform(p,layer,True)
										#draw_pointer(p)
						ints.sort()
						for i in ints:
							splitted_line[-1] +=[ [ i[1], i[2]] ]
							splitted_line += [ [ [ i[1], i[2]] ] ]
						splitted_line[-1] += [  l2  ]
						i = 0
					print_(splitted_line)
					while i < len(splitted_line) :
						# check if the middle point of the first lines segment is inside the path.
						# and remove the subline if not.
						l1,l2 = splitted_line[i][0],splitted_line[i][1]
						p = [(l1[0]+l2[0])/2, (l1[1]+l2[1])/2]
						if not point_inside_csp(p, csp):
							#i +=1 						
							del splitted_line[i]
						else :
							i += 1
					
					
					
					# if we've used spiral method we'll try to save the order of cutting
					do_not_change_order = self.options.area_fill_method == 'spiral' 
					# now let's try connect splitted lines
					#while len(splitted_line)>0 :
					#TODO	
					
					# and apply back transrormations to draw them
					csp_line = csp_from_polyline(splitted_line)
					csp_line = self.transform_csp(csp_line, layer, True)
					
					self.draw_csp(csp_line, group = area_group)
#					draw_csp(lines)
					


					 					


################################################################################
###
###		Engraving
###
#LT Notes to self: See wiki.inkscape.org/wiki/index.php/PythonEffectTutorial
# To create anything in the Inkscape document, look at the XML editor for
# details of how such an element looks in XML, then follow this model.
#layer number n appears in XML as <svg:g id="layern" inkscape:label="layername">
#
#to create it, use
#Mylayer=inkex.etree.SubElement(self.document.getroot(), 'g') #Create a generic element
#Mylayer.set(inkex.addNS('label', 'inkscape'), "layername")   #Gives it a name
#Mylayer.set(inkex.addNS('groupmode', 'inkscape'), 'layer')   #Tells Inkscape it's a layer
#
#group appears in XML as <svg:g id="gnnnnn"> where nnnnn is a number
#
#to create it, use
#Mygroup=inkex.etree.SubElement(parent, inkex.addNS('g','svg'), {"gcodetools":"My group label"})
# where parent may be the layer or a parent group. To get the parent group, you can use
#parent = self.selected_paths[layer][0].getparent()
################################################################################
	def engraving(self) :
		#global x1,y1,rx,ry
		global cspm, wl
		global nlLT, i, j
		global gcode_3Dleft ,gcode_3Dright
		global max_dist #minimum of tool radius and user's requested maximum distance
		global eye_dist
		eye_dist = 100 #3D constant. Try varying it for your eyes


		def bisect((nx1,ny1),(nx2,ny2)) :
			"""LT Find angle bisecting the normals n1 and n2

			Parameters: Normalised normals
			Returns: nx - Normal of bisector, normalised to 1/cos(a)
				   ny -
				   sinBis2 - sin(angle turned/2): positive if turning in
			Note that bisect(n1,n2) and bisect(n2,n1) give opposite sinBis2 results
			If sinturn is less than the user's requested angle tolerance, I return 0
			"""
			#We can get absolute value of cos(bisector vector)
			#Note: Need to use max in case of rounding errors
			cosBis = math.sqrt(max(0,(1.0+nx1*nx2-ny1*ny2)/2.0))
			#We can get correct sign of the sin, assuming cos is positive
			if (abs(ny1-ny2)< engraving_tolerance)  or (abs(cosBis) < engraving_tolerance) :
				if (abs(nx1-nx2)< engraving_tolerance): return(nx1,ny1,0.0)
				sinBis = math.copysign(1,ny1)
			else :
				sinBis = cosBis*(nx2-nx1)/(ny1-ny2)
			#We can correct signs by noting that the dot product
			# of bisector and either normal must be >0
			costurn=cosBis*nx1+sinBis*ny1
			if costurn == 0 : return (ny1*100,-nx1*100,1) #Path doubles back on itself
			sinturn=sinBis*nx1-cosBis*ny1
			if costurn<0 :  sinturn=-sinturn
			if 0 < sinturn*114.6 < (180-self.options.engraving_sharp_angle_tollerance) :
				sinturn=0 #set to zero if less than the user wants to see.
			return (cosBis/costurn,sinBis/costurn, sinturn)
			#end bisect

		def get_radius_to_line((x1,y1),(nx1,ny1), (nx2,ny2),(x2,y2),(nx23,ny23),(x3,y3),(nx3,ny3)):
			"""LT find biggest circle we can engrave here, if constrained by line 2-3

			Parameters:
				x1,y1,nx1,ny1 coordinates and normal of the line we're currently engraving
				nx2,ny2 angle bisector at point 2
				x2,y2 coordinates of first point of line 2-3
				nx23,ny23 normal to the line 2-3
				x3,y3 coordinates of the other end
				nx3,ny3 angle bisector at point 3
			Returns:
				radius or self.options.engraving_max_dist if line doesn't limit radius
			This function can be used in three ways:
			- With nx1=ny1=0 it finds circle centred at x1,y1
			- with nx1,ny1 normalised, it finds circle tangential at x1,y1
			- with nx1,ny1 scaled by 1/cos(a) it finds circle centred on an angle bisector
				 where a is the angle between the bisector and the previous/next normals

			If the centre of the circle tangential to the line 2-3 is outside the
			angle bisectors at its ends, ignore this line. 

			# Note that it handles corners in the conventional manner of letter cutting
			# by mitering, not rounding.
			# Algorithm uses dot products of normals to find radius
			# and hence coordinates of centre
			"""

			global max_dist

			#Start by converting coordinates to be relative to x1,y1
			x2,y2= x2-x1, y2-y1
			x3,y3= x3-x1, y3-y1

			#The logic uses vector arithmetic.
			#The dot product of two vectors gives the product of their lengths
			#multiplied by the cos of the angle between them.
			# So, the perpendicular distance from x1y1 to the line 2-3
			# is equal to the dot product of its normal and x2y2 or x3y3
			#It is also equal to the projection of x1y1-xcyc on the line's normal
			# plus the radius. But, as the normal faces inside the path we must negate it.

			#Make sure the line in question is facing x1,y1 and vice versa
			dist=-x2*nx23-y2*ny23
			if dist<0 : return max_dist
			denom=1.-nx23*nx1-ny23*ny1
			if denom < engraving_tolerance : return max_dist

			#radius and centre are:
			r=dist/denom
			cx=r*nx1
			cy=r*ny1
			#if c is not between the angle bisectors at the ends of the line, ignore
			#Use vector cross products. Not sure if I need the .0001 safety margins:
			if (x2-cx)*ny2 > (y2-cy)*nx2 +0.0001 :
				return max_dist 
			if (x3-cx)*ny3 < (y3-cy)*nx3  -0.0001 :
				return max_dist 
			return min(r, max_dist)
			#end of get_radius_to_line

		def get_radius_to_point((x1,y1),(nx,ny), (x2,y2)):
			"""LT find biggest circle we can engrave here, constrained by point x2,y2

			This function can be used in three ways:
			- With nx=ny=0 it finds circle centred at x1,y1
			- with nx,ny normalised, it finds circle tangential at x1,y1
			- with nx,ny scaled by 1/cos(a) it finds circle centred on an angle bisector
				 where a is the angle between the bisector and the previous/next normals

			Note that I wrote this to replace find_cutter_centre. It is far less
			sophisticated but, I hope, far faster.
			It turns out that finding a circle touching a point is harder than a circle
			touching a line.
			"""

			global max_dist

			#Start by converting coordinates to be relative to x1,y1
			x2,y2= x2-x1, y2-y1
			denom=nx**2+ny**2-1
			if denom<=engraving_tolerance : #Not a corner bisector
				if denom==-1 : #Find circle centre x1,y1
					return math.sqrt(x2**2+y2**2)
				#if x2,y2 not in front of the normal...
				if x2*nx+y2*ny <=0 : return max_dist
				#print_("Straight",x1,y1,nx,ny,x2,y2)
				return (x2**2+y2**2)/(2*(x2*nx+y2*ny) )
			#It is a corner bisector, so..
			discriminator = (x2*nx+y2*ny)**2 - denom*(x2**2+y2**2)
			if discriminator < 0 :
				return max_dist #this part irrelevant
			r=(x2*nx+y2*ny -math.sqrt(discriminator))/denom
			#print_("Corner",x1,y1,nx,ny,x1+x2,y1+y2,discriminator,r)
			return min(r, max_dist)
			#end of get_radius_to_point

		def bez_divide(a,b,c,d):
			"""LT recursively divide a Bezier.

			Divides until difference between each
			part and a straight line is less than some limit
			Note that, as simple as this code is, it is mathematically correct.
			Parameters:
				a,b,c and d are each a list of x,y real values
				Bezier end points a and d, control points b and c
			Returns:
				a list of Beziers.
					Each Bezier is a list with four members,
						each a list holding a coordinate pair
				Note that the final point of one member is the same as
				the first point of the next, and the control points
				there are smooth and symmetrical. I use this fact later.
			"""
			bx=b[0]-a[0]
			by=b[1]-a[1]
			cx=c[0]-a[0]
			cy=c[1]-a[1]
			dx=d[0]-a[0]
			dy=d[1]-a[1]
			limit=8*math.hypot(dx,dy)/self.options.engraving_newton_iterations
			#LT This is the only limit we get from the user currently
			if abs(dx*by-bx*dy)<limit and abs(dx*cy-cx*dy)<limit :
				return [[a,b,c,d]]
			abx=(a[0]+b[0])/2.0
			aby=(a[1]+b[1])/2.0
			bcx=(b[0]+c[0])/2.0
			bcy=(b[1]+c[1])/2.0
			cdx=(c[0]+d[0])/2.0
			cdy=(c[1]+d[1])/2.0
			abcx=(abx+bcx)/2.0
			abcy=(aby+bcy)/2.0
			bcdx=(bcx+cdx)/2.0
			bcdy=(bcy+cdy)/2.0
			m=[(abcx+bcdx)/2.0,(abcy+bcdy)/2.0]
			return bez_divide(a,[abx,aby],[abcx,abcy],m) + bez_divide(m,[bcdx,bcdy],[cdx,cdy],d)
			#end of bez_divide

		def get_biggest((x1,y1),(nx,ny)):
			"""LT Find biggest circle we can draw inside path at point x1,y1 normal nx,ny

			Parameters:
				point - either on a line or at a reflex corner
				normal - normalised to 1 if on a line, to 1/cos(a) at a corner
			Returns:
				tuple (j,i,r)
				..where j and i are indices of limiting segment, r is radius
			"""
			global max_dist, nlLT, i, j
			n1 = nlLT[j][i-1] #current node
			jjmin = -1
			iimin = -1
			r = max_dist
			# set limits within which to look for lines
			xmin, xmax = x1+r*nx-r, x1+r*nx+r
			ymin, ymax = y1+r*ny-r, y1+r*ny+r
			for jj in xrange(0,len(nlLT)) : #for every subpath of this object
				for ii in xrange(0,len(nlLT[jj])) : #for every point and line
					if nlLT[jj][ii-1][2] : #if a point
						if jj==j : #except this one
							if abs(ii-i)<3 or abs(ii-i)>len(nlLT[j])-3 : continue
						t1=get_radius_to_point((x1,y1),(nx,ny),nlLT[jj][ii-1][0] )
						#print_("Try pt   i,ii,t1,x1,y1",i,ii,t1,x1,y1)
					else: #doing a line
						if jj==j : #except this one
							if abs(ii-i)<2 or abs(ii-i)==len(nlLT[j])-1 : continue
							if abs(ii-i)==2  and nlLT[j][(ii+i)/2-1][3]<=0 : continue
							if (abs(ii-i)==len(nlLT[j])-2) and nlLT[j][-1][3]<=0 : continue
						nx2,ny2 = nlLT[jj][ii-2][1]
						x2,y2 = nlLT[jj][ii-1][0]
						nx23,ny23 = nlLT[jj][ii-1][1]
						x3,y3 = nlLT[jj][ii][0]
						nx3,ny3 = nlLT[jj][ii][1]
						if nlLT[jj][ii-2][3]>0 : #acute, so use normal, not bisector
							nx2=nx23
							ny2=ny23
						if nlLT[jj][ii][3]>0 : #acute, so use normal, not bisector
							nx3=nx23
							ny3=ny23
						x23min,x23max=min(x2,x3),max(x2,x3)
						y23min,y23max=min(y2,y3),max(y2,y3)
						#see if line in range
						if n1[2]==False and (x23max<xmin or x23min>xmax or y23max<ymin or y23min>ymax) : continue
						t1=get_radius_to_line((x1,y1),(nx,ny), (nx2,ny2),(x2,y2),(nx23,ny23), (x3,y3),(nx3,ny3))
						#print_("Try line i,ii,t1,x1,y1",i,ii,t1,x1,y1)
					if 0<=t1<r :
						r = t1
						iimin = ii
						jjmin = jj
						xmin, xmax = x1+r*nx-r, x1+r*nx+r
						ymin, ymax = y1+r*ny-r, y1+r*ny+r
				#next ii
			#next jj
			return (jjmin,iimin,r)
			#end of get_biggest

		def line_divide((x0,y0),j0,i0,(x1,y1),j1,i1,(nx,ny),length):
			"""LT recursively divide a line as much as necessary

			NOTE: This function is not currently used
			By noting which other path segment is touched by the circles at each end,
			we can see if anything is to be gained by a further subdivision, since
			if they touch the same bit of path we can move linearly between them.
			Also, we can handle points correctly.
			Parameters:
				end points and indices of limiting path, normal, length
			Returns:
				list of toolpath points
					each a list of 3 reals: x, y coordinates, radius

			"""
			global nlLT, i, j, lmin
			x2=(x0+x1)/2
			y2=(y0+y1)/2
			j2,i2,r2=get_biggest( (x2,y2), (nx,ny))
			if length<lmin : return [ [x2, y2, r2] ]
			if j2==j0 and i2==i0 : #Same as left end. Don't subdivide this part any more
				return [ [x2, y2, r2], line_divide((x2,y2),j2,i2,(x1,y1),j1,i1,(nx,ny),length/2)]
			if j2==j1 and i2==i1 : #Same as right end. Don't subdivide this part any more
				return [ line_divide((x0,y0),j0,i0,(x2,y2),j2,i2,(nx,ny),length/2), [x2, y2, r2] ]
			return [ line_divide((x0,y0),j0,i0,(x2,y2),j2,i2,(nx,ny),length/2), line_divide((x2,y2),j2,i2,(x1,y1),j1,i1,(nx,ny),length/2)]
			#end of line_divide()

		def save_point((x,y),w,i,j,ii,jj):
			"""LT Save this point and delete previous one if linear

			The point is, we generate tons of points but many may be in a straight 3D line.
			There is no benefit in saving the imtermediate points.
			"""
			global wl, cspm
			x=round(x,4) #round to 4 decimals
			y=round(y,4) #round to 4 decimals
			w=round(w,4) #round to 4 decimals
			if len(cspm)>1 :
				xy1a,xy1,xy1b,i1,j1,ii1,jj1=cspm[-1]
				w1=wl[-1]
				if i==i1 and j==j1 and ii==ii1 and jj==jj1 : #one match
					xy1a,xy2,xy1b,i1,j1,ii1,jj1=cspm[-2]
					w2=wl[-2]
					if i==i1 and j==j1 and ii==ii1 and jj==jj1 : #two matches. Now test linearity
						length1=math.hypot(xy1[0]-x,xy1[1]-y)
						length2=math.hypot(xy2[0]-x,xy2[1]-y)
						length12=math.hypot(xy2[0]-xy1[0],xy2[1]-xy1[1])
						#get the xy distance of point 1 from the line 0-2
						if length2>length1 and length2>length12 : #point 1 between them
							xydist=abs( (xy2[0]-x)*(xy1[1]-y)-(xy1[0]-x)*(xy2[1]-y) )/length2
							if xydist<engraving_tolerance : #so far so good
								wdist=w2+(w-w2)*length1/length2 -w1
								if abs(wdist)<engraving_tolerance :
									#print_("pop",j,i,xy1)
									cspm.pop()
									wl.pop()
			cspm+=[ [ [x,y],[x,y],[x,y],i,j,ii,jj ] ]
			wl+=[w]
			#end of save_point

		def draw_point((x0,y0),(x,y),w,t):
			"""LT Draw this point as a circle with a 1px dot in the middle (x,y) 
			and a 3D line from (x0,y0) down to x,y. 3D line thickness should be t/2 

			Note that points that are subsequently erased as being unneeded do get
			displayed, but this helps the user see the total area covered.
			"""
			global gcode_3Dleft ,gcode_3Dright
			if self.options.engraving_draw_calculation_paths :
				inkex.etree.SubElement(	engraving_group, inkex.addNS('path','svg'), 
						{"gcodetools": "Engraving calculation toolpath", 'style':	"fill:#ff00ff; fill-opacity:0.46; stroke:#000000; stroke-width:0.1;", inkex.addNS('cx','sodipodi'):		str(x), inkex.addNS('cy','sodipodi'):		str(y), inkex.addNS('rx','sodipodi'):	str(1), inkex.addNS('ry','sodipodi'): str(1), inkex.addNS('type','sodipodi'):	'arc'})
				#Don't draw zero radius circles
				if w:
					inkex.etree.SubElement(	engraving_group, inkex.addNS('path','svg'), 
						{"gcodetools": "Engraving calculation paths", 'style':	"fill:none; fill-opacity:0.46; stroke:#000000; stroke-width:0.1;", inkex.addNS('cx','sodipodi'):		str(x),  inkex.addNS('cy','sodipodi'):		str(y),inkex.addNS('rx','sodipodi'):		str(w), inkex.addNS('ry','sodipodi'):		str(w), inkex.addNS('type','sodipodi'):	'arc'})
					# Find slope direction for shading
					s=math.atan2(y-y0,x-x0) #-pi to pi
					# convert to 2 hex digits as a shade of red
					s2="#{0:x}0000".format(int(101*(1.5-math.sin(s+0.5))))
					inkex.etree.SubElement(	gcode_3Dleft , inkex.addNS('path','svg'),
					{ "d": "M %f,%f L %f,%f" %(x0-eye_dist,y0,x-eye_dist-0.14*w,y),
						'style': "stroke:" + s2 + "; stroke-opacity:1; stroke-width:" + str(t/2) +" ; fill:none",
						"gcodetools": "Gcode G1R"
					})
					inkex.etree.SubElement(	gcode_3Dright , inkex.addNS('path','svg'),
					{ "d": "M %f,%f L %f,%f" %(x0+eye_dist,y0,x+eye_dist+0.14*r,y),
						'style': "stroke:" + s2 + "; stroke-opacity:1; stroke-width:" + str(t/2) +" ; fill:none",
						"gcodetools": "Gcode G1L"
					})
			#end of draw_point

		#end of subfunction definitions. engraving() starts here:
		gcode = ''
		r,w, wmax = 0,0,0 #theoretical and tool-radius-limited radii in pixels
		x1,y1,nx,ny =0,0,0,0
		cspe =[]
		we = []
		if len(self.selected_paths)<=0:
			self.error(_("Please select at least one path to engrave and run again."),"warning")
			return
		if not self.check_dir() : return
		#Find what units the user uses
		unit=" mm"
		if self.options.unit == "G20 (All units in inches)" :
			unit=" inches"
		elif self.options.unit != "G21 (All units in mm)" :
			self.error(_("Unknown unit selected. mm assumed"),"warning")
		print_("engraving_max_dist mm/inch", self.options.engraving_max_dist )

		#LT See if we can use this parameter for line and Bezier subdivision:
		bitlen=20/self.options.engraving_newton_iterations

		for layer in self.layers :
			if layer in self.selected_paths :
				#Calculate scale in pixels per user unit (mm or inch)
				p1=self.orientation_points[layer][0][0]
				p2=self.orientation_points[layer][0][1]
				ol=math.hypot(p1[0][0]-p2[0][0],p1[0][1]-p2[0][1])
				oluu=math.hypot(p1[1][0]-p2[1][0],p1[1][1]-p2[1][1])
				print_("Orientation2 p1 p2 ol oluu",p1,p2,ol,oluu)
				orientation_scale = ol/oluu

				self.set_tool(layer)
				shape = self.tools[layer][0]['shape']
				if re.search('w', shape) :
					toolshape = eval('lambda w: ' + shape.strip('"'))
				else: 
					self.error(_("Tool '%s' has no shape. 45 degree cone assumed!") % self.tools[layer][0]['name'],"Continue")
					toolshape = lambda w: w
				#Get tool radius in pixels
				toolr=self.tools[layer][0]['diameter'] * orientation_scale/2
				print_("tool radius in pixels=", toolr)
				#max dist from path to engrave in user's units
				max_distuu = min(self.tools[layer][0]['diameter']/2, self.options.engraving_max_dist)
				max_dist=max_distuu*orientation_scale
				print_("max_dist pixels", max_dist )

				engraving_group = inkex.etree.SubElement( self.selected_paths[layer][0].getparent(), inkex.addNS('g','svg') )
				if self.options.engraving_draw_calculation_paths and (self.my3Dlayer  == None) :
					self.my3Dlayer=inkex.etree.SubElement(self.document.getroot(), 'g') #Create a generic element at root level
					self.my3Dlayer.set(inkex.addNS('label', 'inkscape'), "3D") #Gives it a name
					self.my3Dlayer.set(inkex.addNS('groupmode', 'inkscape'), 'layer') #Tells Inkscape it's a layer
				#Create groups for left and right eyes
				if self.options.engraving_draw_calculation_paths :
					gcode_3Dleft = inkex.etree.SubElement(self.my3Dlayer, inkex.addNS('g','svg'), {"gcodetools":"Gcode 3D L"})
					gcode_3Dright = inkex.etree.SubElement(self.my3Dlayer, inkex.addNS('g','svg'), {"gcodetools":"Gcode 3D R"})

				for node in self.selected_paths[layer] :	
					if node.tag == inkex.addNS('path','svg'):
						cspi = cubicsuperpath.parsePath(node.get('d'))
						#LT: Create my own list. n1LT[j] is for subpath j
						nlLT = []
						for j in xrange(len(cspi)): #LT For each subpath...
							# Remove zero length segments, assume closed path
							i = 0 #LT was from i=1
							while i<len(cspi[j]):
								if abs(cspi[j][i-1][1][0]-cspi[j][i][1][0])<engraving_tolerance and abs(cspi[j][i-1][1][1]-cspi[j][i][1][1])<engraving_tolerance:
									cspi[j][i-1][2] = cspi[j][i][2]
									del cspi[j][i]
								else:
									i += 1
						for csp in cspi: #LT6a For each subpath...
							#Create copies in 3D layer
							print_("csp is zz ",csp)
							cspl=[]
							cspr=[]
							#create list containing lines and points, starting with a point
							# line members: [x,y],[nx,ny],False,i
							# x,y is start of line. Normal on engraved side.
							# Normal is normalised (unit length)
							#Note that Y axis increases down the page
							# corner members: [x,y],[nx,ny],True,sin(halfangle)
							# if halfangle>0: radius 0 here. normal is bisector
							# if halfangle<0. reflex angle. normal is bisector
							# corner normals are divided by cos(halfangle)
							#so that they will engrave correctly
							print_("csp is",csp)
							nlLT.append ([])
							for i in range(0,len(csp)): #LT for each point
								#n = []
								sp0, sp1, sp2 = csp[i-2], csp[i-1], csp[i]
								if self.options.engraving_draw_calculation_paths:
									#Copy it to 3D layer objects
									spl=[]
									spr=[]
									for j in range(0,3) :
										pl=[sp2[j][0]-eye_dist,sp2[j][1]]
										pr=[sp2[j][0]+eye_dist,sp2[j][1]]
										spl+=[pl]
										spr+=[pr]
									cspl+=[spl]
									cspr+=[spr]					
								#LT find angle between this and previous segment
								x0,y0 = sp1[1]
								nx1,ny1 = csp_normalized_normal(sp1,sp2,0)
								#I don't trust this function, so test result
								if abs(1-math.hypot(nx1,ny1))> 0.00001 :
									print_("csp_normalised_normal error t=0",nx1,ny1,sp1,sp2)
									self.error(_("csp_normalised_normal error. See log."),"warning")

								nx0, ny0 = csp_normalized_normal(sp0,sp1,1)
								if abs(1-math.hypot(nx0,ny0))> 0.00001 :
									print_("csp_normalised_normal error t=1",nx0,ny0,sp1,sp2)
									self.error(_("csp_normalised_normal error. See log."),"warning")
								bx,by,s=bisect((nx0,ny0),(nx1,ny1))
								#record x,y,normal,ifCorner, sin(angle-turned/2)
								nlLT[-1] += [[ [x0,y0],[bx,by], True, s]]

								#LT now do the line
								if sp1[1]==sp1[2] and sp2[0]==sp2[1] : #straightline
									nlLT[-1]+=[[sp1[1],[nx1,ny1],False,i]]
								else : #Bezier. First, recursively cut it up:
									nn=bez_divide(sp1[1],sp1[2],sp2[0],sp2[1])
									first=True #Flag entry to divided Bezier
									for bLT in nn : #save as two line segments
										for seg in range(3) :
											if seg>0 or first :
												nx1=bLT[seg][1]-bLT[seg+1][1]
												ny1=bLT[seg+1][0]-bLT[seg][0]
												l1=math.hypot(nx1,ny1)
												if l1<engraving_tolerance :
													continue
												nx1=nx1/l1 #normalise them
												ny1=ny1/l1
												nlLT[-1]+=[[bLT[seg],[nx1,ny1], False,i]]
												first=False
											if seg<2 : #get outgoing bisector
												nx0=nx1
												ny0=ny1
												nx1=bLT[seg+1][1]-bLT[seg+2][1]
												ny1=bLT[seg+2][0]-bLT[seg+1][0]
												l1=math.hypot(nx1,ny1)
												if l1<engraving_tolerance :
													continue
												nx1=nx1/l1 #normalise them
												ny1=ny1/l1
												#bisect
												bx,by,s=bisect((nx0,ny0),(nx1,ny1))
												nlLT[-1] += [[bLT[seg+1],[bx,by], True, 0.]]
							#LT for each segment - ends here.
							print_(("engraving_draw_calculation_paths=",self.options.engraving_draw_calculation_paths))
							if self.options.engraving_draw_calculation_paths:
								#Copy complete paths to 3D layer
								#print_("cspl",cspl)
								cspl+=[cspl[0]] #Close paths
								cspr+=[cspr[0]] #Close paths
								inkex.etree.SubElement(	gcode_3Dleft , inkex.addNS('path','svg'),
								{ "d": cubicsuperpath.formatPath([cspl]),
								'style': "stroke:#808080; stroke-opacity:1; stroke-width:0.6; fill:none",
								"gcodetools": "G1L outline"
								})
								inkex.etree.SubElement(	gcode_3Dright , inkex.addNS('path','svg'),
								{ "d": cubicsuperpath.formatPath([cspr]),
								'style': "stroke:#808080; stroke-opacity:1; stroke-width:0.6; fill:none",
								"gcodetools": "G1L outline"
								})

								for p in nlLT[-1]: #For last sub-path
									if p[2]: inkex.etree.SubElement(	engraving_group, inkex.addNS('path','svg'),
										{ "d":	"M %f,%f L %f,%f" %(p[0][0],p[0][1],p[0][0]+p[1][0]*10,p[0][1]+p[1][1]*10),
											'style':	"stroke:#f000af; stroke-opacity:0.46; stroke-width:0.1; fill:none",
											"gcodetools": "Engraving normals"
										})				
									else: inkex.etree.SubElement(	engraving_group, inkex.addNS('path','svg'),
										{ "d":	"M %f,%f L %f,%f" %(p[0][0],p[0][1],p[0][0]+p[1][0]*10,p[0][1]+p[1][1]*10),
											'style':	"stroke:#0000ff; stroke-opacity:0.46; stroke-width:0.1; fill:none",
											"gcodetools": "Engraving bisectors"
										})				


						#LT6a build nlLT[j] for each subpath - ends here
						#for nnn in nlLT :
							#print_("nlLT",nnn) #LT debug stuff
				 		# Calculate offset points
						reflex=False
						for j in xrange(len(nlLT)): #LT6b for each subpath
							cspm=[] #Will be my output. List of csps.
							wl=[] #Will be my w output list
							w = r = 0 #LT initial, as first point is an angle
							for i in xrange(len(nlLT[j])) : #LT for each node
								#LT Note: Python enables wrapping of array indices
								# backwards to -1, -2, but not forwards. Hence:
								n0 = nlLT[j][i-2] #previous node
								n1 = nlLT[j][i-1] #current node
								n2 = nlLT[j][i] #next node
								#if n1[2] == True and n1[3]==0 : # A straight angle
									#continue
								x1a,y1a = n1[0] #this point/start of this line
								nx,ny = n1[1]
								x1b,y1b = n2[0] #next point/end of this line
								if n1[2] == True : # We're at a corner
									bits=1
									bit0=0
									#lastr=r #Remember r from last line
									lastw=w #Remember w from last line
									w = max_dist
									if n1[3]>0 : #acute. Limit radius
										len1=math.hypot( (n0[0][0]-n1[0][0]),( n0[0][1]-n1[0][1]) )
										if i<(len(nlLT[j])-1) :
											len2=math.hypot( (nlLT[j][i+1][0][0]-n1[0][0]),(nlLT[j][i+1][0][1]-n1[0][1]) )
										else:
											len2=math.hypot( (nlLT[j][0][0][0]-n1[0][0]),(nlLT[j][0][0][1]-n1[0][1]) )
										#set initial r value, not to be exceeded
										w = math.sqrt(min(len1,len2))/n1[3]
								else: #line. Cut it up if long.
									if n0[3]>0 and not self.options.engraving_draw_calculation_paths :
										bit0=r*n0[3] #after acute corner
									else : bit0=0.0
									length=math.hypot((x1b-x1a),(y1a-y1b))
									bit0=(min(length,bit0))
									bits=int((length-bit0)/bitlen)
									#split excess evenly at both ends
									bit0+=(length-bit0-bitlen*bits)/2
									#print_("j,i,r,bit0,bits",j,i,w,bit0,bits)
								for b in xrange(bits) : #divide line into bits
									x1=x1a+ny*(b*bitlen+bit0)
									y1=y1a-nx*(b*bitlen+bit0)
									jjmin,iimin,w=get_biggest( (x1,y1), (nx,ny))
									print_("i,j,jjmin,iimin,w",i,j,jjmin,iimin,w)
									#w = min(r, toolr)
									wmax=max(wmax,w)
									if reflex : #just after a reflex corner
										reflex = False
										if w<lastw : #need to adjust it
											draw_point((x1,y1),(n0[0][0]+n0[1][0]*w,n0[0][1]+n0[1][1]*w),w, (lastw-w)/2)
											save_point((n0[0][0]+n0[1][0]*w,n0[0][1]+n0[1][1]*w),w,i,j,iimin,jjmin)
									if n1[2] == True : # We're at a corner
										if n1[3]>0 : #acute
											save_point((x1+nx*w,y1+ny*w),w,i,j,iimin,jjmin)
											draw_point((x1,y1),(x1,y1),0,0)
											save_point((x1,y1),0,i,j,iimin,jjmin)
										elif n1[3]<0  : #reflex
											if w>lastw :
												draw_point((x1,y1),(x1+nx*lastw,y1+ny*lastw),w, (w-lastw)/2)
												wmax=max(wmax,w)
												save_point((x1+nx*w,y1+ny*w),w,i,j,iimin,jjmin)
									elif b>0 and n2[3]>0 and not self.options.engraving_draw_calculation_paths : #acute corner coming up
										if jjmin==j and iimin==i+2 : break
									draw_point((x1,y1),(x1+nx*w,y1+ny*w),w, bitlen)
									save_point((x1+nx*w,y1+ny*w),w,i,j,iimin,jjmin)

								#LT end of for each bit of this line
								if n1[2] == True and n1[3]<0 : #reflex angle
									reflex=True
								lastw = w #remember this w
							#LT next i
							cspm+=[cspm[0]]
							print_("cspm",cspm)
							wl+=[wl[0]]
							print_("wl",wl)
							#Note: Original csp_points was a list, each element
							#being 4 points, with the first being the same as the
							#last of the previous set.
							#Each point is a list of [cx,cy,r,w]
							#I have flattened it to a flat list of points.
					
							if self.options.engraving_draw_calculation_paths==True:
								node =  inkex.etree.SubElement(	engraving_group, inkex.addNS('path','svg'), 										{
														 "d":	 cubicsuperpath.formatPath([cspm]),
														'style':	styles["biarc_style_i"]['biarc1'],
														"gcodetools": "Engraving calculation paths",
													})
								for i in xrange(len(cspm)):
									inkex.etree.SubElement(	engraving_group, inkex.addNS('path','svg'), 
											{"gcodetools": "Engraving calculation paths", 'style':	"fill:none; fill-opacity:0.46; stroke:#000000; stroke-width:0.1;", inkex.addNS('cx','sodipodi'):		str(cspm[i][1][0]),  inkex.addNS('cy','sodipodi'):		str(cspm[i][1][1]),inkex.addNS('rx','sodipodi'):		str(wl[i]), inkex.addNS('ry','sodipodi'):		str(wl[i]), inkex.addNS('type','sodipodi'):	'arc'})
							cspe += [cspm]
							wluu = [] #width list in user units: mm/inches
							for w in wl :
								wluu+=[ w / orientation_scale ]
							print_("wl in pixels",wl)
							print_("wl in user units",wluu)
							#LT previously, we was in pixels so gave wrong depth
							we   +=	[wluu]
						#LT6b For each subpath - ends here			
					#LT5 if it is a path - ends here
					#print_("cspe",cspe)
					#print_("we",we)
				#LT4 for each selected object in this layer - ends here

				if cspe!=[]:
					curve = self.parse_curve(cspe, layer, we, toolshape) #convert to lines
					self.draw_curve(curve, layer, engraving_group)
					gcode += self.generate_gcode(curve, layer, self.options.Zsurface)

			#LT3 for layers loop ends here
		if gcode!='' :
			self.header+="(Tool diameter should be at least "+str(2*wmax/orientation_scale)+unit+ ")\n"
			self.header+="(Depth, as a function of radius w, must be "+ self.tools[layer][0]['shape']+ ")\n"
			self.header+="(Rapid feeds use safe Z="+ str(self.options.Zsafe) + unit + ")\n"
			self.header+="(Material surface at Z="+ str(self.options.Zsurface) + unit + ")\n"
			self.export_gcode(gcode)
		else : 	self.error(_("No need to engrave sharp angles."),"warning")


################################################################################
###
###		Orientation
###
################################################################################
	def orientation(self, layer=None) :

		if layer == None :
			layer = self.current_layer if self.current_layer is not None else self.document.getroot()
		
		transform = self.get_transforms(layer)
		if transform != [] : 
			transform = self.reverse_transform(transform)
			transform = simpletransform.formatTransform(transform)

		if self.options.orientation_points_count == "graffiti" :
			print_(self.graffiti_reference_points)
			print_("Inserting graffiti points")
			if layer in self.graffiti_reference_points: graffiti_reference_points_count =  len(self.graffiti_reference_points[layer])
			else: graffiti_reference_points_count = 0
			axis = ["X","Y","Z","A"][graffiti_reference_points_count%4]
			attr = {'gcodetools': "Gcodetools graffiti reference point"}
			if 	transform != [] :
				attr["transform"] = transform 
			g = inkex.etree.SubElement(layer, inkex.addNS('g','svg'), attr)
			inkex.etree.SubElement(	g, inkex.addNS('path','svg'), 
				{
					'style':	"stroke:none;fill:#00ff00;", 	
					'd':'m %s,%s 2.9375,-6.343750000001 0.8125,1.90625 6.843748640396,-6.84374864039 0,0 0.6875,0.6875 -6.84375,6.84375 1.90625,0.812500000001 z z' % (graffiti_reference_points_count*100, 0),
					'gcodetools': "Gcodetools graffiti reference point arrow"
				})
			
			draw_text(axis,graffiti_reference_points_count*100+10,-10, group = g, gcodetools_tag = "Gcodetools graffiti reference point text")

		elif self.options.orientation_points_count == "in-out reference point" :
			draw_pointer(group = self.current_layer, x = self.view_center, figure="arrow", pointer_type = "In-out reference point", text = "In-out point")
	
		else :
			print_("Inserting orientation points")

			if layer in self.orientation_points:
				self.error(_("Active layer already has orientation points! Remove them or select another layer!"),"active_layer_already_has_orientation_points")
			
			attr = {"gcodetools":"Gcodetools orientation group"}
			if 	transform != [] :
				attr["transform"] = transform 

			orientation_group = inkex.etree.SubElement(layer, inkex.addNS('g','svg'), attr)
			doc_height = self.unittouu(self.document.getroot().get('height'))
			if self.document.getroot().get('height') == "100%" :
				doc_height = 1052.3622047
				print_("Overruding height from 100 percents to %s" % doc_height)
			if self.options.unit == "G21 (All units in mm)" : 
				points = [[0.,0.,self.options.Zsurface],[100.,0.,self.options.Zdepth],[0.,100.,0.]]
				orientation_scale = 3.5433070660
				print_("orientation_scale < 0 ===> switching to mm units=%0.10f"%orientation_scale )
			elif self.options.unit == "G20 (All units in inches)" :
				points = [[0.,0.,self.options.Zsurface],[5.,0.,self.options.Zdepth],[0.,5.,0.]]
				orientation_scale = 90
				print_("orientation_scale < 0 ===> switching to inches units=%0.10f"%orientation_scale )
			if self.options.orientation_points_count == "2" :
				points = points[:2]
			print_(("using orientation scale",orientation_scale,"i=",points))
			for i in points :
				si = [i[0]*orientation_scale, i[1]*orientation_scale]
				g = inkex.etree.SubElement(orientation_group, inkex.addNS('g','svg'), {'gcodetools': "Gcodetools orientation point (%s points)" % self.options.orientation_points_count})
				inkex.etree.SubElement(	g, inkex.addNS('path','svg'), 
					{
						'style':	"stroke:none;fill:#000000;", 	
						'd':'m %s,%s 2.9375,-6.343750000001 0.8125,1.90625 6.843748640396,-6.84374864039 0,0 0.6875,0.6875 -6.84375,6.84375 1.90625,0.812500000001 z z' % (si[0], -si[1]+doc_height),
						'gcodetools': "Gcodetools orientation point arrow"
					})

				draw_text("(%s; %s; %s)" % (i[0],i[1],i[2]), (si[0]+10), (-si[1]-10+doc_height), group = g, gcodetools_tag = "Gcodetools orientation point text")

		
################################################################################
###
###		Tools library
###
################################################################################
	def tools_library(self, layer=None) :
		# Add a tool to the drawing
		if layer == None :
			layer = self.current_layer if self.current_layer is not None else self.document.getroot()
		if layer in self.tools:
			self.error(_("Active layer already has a tool! Remove it or select another layer!"),"active_layer_already_has_tool")

		if self.options.tools_library_type == "cylinder cutter" :
			tool = {
					"name": "Cylindrical cutter",
					"id": "Cylindrical cutter 0001",
					"diameter":10,
					"penetration angle":90,
					"feed":"400",
					"penetration feed":"100",
					"depth step":"1",
					"tool change gcode":" "
			}
		elif self.options.tools_library_type == "lathe cutter" :
			tool = {
					"name": "Lathe cutter",
					"id": "Lathe cutter 0001",
					"diameter":10,
					"penetration angle":90,
					"feed":"400",
					"passing feed":"800",
					"fine feed":"100",
					"penetration feed":"100",
					"depth step":"1",
					"tool change gcode":" "
			}
		elif self.options.tools_library_type == "cone cutter":	
			tool = {
					"name": "Cone cutter",
					"id": "Cone cutter 0001",
					"diameter":10,
					"shape":"w",
					"feed":"400",
					"penetration feed":"100",
					"depth step":"1",
					"tool change gcode":" "
			}
		elif self.options.tools_library_type == "tangent knife":	
			tool = {
					"name": "Tangent knife",
					"id": "Tangent knife 0001",
					"feed":"400",
					"penetration feed":"100",
					"depth step":"100",
					"4th axis meaning": "tangent knife",
					"4th axis scale": 1.,
					"4th axis offset": 0,
					"tool change gcode":" "
			}
			
		elif self.options.tools_library_type == "plasma cutter":	
			tool = {
				"name": "Plasma cutter",
				"id": "Plasma cutter 0001",
				"diameter":10,
				"penetration feed":100,
				"feed":400,
				"gcode before path":"""G31 Z-100 F500 (find metal)
G92 Z0 (zero z)
G00 Z10 F500 (going up)
M03 (turn on plasma)
G04 P0.2 (pause)
G01 Z1 (going to cutting z)\n""",
				"gcode after path":"M05 (turn off plasma)\n",
			}
		elif self.options.tools_library_type == "graffiti":	
			tool = {
				"name": "Graffiti",
				"id": "Graffiti 0001",
				"diameter":10,
				"penetration feed":100,
				"feed":400,
				"gcode before path":"""M03 S1(Turn spray on)\n """,
				"gcode after path":"M05 (Turn spray off)\n ",
				"tool change gcode":"(Add G00 here to change sprayer if needed)\n",
				
			}

		else :
			tool = self.default_tool
		
		tool_num = sum([len(self.tools[i]) for i in self.tools])
		colors = ["00ff00","0000ff","ff0000","fefe00","00fefe", "fe00fe", "fe7e00", "7efe00", "00fe7e", "007efe", "7e00fe", "fe007e"]
		
		tools_group = inkex.etree.SubElement(layer, inkex.addNS('g','svg'), {'gcodetools': "Gcodetools tool definition"})
		bg = inkex.etree.SubElement(	tools_group, inkex.addNS('path','svg'), 
					{'style':	"fill:#%s;fill-opacity:0.5;stroke:#444444; stroke-width:1px;"%colors[tool_num%len(colors)], "gcodetools":"Gcodetools tool background"})

		y = 0
		keys = []
		for key in self.tools_field_order:
			if key in tool: keys += [key]
		for key in tool:
			if key not in keys: keys += [key]
		for key in keys :
			g = inkex.etree.SubElement(tools_group, inkex.addNS('g','svg'), {'gcodetools': "Gcodetools tool parameter"})
			draw_text(key, 0, y, group = g, gcodetools_tag = "Gcodetools tool definition field name", font_size = 10 if key!='name' else 20)
			param = tool[key]
			if type(param)==str and re.match("^\s*$",param) : param = "(None)"
			draw_text(param, 150, y, group = g, gcodetools_tag = "Gcodetools tool definition field value", font_size = 10 if key!='name' else 20)
			v = str(param).split("\n")
			y += 15*len(v) if key!='name' else 20*len(v)

		bg.set('d',"m -20,-20 l 400,0 0,%f -400,0 z " % (y+50))
		tool = []
		tools_group.set("transform", simpletransform.formatTransform([ [1,0,self.view_center[0]-150 ], [0,1,self.view_center[1]] ] ))


################################################################################
###
###		Check tools and OP asignment
###
################################################################################
	def check_tools_and_op(self):
		if len(self.selected)<=0 :
			self.error(_("Selection is empty! Will compute whole drawing."),"selection_is_empty_will_comupe_drawing")
			paths = self.paths
		else :
			paths = self.selected_paths
		#	Set group
		group = inkex.etree.SubElement( self.selected_paths.keys()[0] if len(self.selected_paths.keys())>0 else self.layers[0], inkex.addNS('g','svg') )
		trans_ = [[1,0.3,0],[0,0.5,0]]	

		self.set_markers()

		bounds = [float('inf'),float('inf'),float('-inf'),float('-inf')]
		tools_bounds = {}
		for layer in self.layers :
			if layer in paths :
				self.set_tool(layer)
				tool = self.tools[layer][0]
				tools_bounds[layer] = tools_bounds[layer] if layer in tools_bounds else [float("inf"),float("-inf")]
				style = simplestyle.formatStyle(tool["style"])
				for path in paths[layer] :
					style = "fill:%s; fill-opacity:%s; stroke:#000044; stroke-width:1; marker-mid:url(#CheckToolsAndOPMarker);" % (
					tool["style"]["fill"] if "fill" in tool["style"] else "#00ff00", 
					tool["style"]["fill-opacity"] if "fill-opacity" in tool["style"] else "0.5")
					group.insert( 0, inkex.etree.Element(path.tag, path.attrib))
					new = group.getchildren()[0]
					new.set("style", style)
					
					trans = self.get_transforms(path)
					trans = simpletransform.composeTransform( trans_, trans if trans != [] else [[1.,0.,0.],[0.,1.,0.]])
					csp = cubicsuperpath.parsePath(path.get("d"))
					simpletransform.applyTransformToPath(trans,csp)
					path_bounds = csp_simple_bound(csp)
					trans = simpletransform.formatTransform(trans)
					bounds = [min(bounds[0],path_bounds[0]), min(bounds[1],path_bounds[1]), max(bounds[2],path_bounds[2]), max(bounds[3],path_bounds[3])]
					tools_bounds[layer] = [min(tools_bounds[layer][0], path_bounds[1]), max(tools_bounds[layer][1], path_bounds[3])] 

					new.set("transform", trans) 
					trans_[1][2] += 20
				trans_[1][2] += 100

		for layer in self.layers :
			if layer in self.tools :
				if layer in tools_bounds :
					tool = self.tools[layer][0]
					g = copy.deepcopy(tool["self_group"])
					g.attrib["gcodetools"] = "Check tools and OP asignment"
					trans = [[1,0.3,bounds[2]],[0,0.5,tools_bounds[layer][0]]]	
					g.set("transform",simpletransform.formatTransform(trans))
					group.insert( 0, g )


################################################################################
###		TODO Launch browser on help tab
################################################################################
	def help(self):
		self.error(_("""Tutorials, manuals and support can be found at\nEnglish support forum:\n	http://www.cnc-club.ru/gcodetools\nand Russian support forum:\n	http://www.cnc-club.ru/gcodetoolsru"""),"warning")
		return


################################################################################
###		Lathe
################################################################################
	def generate_lathe_gcode(self, subpath, layer, feed_type) :
		if len(subpath) <2 : return ""
		feed = " F %f" % self.tool[feed_type] 
		x,z = self.options.lathe_x_axis_remap, self.options.lathe_z_axis_remap
		flip_angle = -1 if x.lower()+z.lower() in ["xz", "yx", "zy"] else 1
		alias = {"X":"I", "Y":"J", "Z":"K", "x":"i", "y":"j", "z":"k"} 
		i_, k_ = alias[x], alias[z]
		c = [ [subpath[0][1], "move", 0, 0, 0] ]
		#draw_csp(self.transform_csp([subpath],layer,True), color = "Orange", width = .1)
		for sp1,sp2 in zip(subpath,subpath[1:]) :
			c += biarc(sp1,sp2,0,0)
		for i in range(1,len(c)) : # Just in case check end point of each segment
			c[i-1][4] = c[i][0][:]
		c += [ [subpath[-1][1], "end", 0, 0, 0] ]			
		self.draw_curve(c, layer, style = styles["biarc_style_lathe_%s" % feed_type])
		
		gcode = ("G01 %s %f %s %f" % (x, c[0][4][0], z, c[0][4][1]) ) + feed + "\n" # Just in case move to the start...
		for s in c :
			if s[1] == 'line':
				gcode += ("G01 %s %f %s %f" % (x, s[4][0], z, s[4][1]) ) + feed + "\n"
			elif s[1] == 'arc':
				r = [(s[2][0]-s[0][0]), (s[2][1]-s[0][1])]
				if (r[0]**2 + r[1]**2)>self.options.min_arc_radius**2:
					r1, r2 = (P(s[0])-P(s[2])), (P(s[4])-P(s[2]))
					if abs(r1.mag()-r2.mag()) < 0.001 :
						gcode += ("G02" if s[3]*flip_angle<0 else "G03") + (" %s %f %s %f %s %f %s %f" % (x,s[4][0],z,s[4][1],i_,(s[2][0]-s[0][0]), k_, (s[2][1]-s[0][1]) ) ) + feed + "\n"
					else:
						r = (r1.mag()+r2.mag())/2
						gcode += ("G02" if s[3]*flip_angle<0 else "G03") + (" %s %f %s %f" % (x,s[4][0],z,y[4][1]) ) + " R%f"%r + feed + "\n"
		return gcode


	def lathe(self):
		if not self.check_dir() : return
		x,z = self.options.lathe_x_axis_remap, self.options.lathe_z_axis_remap
		x = re.sub("^\s*([XYZxyz])\s*$",r"\1",x) 
		z = re.sub("^\s*([XYZxyz])\s*$",r"\1",z)
		if x not in ["X", "Y", "Z", "x", "y", "z"] or z not in ["X", "Y", "Z", "x", "y", "z"] :
			self.error(_("Lathe X and Z axis remap should be 'X', 'Y' or 'Z'. Exiting..."),"warning") 
			return
		if x.lower() == z.lower() :
			self.error(_("Lathe X and Z axis remap should be the same. Exiting..."),"warning") 
			return
		if 	x.lower()+z.lower() in ["xy","yx"] : gcode_plane_selection = "G17 (Using XY plane)\n"
		if 	x.lower()+z.lower() in ["xz","zx"] : gcode_plane_selection = "G18 (Using XZ plane)\n"
		if 	x.lower()+z.lower() in ["zy","yz"] : gcode_plane_selection = "G19 (Using YZ plane)\n"
		self.options.lathe_x_axis_remap, self.options.lathe_z_axis_remap = x, z
	
		paths = self.selected_paths
		self.tool = []
		gcode = ""
		for layer in self.layers :
			if layer in paths :
				self.set_tool(layer)
				if self.tool != self.tools[layer][0] :
					self.tool = self.tools[layer][0]
					self.tool["passing feed"]	= float(self.tool["passing feed"] if "passing feed" in self.tool else self.tool["feed"])
					self.tool["feed"]			= float(self.tool["feed"])
					self.tool["fine feed"]		= float(self.tool["fine feed"] if "fine feed" in self.tool else self.tool["feed"]) 
					gcode += ( "(Change tool to %s)\n" % re.sub("\"'\(\)\\\\"," ",self.tool["name"]) ) + self.tool["tool change gcode"] + "\n"
					
				for path in paths[layer]:
					csp = self.transform_csp(cubicsuperpath.parsePath(path.get("d")),layer)
					
					for subpath in csp :
						# Offset the path if fine cut is defined.
						fine_cut = subpath[:]
						if self.options.lathe_fine_cut_width>0 :
							r = self.options.lathe_fine_cut_width
							if self.options.lathe_create_fine_cut_using == "Move path" :
								subpath = [ [  [i2[0],i2[1]+r]  for i2 in i1]  for i1 in subpath]
							else :
								# Close the path to make offset correct 
								bound = csp_simple_bound([subpath])
								minx,miny,maxx,maxy = csp_true_bounds([subpath])
								offsetted_subpath = csp_subpath_line_to(subpath[:], [ [subpath[-1][1][0], miny[1]-r*10 ], [subpath[0][1][0], miny[1]-r*10 ], [subpath[0][1][0], subpath[0][1][1] ]  ])
								left,right = subpath[-1][1][0], subpath[0][1][0]
								if left>right : left, right = right,left
								offsetted_subpath = csp_offset([offsetted_subpath], r if not csp_subpath_ccw(offsetted_subpath) else -r ) 
								offsetted_subpath = csp_clip_by_line(offsetted_subpath,  [left,10], [left,0] )
								offsetted_subpath = csp_clip_by_line(offsetted_subpath,  [right,0], [right,10] )
								offsetted_subpath = csp_clip_by_line(offsetted_subpath,  [0, miny[1]-r], [10, miny[1]-r] )
								#draw_csp(self.transform_csp(offsetted_subpath,layer,True), color = "Green", width = 1)
								# Join offsetted_subpath together 
								# Hope there wont be any cicles
								subpath = csp_join_subpaths(offsetted_subpath)[0]
						
						# Create solid object from path and lathe_width 
						bound = csp_simple_bound([subpath])
						top_start, top_end = [subpath[0][1][0], self.options.lathe_width+self.options.Zsafe+self.options.lathe_fine_cut_width], [subpath[-1][1][0], self.options.lathe_width+self.options.Zsafe+self.options.lathe_fine_cut_width]

						gcode += ("G01 %s %f F %f \n" % (z, top_start[1], self.tool["passing feed"]) )
						gcode += ("G01 %s %f %s %f F %f \n" % (x, top_start[0], z, top_start[1], self.tool["passing feed"]) )

						subpath = csp_concat_subpaths(csp_subpath_line_to([],[top_start,subpath[0][1]]), subpath)
						subpath = csp_subpath_line_to(subpath,[top_end,top_start])
	
						
						width = max(0, self.options.lathe_width - max(0, bound[1]) )
						step = self.tool['depth step']
						steps = int(math.ceil(width/step))
						for i in range(steps+1):
							current_width = self.options.lathe_width - step*i
							intersections = []
							for j in range(1,len(subpath)) :
								sp1,sp2 = subpath[j-1], subpath[j]
								intersections += [[j,k] for k in csp_line_intersection([bound[0]-10,current_width], [bound[2]+10,current_width], sp1, sp2)]
								intersections += [[j,k] for k in csp_line_intersection([bound[0]-10,current_width+step], [bound[2]+10,current_width+step], sp1, sp2)]
							parts = csp_subpath_split_by_points(subpath,intersections)
							for part in parts :
								minx,miny,maxx,maxy = csp_true_bounds([part])
								y = (maxy[1]+miny[1])/2
								if  y > current_width+step :
									gcode += self.generate_lathe_gcode(part,layer,"passing feed")
								elif current_width <= y <= current_width+step :
									gcode += self.generate_lathe_gcode(part,layer,"feed")
								else :
									# full step cut
									part = csp_subpath_line_to([], [part[0][1], part[-1][1]] )
									gcode += self.generate_lathe_gcode(part,layer,"feed")
									
						top_start, top_end = [fine_cut[0][1][0], self.options.lathe_width+self.options.Zsafe+self.options.lathe_fine_cut_width], [fine_cut[-1][1][0], self.options.lathe_width+self.options.Zsafe+self.options.lathe_fine_cut_width]
						gcode += "\n(Fine cutting start)\n(Calculating fine cut using %s)\n"%self.options.lathe_create_fine_cut_using
						for i in range(self.options.lathe_fine_cut_count) :
							width = self.options.lathe_fine_cut_width*(1-float(i+1)/self.options.lathe_fine_cut_count ) 
							if width == 0 : 
								current_pass = fine_cut							
							else :	
								if self.options.lathe_create_fine_cut_using == "Move path" :
									current_pass = [ [  [i2[0],i2[1]+width]  for i2 in i1]  for i1 in fine_cut]
								else :
									minx,miny,maxx,maxy = csp_true_bounds([fine_cut])
									offsetted_subpath = csp_subpath_line_to(fine_cut[:], [ [fine_cut[-1][1][0], miny[1]-r*10 ], [fine_cut[0][1][0], miny[1]-r*10 ], [fine_cut[0][1][0], fine_cut[0][1][1] ]  ])
									left,right = fine_cut[-1][1][0], fine_cut[0][1][0]
									if left>right : left, right = right,left
									offsetted_subpath = csp_offset([offsetted_subpath], width if not csp_subpath_ccw(offsetted_subpath) else -width ) 
									offsetted_subpath = csp_clip_by_line(offsetted_subpath,  [left,10], [left,0] )
									offsetted_subpath = csp_clip_by_line(offsetted_subpath,  [right,0], [right,10] )
									offsetted_subpath = csp_clip_by_line(offsetted_subpath,  [0, miny[1]-r], [10, miny[1]-r] )
									current_pass = csp_join_subpaths(offsetted_subpath)[0]


							gcode += "\n(Fine cut %i-th cicle start)\n"%(i+1)					
							gcode += ("G01 %s %f %s %f F %f \n" % (x, top_start[0], z, top_start[1], self.tool["passing feed"]) )
							gcode += ("G01 %s %f %s %f F %f \n" % (x, current_pass[0][1][0], z, current_pass[0][1][1]+self.options.lathe_fine_cut_width, self.tool["passing feed"]) )
							gcode += ("G01 %s %f %s %f F %f \n" % (x, current_pass[0][1][0], z, current_pass[0][1][1], self.tool["fine feed"]) )
						
							gcode += self.generate_lathe_gcode(current_pass,layer,"fine feed")
							gcode += ("G01 %s %f F %f \n" % (z, top_start[1], self.tool["passing feed"]) )
							gcode += ("G01 %s %f %s %f F %f \n" % (x, top_start[0], z, top_start[1], self.tool["passing feed"]) )
	
		self.export_gcode(gcode)
		
################################################################################
###
###		Lathe modify path 
### 	Modifies path to fit current cutter. As for now straight rect cutter.
###
################################################################################

	def lathe_modify_path(self):
		if self.selected_paths == {} and self.options.auto_select_paths:
			paths=self.paths
			self.error(_("No paths are selected! Trying to work on all available paths."),"warning")
		else :
			paths = self.selected_paths

		for layer in self.layers :
			if layer in paths :
				width = self.options.lathe_rectangular_cutter_width
				#self.set_tool(layer)
				for path in paths[layer]:
					csp = self.transform_csp(cubicsuperpath.parsePath(path.get("d")),layer)
					new_csp = []
					for subpath in csp: 
						orientation = subpath[-1][1][0]>subpath[0][1][0]
						last_n = None
						last_o = 0
						new_subpath = []

						# Split segment at x' and y' == 0
						for sp1, sp2 in zip(subpath[:],subpath[1:]):
							ax,ay,bx,by,cx,cy,dx,dy = csp_parameterize(sp1,sp2)
							roots = cubic_solver_real(0, 3*ax, 2*bx, cx)
							roots += cubic_solver_real(0, 3*ay, 2*by, cy)
							new_subpath = csp_concat_subpaths(new_subpath, csp_seg_split(sp1,sp2,roots))
						subpath = new_subpath
						new_subpath = []
						first_seg = True
						for sp1, sp2 in zip(subpath[:],subpath[1:]):
							n = csp_normalized_normal(sp1,sp2,0)
							a  = math.atan2(n[0],n[1])
							if a == 0 or a == math.pi :
								n = csp_normalized_normal(sp1,sp2,1)
							a  = math.atan2(n[0],n[1])							
							if a!=0 and a!=math.pi:
								o = 0 if 0<a<=math.pi/2 or -math.pi<a<-math.pi/2 else 1
								if not orientation: o = 1-o
								
								# Add first horisontal straight line if needed
								if not first_seg and new_subpath==[] : new_subpath = [ [[subpath[0][i][0] - width*o ,subpath[0][i][1]] for i in range(3)] ]
								
								new_subpath = csp_concat_subpaths(
										new_subpath,
										[
											[[sp1[i][0] - width*o ,sp1[i][1]] for i in range(3)],
											[[sp2[i][0] - width*o ,sp2[i][1]] for i in range(3)]
										]
									)
							first_seg = False
							
						# Add last horisontal straigth line if needed
						if a==0 or a==math.pi :
							new_subpath +=  [ [[subpath[-1][i][0] - width*o ,subpath[-1][i][1]] for i in range(3)] ]

						
					new_csp += [new_subpath]	
					self.draw_csp(new_csp,layer)
#								
#								o = (1 if cross(n, [0,1])>0 else -1)*orientation
#								new_subpath += [  [sp1[i][0] - width*o,sp1[i][1]] for i in range(3)  ]
#							n = csp_normalized_normal(sp1,sp2,1) 
#							o = (1 if cross(n, [0,1])>0 else -1)*orientation
#							new_subpath += [  [sp2[i][0] - width*o,sp2[i][1]] for i in range(3)  ]
							
							
################################################################################
###
### Update function
###
###	Gets file containing version information from the web and compaares it with.
###	current version.
################################################################################
	
	def update(self) :
		try :
			import urllib
			f = urllib.urlopen("http://www.cnc-club.ru/gcodetools_latest_version", proxies = urllib.getproxies())
			a = f.read()
			for s in a.split("\n") :
				r = re.search(r"Gcodetools\s+latest\s+version\s*=\s*(.*)",s)
				if r : 
					ver = r.group(1).strip()
					if ver != gcodetools_current_version :
						self.error("There is a newer version of Gcodetools you can get it at: \nhttp://www.cnc-club.ru/gcodetools (English version). \nhttp://www.cnc-club.ru/gcodetools_ru (Russian version). ","Warning")					
					else :
						self.error("You are currently using latest stable version of Gcodetools.","Warning")					
					return 
			self.error("Can not check the latest version. You can check it manualy at \nhttp://www.cnc-club.ru/gcodetools (English version). \nhttp://www.cnc-club.ru/gcodetools_ru (Russian version). \nCurrent version is Gcodetools %s"%gcodetools_current_version,"Warning")					
		except :
			self.error("Can not check the latest version. You can check it manualy at \nhttp://www.cnc-club.ru/gcodetools (English version). \nhttp://www.cnc-club.ru/gcodetools_ru (Russian version). \nCurrent version is Gcodetools %s"%gcodetools_current_version,"Warning")					
				


################################################################################
### Graffiti function generates Gcode for graffiti drawer
################################################################################
	def graffiti(self) :
		# Get reference points. 
		
		def get_gcode_coordinates(point,layer):
			gcode = ''	
			pos = []
			for ref_point in self.graffiti_reference_points[layer] :
				c = math.sqrt((point[0]-ref_point[0][0])**2 + (point[1]-ref_point[0][1])**2)
				gcode += " %s %f"%(ref_point[1], c)
				pos += [c]
			return pos, gcode
		
		
		def graffiti_preview_draw_point(x1,y1,color,radius=.5):
			self.graffiti_preview = self.graffiti_preview
			r,g,b,a_ = color
			for x in range(int(x1-1-math.ceil(radius)), int(x1+1+math.ceil(radius)+1)):
				for y in range(int(y1-1-math.ceil(radius)), int(y1+1+math.ceil(radius)+1)):
					if x>=0 and y>=0 and y<len(self.graffiti_preview) and x*4<len(self.graffiti_preview[0]) :
						d = math.sqrt( (x1-x)**2 +(y1-y)**2 )
						a = float(a_)*( max(0,(1-(d-radius))) if d>radius else 1 )/256
						self.graffiti_preview[y][x*4] = int(r*a + (1-a)*self.graffiti_preview[y][x*4])
						self.graffiti_preview[y][x*4+1] = int(g*a + (1-a)*self.graffiti_preview[y][x*4+1])
						self.graffiti_preview[y][x*4+2] = int(g*b + (1-a)*self.graffiti_preview[y][x*4+2])
						self.graffiti_preview[y][x*4+3] = min(255,int(self.graffiti_preview[y][x*4+3]+a*256))

		def graffiti_preview_transform(x,y):
			tr = self.graffiti_preview_transform
			d = max(tr[2]-tr[0]+2,tr[3]-tr[1]+2) 
			return  [(x-tr[0]+1)*self.options.graffiti_preview_size/d, self.options.graffiti_preview_size - (y-tr[1]+1)*self.options.graffiti_preview_size/d]


		def draw_graffiti_segment(layer,start,end,feed,color=(0,255,0,40),emmit=1000):
			# Emit = dots per second
			l = math.sqrt(sum([(start[i]-end[i])**2 for i in range(len(start))]))
			time_ = l/feed
			c1,c2 = self.graffiti_reference_points[layer][0][0],self.graffiti_reference_points[layer][1][0]
			d = math.sqrt( (c1[0]-c2[0])**2 + (c1[1]-c2[1])**2 )
			if d == 0 : raise ValueError, "Error! Reference points should not be the same!"
			for i in range(int(time_*emmit+1)) :
				t = i/(time_*emmit)
				r1,r2 = start[0]*(1-t) + end[0]*t, start[1]*(1-t) + end[1]*t
				a = (r1**2-r2**2+d**2)/(2*d)
				h = math.sqrt(r1**2 - a**2)
				xa = c1[0] + a*(c2[0]-c1[0])/d
				ya = c1[1] + a*(c2[1]-c1[1])/d
			
				x1 = xa + h*(c2[1]-c1[1])/d
				x2 = xa - h*(c2[1]-c1[1])/d
				y1 = ya - h*(c2[0]-c1[0])/d
				y2 = ya + h*(c2[0]-c1[0])/d
				
				x = x1 if y1<y2 else x2
				y = min(y1,y2)			
				x,y = graffiti_preview_transform(x,y)
				graffiti_preview_draw_point(x,y,color)
			
		def create_connector(p1,p2,t1,t2):
			P1,P2 = P(p1), P(p2)
			N1, N2  = P(rotate_ccw(t1)), P(rotate_ccw(t2))
			r = self.options.graffiti_min_radius
			C1,C2 = P1+N1*r, P2+N2*r
			# Get closest possible centers of arcs, also we define that arcs are both ccw or both not. 
			dc, N1, N2, m = (
					(
						(((P2-N1*r) - (P1-N2*r)).l2(),-N1,-N2, 1)
							 if  vectors_ccw(t1,t2) else
						(((P2+N1*r) - (P1+N2*r)).l2(), N1, N2,-1) 
					)
					 if vectors_ccw((P1-C1).to_list(),t1) == vectors_ccw((P2-C2).to_list(),t2) else
					(
						(((P2+N1*r) - (P1-N2*r)).l2(), N1,-N2, 1) 
							 if vectors_ccw(t1,t2) else
						(((P2-N1*r) - (P1+N2*r)).l2(),-N1, N2, 1)
					)	 
				)
			dc = math.sqrt(dc)
			C1,C2 = P1+N1*r, P2+N2*r
			Dc = C2-C1 
			
			if dc == 0 :
				# can be joined by one arc
				return csp_from_arc(p1, p2, C1.to_list(), r, t1)

			cos, sin = Dc.x/dc, Dc.y/dc
			#draw_csp(self.transform_csp([[ [[C1.x-r*sin,C1.y+r*cos]]*3,[[C2.x-r*sin,C2.y+r*cos]]*3 ]],layer,reverse=True), color = "#00ff00;" )
			#draw_pointer(self.transform(C1.to_list(),layer,reverse=True))
			#draw_pointer(self.transform(C2.to_list(),layer,reverse=True))
			
			p1_end = [C1.x-r*sin*m,C1.y+r*cos*m]
			p2_st  = [C2.x-r*sin*m,C2.y+r*cos*m]
			if point_to_point_d2(p1,p1_end)<0.0001 and point_to_point_d2(p2,p2_st)<0.0001 :
				return ([[p1,p1,p1],[p2,p2,p2]])
			
			arc1 = csp_from_arc(p1, p1_end, C1.to_list(), r, t1)
			arc2 = csp_from_arc(p2_st, p2, C2.to_list(), r, [cos,sin])
			return csp_concat_subpaths(arc1,arc2)

		if not self.check_dir() : return
		if self.selected_paths == {} and self.options.auto_select_paths:
			paths=self.paths
			self.error(_("No paths are selected! Trying to work on all available paths."),"warning")
		else :
			paths = self.selected_paths
		self.tool = []
		gcode = """(Header)
(Generated by gcodetools from Inkscape.)
(Using graffiti extension.)
(Header end.)"""
		
		minx,miny,maxx,maxy = float("inf"),float("inf"),float("-inf"),float("-inf") 

		# Get all reference points and path's bounds to make preview

		for layer in self.layers :
			if layer in paths :
				# Set reference points
				if layer not in self.graffiti_reference_points:
					reference_points = None
					for i in range(self.layers.index(layer),-1,-1):
						if self.layers[i] in self.graffiti_reference_points :
							reference_points = self.graffiti_reference_points[self.layers[i]]
							self.graffiti_reference_points[layer] = self.graffiti_reference_points[self.layers[i]]
							break
					if reference_points == None :
					 	self.error('There are no graffiti reference points for layer %s'%layer,"error")
					 	
				# Transform reference points
				for i in range(len(self.graffiti_reference_points[layer])):
					self.graffiti_reference_points[layer][i][0] = self.transform(self.graffiti_reference_points[layer][i][0], layer)
					point = self.graffiti_reference_points[layer][i]
					gcode += "(Reference point %f;%f for %s axis)\n"%(point[0][0],point[0][1],point[1])

				if self.options.graffiti_create_preview :
					for point in self.graffiti_reference_points[layer]:
						minx,miny,maxx,maxy = min(minx,point[0][0]), min(miny,point[0][1]), max(maxx,point[0][0]), max(maxy,point[0][1])
					for path in paths[layer]:
						csp = cubicsuperpath.parsePath(path.get("d"))
						csp = self.apply_transforms(path, csp)
						csp = self.transform_csp(csp, layer)
						bounds = csp_simple_bound(csp)
						minx,miny,maxx,maxy = min(minx,bounds[0]), min(miny,bounds[1]), max(maxx,bounds[2]), max(maxy,bounds[3]) 
						
		if self.options.graffiti_create_preview :
			self.graffiti_preview = list([ [255]*(4*self.options.graffiti_preview_size) for i in range(self.options.graffiti_preview_size)])
			self.graffiti_preview_transform = [minx,miny,maxx,maxy]

		for layer in self.layers :
			if layer in paths :

				r = re.match("\s*\(\s*([0-9\-,.]+)\s*;\s*([0-9\-,.]+)\s*\)\s*",self.options.graffiti_start_pos)
				if r :
					start_point = [float(r.group(1)),float(r.group(2))]
				else :
					start_point = [0.,0.]
				last_sp1 = [[start_point[0],start_point[1]-10] for i in range(3)]	
				last_sp2 = [start_point for i in range(3)]

				self.set_tool(layer)
				self.tool = self.tools[layer][0]
				# Change tool every layer. (Probably layer = color so it'll be 
				# better to change it even if the tool has not been changed)
				gcode += ( "(Change tool to %s)\n" % re.sub("\"'\(\)\\\\"," ",self.tool["name"]) ) + self.tool["tool change gcode"] + "\n"
				
				subpaths = []
				for path in paths[layer]:
					# Rebuild the paths to polyline.
					csp = cubicsuperpath.parsePath(path.get("d"))
					csp = self.apply_transforms(path, csp)
					csp = self.transform_csp(csp, layer)
					subpaths += csp 
				polylines = []
				while len(subpaths)>0:
					i = min( [( point_to_point_d2(last_sp2[1],subpaths[i][0][1]),i) for i in range(len(subpaths))] )[1]
					subpath = subpaths[i][:]
					del subpaths[i]
					polylines += [ 
									['connector', create_connector(
													last_sp2[1],
													subpath[0][1],
													csp_normalized_slope(last_sp1,last_sp2,1.),
													csp_normalized_slope(subpath[0],subpath[1],0.),
									)]
								]
					polyline = []
					spl = None
					
					#  remove zerro length segments
					i = 0
					while i<len(subpath)-1:
						if 	(cspseglength(subpath[i],subpath[i+1])<0.00000001 ) : 
							subpath[i][2] = subpath[i+1][2]
							del subpath[i+1]
						else :
							i += 1
					
					for sp1, sp2 in zip(subpath,subpath[1:]) :
						if spl != None and abs(cross( csp_normalized_slope(spl,sp1,1.),csp_normalized_slope(sp1,sp2,0.) )) > 0.1 : # TODO add coefficient into inx
							# We've got sharp angle at sp1.
							polyline += [sp1]
							polylines += [['draw',polyline[:]]]
							polylines += [ 
											['connector', create_connector(
													sp1[1],
													sp1[1],
													csp_normalized_slope(spl,sp1,1.),
													csp_normalized_slope(sp1,sp2,0.),
											)]
										]
							polyline = []
						# max_segment_length 
						polyline += [ sp1 ]
						print_(polyline)
						print_(sp1)
						
						spl = sp1
					polyline += [ sp2 ]	
					polylines += [ ['draw',polyline[:]] ]

					last_sp1, last_sp2 = sp1,sp2
					
					
				# Add return to start_point
				if polylines == [] : continue
				polylines += [ ["connect1",  [ [polylines[-1][1][-1][1] for i in range(3)],[start_point for i in range(3)] ] ] ]
				
				# Make polilynes from polylines. They are still csp.
				for i in range(len(polylines)) :
					polyline = []
					l = 0
					print_("polylines",polylines)
					print_(polylines[i])
					for sp1,sp2 in zip(polylines[i][1],polylines[i][1][1:]) :
						print_(sp1,sp2)
						l = cspseglength(sp1,sp2)
						if l>0.00000001 : 
							polyline += [sp1[1]]
							parts = int(math.ceil(l/self.options.graffiti_max_seg_length))
							for j in range(1,parts):
								polyline += [csp_at_length(sp1,sp2,float(j)/parts) ]
					if l>0.00000001 :
						polyline += [sp2[1]]
					print_(i)
					polylines[i][1] = polyline
									
				t = 0
				last_state = None
				for polyline_ in polylines:
					polyline = polyline_[1]
					# Draw linearization
					if self.options.graffiti_create_linearization_preview :
						t += 1
						csp = [ [polyline[i],polyline[i],polyline[i]] for i in range(len(polyline))]
						draw_csp(self.transform_csp([csp],layer,reverse=True), color = "#00cc00;" if polyline_[0]=='draw' else "#ff5555;")

	
				# Export polyline to gcode
				# we are making trnsform from XYZA coordinates to R1...Rn
				# where R1...Rn are radius vectors from grafiti reference points
				# to current (x,y) point. Also we need to assign custom feed rate 
				# for each segment. And we'll use only G01 gcode.
					last_real_pos, g = get_gcode_coordinates(polyline[0],layer)
					last_pos = polyline[0]
					if polyline_[0] == "draw" and last_state!="draw":
						gcode += self.tool['gcode before path']+"\n"
					for point in polyline :
						real_pos, g = get_gcode_coordinates(point,layer)
						real_l = sum([(real_pos[i]-last_real_pos[i])**2 for i in range(len(last_real_pos))])
						l = (last_pos[0]-point[0])**2 + (last_pos[1]-point[1])**2 
						if l!=0:
							feed = self.tool['feed']*math.sqrt(real_l/l)
							gcode += "G01 " + g + " F %f\n"%feed
							if self.options.graffiti_create_preview :			
								draw_graffiti_segment(layer,real_pos,last_real_pos,feed,color=(0,0,255,200) if polyline_[0] == "draw" else (255,0,0,200),emmit=self.options.graffiti_preview_emmit)
							last_real_pos = real_pos
							last_pos = point[:]
					if polyline_[0] == "draw" and last_state!="draw" :
						gcode += self.tool['gcode after path']+"\n"
					last_state = polyline_[0]
		self.export_gcode(gcode, no_headers=True)				
		if self.options.graffiti_create_preview :
			try :
				# Draw reference points			
				for layer in self.graffiti_reference_points:
					for point in self.graffiti_reference_points[layer] :
						x, y = graffiti_preview_transform(point[0][0],point[0][1])
						graffiti_preview_draw_point(x,y,(0,255,0,255),radius=5)
			
				import png
				writer = png.Writer(width=self.options.graffiti_preview_size, height=self.options.graffiti_preview_size, size=None, greyscale=False, alpha=True, bitdepth=8, palette=None, transparent=None, background=None, gamma=None, compression=None, interlace=False, bytes_per_sample=None, planes=None, colormap=None, maxval=None, chunk_limit=1048576)
				f = open(self.options.directory+self.options.file+".png", 'wb') 
				writer.write(f,self.graffiti_preview) 
				f.close()
				
			except :	
				self.error("Png module have not been found!","warning")
				

										
################################################################################
###
###		Effect
###
###		Main function of Gcodetools class
###
################################################################################
	def effect(self) :
		start_time = time.time()
		global options
		options = self.options
		options.self = self
		options.doc_root = self.document.getroot()

		# define print_ function 
		global print_
		if self.options.log_create_log :
			try :
				if os.path.isfile(self.options.log_filename) : os.remove(self.options.log_filename)
				f = open(self.options.log_filename,"a")
				f.write("Gcodetools log file.\nStarted at %s.\n%s\n" % (time.strftime("%d.%m.%Y %H:%M:%S"),options.log_filename))
				f.write("%s tab is active.\n" % self.options.active_tab)
				f.close()
			except :
				print_  = lambda *x : None 
		else : print_  = lambda *x : None 
		if self.options.active_tab == '"help"' :
			self.help()
			return
		elif self.options.active_tab == '"about"' :
			self.help()
			return
		
		elif self.options.active_tab ==  '"test"' :
			self.test()
			
		elif self.options.active_tab not in ['"dxfpoints"','"path-to-gcode"', '"area_fill"', '"area"', '"area_artefacts"', '"engraving"', '"orientation"', '"tools_library"', '"lathe"', '"offset"', '"arrangement"', '"update"', '"graffiti"', '"lathe_modify_path"', '"plasma-prepare-path"']:
			self.error(_("Select one of the action tabs - Path to Gcode, Area, Engraving, DXF points, Orientation, Offset, Lathe or Tools library.\n Current active tab id is %s" % self.options.active_tab),"error")
		else:
			# Get all Gcodetools data from the scene.
			self.get_info()
			if self.options.active_tab in ['"dxfpoints"','"path-to-gcode"', '"area_fill"', '"area"', '"area_artefacts"', '"engraving"', '"lathe"', '"graffiti"', '"plasma-prepare-path"']:
				if self.orientation_points == {} :
					self.error(_("Orientation points have not been defined! A default set of orientation points has been automatically added."),"warning")
					self.orientation( self.layers[min(1,len(self.layers)-1)] )		
					self.get_info()
				if self.tools == {} :
					self.error(_("Cutting tool has not been defined! A default tool has been automatically added."),"warning")
					self.options.tools_library_type = "default"
					self.tools_library( self.layers[min(1,len(self.layers)-1)] )
					self.get_info()
			if self.options.active_tab == '"path-to-gcode"': 
				self.path_to_gcode()		
			elif self.options.active_tab == '"area_fill"': 
				self.area_fill()		
			elif self.options.active_tab == '"area"': 
				self.area()		
			elif self.options.active_tab == '"area_artefacts"': 
				self.area_artefacts()		
			elif self.options.active_tab == '"dxfpoints"': 
				self.dxfpoints()		
			elif self.options.active_tab == '"engraving"': 
				self.engraving()		
			elif self.options.active_tab == '"orientation"': 
				self.orientation()		
			elif self.options.active_tab == '"graffiti"': 
				self.graffiti()		
			elif self.options.active_tab == '"tools_library"': 
				if self.options.tools_library_type != "check":
					self.tools_library()
				else :	
					self.check_tools_and_op()
			elif self.options.active_tab == '"lathe"': 
				self.lathe()
			elif self.options.active_tab == '"lathe_modify_path"': 
				self.lathe_modify_path()
			elif self.options.active_tab == '"update"': 
				self.update()
			elif self.options.active_tab == '"offset"': 
				if self.options.offset_just_get_distance :
					for layer in self.selected_paths :
						if len(self.selected_paths[layer]) == 2 :
							csp1, csp2 = cubicsuperpath.parsePath(self.selected_paths[layer][0].get("d")), cubicsuperpath.parsePath(self.selected_paths[layer][1].get("d"))
							dist = csp_to_csp_distance(csp1,csp2)
							print_(dist)
							draw_pointer( list(csp_at_t(csp1[dist[1]][dist[2]-1],csp1[dist[1]][dist[2]],dist[3]))
										+list(csp_at_t(csp2[dist[4]][dist[5]-1],csp2[dist[4]][dist[5]],dist[6])),"red","line", comment = math.sqrt(dist[0]))
					return	
				if self.options.offset_step == 0 : self.options.offset_step = self.options.offset_radius
				if self.options.offset_step*self.options.offset_radius <0 : self.options.offset_step *= -1
				time_ = time.time()
				offsets_count = 0
				for layer in self.selected_paths :
					for path in self.selected_paths[layer] :
										
						offset = self.options.offset_step/2
						while abs(offset) <= abs(self.options.offset_radius) :
							offset_ = csp_offset(cubicsuperpath.parsePath(path.get("d")), offset)				
							offsets_count += 1
							if offset_ != [] : 
								for iii in offset_ :
									draw_csp([iii], color="Green", width=1)		
									#print_(offset_)
							else :
								print_("------------Reached empty offset at radius %s"% offset )
								break
							offset += self.options.offset_step
				print_()
				print_("-----------------------------------------------------------------------------------")				
				print_("-----------------------------------------------------------------------------------")				
				print_("-----------------------------------------------------------------------------------")				
				print_()
				print_("Done in %s"%(time.time()-time_))				
				print_("Total offsets count %s"%offsets_count)				
			elif self.options.active_tab == '"arrangement"': 
				self.arrangement()

			elif self.options.active_tab == '"plasma-prepare-path"': 
				self.plasma_prepare_path()		


		print_("------------------------------------------")
		print_("Done in %f seconds"%(time.time()-start_time))
		print_("End at %s."%time.strftime("%d.%m.%Y %H:%M:%S"))
		
		
#						
gcodetools = Gcodetools()
gcodetools.affect()					

