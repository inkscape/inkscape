#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Hershey Text - renders a line of text using "Hershey" fonts for plotters

Copyright 2011, Windell H. Oskay, www.evilmadscientist.com

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
"""
import hersheydata          #data file w/ Hershey font data
import inkex
import simplestyle
from simpletransform import computePointInNode

Debug = False

def draw_svg_text(char, face, offset, vertoffset, parent):
    style = { 'stroke': '#000000', 'fill': 'none' }
    pathString = face[char]
    splitString = pathString.split()  
    midpoint = offset - int(splitString[0]) 
    pathString = pathString[pathString.find("M"):] #portion after first move
    trans = 'translate(' + str(midpoint) + ',' + str(vertoffset) + ')'
    text_attribs = {'style':simplestyle.formatStyle(style), 'd':pathString, 'transform':trans}
    inkex.etree.SubElement(parent, inkex.addNS('path','svg'), text_attribs) 
    return midpoint + int(splitString[1])   #new offset value


class Hershey( inkex.Effect ):
    def __init__( self ):
        inkex.Effect.__init__( self )
        self.OptionParser.add_option( "--tab",  #NOTE: value is not used.
            action="store", type="string",
            dest="tab", default="splash",
            help="The active tab when Apply was pressed" )
        self.OptionParser.add_option( "--text",
            action="store", type="string", 
            dest="text", default="Hershey Text for Inkscape",
            help="The input text to render")
        self.OptionParser.add_option( "--action",
            action="store", type="string",
            dest="action", default="render",
            help="The active option when Apply was pressed" )
        self.OptionParser.add_option( "--fontface",
            action="store", type="string",
            dest="fontface", default="rowmans",
            help="The selected font face when Apply was pressed" )

    def effect( self ):

        # Embed text in group to make manipulation easier:
        g_attribs = {inkex.addNS('label','inkscape'):'Hershey Text' }
        g = inkex.etree.SubElement(self.current_layer, 'g', g_attribs)

        scale = self.unittouu('1px')    # convert to document units
        font = eval('hersheydata.' + str(self.options.fontface))
        clearfont = hersheydata.futural  
        #Baseline: modernized roman simplex from JHF distribution.
        
        w = 0  #Initial spacing offset
        spacing = 3  # spacing between letters

        if self.options.action == "render":
            #evaluate text string
            letterVals = [ord(q) - 32 for q in self.options.text] 
            for q in letterVals:
                if (q < 0) or (q > 95):
                    w += 2*spacing
                else:
                    w = draw_svg_text(q, font, w, 0, g)
        else:
            #Generate glyph table
            wmax = 0;
            for p in range(0,10):
                w = 0
                v = spacing * (15*p - 67 )
                for q in range(0,10):
                    r = p*10 + q 
                    if (r < 0) or (r > 95):
                        w += 5*spacing
                    else:
                        w = draw_svg_text(r, clearfont, w, v, g)
                        w = draw_svg_text(r, font, w, v, g)
                        w += 5*spacing
                if w > wmax:
                    wmax = w
            w = wmax
            
        #  Translate group to center of view, approximately
        view_center = computePointInNode(list(self.view_center), self.current_layer)
        t = 'translate(' + str( view_center[0] - scale*w/2) + ',' + str( view_center[1] ) + ')'
        if scale != 1:
            t += ' scale(' + str(scale) + ')'
        g.set( 'transform',t)



if __name__ == '__main__':
    e = Hershey()
    e.affect()

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
