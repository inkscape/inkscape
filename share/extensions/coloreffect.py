#!/usr/bin/env python 
'''
Copyright (C) 2006 Jos Hirth, kaioa.com

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
import sys, copy, optparse, simplestyle, inkex

import xml.xpath

color_props_fill=('fill:','stop-color:','flood-color:','lighting-color:')
color_props_stroke=('stroke:',)
color_props = color_props_fill + color_props_stroke

class ColorEffect(inkex.Effect):
  def __init__(self):
    inkex.Effect.__init__(self,use_minidom=True)

  def effect(self):
    if len(self.selected)==0:
      self.getAttribs(self.document)
    else:
      for id,node in self.selected.iteritems():
        self.getAttribs(node)

  def getAttribs(self,node):
    self.changeStyle(node)
    if node.hasChildNodes():
      childs=node.childNodes
      for child in childs:
        self.getAttribs(child)
  
  def changeStyle(self,node):
    if node.hasAttributes():
      style=node.getAttribute('style') # fixme: this will break for presentation attributes!
      if style!='':
        #inkex.debug('old style:'+style)
        styles=style.split(';')
        for i in range(len(styles)):
          for c in range(len(color_props)):
            if styles[i].startswith(color_props[c]):
              styles[i]=color_props[c]+self.process_prop(styles[i][len(color_props[c]):])
        #inkex.debug('new style:'+';'.join(styles))
        node.setAttribute('style',';'.join(styles))
  '''
  def changeStyle(self,node):
    if node.hasAttributes():
      sa=node.getAttribute('style')
      if sa!='':
        debug(sa)
        styles=simplestyle.parseStyle(sa)
        for c in range(len(colortags)):
          if colortags[c] in styles.keys():
            styles[colortags[c]]=self.process_prop(styles[colortags[c]])
        node.setAttribute('style',simplestyle.formatStyle(styles))
  '''        
  def process_prop(self,col):
    #debug('got:'+col)
    if simplestyle.isColor(col):
      c=simplestyle.parseColor(col)
      col='#'+self.colmod(c[0],c[1],c[2])
      #debug('made:'+col)
    if col.startswith('url(#'):
	id = col[len('url(#'):col.find(')')]
	#inkex.debug('ID:' + id )
	path = '//*[@id="%s"]' % id
	for node in xml.xpath.Evaluate(path,self.document):
	  self.process_gradient(node)
    return col

  def process_gradient(self, node):
    self.changeStyle(node)
    if node.hasChildNodes():
      for child in node.childNodes:
        self.process_gradient(child)
    if node.hasAttributes():				
      href=node.getAttribute('xlink:href')
      if href.startswith('#'):
        id = href[len('#'):len(href)]
        #inkex.debug('ID:' + id )
        path = '//*[@id="%s"]' % id
        for node in xml.xpath.Evaluate(path,self.document):
          self.process_gradient(node)
 
  def colmod(self,r,g,b):
    pass
