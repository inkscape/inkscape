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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
'''
import inkex, sys, os
#from lxml import etree

class InterpAttG(inkex.Effect):

    def __init__(self):
        inkex.Effect.__init__(self)
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

    def ensureInkWebSupport(self):
      if not self.document.xpath('//*[@id="inkwebjs"]', namespaces=inkex.NSS):
        root = self.document.getroot()
        scriptEl = inkex.etree.Element( "script" )
        scriptEl.set( "id", "inkwebjs" )
        scriptEl.set( "type", "text/javascript" )
        js = open( os.path.join(sys.path[0], "inkweb.js"), 'r' )
        scriptEl.text = inkex.etree.CDATA( js.read() )
        js.close()
        root.insert( 0, scriptEl )

    def effect(self):
      self.ensureInkWebSupport()

      elFrom = self.selected[ self.options.ids[0] ]
      idTo = self.options.ids[1]

      prevEvCode = elFrom.get( self.options.when )
      if prevEvCode == None: prevEvCode = ""

      evCode = "InkWeb.transmitAtt({from:this, to:'"+idTo+"', att:'"+self.options.att+"'})"

      if self.options.compatibility == 'append':
        evCode = prevEvCode +";\n"+ evCode
      if self.options.compatibility == 'prepend':
        evCode = evCode +";\n"+ prevEvCode

      elFrom.set( self.options.when, evCode )

if __name__ == '__main__':
    e = InterpAttG()
    e.affect()