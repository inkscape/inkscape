#!/usr/bin/env python

import random

import coloreffect
import inkex

class C(coloreffect.ColorEffect):
    def __init__(self):
        coloreffect.ColorEffect.__init__(self)
        self.OptionParser.add_option("-y", "--hue_range",
            action="store", type="int", 
            dest="hue_range", default=0,
            help="Hue range")
        self.OptionParser.add_option("-t", "--saturation_range",
            action="store", type="int", 
            dest="saturation_range", default=0,
            help="Saturation range")
        self.OptionParser.add_option("-m", "--lightness_range",
            action="store", type="int", 
            dest="lightness_range", default=0,
            help="Lightness range")
	self.OptionParser.add_option("-o", "--opacity_range",
            action="store", type="int", 
            dest="opacity_range", default=0,
            help="Opacity range")
        self.OptionParser.add_option("--tab",
            action="store", type="string",
            dest="tab",
            help="The selected UI-tab when OK was pressed")

    def randomize_hsl(self, limit, current_value):
        limit = 255.0 * limit / 100.0
        limit /= 2
        max = int((current_value * 255.0) + limit)
        min = int((current_value * 255.0) - limit)
        if max > 255:
            min = min - (max - 255)
            max = 255
        if min < 0:
            max = max - min
            min = 0
        return random.randrange(min, max) / 255.0

    def colmod(self,r,g,b):
        hsl = self.rgb_to_hsl(r/255.0, g/255.0, b/255.0)
        if self.options.hue_range > 0:
            hsl[0] = self.randomize_hsl(self.options.hue_range, hsl[0])
        if self.options.saturation_range > 0:
            hsl[1] = self.randomize_hsl(self.options.saturation_range, hsl[1])
        if self.options.lightness_range > 0:
            hsl[2] = self.randomize_hsl(self.options.lightness_range, hsl[2])
        rgb = self.hsl_to_rgb(hsl[0], hsl[1], hsl[2])
        return '%02x%02x%02x' % (rgb[0]*255, rgb[1]*255, rgb[2]*255)

    def opacmod(self, opacity):
        if self.options.opacity_range > 0:
            # maybe not necessary, but better not change things that shouldn't change
            try: 
                opacity = float(opacity)
            except ValueError:
                return opacity

            limit = self.options.opacity_range
            limit /= 2
            max = opacity*100 + limit
            min = opacity*100 - limit
            if max > 100:
                min = min - (max - 100)
                max = 100
            if min < 0:
                max = max - min
                min = 0
            ret = str(random.uniform(min,max)/100)
            return ret
        return opacity


if __name__ == '__main__':
    c = C()
    c.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
