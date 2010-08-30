#!/usr/bin/env python
import coloreffect, inkex

class C(coloreffect.ColorEffect):
  def colmod(self,r,g,b):
    hsl = self.rgb_to_hsl(r/255.0, g/255.0, b/255.0)
    #inkex.debug("hsl: " + str(hsl[0]) + ", " + str(hsl[1]) + ", " + str(hsl[2]))
    hsl[0] = hsl[0] - 0.05
    if hsl[0] < 0.0:
        hsl[0] = 1.0 + hsl[0]
    rgb = self.hsl_to_rgb(hsl[0], hsl[1], hsl[2])
    return '%02x%02x%02x' % (rgb[0]*255, rgb[1]*255, rgb[2]*255)

c = C()
c.affect()
