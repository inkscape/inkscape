#!/usr/bin/env python
import coloreffect,sys

class C(coloreffect.ColorEffect):
  def __init__(self):
    coloreffect.ColorEffect.__init__(self)
    self.OptionParser.add_option("-t", "--threshold",
                                 action="store", type="int",
                                 dest="threshold", default=127,
                                 help="Threshold Color Level")

  def colmod(self,r,g,b):
    #ITU-R Recommendation BT.709
    #l = 0.2125 * r + 0.7154 * g + 0.0721 * b
    #NTSC and PAL
    l = 0.299 * r + 0.587 * g + 0.114 * b
    if l > self.options.threshold:
       ig = 255
    else:
       ig = 0
    #coloreffect.debug('gs '+hex(r)+' '+hex(g)+' '+hex(b)+'%02x%02x%02x' % (ig,ig,ig))
    return '%02x%02x%02x' % (ig,ig,ig)

c = C()
c.affect()
