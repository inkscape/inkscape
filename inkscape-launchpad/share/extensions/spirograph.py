#! /usr/bin/env python
'''
Copyright (C) 2007 Joel Holdsworth joel@airwebreathe.org.uk

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
'''
import inkex, simplestyle, math
from simpletransform import computePointInNode

class Spirograph(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-R", "--primaryr",
                        action="store", type="float",
                        dest="primaryr", default=60.0,
                        help="The radius of the outer gear")
        self.OptionParser.add_option("-r", "--secondaryr",
                        action="store", type="float",
                        dest="secondaryr", default=100.0,
                        help="The radius of the inner gear")
        self.OptionParser.add_option("-d", "--penr",
                        action="store", type="float",
                        dest="penr", default=50.0,
                        help="The distance of the pen from the inner gear")
        self.OptionParser.add_option("-p", "--gearplacement",
                        action="store", type="string",
                        dest="gearplacement", default="inside",
                        help="Selects whether the gear is inside or outside the ring")
        self.OptionParser.add_option("-a", "--rotation",
                        action="store", type="float",
                        dest="rotation", default=0.0,
                        help="The number of degrees to rotate the image by")
        self.OptionParser.add_option("-q", "--quality",
                        action="store", type="int",
                        dest="quality", default=16,
                        help="The quality of the calculated output")

    def effect(self):
        self.options.primaryr = self.unittouu(str(self.options.primaryr) + 'px')
        self.options.secondaryr = self.unittouu(str(self.options.secondaryr) + 'px')
        self.options.penr = self.unittouu(str(self.options.penr) + 'px')

        if self.options.secondaryr == 0:
            return
        if self.options.quality == 0:
            return

        if(self.options.gearplacement.strip(' ').lower().startswith('outside')):
            a = self.options.primaryr + self.options.secondaryr
            flip = -1
        else:
            a = self.options.primaryr - self.options.secondaryr
            flip = 1

        ratio = a / self.options.secondaryr
        if ratio == 0:
            return
        scale = 2 * math.pi / (ratio * self.options.quality)

        rotation = - math.pi * self.options.rotation / 180;

        new = inkex.etree.Element(inkex.addNS('path','svg'))
        s = { 'stroke': '#000000', 'fill': 'none', 'stroke-width': str(self.unittouu('1px')) }
        new.set('style', simplestyle.formatStyle(s))

        pathString = ''
        maxPointCount = 1000

        for i in range(maxPointCount):

            theta = i * scale

            view_center = computePointInNode(list(self.view_center), self.current_layer)
            x = a * math.cos(theta + rotation) + \
                self.options.penr * math.cos(ratio * theta + rotation) * flip + \
                view_center[0]
            y = a * math.sin(theta + rotation) - \
                self.options.penr * math.sin(ratio * theta + rotation) + \
                view_center[1]

            dx = (-a * math.sin(theta + rotation) - \
                ratio * self.options.penr * math.sin(ratio * theta + rotation) * flip) * scale / 3
            dy = (a * math.cos(theta + rotation) - \
                ratio * self.options.penr * math.cos(ratio * theta + rotation)) * scale / 3

            if i <= 0:
                pathString += 'M ' + str(x) + ',' + str(y) + ' C ' + str(x + dx) + ',' + str(y + dy) + ' '
            else:
                pathString += str(x - dx) + ',' + str(y - dy) + ' ' + str(x) + ',' + str(y)

                if math.fmod(i / ratio, self.options.quality) == 0 and i % self.options.quality == 0:
                    pathString += 'Z'
                    break
                else:
                    if i == maxPointCount - 1:
                        pass # we reached the allowed maximum of points, stop here
                    else:
                        pathString += ' C ' + str(x + dx) + ',' + str(y + dy) + ' '


        new.set('d', pathString)
        self.current_layer.append(new)

if __name__ == '__main__':
    e = Spirograph()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
