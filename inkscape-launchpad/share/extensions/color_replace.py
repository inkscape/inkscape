#!/usr/bin/env python
import coloreffect

import inkex

class C(coloreffect.ColorEffect):
  def __init__(self):
    coloreffect.ColorEffect.__init__(self)
    self.OptionParser.add_option("-f", "--from_color", action="store", type="string", dest="from_color", default="000000", help="Replace color")
    self.OptionParser.add_option("-t", "--to_color", action="store", type="string", dest="to_color", default="000000", help="By color")

  def colmod(self,r,g,b):
    this_color = '%02x%02x%02x' % (r, g, b)

    fr = self.options.from_color.strip('"').replace('#', '').lower()
    to = self.options.to_color.strip('"').replace('#', '').lower()
       
    #inkex.debug(this_color+"|"+fr+"|"+to)
    if this_color == fr:
      return to
    else:
      return this_color

c = C()
c.affect()
