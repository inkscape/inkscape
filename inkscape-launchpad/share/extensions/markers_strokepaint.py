#!/usr/bin/env python 
'''
Copyright (C) 2006 Aaron Spike, aaron@ekips.org
Copyright (C) 2010 Nicolas Dufour, nicoduf@yahoo.fr (color options)

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
# standard library
import random
import copy
# local library
import inkex
import simplestyle


class MyEffect(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-m", "--modify",
                        action="store", type="inkbool", 
                        dest="modify", default=False,
                        help="Do not create a copy, modify the markers")
        self.OptionParser.add_option("-t", "--type",
                        action="store", type="string", 
                        dest="fill_type", default="stroke",
                        help="Replace the markers' fill with the object stroke or fill color")
        self.OptionParser.add_option("-a", "--alpha",
                        action="store", type="inkbool", 
                        dest="assign_alpha", default=True,
                        help="Assign the object fill and stroke alpha to the markers")
        self.OptionParser.add_option("-i", "--invert",
                        action="store", type="inkbool", 
                        dest="invert", default=False,
                        help="Invert fill and stroke colors")
        self.OptionParser.add_option("--assign_fill",
                        action="store", type="inkbool", 
                        dest="assign_fill", default=True,
                        help="Assign a fill color to the markers")
        self.OptionParser.add_option("-f", "--fill_color",
                        action="store", type="int", 
                        dest="fill_color", default=1364325887,
                        help="Choose a custom fill color")
        self.OptionParser.add_option("--assign_stroke",
                        action="store", type="inkbool", 
                        dest="assign_stroke", default=True,
                        help="Assign a stroke color to the markers")
        self.OptionParser.add_option("-s", "--stroke_color",
                        action="store", type="int", 
                        dest="stroke_color", default=1364325887,
                        help="Choose a custom fill color")
        self.OptionParser.add_option("--tab",
                        action="store", type="string",
                        dest="tab",
                        help="The selected UI-tab when OK was pressed")
        self.OptionParser.add_option("--colortab",
                        action="store", type="string",
                        dest="colortab",
                        help="The selected cutom color tab when OK was pressed")

    def effect(self):
        defs = self.xpathSingle('/svg:svg//svg:defs')
        if defs == None:
            defs = inkex.etree.SubElement(self.document.getroot(),inkex.addNS('defs','svg'))

        for id, node in self.selected.iteritems():
            mprops = ['marker','marker-start','marker-mid','marker-end']
            try:
                style = simplestyle.parseStyle(node.get('style'))
            except:
                inkex.errormsg(_("No style attribute found for id: %s") % id)
                continue

            # Use object colors
            if self.options.tab == '"object"':
                temp_stroke = style.get('stroke', '#000000')
                temp_fill = style.get('fill', '#000000')
                if (self.options.invert):
                    fill = temp_stroke
                    stroke = temp_fill
                else:
                    fill = temp_fill
                    stroke = temp_stroke
                if (self.options.assign_alpha):
                    temp_stroke_opacity = style.get('stroke-opacity', '1')
                    temp_fill_opacity = style.get('fill-opacity', '1')
                    if (self.options.invert):
                        fill_opacity = temp_stroke_opacity
                        stroke_opacity = temp_fill_opacity
                    else:
                        fill_opacity = temp_fill_opacity
                        stroke_opacity = temp_stroke_opacity
                if (self.options.fill_type == "solid"):
                    fill = stroke
                    if (self.options.assign_alpha):
                        fill_opacity = stroke_opacity
            # Choose custom colors
            elif self.options.tab == '"custom"':
                fill_red = ((self.options.fill_color >> 24) & 255)
                fill_green = ((self.options.fill_color >> 16) & 255)
                fill_blue = ((self.options.fill_color >>  8) & 255)
                fill = "rgb(%s,%s,%s)" % (fill_red, fill_green, fill_blue)
                fill_opacity = (((self.options.fill_color) & 255) / 255.)
                stroke_red = ((self.options.stroke_color >> 24) & 255)
                stroke_green = ((self.options.stroke_color >> 16) & 255)
                stroke_blue = ((self.options.stroke_color >>  8) & 255)
                stroke = "rgb(%s,%s,%s)" % (stroke_red, stroke_green, stroke_blue)
                stroke_opacity = (((self.options.stroke_color) & 255) / 255.)
                if (not(self.options.assign_fill)):
                    fill = "none";
                if (not(self.options.assign_stroke)):
                    stroke = "none";

            for mprop in mprops:
                if style.has_key(mprop) and style[mprop] != 'none'and style[mprop][:5] == 'url(#':
                    marker_id = style[mprop][5:-1]

                    try:
                        old_mnode = self.xpathSingle('/svg:svg//svg:marker[@id="%s"]' % marker_id)
                        if not self.options.modify:
                            mnode = copy.deepcopy(old_mnode)
                        else:
                            mnode = old_mnode
                    except:
                        inkex.errormsg(_("unable to locate marker: %s") % marker_id)
                        continue
                        
                    new_id = self.uniqueId(marker_id, not self.options.modify)
                    
                    style[mprop] = "url(#%s)" % new_id
                    mnode.set('id', new_id)
                    mnode.set(inkex.addNS('stockid','inkscape'), new_id)
                    defs.append(mnode)
                    
                    children = mnode.xpath('.//*[@style]', namespaces=inkex.NSS)
                    for child in children:
                        cstyle = simplestyle.parseStyle(child.get('style'))
                        if (not('stroke' in cstyle  and self.options.tab == '"object"' and cstyle['stroke'] == 'none' and self.options.fill_type == "filled")):
                            cstyle['stroke'] = stroke
                            if 'stroke_opacity' in locals():
                                cstyle['stroke-opacity'] = stroke_opacity
                        if (not('fill' in cstyle and self.options.tab == '"object"' and cstyle['fill'] == 'none' and self.options.fill_type == "solid")):
                            cstyle['fill'] = fill
                            if 'fill_opacity' in locals():
                                cstyle['fill-opacity'] = fill_opacity
                        child.set('style',simplestyle.formatStyle(cstyle))
            node.set('style',simplestyle.formatStyle(style))

if __name__ == '__main__':
    e = MyEffect()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
