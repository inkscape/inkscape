#!/usr/bin/env python
import coloreffect

class C(coloreffect.ColorEffect):
  def colmod(self,r,g,b):
    return '%02x%02x%02x' % (255-r,255-g,255-b)

c = C()
c.affect()
