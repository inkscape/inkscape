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
# local library
import inkwebeffect
import inkex


class InkWebTransmitAtt(inkwebeffect.InkWebEffect):

    def __init__(self):
        inkwebeffect.InkWebEffect.__init__(self)
        self.OptionParser.add_option("-a", "--att",
                        action="store", type="string",
                        dest="att", default="fill",
                        help="Attribute to transmited.")
        self.OptionParser.add_option("-w", "--when",
                        action="store", type="string",
                        dest="when", default="onclick",
                        help="When it must to transmit?")
        self.OptionParser.add_option("-c", "--compatibility",
                        action="store", type="string",
                        dest="compatibility", default="append",
                        help="Compatibility with previews code to this event.")
        self.OptionParser.add_option("-t", "--from-and-to",
                        action="store", type="string",
                        dest="from_and_to", default="g-to-one",
                        help='Who transmit to Who? "g-to-one" All tramsmit to the last. "one-to-g" The first transmit to all.')
        self.OptionParser.add_option("--tab",
                        action="store", type="string",
                        dest="tab",
                        help="The selected UI-tab when OK was pressed")

    def effect(self):
      self.ensureInkWebSupport()

      if len(self.options.ids) < 2:
        inkwebeffect.inkex.errormsg(_("You must select at least two elements."))
        exit(1)

      elFrom = []
      idTo = []
      if self.options.from_and_to == "g-to-one":
        # All tramsmit to the last
        for selId in self.options.ids[:-1]:
          elFrom.append( self.selected[selId] )
        idTo.append( self.options.ids[-1] )
      else:
        # The first transmit to all
        elFrom.append( self.selected[ self.options.ids[0] ] )
        for selId in self.options.ids[1:]:
          idTo.append( selId )

      evCode = "InkWeb.transmitAtt({from:this, " + \
                                   "to:['"+ "','".join(idTo) +"'], " + \
                                   "att:'"+ self.options.att +"'})"

      for el in elFrom:
        prevEvCode = el.get( self.options.when )
        if prevEvCode == None: prevEvCode = ""

        if self.options.compatibility == 'append':
          elEvCode = prevEvCode +";\n"+ evCode
        if self.options.compatibility == 'prepend':
          elEvCode = evCode +";\n"+ prevEvCode

        el.set( self.options.when, elEvCode )

if __name__ == '__main__':
    e = InkWebTransmitAtt()
    e.affect()

