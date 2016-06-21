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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
barraud@math.univ-lille1.fr

This code defines a basic class (PathModifier) of effects whose purpose is
to somehow deform given objects: one common tasks for all such effect is to
convert shapes, groups, clones to paths. The class has several functions to
make this (more or less!) easy.
As an exemple, a second class (Diffeo) is derived from it,
to implement deformations of the form X=f(x,y), Y=g(x,y)...

TODO: Several handy functions are defined, that might in fact be of general
interest and that should be shipped out in separate files...
'''
# standard library
import copy
import math
import re
import random
# local library
import inkex
import cubicsuperpath
import bezmisc
import simplestyle
from simpletransform import *

####################################################################
##-- zOrder computation...
##-- this should be shipped out in a separate file. inkex.py?

def zSort(inNode,idList):
    sortedList=[]
    theid = inNode.get("id")
    if theid in idList:
        sortedList.append(theid)
    for child in inNode:
        if len(sortedList)==len(idList):
            break
        sortedList+=zSort(child,idList)
    return sortedList


class PathModifier(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)

##################################
#-- Selectionlists manipulation --
##################################

    def duplicateNodes(self, aList):
        clones={}
        for id,node in aList.iteritems():
            clone=copy.deepcopy(node)
            #!!!--> should it be given an id?
            #seems to work without this!?!
            myid = node.tag.split('}')[-1]
            clone.set("id", self.uniqueId(myid))
            node.getparent().append(clone)
            clones[clone.get("id")]=clone
        return(clones)

    def uniqueId(self, prefix):
        id="%s%04i"%(prefix,random.randint(0,9999))
        while len(self.document.getroot().xpath('//*[@id="%s"]' % id,namespaces=inkex.NSS)):
            id="%s%04i"%(prefix,random.randint(0,9999))
        return(id)

    def expandGroups(self,aList,transferTransform=True):
        for id, node in aList.items():      
            if node.tag == inkex.addNS('g','svg') or node.tag=='g':
                mat=parseTransform(node.get("transform"))
                for child in node:
                    if transferTransform:
                        applyTransformToNode(mat,child)
                    aList.update(self.expandGroups({child.get('id'):child}))
                if transferTransform and node.get("transform"):
                    del node.attrib["transform"]
                del aList[id]
        return(aList)

    def expandGroupsUnlinkClones(self,aList,transferTransform=True,doReplace=True):
        for id in aList.keys()[:]:     
            node=aList[id]
            if node.tag == inkex.addNS('g','svg') or node.tag=='g':
                self.expandGroups(aList,transferTransform)
                self.expandGroupsUnlinkClones(aList,transferTransform,doReplace)
                #Hum... not very efficient if there are many clones of groups...

            elif node.tag == inkex.addNS('use','svg') or node.tag=='use':
                refnode=self.refNode(node)
                newnode=self.unlinkClone(node,doReplace)
                del aList[id]

                style = simplestyle.parseStyle(node.get('style') or "")
                refstyle=simplestyle.parseStyle(refnode.get('style') or "")
                style.update(refstyle)
                newnode.set('style',simplestyle.formatStyle(style))

                newid=newnode.get('id')
                aList.update(self.expandGroupsUnlinkClones({newid:newnode},transferTransform,doReplace))
        return aList
    
    def recursNewIds(self,node):
        if node.get('id'):
            node.set('id',self.uniqueId(node.tag))
        for child in node:
            self.recursNewIds(child)
            
    def refNode(self,node):
        if node.get(inkex.addNS('href','xlink')):
            refid=node.get(inkex.addNS('href','xlink'))
            path = '//*[@id="%s"]' % refid[1:]
            newNode = self.document.getroot().xpath(path, namespaces=inkex.NSS)[0]
            return newNode
        else:
            raise AssertionError, "Trying to follow empty xlink.href attribute."

    def unlinkClone(self,node,doReplace):
        if node.tag == inkex.addNS('use','svg') or node.tag=='use':
            newNode = copy.deepcopy(self.refNode(node))
            self.recursNewIds(newNode)
            applyTransformToNode(parseTransform(node.get('transform')),newNode)

            if doReplace:
                parent=node.getparent()
                parent.insert(parent.index(node),newNode)
                parent.remove(node)

            return newNode
        else:
            raise AssertionError, "Only clones can be unlinked..."



################################
#-- Object conversion ----------
################################

    def rectToPath(self,node,doReplace=True):
        if node.tag == inkex.addNS('rect','svg'):
            x =float(node.get('x'))
            y =float(node.get('y'))
            #FIXME: no exception anymore and sometimes just one
            try:
                rx=float(node.get('rx'))
                ry=float(node.get('ry'))
            except:
                rx=0
                ry=0
            w =float(node.get('width' ))
            h =float(node.get('height'))
            d ='M %f,%f '%(x+rx,y)
            d+='L %f,%f '%(x+w-rx,y)
            d+='A %f,%f,%i,%i,%i,%f,%f '%(rx,ry,0,0,1,x+w,y+ry)
            d+='L %f,%f '%(x+w,y+h-ry)
            d+='A %f,%f,%i,%i,%i,%f,%f '%(rx,ry,0,0,1,x+w-rx,y+h)
            d+='L %f,%f '%(x+rx,y+h)
            d+='A %f,%f,%i,%i,%i,%f,%f '%(rx,ry,0,0,1,x,y+h-ry)
            d+='L %f,%f '%(x,y+ry)
            d+='A %f,%f,%i,%i,%i,%f,%f '%(rx,ry,0,0,1,x+rx,y)

            newnode=inkex.etree.Element('path')
            newnode.set('d',d)
            newnode.set('id', self.uniqueId('path'))
            newnode.set('style',node.get('style'))
            nnt = node.get('transform')
            if nnt:
                newnode.set('transform',nnt)
                fuseTransform(newnode)
            if doReplace:
                parent=node.getparent()
                parent.insert(parent.index(node),newnode)
                parent.remove(node)
            return newnode

    def groupToPath(self,node,doReplace=True):
        if node.tag == inkex.addNS('g','svg'):
            newNode = inkex.etree.SubElement(self.current_layer,inkex.addNS('path','svg'))    

            newstyle = simplestyle.parseStyle(node.get('style') or "")
            newp = []
            for child in node:
                childstyle = simplestyle.parseStyle(child.get('style') or "")
                childstyle.update(newstyle)
                newstyle.update(childstyle)
                childAsPath = self.objectToPath(child,False)
                newp += cubicsuperpath.parsePath(childAsPath.get('d'))
            newNode.set('d',cubicsuperpath.formatPath(newp))
            newNode.set('style',simplestyle.formatStyle(newstyle))

            self.current_layer.remove(newNode)
            if doReplace:
                parent=node.getparent()
                parent.insert(parent.index(node),newNode)
                parent.remove(node)

            return newNode
        else:
            raise AssertionError
        
    def objectToPath(self,node,doReplace=True):
        #--TODO: support other object types!!!!
        #--TODO: make sure cubicsuperpath supports A and Q commands... 
        if node.tag == inkex.addNS('rect','svg'):
            return(self.rectToPath(node,doReplace))
        if node.tag == inkex.addNS('g','svg'):
            return(self.groupToPath(node,doReplace))
        elif node.tag == inkex.addNS('path','svg') or node.tag == 'path':
            #remove inkscape attributes, otherwise any modif of 'd' will be discarded!
            for attName in node.attrib.keys():
                if ("sodipodi" in attName) or ("inkscape" in attName):
                    del node.attrib[attName]
            fuseTransform(node)
            return node
        elif node.tag == inkex.addNS('use','svg') or node.tag == 'use':
            newNode = self.unlinkClone(node,doReplace)
            return self.objectToPath(newNode,doReplace)
        else:
            inkex.errormsg(_("Please first convert objects to paths!  (Got [%s].)") % node.tag)
            return None

    def objectsToPaths(self,aList,doReplace=True):
        newSelection={}
        for id,node in aList.items():
            newnode=self.objectToPath(node,doReplace)
            del aList[id]
            aList[newnode.get('id')]=newnode


################################
#-- Action ----------
################################
        
    #-- overwrite this method in subclasses...
    def effect(self):
        #self.duplicateNodes(self.selected)
        #self.expandGroupsUnlinkClones(self.selected, True)
        self.objectsToPaths(self.selected, True)
        self.bbox=computeBBox(self.selected.values())
        for id, node in self.selected.iteritems():
            if node.tag == inkex.addNS('path','svg'):
                d = node.get('d')
                p = cubicsuperpath.parsePath(d)

                #do what ever you want with p!

                node.set('d',cubicsuperpath.formatPath(p))


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
        self.bbox=computeBBox(self.selected.values())
        for id, node in self.selected.iteritems():
            if node.tag == inkex.addNS('path','svg') or node.tag=='path':
                d = node.get('d')
                p = cubicsuperpath.parsePath(d)

                for sub in p:
                    for ctlpt in sub:
                        self.applyDiffeo(ctlpt[1],(ctlpt[0],ctlpt[2]))

                node.set('d',cubicsuperpath.formatPath(p))

#e = Diffeo()
#e.affect()

    
# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
