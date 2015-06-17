#!/usr/bin/env python

# Written by Tavmjong Bah

import inkex
import re

class C(inkex.Effect):
  def __init__(self):
    inkex.Effect.__init__(self)
    self.OptionParser.add_option("-w", "--width",  action="store", type="int",    dest="generic_width",  default="1920", help="Custom width")
    self.OptionParser.add_option("-z", "--height", action="store", type="int",    dest="generic_height", default="1080", help="Custom height")
    self.OptionParser.add_option("-u", "--unit",   action="store", type="string", dest="generic_unit",   default="px",   help="SVG Unit")
    self.OptionParser.add_option("-b", "--background", action="store", type="string", dest="generic_background", default="normal", help="Canvas background")
    self.OptionParser.add_option("-n", "--noborder", action="store", type="inkbool", dest="generic_noborder", default=False)
    # self.OptionParser.add_option("-l", "--layer", action="store", type="inkbool", dest="generic_layer", default=True)

  def effect(self):

    width  = self.options.generic_width
    height = self.options.generic_height
    unit   = self.options.generic_unit

    root = self.document.getroot()
    root.set("id", "SVGRoot")
    root.set("width",  str(width) + unit)
    root.set("height", str(height) + unit)
    root.set("viewBox", "0 0 " + str(width) + " " + str(height) )

    namedview = root.find(inkex.addNS('namedview', 'sodipodi'))
    if namedview is None:
        namedview = inkex.etree.SubElement( root, inkex.addNS('namedview', 'sodipodi') );

    namedview.set(inkex.addNS('document-units', 'inkscape'), unit)

    # Until units are supported in 'cx', etc.
    namedview.set(inkex.addNS('zoom', 'inkscape'), str(512.0/self.uutounit( width,  'px' )) )
    namedview.set(inkex.addNS('cx',   'inkscape'), str(self.uutounit( width,  'px' )/2.0 ) )
    namedview.set(inkex.addNS('cy',   'inkscape'), str(self.uutounit( height, 'px' )/2.0 ) )

    if self.options.generic_background == "white":
      namedview.set( 'pagecolor', "#ffffff" )
      namedview.set( 'bordercolor', "#666666" )
      namedview.set(inkex.addNS('pageopacity', 'inkscape'), "1.0" )
      namedview.set(inkex.addNS('pageshadow', 'inkscape'), "0" )

    if self.options.generic_background == "gray":
      namedview.set( 'pagecolor', "#808080" )
      namedview.set( 'bordercolor', "#444444" )
      namedview.set(inkex.addNS('pageopacity', 'inkscape'), "1.0" )
      namedview.set(inkex.addNS('pageshadow', 'inkscape'), "0" )

    if self.options.generic_background == "black":
      namedview.set( 'pagecolor', "#000000" )
      namedview.set( 'bordercolor', "#999999" )
      namedview.set(inkex.addNS('pageopacity', 'inkscape'), "1.0" )
      namedview.set(inkex.addNS('pageshadow', 'inkscape'), "0" )

    if self.options.generic_noborder:
      pagecolor = namedview.get( 'pagecolor' )
      namedview.set( 'bordercolor', pagecolor )
      namedview.set( 'borderopacity', "0" )

    # This nees more thought... we need to set "Current layer" to (root), how?
    # if self.options.generic_layer:
    #   # Add layer
    #   inkex.debug( "We want a layer" )
    # else:
    #   # Remove layer id default document (assuming only one)
    #   inkex.debug( "We don't want a layer" )
    #   layer_node = self.current_layer
    #   if layer_node is not None:
    #     inkex.debug( "We have layer" )
    #     root.remove(layer_node)
    #     try:
    #       del namedview.attrib[ inkex.addNS('current-layer', 'inkscape') ]
    #     except:
    #       pass


c = C()
c.affect()
