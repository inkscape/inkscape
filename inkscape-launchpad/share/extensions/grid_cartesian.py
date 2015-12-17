#!/usr/bin/env python 
'''
Copyright (C) 2007 John Beard john.j.beard@gmail.com

##This extension allows you to draw a Cartesian grid in Inkscape.
##There is a wide range of options including subdivision, subsubdivions
## and logarithmic scales. Custom line widths are also possible.
##All elements are grouped with similar elements (eg all x-subdivs)

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

import inkex
import simplestyle, sys
from math import *
from simpletransform import computePointInNode

def draw_SVG_line(x1, y1, x2, y2, width, name, parent):
    style = { 'stroke': '#000000', 'stroke-width':str(width), 'fill': 'none' }
    line_attribs = {'style':simplestyle.formatStyle(style),
                    inkex.addNS('label','inkscape'):name,
                    'd':'M '+str(x1)+','+str(y1)+' L '+str(x2)+','+str(y2)}
    inkex.etree.SubElement(parent, inkex.addNS('path','svg'), line_attribs )
    
def draw_SVG_rect(x,y,w,h, width, fill, name, parent):
    style = { 'stroke': '#000000', 'stroke-width':str(width), 'fill':fill}
    rect_attribs = {'style':simplestyle.formatStyle(style),
                    inkex.addNS('label','inkscape'):name,
                    'x':str(x), 'y':str(y), 'width':str(w), 'height':str(h)}
    inkex.etree.SubElement(parent, inkex.addNS('rect','svg'), rect_attribs )

class Grid_Polar(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--x_divs",
                        action="store", type="int", 
                        dest="x_divs", default=5,
                        help="Major X Divisions")
        self.OptionParser.add_option("--dx",
                        action="store", type="float", 
                        dest="dx", default=100.0,
                        help="Major X divison Spacing")
        self.OptionParser.add_option("--x_subdivs",
                        action="store", type="int", 
                        dest="x_subdivs", default=2,
                        help="Subdivisions per Major X division")
        self.OptionParser.add_option("--x_log",
                        action="store", type="inkbool", 
                        dest="x_log", default=False,
                        help="Logarithmic x subdivisions if true")
        self.OptionParser.add_option("--x_subsubdivs",
                        action="store", type="int", 
                        dest="x_subsubdivs", default=5,
                        help="Subsubdivisions per Minor X division")
        self.OptionParser.add_option("--x_half_freq",
                        action="store", type="int", 
                        dest="x_half_freq", default=4,
                        help="Halve Subsubdiv. Frequency after 'n' Subdivs. (log only)")
        self.OptionParser.add_option("--x_divs_th",
                        action="store", type="float", 
                        dest="x_divs_th", default=2,
                        help="Major X Division Line thickness")
        self.OptionParser.add_option("--x_subdivs_th",
                        action="store", type="float", 
                        dest="x_subdivs_th", default=1,
                        help="Minor X Division Line thickness")
        self.OptionParser.add_option("--x_subsubdivs_th",
                        action="store", type="float", 
                        dest="x_subsubdivs_th", default=1,
                        help="Subminor X Division Line thickness")
        self.OptionParser.add_option("--y_divs",
                        action="store", type="int", 
                        dest="y_divs", default=6,
                        help="Major Y Divisions")
        self.OptionParser.add_option("--dy",
                        action="store", type="float", 
                        dest="dy", default=100.0,
                        help="Major Gridline Increment")
        self.OptionParser.add_option("--y_subdivs",
                        action="store", type="int", 
                        dest="y_subdivs", default=2,
                        help="Minor Divisions per Major Y division")
        self.OptionParser.add_option("--y_log",
                        action="store", type="inkbool", 
                        dest="y_log", default=False,
                        help="Logarithmic y subdivisions if true")
        self.OptionParser.add_option("--y_subsubdivs",
                        action="store", type="int", 
                        dest="y_subsubdivs", default=5,
                        help="Subsubdivisions per Minor Y division")
        self.OptionParser.add_option("--y_half_freq",
                        action="store", type="int", 
                        dest="y_half_freq", default=4,
                        help="Halve Y Subsubdiv. Frequency after 'n' Subdivs. (log only)")
        self.OptionParser.add_option("--y_divs_th",
                        action="store", type="float", 
                        dest="y_divs_th", default=2,
                        help="Major Y Division Line thickness")
        self.OptionParser.add_option("--y_subdivs_th",
                        action="store", type="float", 
                        dest="y_subdivs_th", default=1,
                        help="Minor Y Division Line thickness")
        self.OptionParser.add_option("--y_subsubdivs_th",
                        action="store", type="float", 
                        dest="y_subsubdivs_th", default=1,
                        help="Subminor Y Division Line thickness")
        self.OptionParser.add_option("--border_th",
                        action="store", type="float", 
                        dest="border_th", default=3,
                        help="Border Line thickness")


    def effect(self):

        self.options.border_th = self.unittouu(str(self.options.border_th) + 'px')
        self.options.dx = self.unittouu(str(self.options.dx) + 'px')
        self.options.x_divs_th = self.unittouu(str(self.options.x_divs_th) + 'px')
        self.options.x_subdivs_th = self.unittouu(str(self.options.x_subdivs_th) + 'px')
        self.options.x_subsubdivs_th = self.unittouu(str(self.options.x_subsubdivs_th) + 'px')
        self.options.dy = self.unittouu(str(self.options.dy) + 'px')
        self.options.y_divs_th = self.unittouu(str(self.options.y_divs_th) + 'px')
        self.options.y_subdivs_th = self.unittouu(str(self.options.y_subdivs_th) + 'px')
        self.options.y_subsubdivs_th = self.unittouu(str(self.options.y_subsubdivs_th) + 'px')

        #find the pixel dimensions of the overall grid
        ymax = self.options.dy * self.options.y_divs
        xmax = self.options.dx * self.options.x_divs
        
        # Embed grid in group
        #Put in in the centre of the current view
        view_center = computePointInNode(list(self.view_center), self.current_layer)
        t = 'translate(' + str( view_center[0]- xmax/2.0) + ',' + \
                           str( view_center[1]- ymax/2.0) + ')'
        g_attribs = {inkex.addNS('label','inkscape'):'Grid_Polar:X' + \
                     str( self.options.x_divs )+':Y'+str( self.options.y_divs ),
                     'transform':t }
        grid = inkex.etree.SubElement(self.current_layer, 'g', g_attribs)
        
        #Group for major x gridlines
        g_attribs = {inkex.addNS('label','inkscape'):'MajorXGridlines'}
        majglx = inkex.etree.SubElement(grid, 'g', g_attribs)

        #Group for major y gridlines
        g_attribs = {inkex.addNS('label','inkscape'):'MajorYGridlines'}
        majgly = inkex.etree.SubElement(grid, 'g', g_attribs)
        
        #Group for minor x gridlines
        if self.options.x_subdivs > 1:#if there are any minor x gridlines
            g_attribs = {inkex.addNS('label','inkscape'):'MinorXGridlines'}
            minglx = inkex.etree.SubElement(grid, 'g', g_attribs)
        
        #Group for subminor x gridlines
        if self.options.x_subsubdivs > 1:#if there are any minor minor x gridlines
            g_attribs = {inkex.addNS('label','inkscape'):'SubMinorXGridlines'}
            mminglx = inkex.etree.SubElement(grid, 'g', g_attribs)
        
        #Group for minor y gridlines
        if self.options.y_subdivs > 1:#if there are any minor y gridlines
            g_attribs = {inkex.addNS('label','inkscape'):'MinorYGridlines'}
            mingly = inkex.etree.SubElement(grid, 'g', g_attribs)
        
        #Group for subminor y gridlines
        if self.options.y_subsubdivs > 1:#if there are any minor minor x gridlines
            g_attribs = {inkex.addNS('label','inkscape'):'SubMinorYGridlines'}
            mmingly = inkex.etree.SubElement(grid, 'g', g_attribs)

            
        draw_SVG_rect(0, 0, xmax, ymax, self.options.border_th,
                      'none', 'Border', grid) #border rectangle
        
        #DO THE X DIVISONS======================================
        sd  = self.options.x_subdivs #sub divs per div
        ssd = self.options.x_subsubdivs #subsubdivs per subdiv
        
        for i in range(0, self.options.x_divs): #Major x divisons
            if i>0: #dont draw first line (we made a proper border)
                draw_SVG_line(self.options.dx*i, 0,
                              self.options.dx*i,ymax,
                              self.options.x_divs_th,
                              'MajorXDiv'+str(i), majglx)
            
            if self.options.x_log: #log x subdivs
                for j in range (1, sd):
                    if j>1: #the first loop is only for subsubdivs
                        draw_SVG_line(self.options.dx*(i+log(j, sd)), 0,
                                      self.options.dx*(i+log(j, sd)), ymax,
                                      self.options.x_subdivs_th,
                                      'MinorXDiv'+str(i)+':'+str(j), minglx)
                                  
                    for k in range (1, ssd): #subsub divs
                        if (j <= self.options.x_half_freq) or (k%2 == 0):#only draw half the subsubdivs past the half-freq point
                            if (ssd%2 > 0) and (j > self.options.y_half_freq): #half frequency won't work with odd numbers of subsubdivs,
                                ssd2 = ssd+1 #make even
                            else:
                                ssd2 = ssd #no change
                            draw_SVG_line(self.options.dx*(i+log(j+k/float(ssd2),sd )), 0,
                                          self.options.dx*(i+log(j+k/float(ssd2),sd )), ymax,
                                          self.options.x_subsubdivs_th,'SubminorXDiv'+str(i)+':'+str(j)+':'+str(k), mminglx)
            
            else: #linear x subdivs
                for j in range (0, sd):
                    if j>0: #not for the first loop (this loop is for the subsubdivs before the first subdiv)
                        draw_SVG_line(self.options.dx*(i+j/float(sd)), 0,
                                      self.options.dx*(i+j/float(sd)), ymax,
                                      self.options.x_subdivs_th,
                                      'MinorXDiv'+str(i)+':'+str(j), minglx)
                    
                    for k in range (1, ssd): #subsub divs
                        draw_SVG_line(self.options.dx*(i+(j*ssd+k)/((float(sd)*ssd))) , 0,
                                      self.options.dx*(i+(j*ssd+k)/((float(sd)*ssd))) , ymax,
                                      self.options.x_subsubdivs_th,
                                      'SubminorXDiv'+str(i)+':'+str(j)+':'+str(k), mminglx)
         
        #DO THE Y DIVISONS========================================
        sd  = self.options.y_subdivs    #sub divs per div
        ssd = self.options.y_subsubdivs #subsubdivs per subdiv
                                      
        for i in range(0, self.options.y_divs): #Major y divisons
            if i>0:#dont draw first line (we will make a border)
                draw_SVG_line(0, self.options.dy*i,
                              xmax, self.options.dy*i,
                              self.options.y_divs_th,
                              'MajorYDiv'+str(i), majgly)
            
            if self.options.y_log: #log y subdivs
                for j in range (1, sd):
                    if j>1: #the first loop is only for subsubdivs
                        draw_SVG_line(0,    self.options.dy*(i+1-log(j,sd)),
                                      xmax, self.options.dy*(i+1-log(j,sd)),
                                      self.options.y_subdivs_th,
                                      'MinorXDiv'+str(i)+':'+str(j), mingly)
                    
                    for k in range (1, ssd): #subsub divs
                        if (j <= self.options.y_half_freq) or (k%2 == 0):#only draw half the subsubdivs past the half-freq point
                            if (ssd%2 > 0) and (j > self.options.y_half_freq): #half frequency won't work with odd numbers of subsubdivs,
                                ssd2 = ssd+1
                            else:
                                ssd2 = ssd #no change
                            draw_SVG_line(0,    self.options.dx*(i+1-log(j+k/float(ssd2),sd )),
                                          xmax, self.options.dx*(i+1-log(j+k/float(ssd2),sd )),
                                          self.options.y_subsubdivs_th,
                                          'SubminorXDiv'+str(i)+':'+str(j)+':'+str(k), mmingly)
            else: #linear y subdivs
                for j in range (0, self.options.y_subdivs):
                    if j>0:#not for the first loop (this loop is for the subsubdivs before the first subdiv)
                        draw_SVG_line(0,    self.options.dy*(i+j/float(sd)),
                                      xmax, self.options.dy*(i+j/float(sd)),
                                      self.options.y_subdivs_th,
                                      'MinorXYiv'+str(i)+':'+str(j), mingly)
                    
                    for k in range (1, ssd): #subsub divs
                        draw_SVG_line(0,    self.options.dy*(i+(j*ssd+k)/((float(sd)*ssd))),
                                      xmax, self.options.dy*(i+(j*ssd+k)/((float(sd)*ssd))),
                                      self.options.y_subsubdivs_th,
                                      'SubminorXDiv'+str(i)+':'+str(j)+':'+str(k), mmingly)



if __name__ == '__main__':
    e = Grid_Polar()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
