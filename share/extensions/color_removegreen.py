#!/usr/bin/env python
import coloreffect

class C(coloreffect.ColorEffect):
  def colmod(self,r,g,b):
    return '%02x%02x%02x' % (r,0,b)

c = C()
c.affect()
