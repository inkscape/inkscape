#!/usr/bin/env python 
'''
Copyright (C) 2006 Jos Hirth, kaioa.com
Copyright (C) 2007 Aaron C. Spike

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
import sys, copy, optparse, simplestyle, inkex, copy

import random

color_props_fill=('fill:','stop-color:','flood-color:','lighting-color:')
color_props_stroke=('stroke:',)
color_props = color_props_fill + color_props_stroke


class ColorEffect(inkex.Effect):
  def __init__(self):
    inkex.Effect.__init__(self)
    self.visited = []

  def effect(self):
    if len(self.selected)==0:
      self.getAttribs(self.document.getroot())
    else:
      for id,node in self.selected.iteritems():
        self.getAttribs(node)

  def getAttribs(self,node):
    self.changeStyle(node)
    for child in node:
      self.getAttribs(child)
  
  def changeStyle(self,node):
    if node.attrib.has_key('style'):
      style=node.get('style') # fixme: this will break for presentation attributes!
      if style!='':
        #inkex.debug('old style:'+style)
        styles=style.split(';')
        for i in range(len(styles)):
          for c in range(len(color_props)):
            if styles[i].startswith(color_props[c]):
              styles[i]=color_props[c]+self.process_prop(styles[i][len(color_props[c]):])
        #inkex.debug('new style:'+';'.join(styles))
        node.set('style',';'.join(styles))

  def process_prop(self,col):
    #debug('got:'+col)
    if simplestyle.isColor(col):
      c=simplestyle.parseColor(col)
      col='#'+self.colmod(c[0],c[1],c[2])
      #debug('made:'+col)
    if col.startswith('url(#'):
      id = col[len('url(#'):col.find(')')]
      newid = '%s-%d' % (id, int(random.random() * 1000))
      #inkex.debug('ID:' + id )
      path = '//*[@id="%s"]' % id
      for node in self.document.xpath(path, namespaces=inkex.NSS):
        self.process_gradient(node, newid)
      col = 'url(#%s)' % newid
    return col

  def process_gradient(self, node, newid):
    #if node.hasAttributes():
       #this_id=node.getAttribute('id')
       #if this_id in self.visited:
         ## prevent multiple processing of the same gradient if it is used by more than one selected object
         ##inkex.debug("already had: " + this_id)
         #return
       #self.visited.append(this_id)
       #inkex.debug("visited: " + str(self.visited))
    newnode = copy.deepcopy(node)
    newnode.set('id', newid)
    node.getparent().append(newnode)
    self.changeStyle(newnode)
    for child in newnode:
      self.changeStyle(child)
    xlink = inkex.addNS('href','xlink')
    if newnode.attrib.has_key(xlink):
      href=newnode.get(xlink)
      if href.startswith('#'):
        id = href[len('#'):len(href)]
        #inkex.debug('ID:' + id )
        newhref = '%s-%d' % (id, int(random.random() * 1000))
        newnode.set(xlink, '#%s' % newhref)
        path = '//*[@id="%s"]' % id
        for node in self.document.xpath(path, namespaces=inkex.NSS):
          self.process_gradient(node, newhref)
 
  def colmod(self,r,g,b):
    pass

  def rgb_to_hsl(self,r, g, b):
    rgb_max = max (max (r, g), b)
    rgb_min = min (min (r, g), b)
    delta = rgb_max - rgb_min
    hsl = [0.0, 0.0, 0.0]
    hsl[2] = (rgb_max + rgb_min)/2.0
    if delta == 0:
        hsl[0] = 0.0
        hsl[1] = 0.0
    else:
        if hsl[2] <= 0.5:
            hsl[1] = delta / (rgb_max + rgb_min)
        else:
            hsl[1] = delta / (2 - rgb_max - rgb_min)
        if r == rgb_max:
            hsl[0] = (g - b) / delta
        else:
            if g == rgb_max:
                hsl[0] = 2.0 + (b - r) / delta
            else:
                if b == rgb_max:
                    hsl[0] = 4.0 + (r - g) / delta
        hsl[0] = hsl[0] / 6.0
        if hsl[0] < 0:
            hsl[0] = hsl[0] + 1
        if hsl[0] > 1:
            hsl[0] = hsl[0] - 1
    return hsl

  def hue_2_rgb (self, v1, v2, h):
    if h < 0:
        h += 6.0
    if h > 6:
        h -= 6.0
    if h < 1:
        return v1 + (v2 - v1) * h
    if h < 3:
        return v2
    if h < 4:
        return v1 + (v2 - v1) * (4 - h)
    return v1

  def hsl_to_rgb (self,h, s, l):
    rgb = [0, 0, 0]
    if s == 0:
        rgb[0] = l
        rgb[1] = l
        rgb[2] = l
    else:
        if l < 0.5:
            v2 = l * (1 + s)
        else:
            v2 = l + s - l*s
        v1 = 2*l - v2
        rgb[0] = self.hue_2_rgb (v1, v2, h*6 + 2.0)
        rgb[1] = self.hue_2_rgb (v1, v2, h*6)
        rgb[2] = self.hue_2_rgb (v1, v2, h*6 - 2.0)
    return rgb
