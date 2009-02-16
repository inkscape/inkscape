#! /usr/bin/env python
'''
Copyright (C) 2009 Aurelio A. Heckert <aurium (a) gmail dot com>

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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
'''

__version__ = "0.1"

import inkex, simplestyle
from math import *
from simplepath import formatPath

class FoldableBox(inkex.Effect):

    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-x", "--width",
                        action="store", type="float",
                        dest="width", default=10.0,
                        help="The Box Width - in the X dimension")
        self.OptionParser.add_option("-y", "--height",
                        action="store", type="float",
                        dest="height", default=15.0,
                        help="The Box Height - in the Y dimension")
        self.OptionParser.add_option("-z", "--depth",
                        action="store", type="float",
                        dest="depth", default=3.0,
                        help="The Box Depth - in the Z dimension")
        self.OptionParser.add_option("-u", "--unit",
                        action="store", type="string",
                        dest="unit", default="cm",
                        help="The unit of the box dimensions")
        self.OptionParser.add_option("-p", "--paper-spessure",
                        action="store", type="float",
                        dest="spessure", default=0.01,
                        help="Paper Spessure - sometimes that is important")
        self.OptionParser.add_option("-t", "--tab-proportion",
                        action="store", type="float",
                        dest="tabProportion", default=0.6,
                        help="Inner tab propotion for upper tab")
        self.OptionParser.add_option("-g", "--guide-line",
                        action="store", type="inkbool",
                        dest="guideLine", default=True,
                        help="Add guide lines to help the drawing limits")

    def effect(self):

        docW = inkex.unittouu(self.document.getroot().get('width'))
        docH = inkex.unittouu(self.document.getroot().get('height'))

        boxW = inkex.unittouu( str(self.options.width)  + self.options.unit )
        boxH = inkex.unittouu( str(self.options.height) + self.options.unit )
        boxD = inkex.unittouu( str(self.options.depth)  + self.options.unit )
        tabProp = self.options.tabProportion
        tabH = boxD * tabProp

        box_id = self.uniqueId('box')
        g = inkex.etree.SubElement(self.current_layer, 'g', {'id':box_id})

        line_style = simplestyle.formatStyle({ 'stroke': '#000000', 'fill': 'none' })

        #self.createGuide( 0, docH, 0 );

        # Inner Close Tab
        line_path = [
                      [ 'M', [ boxW-(boxD/4), 0 ] ],
                      [ 'C', [ boxW-(boxD/4), 0, boxW, tabH/3, boxW, tabH/1.3 ] ],
                      [ 'L', [ boxW, tabH     ] ],
                      [ 'L', [ 0,    tabH     ] ],
                      [ 'L', [ 0,    tabH/1.3 ] ],
                      [ 'C', [ 0,    tabH/3, boxD/4, 0, boxD/4, 0 ] ],
                      [ 'Z', [] ]
                    ]
        line_atts = { 'style':line_style, 'id':box_id+'-inner-close-tab', 'd':formatPath(line_path) }
        inkex.etree.SubElement(g, inkex.addNS('path','svg'), line_atts )

        lower_pos = boxD+tabH
        left_pos = 0

        #self.createGuide( 0, docH-tabH, 0 );

        # Upper Close Tab
        line_path = [
                      [ 'M', [ left_pos,        tabH      ] ],
                      [ 'L', [ left_pos + boxW, tabH      ] ],
                      [ 'L', [ left_pos + boxW, lower_pos ] ],
                      [ 'L', [ left_pos + 0,    lower_pos ] ],
                      [ 'Z', [] ]
                    ]
        line_atts = { 'style':line_style, 'id':box_id+'-upper-close-tab', 'd':formatPath(line_path) }
        inkex.etree.SubElement(g, inkex.addNS('path','svg'), line_atts )
        left_pos += boxW

        # Upper Right Tab
        tabH = lower_pos - (boxW/3)
        line_path = [
                      [ 'M', [ left_pos,        tabH      ] ],
                      [ 'L', [ left_pos + boxD, tabH      ] ],
                      [ 'L', [ left_pos + boxD, lower_pos ] ],
                      [ 'L', [ left_pos + 0,    lower_pos ] ],
                      [ 'Z', [] ]
                    ]
        line_atts = { 'style':line_style, 'id':box_id+'-upper-right-tab', 'd':formatPath(line_path) }
        inkex.etree.SubElement(g, inkex.addNS('path','svg'), line_atts )
        left_pos += boxW + boxD

        # Upper Left Tab
        line_path = [
                      [ 'M', [ left_pos,        tabH      ] ],
                      [ 'L', [ left_pos + boxD, tabH      ] ],
                      [ 'L', [ left_pos + boxD, lower_pos ] ],
                      [ 'L', [ left_pos + 0,    lower_pos ] ],
                      [ 'Z', [] ]
                    ]
        line_atts = { 'style':line_style, 'id':box_id+'-upper-left-tab', 'd':formatPath(line_path) }
        inkex.etree.SubElement(g, inkex.addNS('path','svg'), line_atts )
        left_pos = 0

        #self.createGuide( 0, docH-tabH-boxD, 0 );

        # Front
        line_path = [
                      [ 'M', [ left_pos,        lower_pos        ] ],
                      [ 'L', [ left_pos + boxW, lower_pos        ] ],
                      [ 'L', [ left_pos + boxW, lower_pos + boxH ] ],
                      [ 'L', [ left_pos,        lower_pos + boxH ] ],
                      [ 'Z', [] ]
                    ]
        line_atts = { 'style':line_style, 'id':box_id+'-front', 'd':formatPath(line_path) }
        inkex.etree.SubElement(g, inkex.addNS('path','svg'), line_atts )
        left_pos += boxW

        # Right
        line_path = [
                      [ 'M', [ left_pos,        lower_pos        ] ],
                      [ 'L', [ left_pos + boxD, lower_pos        ] ],
                      [ 'L', [ left_pos + boxD, lower_pos + boxH ] ],
                      [ 'L', [ left_pos,        lower_pos + boxH ] ],
                      [ 'Z', [] ]
                    ]
        line_atts = { 'style':line_style, 'id':box_id+'-right', 'd':formatPath(line_path) }
        inkex.etree.SubElement(g, inkex.addNS('path','svg'), line_atts )
        left_pos += boxD

        # Back
        line_path = [
                      [ 'M', [ left_pos,        lower_pos        ] ],
                      [ 'L', [ left_pos + boxW, lower_pos        ] ],
                      [ 'L', [ left_pos + boxW, lower_pos + boxH ] ],
                      [ 'L', [ left_pos,        lower_pos + boxH ] ],
                      [ 'Z', [] ]
                    ]
        line_atts = { 'style':line_style, 'id':box_id+'-back', 'd':formatPath(line_path) }
        inkex.etree.SubElement(g, inkex.addNS('path','svg'), line_atts )
        left_pos += boxW

        # Left
        line_path = [
                      [ 'M', [ left_pos,        lower_pos        ] ],
                      [ 'L', [ left_pos + boxD, lower_pos        ] ],
                      [ 'L', [ left_pos + boxD, lower_pos + boxH ] ],
                      [ 'L', [ left_pos,        lower_pos + boxH ] ],
                      [ 'Z', [] ]
                    ]
        line_atts = { 'style':line_style, 'id':box_id+'-left', 'd':formatPath(line_path) }
        inkex.etree.SubElement(g, inkex.addNS('path','svg'), line_atts )
        left_pos += boxD

        # Left Tab
        line_path = [
                      [ 'M', [ left_pos,            lower_pos        ] ],
                      [ 'L', [ left_pos + (boxD/2), lower_pos        ] ],
                      [ 'L', [ left_pos + (boxD/2), lower_pos + boxH ] ],
                      [ 'L', [ left_pos,            lower_pos + boxH ] ],
                      [ 'Z', [] ]
                    ]
        line_atts = { 'style':line_style, 'id':box_id+'-left-tab', 'd':formatPath(line_path) }
        inkex.etree.SubElement(g, inkex.addNS('path','svg'), line_atts )
        lower_pos += boxH
        left_pos = 0

        # Bottom Front Tab
        line_path = [
                      [ 'M', [ left_pos,        lower_pos              ] ],
                      [ 'L', [ left_pos,        lower_pos + (boxD/1.5) ] ],
                      [ 'L', [ left_pos + boxW, lower_pos + (boxD/1.5) ] ],
                      [ 'L', [ left_pos + boxW, lower_pos              ] ],
                      [ 'Z', [] ]
                    ]
        line_atts = { 'style':line_style, 'id':box_id+'-bottom-front-tab', 'd':formatPath(line_path) }
        inkex.etree.SubElement(g, inkex.addNS('path','svg'), line_atts )
        left_pos += boxW

        # Bottom Right Tab
        line_path = [
                      [ 'M', [ left_pos,        lower_pos              ] ],
                      [ 'L', [ left_pos,        lower_pos + (boxD/1.5) ] ],
                      [ 'L', [ left_pos + boxD, lower_pos + (boxD/1.5) ] ],
                      [ 'L', [ left_pos + boxD, lower_pos              ] ],
                      [ 'Z', [] ]
                    ]
        line_atts = { 'style':line_style, 'id':box_id+'-bottom-right-tab', 'd':formatPath(line_path) }
        inkex.etree.SubElement(g, inkex.addNS('path','svg'), line_atts )
        left_pos += boxD

        # Bottom Back Tab
        line_path = [
                      [ 'M', [ left_pos,        lower_pos              ] ],
                      [ 'L', [ left_pos,        lower_pos + (boxD/1.5) ] ],
                      [ 'L', [ left_pos + boxW, lower_pos + (boxD/1.5) ] ],
                      [ 'L', [ left_pos + boxW, lower_pos              ] ],
                      [ 'Z', [] ]
                    ]
        line_atts = { 'style':line_style, 'id':box_id+'-bottom-back-tab', 'd':formatPath(line_path) }
        inkex.etree.SubElement(g, inkex.addNS('path','svg'), line_atts )
        left_pos += boxW

        # Bottom Left Tab
        line_path = [
                      [ 'M', [ left_pos,        lower_pos              ] ],
                      [ 'L', [ left_pos,        lower_pos + (boxD/1.5) ] ],
                      [ 'L', [ left_pos + boxD, lower_pos + (boxD/1.5) ] ],
                      [ 'L', [ left_pos + boxD, lower_pos              ] ],
                      [ 'Z', [] ]
                    ]
        line_atts = { 'style':line_style, 'id':box_id+'-bottom-left-tab', 'd':formatPath(line_path) }
        inkex.etree.SubElement(g, inkex.addNS('path','svg'), line_atts )
        left_pos += boxD


if __name__ == '__main__':
    e = FoldableBox()
    e.affect()

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 encoding=utf-8 textwidth=99
