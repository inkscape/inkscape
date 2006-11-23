#!/usr/bin/env python
'''
Copyright (C) 2006 Jean-Francois Barraud, barraud@math.univ-lille1.fr

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
barraud@math.univ-lille1.fr
'''
import inkex, cubicsuperpath, bezmisc, simplestyle
import copy, math, re, random, xml.xpath

def parseTransform(transf,mat=[[1.0,0.0,0.0],[0.0,1.0,0.0]]):
    if transf=="":
        return(mat)
    result=re.match("(translate|scale|rotate|skewX|skewY|matrix)\(([^)]*)\)",transf)
#-- translate --
    if result.group(1)=="translate":
        args=result.group(2).split(",")
        dx=float(args[0])
        if len(args)==1:
            dy=0.0
        else:
            dy=float(args[1])
        matrix=[[1,0,dx],[0,1,dy]]
#-- scale --
    if result.groups(1)=="scale":
        args=result.group(2).split(",")
        sx=float(args[0])
        if len(args)==1:
            sy=sx
        else:
            sy=float(args[1])
        matrix=[[sx,0,0],[0,sy,0]]
#-- rotate --
    if result.groups(1)=="rotate":
        args=result.group(2).split(",")
        a=float(args[0])*math.pi/180
        if len(args)==1:
            cx,cy=(0.0,0.0)
        else:
            cx,cy=args[1:]
        matrix=[[math.cos(a),-math.sin(a),cx],[math.sin(a),math.cos(a),cy]]
#-- skewX --
    if result.groups(1)=="skewX":
        a=float(result.group(2))*math.pi/180
        matrix=[[1,math.tan(a),0],[0,1,0]]
#-- skewX --
    if result.groups(1)=="skewX":
        a=float(result.group(2))*math.pi/180
        matrix=[[1,0,0],[math.tan(a),1,0]]
#-- matrix --
    if result.group(1)=="matrix":
        a11,a21,a12,a22,v1,v2=result.group(2).split(",")
        matrix=[[float(a11),float(a12),float(v1)],[float(a21),float(a22),float(v2)]]
    
    matrix=composeTransform(mat,matrix)
    if result.end()<len(transf):
        return(parseTransform(transf[result.end():],matrix))
    else:
        return matrix

def formatTransform(mat):
    return("matrix(%f,%f,%f,%f,%f,%f)"%(mat[0][0],mat[1][0],mat[0][1],mat[1][1],mat[0][2],mat[1][2]))

def composeTransform(M1,M2):
    a11=M1[0][0]*M2[0][0]+M1[0][1]*M2[1][0]
    a12=M1[0][0]*M2[0][1]+M1[0][1]*M2[1][1]
    a21=M1[1][0]*M2[0][0]+M1[1][1]*M2[1][0]
    a22=M1[1][0]*M2[0][1]+M1[1][1]*M2[1][1]

    v1=M1[0][0]*M2[0][2]+M1[0][1]*M2[1][2]+M1[0][2]
    v2=M1[1][0]*M2[0][2]+M1[1][1]*M2[1][2]+M1[1][2]
    return [[a11,a12,v1],[a21,a22,v2]]

def applyTransformToNode(mat,node):
    m=parseTransform(node.getAttributeNS(None,"transform"))
    newtransf=formatTransform(composeTransform(mat,m))
    node.setAttributeNS(None,"transform", newtransf)

def applyTransformToPoint(mat,pt):
    x=mat[0][0]*pt[0]+mat[0][1]*pt[1]+mat[0][2]
    y=mat[1][0]*pt[0]+mat[1][1]*pt[1]+mat[1][2]
    pt[0]=x
    pt[1]=y

def fuseTransform(node):
    m=parseTransform(node.getAttributeNS(None,"transform"))
    d = node.getAttributeNS(None,'d')
    p=cubicsuperpath.parsePath(d)
    for comp in p:
        for ctl in comp:
            for pt in ctl:
                applyTransformToPoint(m,pt)
    node.setAttributeNS(None,'d', cubicsuperpath.formatPath(p))
    node.removeAttributeNS(None,"transform")


def boxunion(b1,b2):
    if b1 is None:
        return b2
    elif b2 is None:
        return b1    
    else:
        return((min(b1[0],b2[0]),max(b1[1],b2[1]),min(b1[2],b2[2]),max(b1[3],b2[3])))

def roughBBox(path):
    xmin,xMax,ymin,yMax=path[0][0][0][0],path[0][0][0][0],path[0][0][0][1],path[0][0][0][1]
    for pathcomp in path:
        for ctl in pathcomp:
           for pt in ctl:
               xmin=min(xmin,pt[0])
               xMax=max(xMax,pt[0])
               ymin=min(ymin,pt[1])
               yMax=max(yMax,pt[1])
    return xmin,xMax,ymin,yMax


class PathModifier(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)

##################################
#-- Selectionlists manipulation --
##################################
    def computeBBox(self, aList):
	bbox=None
	for id, node in aList.iteritems():
	    if node.tagName == 'path':
		d = node.attributes.getNamedItem('d')
		p = cubicsuperpath.parsePath(d.value)
		bbox=boxunion(roughBBox(p),bbox)
	return bbox

    def duplicateNodes(self, aList):
        clones={}
        for id,node in aList.iteritems():
            clone=node.cloneNode(True)
    #!!!--> should it be given an id?
    #seems to work without this!?!
            clone.setAttributeNS(None,"id", self.uniqueId(node.tagName))
            node.parentNode.appendChild(clone)
            clones[clone.getAttributeNS(None,"id")]=clone
        return(clones)

    def uniqueId(self, prefix):
        id="%s%04i"%(prefix,random.randint(0,9999))
        while len(xml.xpath.Evaluate('//*[@id="%s"]' % id,self.document)):
            id="%s%04i"%(prefix,random.randint(0,9999))
        return(id)

    def expandGroups(self,aList,transferTransform=True):
        for id, node in aList.items():      
            if node.tagName == 'g':
                mat=parseTransform(node.getAttributeNS(None,"transform"))
                for child in node.childNodes:
                    if child.nodeType==child.ELEMENT_NODE:
                        if transferTransform:
                            applyTransformToNode(mat,child)
                        aList.update(self.expandGroups({child.getAttribute('id'):child}))
                if transferTransform:
                    node.removeAttribute("transform")
                del aList[id]
	return(aList)

    def expandGroupsUnlinkClones(self,aList,transferTransform=True,doReplace=True):
        for id in aList.keys()[:]:     
            node=aList[id]
            if node.tagName == 'g':
                self.expandGroups(aList,transferTransform)
                self.expandGroupsUnlinkClones(aList,transferTransform,doReplace)
		#Hum... not very efficient if there are many clones of groups...
            elif node.tagName == 'use':
                refid=node.getAttributeNS(inkex.NSS[u'xlink'],'href')
                path = '//*[@id="%s"]' % refid[1:]
                refnode = xml.xpath.Evaluate(path,self.document)[0]
                newnode=refnode.cloneNode(True)
		self.recursNewIds(newnode)

		if node.hasAttributeNS(None,u'style'):
		    style=simplestyle.parseStyle(node.getAttributeNS(None,u'style'))
		    refstyle=simplestyle.parseStyle(refnode.getAttributeNS(None,u'style'))
		    style.update(refstyle)
		    newnode.setAttributeNS(None,'style',simplestyle.formatStyle(style))
                applyTransformToNode(parseTransform(node.getAttributeNS(None,'transform')),newnode)
                if doReplace:
                    parent=node.parentNode
                    parent.insertBefore(newnode,node)
                    parent.removeChild(node)
                del aList[id]
		newid=newnode.getAttributeNS(None,'id')
                aList.update(self.expandGroupsUnlinkClones({newid:newnode},transferTransform,doReplace))
        return aList
    
    def recursNewIds(self,node):
	if node.nodeType==node.ELEMENT_NODE and node.hasAttributeNS(None,u'id'):
	    node.setAttributeNS(None,u'id',self.uniqueId(node.tagName))
	for child in node.childNodes:
	    self.recursNewIds(child)
		

	

#     def makeClonesReal(self,aList,doReplace=True,recursivelytransferTransform=True):
#         for id in aList.keys():     
#             node=aList[id]
#             if node.tagName == 'g':
#                 childs={}
#                 for child in node.childNodes:
#                     if child.nodeType==child.ELEMENT_NODE:
# 			childid=child.getAttributeNS(None,'id')
# 			del aList[childid]
#                         aList.update(self.makeClonesReal({childid:child},doReplace))
#             elif node.tagName == 'use':
#                 refid=node.getAttributeNS(inkex.NSS[u'xlink'],'href')
#                 path = '//*[@id="%s"]' % refid[1:]
#                 refnode = xml.xpath.Evaluate(path,document)[0]
#                 clone=refnode.cloneNode(True)
# 		cloneid=self.uniqueId(clone.tagName)
#                 clone.setAttributeNS(None,'id', cloneid)
#                 style=simplestyle.parseStyle(node.getAttributeNS(None,u'style'))
#                 refstyle=simplestyle.parseStyle(refnode.getAttributeNS(None,u'style'))
#                 style.update(refstyle)
#                 clone.setAttributeNS(None,'style',simplestyle.formatStyle(style))
#                 applyTransformToNode(parseTransform(node.getAttributeNS(None,'transform')),clone)
#                 if doReplace:
#                     parent=node.parentNode
#                     parent.insertBefore(clone,node)
#                     parent.removeChild(node)
#                 del aList[id]
#                 aList.update(self.expandGroupsUnlinkClones({cloneid:clone}))
#         return aList

################################
#-- Object conversion ----------
################################

    def rectToPath(self,node,doReplace=True):
        if node.tagName == 'rect':
            x =float(node.getAttributeNS(None,u'x'))
            y =float(node.getAttributeNS(None,u'y'))
            try:
                rx=float(node.getAttributeNS(None,u'rx'))
                ry=float(node.getAttributeNS(None,u'ry'))
            except:
                rx=0
                ry=0
            w =float(node.getAttributeNS(None,u'width' ))
            h =float(node.getAttributeNS(None,u'height'))
            d ='M %f,%f '%(x+rx,y)
            d+='L %f,%f '%(x+w-rx,y)
            d+='A %f,%f,%i,%i,%i,%f,%f '%(rx,ry,0,0,1,x+w,y+ry)
            d+='L %f,%f '%(x+w,y+h-ry)
            d+='A %f,%f,%i,%i,%i,%f,%f '%(rx,ry,0,0,1,x+w-rx,y+h)
            d+='L %f,%f '%(x+rx,y+h)
            d+='A %f,%f,%i,%i,%i,%f,%f '%(rx,ry,0,0,1,x,y+h-ry)
            d+='L %f,%f '%(x,y+ry)
            d+='A %f,%f,%i,%i,%i,%f,%f '%(rx,ry,0,0,1,x+rx,y)

            newnode=self.document.createElement('path')
            newnode.setAttributeNS(None,'d',d)
            newnode.setAttributeNS(None,'id', self.uniqueId('path'))
            newnode.setAttributeNS(None,'style',node.getAttributeNS(None,u'style'))
            newnode.setAttributeNS(None,'transform',node.getAttributeNS(None,u'transform'))
            fuseTransform(newnode)
            if doReplace:
                parent=node.parentNode
                parent.insertBefore(newnode,node)
                parent.removeChild(node)
            return newnode

    def objectToPath(self,node,doReplace=True):
    #--TODO: support other object types!!!!
    #--TODO: make sure cubicsuperpath supports A and Q commands... 
        if node.tagName == 'rect':
	    return(self.rectToPath(node,doReplace))
        elif node.tagName == 'path':
            attributes = node.attributes.keys()
            for uri,attName in attributes:
		if uri in [inkex.NSS[u'sodipodi'],inkex.NSS[u'inkscape']]:
#                if attName not in ["d","id","style","transform"]:
                    node.removeAttributeNS(uri,attName)
            fuseTransform(node)
            return node
        else:
            inkex.debug("Please first convert objects to paths!...(got '%s')"%node.tagName)
            return None

    def objectsToPaths(self,aList,doReplace=True):
        newSelection={}
        for id,node in aList.items():
            newnode=self.objectToPath(node,self.document)
            del aList[id]
	    aList[newnode.getAttributeNS(None,u'id')]=newnode


################################
#-- Action ----------
################################
	    
    #-- overwrite this method in subclasses...
    def effect(self):
        #self.duplicateNodes(self.selected)
        self.expandGroupsUnlinkClones(self.selected, True)
	self.objectsToPaths(self.selected, True)
        self.bbox=self.computeBBox(self.selected)
        for id, node in self.selected.iteritems():
            if node.tagName == 'path':
                d = node.attributes.getNamedItem('d')
                p = cubicsuperpath.parsePath(d.value)

		#do what ever you want with p!

		d.value = cubicsuperpath.formatPath(p)


class Diffeo(PathModifier):
    def __init__(self):
        inkex.Effect.__init__(self)

    def applyDiffeo(self,bpt,vects=()):
        '''
        bpt is a base point and for v in vectors, v'=v-p is a tangent vector at bpt. 
        Defaults to identity!
        '''
        for v in vects:
            v[0]-=bpt[0]
            v[1]-=bpt[1]

        #-- your transformations go here:
        #x,y=bpt
        #bpt[0]=f(x,y)
        #bpt[1]=g(x,y)
        #for v in vects:
        #    vx,vy=v
        #    v[0]=df/dx(x,y)*vx+df/dy(x,y)*vy
        #    v[1]=dg/dx(x,y)*vx+dg/dy(x,y)*vy
        #
        #-- !caution! y-axis is pointing downward!

        for v in vects:
            v[0]+=bpt[0]
            v[1]+=bpt[1]


    def effect(self):
        #self.duplicateNodes(self.selected)
	self.expandGroupsUnlinkClones(self.selected, True)
        self.expandGroups(self.selected, True)
	self.objectsToPaths(self.selected, True)
        self.bbox=self.computeBBox(self.selected)
        for id, node in self.selected.iteritems():
            if node.tagName == 'path':
                d = node.attributes.getNamedItem('d')
                p = cubicsuperpath.parsePath(d.value)

                for sub in p:
                    for ctlpt in sub:
                        self.applyDiffeo(ctlpt[1],(ctlpt[0],ctlpt[2]))

		d.value = cubicsuperpath.formatPath(p)

#e = Diffeo()
#e.affect()

 	  	 
