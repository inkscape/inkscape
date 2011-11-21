#!/usr/bin/env python
import coloreffect

class C(coloreffect.ColorEffect):
  def colmod(self,r,g,b):
    FACTOR=0.9
    r=int(round(max(r*FACTOR,0)))
    g=int(round(max(g*FACTOR,0)))
    b=int(round(max(b*FACTOR,0)))
    return '%02x%02x%02x' % (r,g,b)

c = C()
c.affect()
