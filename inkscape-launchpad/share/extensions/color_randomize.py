#!/usr/bin/env python
import coloreffect,random,inkex

class C(coloreffect.ColorEffect):
    def __init__(self):
        coloreffect.ColorEffect.__init__(self)
        self.OptionParser.add_option("-x", "--hue",
            action="store", type="inkbool", 
            dest="hue", default=True,
            help="Randomize hue")
        self.OptionParser.add_option("-y", "--hue_range",
            action="store", type="int", 
            dest="hue_range", default=0,
            help="Hue range")
        self.OptionParser.add_option("-s", "--saturation",
            action="store", type="inkbool", 
            dest="saturation", default=True,
            help="Randomize saturation")
        self.OptionParser.add_option("-t", "--saturation_range",
            action="store", type="int", 
            dest="saturation_range", default=0,
            help="Saturation range")
        self.OptionParser.add_option("-l", "--lightness",
            action="store", type="inkbool", 
            dest="lightness", default=True,
            help="Randomize lightness")
        self.OptionParser.add_option("-m", "--lightness_range",
            action="store", type="int", 
            dest="lightness_range", default=0,
            help="Lightness range")
        self.OptionParser.add_option("--tab",
            action="store", type="string",
            dest="tab",
            help="The selected UI-tab when OK was pressed")

    def colmod(self,r,g,b):
        hsl = self.rgb_to_hsl(r/255.0, g/255.0, b/255.0)
        
        if(self.options.hue):
            limit = 255.0 * (1 + self.options.hue_range) / 100.0
            limit /= 2
            max = int((hsl[0] * 255.0) + limit)
            min = int((hsl[0] * 255.0) - limit)
            if max > 255:
                min = min - (max - 255)
                max = 255
            if min < 0:
                max = max - min
                min = 0
            hsl[0] = random.randrange(min, max)
            hsl[0] /= 255.0
        if(self.options.saturation):
            limit = 255.0 * (1 + self.options.saturation_range) / 100.0
            limit /= 2
            max = int((hsl[1] * 255.0) + limit)
            min = int((hsl[1] * 255.0) - limit)
            if max > 255:
                min = min - (max - 255)
                max = 255
            if min < 0:
                max = max - min
                min = 0
            hsl[1] = random.randrange(min, max)
            hsl[1] /= 255.0
        if(self.options.lightness):
            limit = 255.0 * (1 + self.options.lightness_range) / 100.0
            limit /= 2
            max = int((hsl[2] * 255.0) + limit)
            min = int((hsl[2] * 255.0) - limit)
            if max > 255:
                min = min - (max - 255)
                max = 255
            if min < 0:
                max = max - min
                min = 0
            hsl[2] = random.randrange(min, max)
            hsl[2] /= 255.0
        rgb = self.hsl_to_rgb(hsl[0], hsl[1], hsl[2])
        return '%02x%02x%02x' % (rgb[0]*255, rgb[1]*255, rgb[2]*255)

c = C()
c.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
