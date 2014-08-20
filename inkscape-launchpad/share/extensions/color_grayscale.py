#!/usr/bin/env python
import coloreffect

class C(coloreffect.ColorEffect):
  def colmod(self,r,g,b):
    #ITU-R Recommendation BT.709
    #l = 0.2125 * r + 0.7154 * g + 0.0721 * b
    #NTSC and PAL
    l = 0.299 * r + 0.587 * g + 0.114 * b
    ig=int(round(l))
    #coloreffect.debug('gs '+hex(r)+' '+hex(g)+' '+hex(b)+'%02x%02x%02x' % (ig,ig,ig))
    return '%02x%02x%02x' % (ig,ig,ig)

c = C()
c.affect()
