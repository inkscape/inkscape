#!/usr/bin/env python

# standard library
import random
# local library
import coloreffect
import inkex

class C(coloreffect.ColorEffect):
    def __init__(self):
        coloreffect.ColorEffect.__init__(self)
        self.OptionParser.add_option("-x", "--hue",
            action="store", type="int", 
            dest="hue", default="0",
            help="Adjust hue")
        self.OptionParser.add_option("-s", "--saturation",
            action="store", type="int", 
            dest="saturation", default="0",
            help="Adjust saturation")
        self.OptionParser.add_option("-l", "--lightness",
            action="store", type="int", 
            dest="lightness", default="0",
            help="Adjust lightness")
        self.OptionParser.add_option("", "--random_h",
            action="store", type="inkbool", 
            dest="random_hue", default=False,
            help="Randomize hue")
        self.OptionParser.add_option("", "--random_s",
            action="store", type="inkbool", 
            dest="random_saturation", default=False,
            help="Randomize saturation")
        self.OptionParser.add_option("", "--random_l",
            action="store", type="inkbool", 
            dest="random_lightness", default=False,
            help="Randomize lightness")
        self.OptionParser.add_option("--tab",
            action="store", type="string",
            dest="tab",
            help="The selected UI-tab when OK was pressed")

    def clamp(self, minimum, x, maximum):
        return max(minimum, min(x, maximum))
        
    def colmod(self, r, g, b):
        hsl = self.rgb_to_hsl(r/255.0, g/255.0, b/255.0)
        #inkex.debug("hsl old: " + str(hsl[0]) + ", " + str(hsl[1]) + ", " + str(hsl[2]))
        if (self.options.random_hue):
            hsl[0] = random.random()
        elif (self.options.hue):
            hueval = hsl[0] + (self.options.hue / 360.0)
            hsl[0] = hueval % 1
        if(self.options.random_saturation):
            hsl[1] = random.random()
        elif (self.options.saturation):
            satval = hsl[1] + (self.options.saturation / 100.0)
            hsl[1] = self.clamp(0.0, satval, 1.0)
        if(self.options.random_lightness):
            hsl[2] = random.random()
        elif (self.options.lightness):
            lightval = hsl[2] + (self.options.lightness / 100.0)
            hsl[2] = self.clamp(0.0, lightval, 1.0)
        #inkex.debug("hsl new: " + str(hsl[0]) + ", " + str(hsl[1]) + ", " + str(hsl[2]))
        rgb = self.hsl_to_rgb(hsl[0], hsl[1], hsl[2])
        return '%02x%02x%02x' % (rgb[0]*255, rgb[1]*255, rgb[2]*255)

c = C()
c.affect()

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
