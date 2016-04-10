#!/usr/bin/env python 

#Copyright (C) 2010 Jean-Luc JOULIN "JeanJouX" jean-luc.joulin@laposte.net

#This extension allow you to draw a isometric grid with inkscape
#There is some options including subdivision, subsubdivions and custom line width
#All elements are grouped with similar elements
#These grid are used for isometric view in mechanical drawing or piping schematic
#!!! Y Divisions can't be smaller than half the X Divions

#This program is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.

#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with this program; if not, write to the Free Software
#Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


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
        self.OptionParser.add_option("--y_divs",
                        action="store", type="int", 
                        dest="y_divs", default=5,
                        help="Major X Divisions")        
        self.OptionParser.add_option("--dx",
                        action="store", type="float", 
                        dest="dx", default=10.0,
                        help="Major X divison Spacing")
        self.OptionParser.add_option("--subdivs",
                        action="store", type="int", 
                        dest="subdivs", default=2,
                        help="Subdivisions per Major X division")
        self.OptionParser.add_option("--subsubdivs",
                        action="store", type="int", 
                        dest="subsubdivs", default=5,
                        help="Subsubdivisions per Minor X division")
        self.OptionParser.add_option("--divs_th",
                        action="store", type="float", 
                        dest="divs_th", default=2,
                        help="Major X Division Line thickness")
        self.OptionParser.add_option("--subdivs_th",
                        action="store", type="float", 
                        dest="subdivs_th", default=1,
                        help="Minor X Division Line thickness")
        self.OptionParser.add_option("--subsubdivs_th",
                        action="store", type="float", 
                        dest="subsubdivs_th", default=0.3,
                        help="Subminor X Division Line thickness")
        self.OptionParser.add_option("--border_th",
                        action="store", type="float", 
                        dest="border_th", default=3,
                        help="Border Line thickness")

    def effect(self):

        self.options.dx = self.unittouu(str(self.options.dx) + 'px')
        self.options.divs_th = self.unittouu(str(self.options.divs_th) + 'px')
        self.options.subdivs_th = self.unittouu(str(self.options.subdivs_th) + 'px')
        self.options.subsubdivs_th = self.unittouu(str(self.options.subsubdivs_th) + 'px')
        self.options.border_th = self.unittouu(str(self.options.border_th) + 'px')

        #Can't generate a grid too flat
        #If the Y dimension is smallest than half the X dimension, fix it.
        if self.options.y_divs<((self.options.x_divs+1)/2):
            self.options.y_divs=int((self.options.x_divs+1)/2)

        #Find the pixel dimensions of the overall grid
        xmax = self.options.dx * (2*self.options.x_divs) 
        ymax = self.options.dx * (2*self.options.y_divs) / 0.866025

        #Embed grid in group
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
        #Group for major z gridlines
        g_attribs = {inkex.addNS('label','inkscape'):'MajorZGridlines'}
        majglz = inkex.etree.SubElement(grid, 'g', g_attribs)
        #Group for minor x gridlines
        if self.options.subdivs > 1:#if there are any minor x gridlines
            g_attribs = {inkex.addNS('label','inkscape'):'MinorXGridlines'}
            minglx = inkex.etree.SubElement(grid, 'g', g_attribs)
        #Group for subminor x gridlines
        if self.options.subsubdivs > 1:#if there are any minor minor x gridlines
            g_attribs = {inkex.addNS('label','inkscape'):'SubMinorXGridlines'}
            mminglx = inkex.etree.SubElement(grid, 'g', g_attribs)
        #Group for minor y gridlines
        if self.options.subdivs > 1:#if there are any minor y gridlines
            g_attribs = {inkex.addNS('label','inkscape'):'MinorYGridlines'}
            mingly = inkex.etree.SubElement(grid, 'g', g_attribs)
        #Group for subminor y gridlines
        if self.options.subsubdivs > 1:#if there are any minor minor x gridlines
            g_attribs = {inkex.addNS('label','inkscape'):'SubMinorYGridlines'}
            mmingly = inkex.etree.SubElement(grid, 'g', g_attribs)
        #Group for minor z gridlines
        if self.options.subdivs > 1:#if there are any minor y gridlines
            g_attribs = {inkex.addNS('label','inkscape'):'MinorZGridlines'}
            minglz = inkex.etree.SubElement(grid, 'g', g_attribs)
        #Group for subminor z gridlines
        if self.options.subsubdivs > 1:#if there are any minor minor x gridlines
            g_attribs = {inkex.addNS('label','inkscape'):'SubMinorZGridlines'}
            mminglz = inkex.etree.SubElement(grid, 'g', g_attribs)


        draw_SVG_rect(0, 0, xmax, ymax, self.options.border_th,
                      'none', 'Border', grid) #Border of grid

        #X DIVISION
        #Shortcuts for divisions
        sd  = self.options.subdivs   
        ssd = self.options.subsubdivs 
        
        #Initializing variable
        cpt_div=0
        cpt_subdiv=0
        cpt_subsubdiv=0
        com_div=0
        com_subdiv=0
        com_subsubdiv=0

        for i in range(1, (2*self.options.x_divs*sd*ssd)): 
            cpt_subsubdiv=cpt_subsubdiv+1
            com_subsubdiv=1
            if cpt_subsubdiv==self.options.subsubdivs:
                cpt_subsubdiv=0
                cpt_subdiv=cpt_subdiv+1
                com_subsubdiv=0
                com_subdiv=1
                com_div=0

            if cpt_subdiv==self.options.subdivs:
                cpt_subdiv=0
                com_subsubdiv=0
                com_subdiv=0
                com_div=1

            if com_subsubdiv==1:
                draw_SVG_line(self.options.dx*i/sd/ssd, 0,
                              self.options.dx*i/sd/ssd,ymax,
                              self.options.subsubdivs_th,
                              'MajorXDiv'+str(i), mminglx)
            if com_subdiv==1:
                com_subdiv=0
                draw_SVG_line(self.options.dx*i/sd/ssd, 0,
                              self.options.dx*i/sd/ssd,ymax,
                              self.options.subdivs_th,
                              'MajorXDiv'+str(i), minglx)
            if com_div==1:
                com_div=0
                draw_SVG_line(self.options.dx*i/sd/ssd, 0,
                              self.options.dx*i/sd/ssd,ymax,
                              self.options.divs_th,
                              'MajorXDiv'+str(i), majglx)

         
        #Y DIVISONS
        #Shortcuts for divisions
        sd  = self.options.subdivs   
        ssd = self.options.subsubdivs 

        taille=self.options.dx/sd/ssd   #Size of unity
        nb_ligne=(self.options.x_divs+self.options.y_divs)*self.options.subdivs*self.options.subsubdivs  #Global number of lines
        nb_ligne_x=self.options.x_divs*self.options.subdivs*self.options.subsubdivs   #Number of lines X
        nb_ligne_y=self.options.y_divs*self.options.subdivs*self.options.subsubdivs   #Number of lines Y

        #Initializing variable
        cpt_div=0
        cpt_subdiv=0
        cpt_subsubdiv=0
        com_div=0
        com_subdiv=0
        com_subsubdiv=0


        for l in range(1, int(nb_ligne*4)):
            cpt_subsubdiv=cpt_subsubdiv+1
            com_subsubdiv=1
            if cpt_subsubdiv==self.options.subsubdivs:
                cpt_subsubdiv=0
                cpt_subdiv=cpt_subdiv+1
                com_subsubdiv=0
                com_subdiv=1
                com_div=0

            if cpt_subdiv==self.options.subdivs:
                cpt_subdiv=0
                com_subsubdiv=0
                com_subdiv=0
                com_div=1                

            if ((2*l)-1)< (2*nb_ligne_x):
                txa=taille*((2*l)-1)
                tya=ymax
                txb=0
                tyb=ymax-(taille)/(2*0.866025)-(taille*((l-1))/(0.866025))
        
                if com_subsubdiv==1:
                    draw_SVG_line(txa, tya,
                              txb,tyb,
                              self.options.subsubdivs_th,
                              'MajorYDiv'+str(i), mmingly)
                    draw_SVG_line(xmax-txa, tya,
                              xmax-txb,tyb,
                              self.options.subsubdivs_th,
                              'MajorZDiv'+str(l), mminglz) 
                if com_subdiv==1:
                    com_subdiv=0
                    draw_SVG_line(txa, tya,
                              txb,tyb,
                              self.options.subdivs_th,
                              'MajorYDiv'+str(i), mingly)
                    draw_SVG_line(xmax-txa, tya,
                              xmax-txb,tyb,
                              self.options.subdivs_th,
                              'MajorZDiv'+str(l), minglz) 
                if com_div==1:
                    com_div=0
                    draw_SVG_line(txa, tya,
                              txb,tyb,
                              self.options.divs_th,
                              'MajorYDiv'+str(i), majgly)        
                    draw_SVG_line(xmax-txa, tya,
                              xmax-txb,tyb,
                              self.options.divs_th,
                              'MajorZDiv'+str(l), majglz) 

            if ((2*l)-1)==(2*nb_ligne_x):
                txa=taille*((2*l)-1)
                tya=ymax
                txb=0
                tyb=ymax-(taille)/(2*0.866025)-(taille*((l-1))/(0.866025))

                if com_subsubdiv==1:
                    draw_SVG_line(txa, tya,
                              txb,tyb,
                              self.options.subsubdivs_th,
                              'MajorYDiv'+str(i), mmingly)
                    draw_SVG_line(xmax-txa, tya,
                              xmax-txb,tyb,
                              self.options.subsubdivs_th,
                              'MajorZDiv'+str(l), mminglz) 
                if com_subdiv==1:
                    com_subdiv=0
                    draw_SVG_line(txa, tya,
                              txb,tyb,
                              self.options.subdivs_th,
                              'MajorYDiv'+str(i), mingly)
                    draw_SVG_line(xmax-txa, tya,
                              xmax-txb,tyb,
                              self.options.subdivs_th,
                              'MajorZDiv'+str(l), minglz) 
                if com_div==1:
                    com_div=0
                    draw_SVG_line(txa, tya,
                              txb,tyb,
                              self.options.divs_th,
                              'MajorYDiv'+str(i), majgly)        
                    draw_SVG_line(xmax-txa, tya,
                              xmax-txb,tyb,
                              self.options.divs_th,
                              'MajorZDiv'+str(l), majglz) 

            if ((2*l)-1)> (2*nb_ligne_x):
                txa=xmax
                tya=ymax-(taille)/(2*0.866025)-(taille*((l-1-((2*nb_ligne_x)/2)))/(0.866025))
                txb=0
                tyb=ymax-(taille)/(2*0.866025)-(taille*((l-1))/(0.866025))

                if tyb<=0:
                    txa=xmax
                    tya=ymax-(taille)/(2*0.866025)-(taille*((l-1-((2*nb_ligne_x)/2)))/(0.866025))
                    txb=taille*((2*(l-(2*nb_ligne_y))-1))
                    tyb=0
 
                    if txb<xmax:
                        if com_subsubdiv==1:
                                draw_SVG_line(txa, tya,
                                txb,tyb,
                                self.options.subsubdivs_th,
                                'MajorYDiv'+str(i), mmingly)
                                draw_SVG_line(xmax-txa, tya,
                                xmax-txb,tyb,
                                self.options.subsubdivs_th,
                                'MajorZDiv'+str(l), mminglz) 
                        if com_subdiv==1:
                                com_subdiv=0
                                draw_SVG_line(txa, tya,
                                txb,tyb,
                                self.options.subdivs_th,
                                'MajorYDiv'+str(i), mingly)
                                draw_SVG_line(xmax-txa, tya,
                                xmax-txb,tyb,
                                self.options.subdivs_th,
                                'MajorZDiv'+str(l), minglz) 
                        if com_div==1:
                                com_div=0
                                draw_SVG_line(txa, tya,
                                txb,tyb,
                                self.options.divs_th,
                                'MajorYDiv'+str(i), majgly)        
                                draw_SVG_line(xmax-txa, tya,
                                xmax-txb,tyb,
                                self.options.divs_th,
                                'MajorZDiv'+str(l), majglz) 

                else:
                    if txb<xmax:
                        if com_subsubdiv==1:
                            draw_SVG_line(txa, tya,
                                    txb,tyb,
                                    self.options.subsubdivs_th,
                                    'MajorYDiv'+str(i), mmingly)
                            draw_SVG_line(xmax-txa, tya,
                                    xmax-txb,tyb,
                                    self.options.subsubdivs_th,
                                    'MajorZDiv'+str(l), mminglz) 
                        if com_subdiv==1:
                            com_subdiv=0
                            draw_SVG_line(txa, tya,
                                    txb,tyb,
                                    self.options.subdivs_th,
                                    'MajorYDiv'+str(i), mingly)
                            draw_SVG_line(xmax-txa, tya,
                                    xmax-txb,tyb,
                                    self.options.subdivs_th,
                                    'MajorZDiv'+str(l), minglz) 
                        if com_div==1:
                            com_div=0
                            draw_SVG_line(txa, tya,
                                    txb,tyb,
                                    self.options.divs_th,
                                    'MajorYDiv'+str(i), majgly)        
                            draw_SVG_line(xmax-txa, tya,
                                    xmax-txb,tyb,
                                    self.options.divs_th,
                                    'MajorZDiv'+str(l), majglz) 



if __name__ == '__main__':
    e = Grid_Polar()
    e.affect()

#End of file


