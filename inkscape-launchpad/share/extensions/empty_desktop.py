#!/usr/bin/env python

# Written by Tavmjong Bah

import inkex
import re

class C(inkex.Effect):
  def __init__(self):
    inkex.Effect.__init__(self)
    self.OptionParser.add_option("-s", "--size",   action="store", type="string", dest="desktop_size",   default="16",   help="Desktop size")

    self.OptionParser.add_option("-w", "--width",  action="store", type="int",    dest="desktop_width",  default="1920", help="Custom width")
    self.OptionParser.add_option("-z", "--height", action="store", type="int",    dest="desktop_height", default="1080", help="Custom height")

  def effect(self):

    size   = self.options.desktop_size
    width  = self.options.desktop_width
    height = self.options.desktop_height

    if size != "Custom":
      p = re.compile('([0-9]*)x([0-9]*)')
      m = p.match( size )
      width  = int(m.group(1))
      height = int(m.group(2))


    root = self.document.getroot()
    root.set("id", "SVGRoot")
    root.set("width",  str(width) + 'px')
    root.set("height", str(height) + 'px')
    root.set("viewBox", "0 0 " + str(width) + " " + str(height) )

    namedview = root.find(inkex.addNS('namedview', 'sodipodi'))
    if namedview is None:
        namedview = inkex.etree.SubElement( root, inkex.addNS('namedview', 'sodipodi') );
     
    namedview.set(inkex.addNS('document-units', 'inkscape'), 'px')

    namedview.set(inkex.addNS('cx',        'inkscape'), str(width/2.0) )
    namedview.set(inkex.addNS('cy',        'inkscape'), str(height/2.0) )


c = C()
c.affect()
