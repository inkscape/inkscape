#!/usr/bin/env python

# Rewritten by Tavmjong Bah to add correct viewBox, inkscape:cx, etc. attributes

import inkex

class C(inkex.Effect):
  def __init__(self):
    inkex.Effect.__init__(self)
    self.OptionParser.add_option("-s", "--size", action="store", type="string", dest="page_size", default="a4", help="Page size")
    self.OptionParser.add_option("-o", "--orientation", action="store", type="string", dest="page_orientation", default="vertical", help="Page orientation")
    self.OptionParser.add_option("-b", "--background", action="store", type="string", dest="page_background", default="normal", help="Page background")
    self.OptionParser.add_option("-n", "--noborder", action="store", type="inkbool", dest="page_noborder", default=False)

  def effect(self):

    width  = 300
    height = 300
    units  = 'px'

    if self.options.page_size == "a5":
      width = 148
      height= 210
      units = 'mm'

    if self.options.page_size == "a4":
      width = 210
      height= 297
      units = 'mm'

    if self.options.page_size == "a3":
      width = 297
      height= 420
      units = 'mm'

    if self.options.page_size == "letter":
      width  = 8.5
      height = 11
      units  = 'in'

    if self.options.page_orientation == "horizontal":
      width, height = height, width

    root = self.document.getroot()
    root.set("id", "SVGRoot")
    root.set("width",  str(width)  + units)
    root.set("height", str(height) + units)
    root.set("viewBox", "0 0 " + str(width) + " " + str(height) )

    namedview = root.find(inkex.addNS('namedview', 'sodipodi'))
    if namedview is None:
        namedview = inkex.etree.SubElement( root, inkex.addNS('namedview', 'sodipodi') );
     
    namedview.set(inkex.addNS('document-units', 'inkscape'), units)

    # Until units are supported in 'cx', etc.
    namedview.set(inkex.addNS('cx', 'inkscape'), str(self.uutounit( width,  'px' )/2.0 ) )
    namedview.set(inkex.addNS('cy', 'inkscape'), str(self.uutounit( height, 'px' )/2.0 ) )


    if self.options.page_background == "white":
      namedview.set( 'pagecolor', "#ffffff" )
      namedview.set( 'bordercolor', "#666666" )
      namedview.set(inkex.addNS('pageopacity', 'inkscape'), "1.0" )
      namedview.set(inkex.addNS('pageshadow', 'inkscape'), "0" )

    if self.options.page_background == "gray":
      namedview.set( 'pagecolor', "#808080" )
      namedview.set( 'bordercolor', "#444444" )
      namedview.set(inkex.addNS('pageopacity', 'inkscape'), "1.0" )
      namedview.set(inkex.addNS('pageshadow', 'inkscape'), "0" )

    if self.options.page_background == "black":
      namedview.set( 'pagecolor', "#000000" )
      namedview.set( 'bordercolor', "#999999" )
      namedview.set(inkex.addNS('pageopacity', 'inkscape'), "1.0" )
      namedview.set(inkex.addNS('pageshadow', 'inkscape'), "0" )

    if self.options.page_noborder:
      pagecolor = namedview.get( 'pagecolor' )
      namedview.set( 'bordercolor', pagecolor )
      namedview.set( 'borderopacity', "0" )

c = C()
c.affect()
