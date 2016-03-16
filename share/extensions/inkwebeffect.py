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

import inkex, sys, os, re

class InkWebEffect(inkex.Effect):
  def __init__(self):
    inkex.Effect.__init__(self)
    self.reUpdateJS = '/\\*\\s* inkweb.js [^*]* InkWebEffect:AutoUpdate \\s*\\*/'

  def mustAddInkWebJSCode(self, scriptEl):
    if not scriptEl.text: return True
    if len(scriptEl.text) == 0: return True
    if re.search(self.reUpdateJS, scriptEl.text): return True
    return False

  def addInkWebJSCode(self, scriptEl):
    js = open( os.path.join(sys.path[0], "inkweb.js"), 'r' )
    if hasattr(inkex.etree, "CDATA"):
      scriptEl.text = \
        inkex.etree.CDATA(
          "\n/* inkweb.js - InkWebEffect:AutoUpdate */\n" + js.read()
        )
    else:
      scriptEl.text = \
          "\n/* inkweb.js - InkWebEffect:AutoUpdate */\n" + js.read()
    js.close()

  def ensureInkWebSupport(self):
    # Search for the script tag with the inkweb.js code:
    scriptEl = None
    scripts = self.document.xpath('//svg:script', namespaces=inkex.NSS)
    for s in scripts:
      if re.search(self.reUpdateJS, s.text):
        scriptEl = s

    if scriptEl is None:
      root = self.document.getroot()
      scriptEl = inkex.etree.Element( "script" )
      scriptEl.set( "id", "inkwebjs" )
      scriptEl.set( "type", "text/javascript" )
      root.insert( 0, scriptEl )

    if self.mustAddInkWebJSCode(scriptEl):
      self.addInkWebJSCode(scriptEl)

