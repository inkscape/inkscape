'''
Copyright (C) 2006 Jean-Francois Barraud, barraud@math.univ-lille1.fr
Copyright (C) 2010 Alvin Penner, penner@vaxxine.com

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
barraud@math.univ-lille1.fr

This code defines several functions to make handling of transform
attribute easier.
'''
import inkex, cubicsuperpath, bezmisc, simplestyle
import copy, math, re

def parseTransform(transf,mat=[[1.0, 0.0, 0.0], [0.0, 1.0, 0.0]]):
    if transf=="" or transf==None:
        return(mat)
    stransf = transf.strip()
    result=re.match("(translate|scale|rotate|skewX|skewY|matrix)\s*\(([^)]*)\)\s*,?",stransf)
#-- translate --
    if result.group(1)=="translate":
        args=result.group(2).replace(',',' ').split()
        dx=float(args[0])
        if len(args)==1:
            dy=0.0
        else:
            dy=float(args[1])
        matrix=[[1,0,dx],[0,1,dy]]
#-- scale --
    if result.group(1)=="scale":
        args=result.group(2).replace(',',' ').split()
        sx=float(args[0])
        if len(args)==1:
            sy=sx
        else:
            sy=float(args[1])
        matrix=[[sx,0,0],[0,sy,0]]
#-- rotate --
    if result.group(1)=="rotate":
        args=result.group(2).replace(',',' ').split()
        a=float(args[0])*math.pi/180
        if len(args)==1:
            cx,cy=(0.0,0.0)
        else:
            cx,cy=map(float,args[1:])
        matrix=[[math.cos(a),-math.sin(a),cx],[math.sin(a),math.cos(a),cy]]
        matrix=composeTransform(matrix,[[1,0,-cx],[0,1,-cy]])
#-- skewX --
    if result.group(1)=="skewX":
        a=float(result.group(2))*math.pi/180
        matrix=[[1,math.tan(a),0],[0,1,0]]
#-- skewY --
    if result.group(1)=="skewY":
        a=float(result.group(2))*math.pi/180
        matrix=[[1,0,0],[math.tan(a),1,0]]
#-- matrix --
    if result.group(1)=="matrix":
        a11,a21,a12,a22,v1,v2=result.group(2).replace(',',' ').split()
        matrix=[[float(a11),float(a12),float(v1)], [float(a21),float(a22),float(v2)]]

    matrix=composeTransform(mat,matrix)
    if result.end() < len(stransf):
        return(parseTransform(stransf[result.end():], matrix))
    else:
        return matrix

def formatTransform(mat):
    return ("matrix(%f,%f,%f,%f,%f,%f)" % (mat[0][0], mat[1][0], mat[0][1], mat[1][1], mat[0][2], mat[1][2]))

def invertTransform(mat):
    det = mat[0][0]*mat[1][1] - mat[0][1]*mat[1][0]
    if det !=0:  # det is 0 only in case of 0 scaling
        # invert the rotation/scaling part
        a11 =  mat[1][1]/det
        a12 = -mat[0][1]/det
        a21 = -mat[1][0]/det
        a22 =  mat[0][0]/det
        # invert the translational part
        a13 = -(a11*mat[0][2] + a12*mat[1][2])
        a23 = -(a21*mat[0][2] + a22*mat[1][2])
        return [[a11,a12,a13],[a21,a22,a23]]
    else:
        return[[0,0,-mat[0][2]],[0,0,-mat[1][2]]]

def composeTransform(M1,M2):
    a11 = M1[0][0]*M2[0][0] + M1[0][1]*M2[1][0]
    a12 = M1[0][0]*M2[0][1] + M1[0][1]*M2[1][1]
    a21 = M1[1][0]*M2[0][0] + M1[1][1]*M2[1][0]
    a22 = M1[1][0]*M2[0][1] + M1[1][1]*M2[1][1]

    v1 = M1[0][0]*M2[0][2] + M1[0][1]*M2[1][2] + M1[0][2]
    v2 = M1[1][0]*M2[0][2] + M1[1][1]*M2[1][2] + M1[1][2]
    return [[a11,a12,v1],[a21,a22,v2]]

def composeParents(node, mat):
    trans = node.get('transform')
    if trans:
        mat = composeTransform(parseTransform(trans), mat)
    if node.getparent().tag == inkex.addNS('g','svg'):
        mat = composeParents(node.getparent(), mat)
    return mat

def applyTransformToNode(mat,node):
    m=parseTransform(node.get("transform"))
    newtransf=formatTransform(composeTransform(mat,m))
    node.set("transform", newtransf)

def applyTransformToPoint(mat,pt):
    x = mat[0][0]*pt[0] + mat[0][1]*pt[1] + mat[0][2]
    y = mat[1][0]*pt[0] + mat[1][1]*pt[1] + mat[1][2]
    pt[0]=x
    pt[1]=y

def applyTransformToPath(mat,path):
    for comp in path:
        for ctl in comp:
            for pt in ctl:
                applyTransformToPoint(mat,pt)

def fuseTransform(node):
    if node.get('d')==None:
        #FIXME: how do you raise errors?
        raise AssertionError, 'can not fuse "transform" of elements that have no "d" attribute'
    t = node.get("transform")
    if t == None:
        return
    m = parseTransform(t)
    d = node.get('d')
    p = cubicsuperpath.parsePath(d)
    applyTransformToPath(m,p)
    node.set('d', cubicsuperpath.formatPath(p))
    del node.attrib["transform"]

####################################################################
##-- Some functions to compute a rough bbox of a given list of objects.
##-- this should be shipped out in an separate file...

def boxunion(b1,b2):
    if b1 is None:
        return b2
    elif b2 is None:
        return b1    
    else:
        return((min(b1[0],b2[0]), max(b1[1],b2[1]), min(b1[2],b2[2]), max(b1[3],b2[3])))

def roughBBox(path):
    xmin,xMax,ymin,yMax = path[0][0][0][0],path[0][0][0][0],path[0][0][0][1],path[0][0][0][1]
    for pathcomp in path:
        for ctl in pathcomp:
            for pt in ctl:
                xmin = min(xmin,pt[0])
                xMax = max(xMax,pt[0])
                ymin = min(ymin,pt[1])
                yMax = max(yMax,pt[1])
    return xmin,xMax,ymin,yMax

def refinedBBox(path):
    xmin,xMax,ymin,yMax = path[0][0][1][0],path[0][0][1][0],path[0][0][1][1],path[0][0][1][1]
    for pathcomp in path:
        for i in range(1, len(pathcomp)):
            cmin, cmax = cubicExtrema(pathcomp[i-1][1][0], pathcomp[i-1][2][0], pathcomp[i][0][0], pathcomp[i][1][0])
            xmin = min(xmin, cmin)
            xMax = max(xMax, cmax)
            cmin, cmax = cubicExtrema(pathcomp[i-1][1][1], pathcomp[i-1][2][1], pathcomp[i][0][1], pathcomp[i][1][1])
            ymin = min(ymin, cmin)
            yMax = max(yMax, cmax)
    return xmin,xMax,ymin,yMax

def cubicExtrema(y0, y1, y2, y3):
    cmin = min(y0, y3)
    cmax = max(y0, y3)
    d1 = y1 - y0
    d2 = y2 - y1
    d3 = y3 - y2
    if (d1 - 2*d2 + d3):
        if (d2*d2 > d1*d3):
            t = (d1 - d2 + math.sqrt(d2*d2 - d1*d3))/(d1 - 2*d2 + d3)
            if (t > 0) and (t < 1):
                y = y0*(1-t)*(1-t)*(1-t) + 3*y1*t*(1-t)*(1-t) + 3*y2*t*t*(1-t) + y3*t*t*t
                cmin = min(cmin, y)
                cmax = max(cmax, y)
            t = (d1 - d2 - math.sqrt(d2*d2 - d1*d3))/(d1 - 2*d2 + d3)
            if (t > 0) and (t < 1):
                y = y0*(1-t)*(1-t)*(1-t) + 3*y1*t*(1-t)*(1-t) + 3*y2*t*t*(1-t) + y3*t*t*t
                cmin = min(cmin, y)
                cmax = max(cmax, y)
    elif (d3 - d1):
        t = -d1/(d3 - d1)
        if (t > 0) and (t < 1):
            y = y0*(1-t)*(1-t)*(1-t) + 3*y1*t*(1-t)*(1-t) + 3*y2*t*t*(1-t) + y3*t*t*t
            cmin = min(cmin, y)
            cmax = max(cmax, y)
    return cmin, cmax

def computeBBox(aList,mat=[[1,0,0],[0,1,0]]):
    bbox=None
    for node in aList:
        m = parseTransform(node.get('transform'))
        m = composeTransform(mat,m)
        #TODO: text not supported!
        d = None
        if node.get("d"):
            d = node.get('d')
        elif node.get('points'):
            d = 'M' + node.get('points')
        elif node.tag in [ inkex.addNS('rect','svg'), 'rect', inkex.addNS('image','svg'), 'image' ]:
            d = 'M' + node.get('x', '0') + ',' + node.get('y', '0') + \
                'h' + node.get('width') + 'v' + node.get('height') + \
                'h-' + node.get('width')
        elif node.tag in [ inkex.addNS('line','svg'), 'line' ]:
            d = 'M' + node.get('x1') + ',' + node.get('y1') + \
                ' ' + node.get('x2') + ',' + node.get('y2')
        elif node.tag in [ inkex.addNS('circle','svg'), 'circle', \
                            inkex.addNS('ellipse','svg'), 'ellipse' ]:
            rx = node.get('r')
            if rx is not None:
                ry = rx
            else:
                rx = node.get('rx')
                ry = node.get('ry')
            cx = float(node.get('cx', '0'))
            cy = float(node.get('cy', '0'))
            x1 = cx - float(rx)
            x2 = cx + float(rx)
            d = 'M %f %f ' % (x1, cy) + \
                'A' + rx + ',' + ry + ' 0 1 0 %f,%f' % (x2, cy) + \
                'A' + rx + ',' + ry + ' 0 1 0 %f,%f' % (x1, cy)
 
        if d is not None:
            p = cubicsuperpath.parsePath(d)
            applyTransformToPath(m,p)
            bbox=boxunion(refinedBBox(p),bbox)

        elif node.tag == inkex.addNS('use','svg') or node.tag=='use':
            refid=node.get(inkex.addNS('href','xlink'))
            path = '//*[@id="%s"]' % refid[1:]
            refnode = node.xpath(path)
            bbox=boxunion(computeBBox(refnode,m),bbox)
            
        bbox=boxunion(computeBBox(node,m),bbox)
    return bbox


def computePointInNode(pt, node, mat=[[1.0, 0.0, 0.0], [0.0, 1.0, 0.0]]):
    if node.getparent() is not None:
        applyTransformToPoint(invertTransform(composeParents(node, mat)), pt)
    return pt


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
