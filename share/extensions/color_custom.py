#!/usr/bin/env python
import coloreffect

class C(coloreffect.ColorEffect):
    def __init__(self):
        coloreffect.ColorEffect.__init__(self)
        self.OptionParser.add_option("--r",
            action="store", type="string",
            dest="rFunction", default="r",
            help="red channel function")
        self.OptionParser.add_option("--g",
            action="store", type="string",
            dest="gFunction", default="g",
            help="green channel function")
        self.OptionParser.add_option("--b",
            action="store", type="string",
            dest="bFunction", default="b",
            help="blue channel function")
        self.OptionParser.add_option("--tab",
            action="store", type="string",
            dest="tab",
            help="The selected UI-tab when OK was pressed")
        self.OptionParser.add_option("--scale",
            action="store", type="string",
            dest="scale",
            help="The input (r,g,b) range")

    def normalize(self, v):
        if v<0:
            return 0.0
        if v > float(self.options.scale):
            return float(self.options.scale)
        return v

    def _hexstr(self,r,g,b):
        return '%02x%02x%02x' % (int(round(r)),int(round(g)),int(round(b)))

    def colmod(self,_r,_g,_b):
        factor = 255.0/float(self.options.scale)
        r=float(_r)/factor
        g=float(_g)/factor
        b=float(_b)/factor

        # add stuff to be accessible from within the custom color function here.
        safeenv = {'__builtins__':{},'r':r,'g':g,'b':b}

        try:
            r2=self.normalize(eval(self.options.rFunction,safeenv))
            g2=self.normalize(eval(self.options.gFunction,safeenv))
            b2=self.normalize(eval(self.options.bFunction,safeenv))
        except:
            return self._hexstr(255.0,0.0,0.0)
        return self._hexstr(r2*factor,g2*factor,b2*factor)

c = C()
c.affect()
