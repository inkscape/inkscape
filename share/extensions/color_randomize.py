#!/usr/bin/env python
import coloreffect,random,inkex

class C(coloreffect.ColorEffect):
    def __init__(self):
        coloreffect.ColorEffect.__init__(self)
        self.OptionParser.add_option("-x", "--hue",
            action="store", type="inkbool", 
            dest="hue", default=True,
            help="randomize hue")
        self.OptionParser.add_option("-s", "--saturation",
            action="store", type="inkbool", 
            dest="saturation", default=True,
            help="randomize saturation")
        self.OptionParser.add_option("-l", "--lightness",
            action="store", type="inkbool", 
            dest="lightness", default=True,
            help="randomize lightness")
        self.OptionParser.add_option("--tab",
            action="store", type="string",
            dest="tab",
            help="The selected UI-tab when OK was pressed")

    def colmod(self,r,g,b):
        hsl = self.rgb_to_hsl(r/255.0, g/255.0, b/255.0)
        if(self.options.hue):
            hsl[0]=random.random()
        if(self.options.saturation):
            hsl[1]=random.random()
        if(self.options.lightness):
            hsl[2]=random.random()
        rgb = self.hsl_to_rgb(hsl[0], hsl[1], hsl[2])
        return '%02x%02x%02x' % (rgb[0]*255, rgb[1]*255, rgb[2]*255)

c = C()
c.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
