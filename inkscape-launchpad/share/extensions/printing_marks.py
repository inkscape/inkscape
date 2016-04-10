#!/usr/bin/env python
'''
This extension allows you to draw crop, registration and other
printing marks in Inkscape.

Authors:
  Nicolas Dufour - Association Inkscape-fr
  Aurelio A. Heckert <aurium(a)gmail.com>

Copyright (C) 2008 Authors

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

from subprocess import Popen, PIPE, STDOUT
import math

import inkex
import simplestyle

class Printing_Marks (inkex.Effect):

    # Default parameters
    stroke_width = 0.25

    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--where",
                                     action="store", type="string",
                                     dest="where_to_crop", default=True,
                                     help="Apply crop marks to...")
        self.OptionParser.add_option("--crop_marks",
                                     action="store", type="inkbool",
                                     dest="crop_marks", default=True,
                                     help="Draw crop Marks?")
        self.OptionParser.add_option("--bleed_marks",
                                     action="store", type="inkbool",
                                     dest="bleed_marks", default=False,
                                     help="Draw Bleed Marks?")
        self.OptionParser.add_option("--registration_marks",
                                     action="store", type="inkbool",
                                     dest="reg_marks", default=False,
                                     help="Draw Registration Marks?")
        self.OptionParser.add_option("--star_target",
                                     action="store", type="inkbool",
                                     dest="star_target", default=False,
                                     help="Draw Star Target?")
        self.OptionParser.add_option("--colour_bars",
                                     action="store", type="inkbool",
                                     dest="colour_bars", default=False,
                                     help="Draw Colour Bars?")
        self.OptionParser.add_option("--page_info",
                                     action="store", type="inkbool",
                                     dest="page_info", default=False,
                                     help="Draw Page Information?")
        self.OptionParser.add_option("--unit",
                                     action="store", type="string",
                                     dest="unit", default="px",
                                     help="Draw measurment")
        self.OptionParser.add_option("--crop_offset",
                                     action="store", type="float",
                                     dest="crop_offset", default=0,
                                     help="Offset")
        self.OptionParser.add_option("--bleed_top",
                                     action="store", type="float",
                                     dest="bleed_top", default=0,
                                     help="Bleed Top Size")
        self.OptionParser.add_option("--bleed_bottom",
                                     action="store", type="float",
                                     dest="bleed_bottom", default=0,
                                     help="Bleed Bottom Size")
        self.OptionParser.add_option("--bleed_left",
                                     action="store", type="float",
                                     dest="bleed_left", default=0,
                                     help="Bleed Left Size")
        self.OptionParser.add_option("--bleed_right",
                                     action="store", type="float",
                                     dest="bleed_right", default=0,
                                     help="Bleed Right Size")
        self.OptionParser.add_option("--tab",
                                     action="store", type="string",
                                     dest="tab",
                                     help="The selected UI-tab when OK was pressed")

    def draw_crop_line(self, x1, y1, x2, y2, name, parent):
        style = { 'stroke': '#000000', 'stroke-width': str(self.stroke_width),
                  'fill': 'none'}
        line_attribs = {'style': simplestyle.formatStyle(style),
                        'id': name,
                        'd': 'M '+str(x1)+','+str(y1)+' L '+str(x2)+','+str(y2)}
        inkex.etree.SubElement(parent, 'path', line_attribs)

    def draw_bleed_line(self, x1, y1, x2, y2, name, parent):
        style = { 'stroke': '#000000', 'stroke-width': str(self.stroke_width),
                  'fill': 'none',
                  'stroke-miterlimit': '4', 'stroke-dasharray': '4, 2, 1, 2',
                  'stroke-dashoffset': '0' }
        line_attribs = {'style': simplestyle.formatStyle(style),
                        'id': name,
                        'd': 'M '+str(x1)+','+str(y1)+' L '+str(x2)+','+str(y2)}
        inkex.etree.SubElement(parent, 'path', line_attribs)

    def draw_reg_circles(self, cx, cy, r, name, colours, parent):
        for i in range(len(colours)):
            style = {'stroke':colours[i], 'stroke-width':str(r / len(colours)),
                     'fill':'none'}
            circle_attribs = {'style':simplestyle.formatStyle(style),
                              inkex.addNS('label','inkscape'):name,
                              'cx':str(cx), 'cy':str(cy),
                              'r':str((r / len(colours)) * (i + 0.5))}
            inkex.etree.SubElement(parent, inkex.addNS('circle','svg'),
                                   circle_attribs)

    def draw_reg_marks(self, cx, cy, rotate, name, parent):
        colours = ['#000000','#00ffff','#ff00ff','#ffff00','#000000']
        g = inkex.etree.SubElement(parent, 'g', { 'id': name })
        for i in range(len(colours)):
            style = {'fill':colours[i], 'fill-opacity':'1', 'stroke':'none'}
            r = (self.mark_size/2)
            step = r
            stroke = r / len(colours)
            regoffset = stroke * i
            regmark_attribs = {'style': simplestyle.formatStyle(style),
                               'd': 'm' +\
                               ' '+str(-regoffset)+','+str(r)  +\
                               ' '+str(-stroke)   +',0'        +\
                               ' '+str(step)      +','+str(-r) +\
                               ' '+str(-step)     +','+str(-r) +\
                               ' '+str(stroke)    +',0'        +\
                               ' '+str(step)      +','+str(r)  +\
                               ' '+str(-step)     +','+str(r)  +\
                               ' z',
                               'transform': 'translate('+str(cx)+','+str(cy)+ \
                                            ') rotate('+str(rotate)+')'}
            inkex.etree.SubElement(g, 'path', regmark_attribs)

    def draw_star_target(self, cx, cy, name, parent):
        r = (self.mark_size/2)
        style = {'fill':'#000 device-cmyk(1,1,1,1)', 'fill-opacity':'1', 'stroke':'none'}
        d = ' M 0,0'
        i = 0
        while i < ( 2 * math.pi ):
            i += math.pi / 16
            d += ' L 0,0 ' +\
                 ' L '+ str(math.sin(i)*r) +','+ str(math.cos(i)*r) +\
                 ' L '+ str(math.sin(i+0.09)*r) +','+ str(math.cos(i+0.09)*r)
        regmark_attribs = {'style':simplestyle.formatStyle(style),
                          inkex.addNS('label','inkscape'):name,
                          'transform':'translate('+str(cx)+','+str(cy)+')',
                          'd':d}
        inkex.etree.SubElement(parent, inkex.addNS('path','svg'),
                               regmark_attribs)

    def draw_coluor_bars(self, cx, cy, rotate, name, parent):
        g = inkex.etree.SubElement(parent, 'g', {
                'id':name,
                'transform':'translate('+str(cx)+','+str(cy)+\
                            ') rotate('+str(rotate)+')' })
        l = min( self.mark_size / 3, max(self.area_w,self.area_h) / 45 )
        for bar in [{'c':'*', 'stroke':'#000', 'x':0,        'y':-(l+1)},
                    {'c':'r', 'stroke':'#0FF', 'x':0,        'y':0},
                    {'c':'g', 'stroke':'#F0F', 'x':(l*11)+1, 'y':-(l+1)},
                    {'c':'b', 'stroke':'#FF0', 'x':(l*11)+1, 'y':0}
                   ]:
            i = 0
            while i <= 1:
                cr = '255'
                cg = '255'
                cb = '255'
                if bar['c'] == 'r' or bar['c'] == '*' : cr = str(255*i)
                if bar['c'] == 'g' or bar['c'] == '*' : cg = str(255*i)
                if bar['c'] == 'b' or bar['c'] == '*' : cb = str(255*i)
                r_att = {'fill':'rgb('+cr+','+cg+','+cb+')',
                         'stroke':bar['stroke'],
                         'stroke-width':'0.5',
                         'x':str((l*i*10)+bar['x']), 'y':str(bar['y']),
                         'width':str(l), 'height':str(l)}
                r = inkex.etree.SubElement(g, 'rect', r_att)
                i += 0.1

    def get_selection_area(self):
        scale = self.unittouu('1px')    # convert to document units
        sel_area = {}
        min_x, min_y, max_x, max_y = False, False, False, False
        for id in self.options.ids:
            sel_area[id] = {}
            for att in [ "x", "y", "width", "height" ]:
                args = [ "inkscape", "-I", id, "--query-"+att, self.svg_file ]
                sel_area[id][att] = scale* \
                    float(Popen(args, stdout=PIPE, stderr=PIPE).communicate()[0])
            current_min_x = sel_area[id]["x"]
            current_min_y = sel_area[id]["y"]
            current_max_x = sel_area[id]["x"] + \
                            sel_area[id]["width"]
            current_max_y = sel_area[id]["y"] + \
                            sel_area[id]["height"]
            if not min_x: min_x = current_min_x
            if not min_y: min_y = current_min_y
            if not max_x: max_x = current_max_x
            if not max_y: max_y = current_max_y
            if current_min_x < min_x: min_x = current_min_x
            if current_min_y < min_y: min_y = current_min_y
            if current_max_x > max_x: max_x = current_max_x
            if current_max_y > max_y: max_y = current_max_y
            #inkex.errormsg( '>> '+ id +
            #                ' min_x:'+ str(min_x) +
            #                ' min_y:'+ str(min_y) +
            #                ' max_x:'+ str(max_x) +
            #                ' max_y:'+ str(max_y) )
        self.area_x1 = min_x
        self.area_y1 = min_y
        self.area_x2 = max_x
        self.area_y2 = max_y
        self.area_w = max_x - min_x
        self.area_h = max_y - min_y

    def effect(self):
        self.mark_size = self.unittouu('1cm')
        self.min_mark_margin = self.unittouu('3mm')

        if self.options.where_to_crop == 'selection' :
            self.get_selection_area()
            #inkex.errormsg('Sory, the crop to selection is a TODO feature')
            #exit(1)
        else :
            svg = self.document.getroot()
            self.area_w  = self.unittouu(svg.get('width'))
            self.area_h  = self.unittouu(svg.attrib['height'])
            self.area_x1 = 0
            self.area_y1 = 0
            self.area_x2 = self.area_w
            self.area_y2 = self.area_h

        # Get SVG document dimensions
        # self.width must be replaced by self.area_x2. same to others.
        svg = self.document.getroot()
        #self.width  = width  = self.unittouu(svg.get('width'))
        #self.height = height = self.unittouu(svg.attrib['height'])

        # Convert parameters to user unit
        offset = self.unittouu(str(self.options.crop_offset) + \
                                self.options.unit)
        bt = self.unittouu(str(self.options.bleed_top)    + self.options.unit)
        bb = self.unittouu(str(self.options.bleed_bottom) + self.options.unit)
        bl = self.unittouu(str(self.options.bleed_left)   + self.options.unit)
        br = self.unittouu(str(self.options.bleed_right)  + self.options.unit)
        # Bleed margin
        if bt < offset : bmt = 0
        else :           bmt = bt - offset
        if bb < offset : bmb = 0
        else :           bmb = bb - offset
        if bl < offset : bml = 0
        else :           bml = bl - offset
        if br < offset : bmr = 0
        else :           bmr = br - offset

        # Define the new document limits
        offset_left   = self.area_x1 - offset
        offset_right  = self.area_x2 + offset
        offset_top    = self.area_y1 - offset
        offset_bottom = self.area_y2 + offset

        # Get middle positions
        middle_vertical   = self.area_y1 + ( self.area_h / 2 )
        middle_horizontal = self.area_x1 + ( self.area_w / 2 )

        # Test if printing-marks layer existis
        layer = self.document.xpath(
                     '//*[@id="printing-marks" and @inkscape:groupmode="layer"]',
                     namespaces=inkex.NSS)
        if layer: svg.remove(layer[0]) # remove if it existis
        # Create a new layer
        layer = inkex.etree.SubElement(svg, 'g')
        layer.set('id', 'printing-marks')
        layer.set(inkex.addNS('label', 'inkscape'), 'Printing Marks')
        layer.set(inkex.addNS('groupmode', 'inkscape'), 'layer')
        layer.set(inkex.addNS('insensitive', 'sodipodi'), 'true')

        # Crop Mark
        if self.options.crop_marks == True:
            # Create a group for Crop Mark
            g_attribs = {inkex.addNS('label','inkscape'):'CropMarks',
                                                    'id':'CropMarks'}
            g_crops = inkex.etree.SubElement(layer, 'g', g_attribs)

            # Top left Mark
            self.draw_crop_line(self.area_x1, offset_top,
                                self.area_x1, offset_top - self.mark_size,
                                'cropTL1', g_crops)
            self.draw_crop_line(offset_left, self.area_y1,
                                offset_left - self.mark_size, self.area_y1,
                                'cropTL2', g_crops)

            # Top right Mark
            self.draw_crop_line(self.area_x2, offset_top,
                                self.area_x2, offset_top - self.mark_size,
                                'cropTR1', g_crops)
            self.draw_crop_line(offset_right, self.area_y1,
                                offset_right + self.mark_size, self.area_y1,
                                'cropTR2', g_crops)

            # Bottom left Mark
            self.draw_crop_line(self.area_x1, offset_bottom,
                                self.area_x1, offset_bottom + self.mark_size,
                                'cropBL1', g_crops)
            self.draw_crop_line(offset_left, self.area_y2,
                                offset_left - self.mark_size, self.area_y2,
                                'cropBL2', g_crops)

            # Bottom right Mark
            self.draw_crop_line(self.area_x2, offset_bottom,
                                self.area_x2, offset_bottom + self.mark_size,
                                'cropBR1', g_crops)
            self.draw_crop_line(offset_right, self.area_y2,
                                offset_right + self.mark_size, self.area_y2,
                                'cropBR2', g_crops)

        # Bleed Mark
        if self.options.bleed_marks == True:
            # Create a group for Bleed Mark
            g_attribs = {inkex.addNS('label','inkscape'):'BleedMarks',
                                                    'id':'BleedMarks'}
            g_bleed = inkex.etree.SubElement(layer, 'g', g_attribs)

            # Top left Mark
            self.draw_bleed_line(self.area_x1 - bl, offset_top - bmt,
                                 self.area_x1 - bl, offset_top - bmt - self.mark_size,
                                 'bleedTL1', g_bleed)
            self.draw_bleed_line(offset_left - bml, self.area_y1 - bt,
                                 offset_left - bml - self.mark_size, self.area_y1 - bt,
                                 'bleedTL2', g_bleed)

            # Top right Mark
            self.draw_bleed_line(self.area_x2 + br, offset_top - bmt,
                                 self.area_x2 + br, offset_top - bmt - self.mark_size,
                                 'bleedTR1', g_bleed)
            self.draw_bleed_line(offset_right + bmr, self.area_y1 - bt,
                                 offset_right + bmr + self.mark_size, self.area_y1 - bt,
                                 'bleedTR2', g_bleed)

            # Bottom left Mark
            self.draw_bleed_line(self.area_x1 - bl, offset_bottom + bmb,
                                 self.area_x1 - bl, offset_bottom + bmb + self.mark_size,
                                 'bleedBL1', g_bleed)
            self.draw_bleed_line(offset_left - bml, self.area_y2 + bb,
                                 offset_left - bml - self.mark_size, self.area_y2 + bb,
                                 'bleedBL2', g_bleed)

            # Bottom right Mark
            self.draw_bleed_line(self.area_x2 + br, offset_bottom + bmb,
                                 self.area_x2 + br, offset_bottom + bmb + self.mark_size,
                                 'bleedBR1', g_bleed)
            self.draw_bleed_line(offset_right + bmr, self.area_y2 + bb,
                                 offset_right + bmr + self.mark_size, self.area_y2 + bb,
                                 'bleedBR2', g_bleed)

        # Registration Mark
        if self.options.reg_marks == True:
            # Create a group for Registration Mark
            g_attribs = {inkex.addNS('label','inkscape'):'RegistrationMarks',
                                                    'id':'RegistrationMarks'}
            g_center = inkex.etree.SubElement(layer, 'g', g_attribs)

            # Left Mark
            cx = max( bml + offset, self.min_mark_margin )
            self.draw_reg_marks(self.area_x1 - cx - (self.mark_size/2),
                                middle_vertical - self.mark_size*1.5,
                                '0', 'regMarkL', g_center)

            # Right Mark
            cx = max( bmr + offset, self.min_mark_margin )
            self.draw_reg_marks(self.area_x2 + cx + (self.mark_size/2),
                                middle_vertical - self.mark_size*1.5,
                                '180', 'regMarkR', g_center)

            # Top Mark
            cy = max( bmt + offset, self.min_mark_margin )
            self.draw_reg_marks(middle_horizontal,
                                self.area_y1 - cy - (self.mark_size/2),
                                '90', 'regMarkT', g_center)

            # Bottom Mark
            cy = max( bmb + offset, self.min_mark_margin )
            self.draw_reg_marks(middle_horizontal,
                                self.area_y2 + cy + (self.mark_size/2),
                                '-90', 'regMarkB', g_center)

        # Star Target
        if self.options.star_target == True:
            # Create a group for Star Target
            g_attribs = {inkex.addNS('label','inkscape'):'StarTarget',
                                                    'id':'StarTarget'}
            g_center = inkex.etree.SubElement(layer, 'g', g_attribs)

            if self.area_h < self.area_w :
                # Left Star
                cx = max( bml + offset, self.min_mark_margin )
                self.draw_star_target(self.area_x1 - cx - (self.mark_size/2),
                                      middle_vertical,
                                      'starTargetL', g_center)
                # Right Star
                cx = max( bmr + offset, self.min_mark_margin )
                self.draw_star_target(self.area_x2 + cx + (self.mark_size/2),
                                      middle_vertical,
                                      'starTargetR', g_center)
            else :
                # Top Star
                cy = max( bmt + offset, self.min_mark_margin )
                self.draw_star_target(middle_horizontal - self.mark_size*1.5,
                                      self.area_y1 - cy - (self.mark_size/2),
                                      'starTargetT', g_center)
                # Bottom Star
                cy = max( bmb + offset, self.min_mark_margin )
                self.draw_star_target(middle_horizontal - self.mark_size*1.5,
                                      self.area_y2 + cy + (self.mark_size/2),
                                      'starTargetB', g_center)


        # Colour Bars
        if self.options.colour_bars == True:
            # Create a group for Colour Bars
            g_attribs = {inkex.addNS('label','inkscape'):'ColourBars',
                                                    'id':'PrintingColourBars'}
            g_center = inkex.etree.SubElement(layer, 'g', g_attribs)

            if self.area_h > self.area_w :
                # Left Bars
                cx = max( bml + offset, self.min_mark_margin )
                self.draw_coluor_bars(self.area_x1 - cx - (self.mark_size/2),
                                      middle_vertical + self.mark_size,
                                      90,
                                      'PrintingColourBarsL', g_center)
                # Right Bars
                cx = max( bmr + offset, self.min_mark_margin )
                self.draw_coluor_bars(self.area_x2 + cx + (self.mark_size/2),
                                      middle_vertical + self.mark_size,
                                      90,
                                      'PrintingColourBarsR', g_center)
            else :
                # Top Bars
                cy = max( bmt + offset, self.min_mark_margin )
                self.draw_coluor_bars(middle_horizontal + self.mark_size,
                                      self.area_y1 - cy - (self.mark_size/2),
                                      0,
                                      'PrintingColourBarsT', g_center)
                # Bottom Bars
                cy = max( bmb + offset, self.min_mark_margin )
                self.draw_coluor_bars(middle_horizontal + self.mark_size,
                                      self.area_y2 + cy + (self.mark_size/2),
                                      0,
                                      'PrintingColourBarsB', g_center)


        # Page Information
        if self.options.page_info == True:
            # Create a group for Page Information
            g_attribs = {inkex.addNS('label','inkscape'):'PageInformation',
                                                    'id':'PageInformation'}
            g_pag_info = inkex.etree.SubElement(layer, 'g', g_attribs)
            y_margin = max( bmb + offset, self.min_mark_margin )
            txt_attribs = {
                    'style': 'font-size:12px;font-style:normal;font-weight:normal;fill:#000000;font-family:Bitstream Vera Sans,sans-serif;text-anchor:middle;text-align:center',
                    'x': str(middle_horizontal),
                    'y': str(self.area_y2+y_margin+self.mark_size+20)
                }
            txt = inkex.etree.SubElement(g_pag_info, 'text', txt_attribs)
            txt.text = 'Page size: ' +\
                       str(round(self.uutounit(self.area_w,self.options.unit),2)) +\
                       'x' +\
                       str(round(self.uutounit(self.area_h,self.options.unit),2)) +\
                       ' ' + self.options.unit


if __name__ == '__main__':
    e = Printing_Marks()
    e.affect()
