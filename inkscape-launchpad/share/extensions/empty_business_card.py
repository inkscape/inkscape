#!/usr/bin/env python

# Written by Tavmjong Bah

import inkex
import re

class C(inkex.Effect):
  def __init__(self):
    inkex.Effect.__init__(self)
    self.OptionParser.add_option("-s", "--size",   action="store", type="string", dest="card_size",   default="90mmx55mm",   help="Business card size")

  def effect(self):

    size   = self.options.card_size

    p = re.compile('([0-9.]*)([a-z][a-z])x([0-9.]*)([a-z][a-z])')
    m = p.match( size )
    width       = m.group(1)
    width_unit  = m.group(2)
    height      = m.group(3)
    height_unit = m.group(4)

    root = self.document.getroot()
    root.set("id", "SVGRoot")
    root.set("width",  width + width_unit)
    root.set("height", height + height_unit)
    root.set("viewBox", "0 0 " + width + " " + height )

    namedview = root.find(inkex.addNS('namedview', 'sodipodi'))
    if namedview is None:
        namedview = inkex.etree.SubElement( root, inkex.addNS('namedview', 'sodipodi') );
     
    namedview.set(inkex.addNS('document-units', 'inkscape'), width_unit)

    width_int  = int(self.uutounit(float(width), 'px'))
    height_int = int(self.uutounit(float(height), 'px'))

    namedview.set(inkex.addNS('zoom',      'inkscape'), str(2) )
    namedview.set(inkex.addNS('cx',        'inkscape'), str(width_int/2.0) )
    namedview.set(inkex.addNS('cy',        'inkscape'), str(height_int/2.0) )


c = C()
c.affect()
