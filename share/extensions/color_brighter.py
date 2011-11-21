#!/usr/bin/env python
import coloreffect

class C(coloreffect.ColorEffect):
  def colmod(self,r,g,b):
    FACTOR=0.9
   
    i=int(1.0/(1.0-FACTOR))
    if r==0 and g==0 and b==0:
      return '%02x%02x%02x' % (i,i,i)
    if r>0 and r<i:
      r=i
    if g>0 and g<i:
      g=i
    if b>0 and b<i:
      b=i;

    r=min(int(round((r/FACTOR))), 255)
    g=min(int(round((g/FACTOR))), 255)
    b=min(int(round((b/FACTOR))), 255)
   
    return '%02x%02x%02x' % (r,g,b)

c = C()
c.affect()
