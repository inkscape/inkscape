#!/usr/bin/env python
import coloreffect

class C(coloreffect.ColorEffect):
  def colmod(self,r,g,b):
    l = (max(r,g,b)+min(r,g,b))/2
    ig=int(round(l))
    return '%02x%02x%02x' % (ig,ig,ig)

c = C()
c.affect()
