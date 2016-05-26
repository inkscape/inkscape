#!/usr/bin/env python
'''
Copyright (C) 2009 Aurelio A. Heckert, aurium (a) gmail dot com

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
# standard library
import math
import re
import string
# local library
import inkex
import simplestyle
from pathmodifier import zSort


class InterpAttG(inkex.Effect):

    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-a", "--att",
                        action="store", type="string",
                        dest="att", default="fill",
                        help="Attribute to be interpolated.")
        self.OptionParser.add_option("-o", "--att-other",
                        action="store", type="string",
                        dest="att_other",
                        help="Other attribute (for a limited UI).")
        self.OptionParser.add_option("-t", "--att-other-type",
                        action="store", type="string",
                        dest="att_other_type",
                        help="The other attribute type.")
        self.OptionParser.add_option("-w", "--att-other-where",
                        action="store", type="string",
                        dest="att_other_where",
                        help="That is a tag attribute or a style attribute?")
        self.OptionParser.add_option("-s", "--start-val",
                        action="store", type="string",
                        dest="start_val", default="#F00",
                        help="Initial interpolation value.")
        self.OptionParser.add_option("-e", "--end-val",
                        action="store", type="string",
                        dest="end_val", default="#00F",
                        help="End interpolation value.")
        self.OptionParser.add_option("-u", "--unit",
                        action="store", type="string",
                        dest="unit", default="color",
                        help="Values unit.")
        self.OptionParser.add_option("--zsort",
                        action="store", type="inkbool",
                        dest="zsort", default=True,
                        help="use z-order instead of selection order")
        self.OptionParser.add_option("--tab",
                        action="store", type="string",
                        dest="tab",
                        help="The selected UI-tab when OK was pressed")

    def getColorValues(self):
      sv = string.replace( self.options.start_val, '#', '' )
      ev = string.replace( self.options.end_val, '#', '' )
      if re.search('\s|,', sv):
        # There are separators. That must be a integer RGB color definition.
        sv = re.split( '[\s,]+', sv )
        ev = re.split( '[\s,]+', ev )
        self.R_ini = int( sv[0] )
        self.G_ini = int( sv[1] )
        self.B_ini = int( sv[2] )
        self.R_end = int( ev[0] )
        self.G_end = int( ev[1] )
        self.B_end = int( ev[2] )
      else:
        # There is no separator. That must be a Hex RGB color definition.
        if len(sv) == 3:
          self.R_ini = int( sv[0] + sv[0], 16 )
          self.G_ini = int( sv[1] + sv[1], 16 )
          self.B_ini = int( sv[2] + sv[2], 16 )
          self.R_end = int( ev[0] + ev[0], 16 )
          self.G_end = int( ev[1] + ev[1], 16 )
          self.B_end = int( ev[2] + ev[2], 16 )
        else: #the len must be 6
          self.R_ini = int( sv[0] + sv[1], 16 )
          self.G_ini = int( sv[2] + sv[3], 16 )
          self.B_ini = int( sv[4] + sv[5], 16 )
          self.R_end = int( ev[0] + ev[1], 16 )
          self.G_end = int( ev[2] + ev[3], 16 )
          self.B_end = int( ev[4] + ev[5], 16 )
      self.R_inc = ( self.R_end - self.R_ini ) / float( self.tot_el - 1 )
      self.G_inc = ( self.G_end - self.G_ini ) / float( self.tot_el - 1 )
      self.B_inc = ( self.B_end - self.B_ini ) / float( self.tot_el - 1 )
      self.R_cur = self.R_ini
      self.G_cur = self.G_ini
      self.B_cur = self.B_ini

    def getNumberValues(self):
      sv = self.options.start_val
      ev = self.options.end_val
      if self.inte_att_type and self.inte_att_type != 'none':
        sv = self.unittouu( sv + self.inte_att_type )
        ev = self.unittouu( ev + self.inte_att_type )
      self.val_cur = self.val_ini = sv
      self.val_end = ev
      self.val_inc = ( ev - sv ) / float( self.tot_el - 1 )

    def getTotElements(self):
      self.tot_el = 0
      self.collection = None
      if len( self.selected ) == 0:
        return False
      if len( self.selected ) > 1:
        # multiple selection
        if self.options.zsort:
            sorted_ids = zSort(self.document.getroot(),self.selected.keys())
        else:
            sorted_ids = self.options.ids
        self.collection = list(sorted_ids)
        for i in sorted_ids:
          path = '//*[@id="%s"]' % i
          self.collection[self.tot_el] = self.document.xpath(path, namespaces=inkex.NSS)[0]
          self.tot_el += 1
      else:
        # must be a group
        self.collection = self.selected[ self.options.ids[0] ]
        for i in self.collection:
          self.tot_el += 1

    def effect(self):
      if self.options.att == 'other':
        self.inte_att = self.options.att_other
        self.inte_att_type = self.options.att_other_type
        self.where = self.options.att_other_where
      else:
        self.inte_att = self.options.att
        if   self.inte_att == 'width':
          self.inte_att_type = 'float'
          self.where = 'tag'
        elif self.inte_att == 'height':
          self.inte_att_type = 'float'
          self.where = 'tag'
        elif self.inte_att == 'scale':
          self.inte_att_type = 'float'
          self.where = 'transform'
        elif self.inte_att == 'trans-x':
          self.inte_att_type = 'float'
          self.where = 'transform'
        elif self.inte_att == 'trans-y':
          self.inte_att_type = 'float'
          self.where = 'transform'
        elif self.inte_att == 'fill':
          self.inte_att_type = 'color'
          self.where = 'style'
        elif self.inte_att == 'opacity':
          self.inte_att_type = 'float'
          self.where = 'style'

      self.getTotElements()

      if self.inte_att_type == 'color':
        self.getColorValues()
      else:
        self.getNumberValues()

      if self.collection is None:
        inkex.errormsg( _('There is no selection to interpolate' ))
        return False

      for node in self.collection:
        if self.inte_att_type == 'color':
          val = 'rgb('+ \
                  str(int(round(self.R_cur))) +','+ \
                  str(int(round(self.G_cur))) +','+ \
                  str(int(round(self.B_cur))) +')'
        else:
          if self.inte_att_type == 'float':
            val = self.val_cur
          else: # inte_att_type == 'int'
            val = round(self.val_cur)

        if self.where == 'style':
          s = node.get('style')
          re_find = '(^|;)'+ self.inte_att +':[^;]*(;|$)'
          if re.search( re_find, s ):
            s = re.sub( re_find, '\\1'+ self.inte_att +':'+ str(val) +'\\2', s )
          else:
            s += ';'+ self.inte_att +':'+ str(val)
          node.set( 'style', s )
        elif self.where == 'transform':
          t = node.get('transform')
          if t == None: t = ""
          if self.inte_att == 'trans-x':
            val = "translate("+ str(val) +",0)"
          elif self.inte_att == 'trans-y':
            val = "translate(0,"+ str(val) +")"
          else:
            val = self.inte_att + "("+ str(val) +")"
          node.set( 'transform', t +" "+ val )
        else: # self.where == 'tag':
          node.set( self.inte_att, str(val) )

        if self.inte_att_type == 'color':
          self.R_cur += self.R_inc
          self.G_cur += self.G_inc
          self.B_cur += self.B_inc
        else:
          self.val_cur += self.val_inc

      return True

if __name__ == '__main__':   #pragma: no cover
    e = InterpAttG()
    if e.affect():
      exit(0)
    else:
      exit(1)
