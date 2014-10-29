#!/usr/bin/env python

# Written by Tavmjong Bah

import inkex

class C(inkex.Effect):
  def __init__(self):
    inkex.Effect.__init__(self)
    self.OptionParser.add_option("-s", "--size", action="store", type="int", dest="icon_size", default="16", help="Icon size")

  def effect(self):

    size  = self.options.icon_size

    root = self.document.getroot()
    root.set("id", "SVGRoot")
    root.set("width",  str(size) + 'px')
    root.set("height", str(size) + 'px')
    root.set("viewBox", "0 0 " + str(size) + " " + str(size) )

    namedview = root.find(inkex.addNS('namedview', 'sodipodi'))
    if namedview is None:
        namedview = inkex.etree.SubElement( root, inkex.addNS('namedview', 'sodipodi') );
     
    namedview.set(inkex.addNS('document-units', 'inkscape'), 'px')

    namedview.set(inkex.addNS('zoom',      'inkscape'), str(256.0/size) )
    namedview.set(inkex.addNS('cx',        'inkscape'), str(size/2.0) )
    namedview.set(inkex.addNS('cy',        'inkscape'), str(size/2.0) )
    namedview.set(inkex.addNS('grid-bbox', 'inkscape'), "true" )



c = C()
c.affect()
