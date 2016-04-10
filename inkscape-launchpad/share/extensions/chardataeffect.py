#!/usr/bin/env python 
'''
Copyright (C) 2006 Jos Hirth, kaioa.com
Copyright (C) 2007 bulia byak
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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
'''
import sys, optparse, inkex

class CharDataEffect(inkex.Effect):
  def __init__(self):
    inkex.Effect.__init__(self)
    self.visited = []

  newline = True
  newpar = True

  def effect(self):
    if len(self.selected)==0:
      self.recurse(self.document.getroot())
    else:
      for id,node in self.selected.iteritems():
        self.recurse(node)

  def recurse(self,node):
    istext = (node.tag == '{http://www.w3.org/2000/svg}flowPara' or node.tag == '{http://www.w3.org/2000/svg}flowDiv' or node.tag == '{http://www.w3.org/2000/svg}text')
    if node.get('{http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd}role') == 'line':
      self.newline = True
    elif istext:
      self.newline = True
      self.newpar = True

    if node.text != None:
      node.text = self.process_chardata(node.text, self.newline, self.newpar)
      self.newline = False
      self.newpar = False

    for child in node:
      self.recurse(child)

    if node.tail != None:
      node.tail = self.process_chardata(node.tail, self.newline, self.newpar)

  def process_chardata(self,text, line, par):
    pass

