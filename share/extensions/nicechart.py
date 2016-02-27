#!/usr/bin/env python
# -*- coding: utf-8 -*-
#  nicechart.py
#
#  Copyright 2011-2016
#  
#  Christoph Sterz 
#  Florian Weber
#  Maren Hachmann
#  
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#  

# TODO / Ideas: 
# allow negative values for bar charts
# show values for stacked bar charts
# don't create a new layer for each chart, but a normal group
# correct bar height for stacked bars (it's only half as high as it should be, double)
# adjust position of heading
# use aliasing workaround for stacked bars (e.g. let the rectangles overlap)

# Example CSV file contents:
'''
Month;1978;1979;1980;1981
January;2;1,3;0.1;2.3
February;6.5;2.4;1.2;6.1
March;7.4;6.7;7.9;4.7
April;7.7;6.4;8.2;8.9
May;10.9;11.7;18.7;11.1
June;12.6;14.2;14.7;14.7
July;16.5;15.5;17.5;15.1
August;15.9;15.4;14.6;16.6
September;14;14.5;13.2;15.3
October;11.9;13.9;11.5;9.2
November;6.7;8.5;7;6.6
December;6.4;2.2;6.3;3.5
'''
# The extension creates one chart for a single value column in one go,
# e.g. chart all temperatures for all months of the year 1978 into one chart.
# (for this, select column 0 for labels and column 1 for values).
# "1978" etc. can be used as heading (Need not be numeric. If not used delete the heading line.)
# Month names can be used as labels
# Values can be shown, in addition to labels (doesn't work with stacked bar charts)
# Values can contain commas as decimal separator, as long as delimiter isn't comma
# Negative values are not yet supported.


import re
import sys
import math
import inkex

from simplestyle import *

#www.sapdesignguild.org/goodies/diagram_guidelines/color_palettes.html#mss
COLOUR_TABLE = {
  "red": ["#460101", "#980101", "#d40000", "#f44800", "#fb8b00", "#eec73e", "#d9bb7a", "#fdd99b"],
  "blue": ["#000442", "#0F1781", "#252FB7", "#3A45E1", "#656DDE", "#8A91EC"],
  "gray": ["#222222", "#444444", "#666666", "#888888", "#aaaaaa", "#cccccc", "#eeeeee"],
  "contrast": ["#0000FF", "#FF0000", "#00FF00", "#CF9100", "#FF00FF", "#00FFFF"],
  "sap": ["#f8d753", "#5c9746", "#3e75a7", "#7a653e", "#e1662a", "#74796f", "#c4384f",
          "#fff8a3", "#a9cc8f", "#b2c8d9", "#bea37a", "#f3aa79", "#b5b5a9", "#e6a5a5"]
}

def get_color_scheme(name="default"):
    return COLOUR_TABLE.get(name.lower(), COLOUR_TABLE['red'])


class NiceChart(inkex.Effect):
    """
    Inkscape extension that can draw pie charts and bar charts 
    (stacked, single, horizontally or vertically) 
    with optional drop shadow, from a csv file or from pasted text
    """
    
    def __init__(self):
        """
        Constructor.
        Defines the "--what" option of a script.
        """
        # Call the base class constructor.
        inkex.Effect.__init__(self)
        
        # Define string option "--what" with "-w" shortcut and default chart values.
        self.OptionParser.add_option('-w', '--what', action='store',
              type='string', dest='what', default='22,11,67',
              help='Chart Values')
            
        # Define string option "--type" with "-t" shortcut.
        self.OptionParser.add_option("-t", "--type", action="store",
              type="string", dest="type", default='',
              help="Chart Type")
        
        # Define bool option "--blur" with "-b" shortcut.
        self.OptionParser.add_option("-b", "--blur", action="store",
              type="inkbool", dest="blur", default='True',
              help="Blur Type")
        
        # Define string option "--file" with "-f" shortcut.
        self.OptionParser.add_option("-f", "--filename", action="store",
              type="string", dest="filename", default='',
              help="Name of File")
        
        # Define string option "--input_type" with "-i" shortcut.
        self.OptionParser.add_option("-i", "--input_type", action="store",
              type="string", dest="input_type", default='file',
              help="Chart Type")
        
        # Define string option "--delimiter" with "-d" shortcut.
        self.OptionParser.add_option("-d", "--delimiter", action="store",
              type="string", dest="csv_delimiter", default=';',
              help="delimiter")
              
        # Define string option "--colors" with "-c" shortcut.
        self.OptionParser.add_option("-c", "--colors", action="store",
              type="string", dest="colors", default='default',
              help="color-scheme")

        # Define string option "--colors_override"
        self.OptionParser.add_option("", "--colors_override", action="store",
              type="string", dest="colors_override", default='',
              help="color-scheme-override")
        

        self.OptionParser.add_option("", "--reverse_colors", action="store",
              type="inkbool", dest="reverse_colors", default='False',
              help="reverse color-scheme")
        
        self.OptionParser.add_option("-k", "--col_key", action="store",
              type="int", dest="col_key", default='0',
              help="column that contains the keys")
        
        
        self.OptionParser.add_option("-v", "--col_val", action="store",
              type="int", dest="col_val", default='1',
              help="column that contains the values")
              
        self.OptionParser.add_option("", "--encoding", action="store",
              type="string", dest="encoding", default='utf-8',
              help="encoding of the CSV file, e.g. utf-8")
        
        self.OptionParser.add_option("", "--headings", action="store",
              type="inkbool", dest="headings", default='False',
              help="the first line of the CSV file consists of headings for the columns")
              
        self.OptionParser.add_option("-r", "--rotate", action="store",
              type="inkbool", dest="rotate", default='False',
              help="Draw barchart horizontally")
            
        self.OptionParser.add_option("-W", "--bar-width", action="store",
            type="int", dest="bar_width", default='10',
            help="width of bars")
        
        self.OptionParser.add_option("-p", "--pie-radius", action="store",
            type="int", dest="pie_radius", default='100',
            help="radius of pie-charts")
            
        self.OptionParser.add_option("-H", "--bar-height", action="store",
            type="int", dest="bar_height", default='100',
            help="height of bars")
            
        self.OptionParser.add_option("-O", "--bar-offset", action="store",
            type="int", dest="bar_offset", default='5',
            help="distance between bars")
            
        self.OptionParser.add_option("", "--stroke-width", action="store",
            type="float", dest="stroke_width", default='1')
            
        self.OptionParser.add_option("-o", "--text-offset", action="store",
            type="int", dest="text_offset", default='5',
            help="distance between bar and descriptions")
        
        self.OptionParser.add_option("", "--heading-offset", action="store",
            type="int", dest="heading_offset", default='50',
            help="distance between chart and chart title")
        
        self.OptionParser.add_option("", "--segment-overlap", action="store",
            type="inkbool", dest="segment_overlap", default='False',
            help="work around aliasing effects by letting pie chart segments overlap")
            
        self.OptionParser.add_option("-F", "--font", action="store",
            type="string", dest="font", default='sans-serif',
            help="font of description")
            
        self.OptionParser.add_option("-S", "--font-size", action="store",
            type="int", dest="font_size", default='10',
            help="font size of description")
        
        self.OptionParser.add_option("-C", "--font-color", action="store",
            type="string", dest="font_color", default='black',
            help="font color of description")
        #Dummy:
        self.OptionParser.add_option("","--input_sections")

        self.OptionParser.add_option("-V", "--show_values", action="store",
            type="inkbool", dest="show_values", default='False',
            help="Show values in chart")
    
    def effect(self):
        """
        Effect behaviour.
        Overrides base class' method and inserts a nice looking chart into SVG document.
        """
        # Get script's "--what" option value and process the data type --- i concess the if term is a little bit of magic
        what = self.options.what
        keys = []
        values = []
        orig_values = []
        keys_present = True
        pie_abs = False
        cnt = 0
        csv_file_name = self.options.filename
        csv_delimiter = self.options.csv_delimiter
        input_type = self.options.input_type
        col_key = self.options.col_key
        col_val = self.options.col_val
        show_values = self.options.show_values
        encoding = self.options.encoding.strip() or 'utf-8'
        headings = self.options.headings
        heading_offset = self.options.heading_offset
        
        if input_type == "\"file\"":
            csv_file = open(csv_file_name, "r")
            
            for linenum, line in enumerate(csv_file):
                value = line.decode(encoding).split(csv_delimiter)
                #make sure that there is at least one value (someone may want to use it as description)
                if len(value) >= 1:
                    # allow to parse headings as strings
                    if linenum == 0 and headings:
                        heading = value[col_val]
                    else:
                        keys.append(value[col_key])
                        # replace comma decimal separator from file by colon, 
                        # to avoid file editing for people whose programs output
                        # values with comma
                        values.append(float(value[col_val].replace(",",".")))
            csv_file.close()
            
        elif input_type == "\"direct_input\"":
            what = re.findall("([A-Z|a-z|0-9]+:[0-9]+\.?[0-9]*)", what)
            for value in what:
                value = value.split(":")
                keys.append(value[0])
                values.append(float(value[1]))

        # warn about negative values (not yet supported)
        for value in values:
            if value < 0:
              inkex.errormsg("Negative values are currently not supported!")
              return

        # Get script's "--type" option value.
        charttype = self.options.type

        if charttype == "pie_abs":
            pie_abs = True
            charttype = "pie"

        # Get access to main SVG document element and get its dimensions.
        svg = self.document.getroot()
        
        # Get the page attibutes:
        width  = self.getUnittouu(svg.get('width'))
        height = self.getUnittouu(svg.attrib['height'])
        
        # Create a new layer.
        layer = inkex.etree.SubElement(svg, 'g')
        layer.set(inkex.addNS('label', 'inkscape'), 'Chart-Layer: %s' % (what))
        layer.set(inkex.addNS('groupmode', 'inkscape'), 'layer')
        
        # Check if a drop shadow should be drawn:
        draw_blur = self.options.blur
        
        if draw_blur:
            # Get defs of Document
            defs = self.xpathSingle('/svg:svg//svg:defs')
            if defs == None:
                defs = inkex.etree.SubElement(self.document.getroot(), inkex.addNS('defs', 'svg'))
            
            # Create new Filter
            filt = inkex.etree.SubElement(defs,inkex.addNS('filter', 'svg'))
            filtId = self.uniqueId('filter')
            self.filtId = 'filter:url(#%s);' % filtId
            for k, v in [('id', filtId), ('height', "3"),
                         ('width', "3"),
                         ('x', '-0.5'), ('y', '-0.5')]:
                filt.set(k, v)
            
            # Append Gaussian Blur to that Filter
            fe = inkex.etree.SubElement(filt, inkex.addNS('feGaussianBlur', 'svg'))
            fe.set('stdDeviation', "1.1")
        
        # Set Default Colors
        self.options.colors_override.strip()
        if len(self.options.colors_override) > 0:
            colors = self.options.colors_override
        else:
            colors = self.options.colors

        if colors[0].isalpha():
            colors = get_color_scheme(colors)
        else:
            colors = re.findall("(#[0-9a-fA-F]{6})", colors)
            #to be sure we create a fallback:
            if len(colors) == 0:
                colors = get_color_scheme()
        
        color_count = len(colors)
        
        if self.options.reverse_colors:
            colors.reverse()
        
        # Those values should be self-explanatory:
        bar_height = self.options.bar_height
        bar_width = self.options.bar_width
        bar_offset = self.options.bar_offset
        # offset of the description in stacked-bar-charts:
        # stacked_bar_text_offset=self.options.stacked_bar_text_offset
        text_offset = self.options.text_offset
        # prevents ugly aliasing effects between pie chart segments by overlapping
        segment_overlap = self.options.segment_overlap
        
        # get font
        font = self.options.font
        font_size = self.options.font_size
        font_color = self.options.font_color
        
        # get rotation
        rotate = self.options.rotate
        
        pie_radius = self.options.pie_radius
        stroke_width = self.options.stroke_width

        if charttype == "bar":
        #########
        ###BAR###
        #########
            
            # iterate all values, use offset to draw the bars in different places
            offset = 0
            color = 0
            
            # Normalize the bars to the largest value
            try:
                value_max = max(values)
            except ValueError:
                value_max = 0.0

            for x in range(len(values)):
                orig_values.append(values[x])
                values[x] = (values[x]/value_max) * bar_height

            # Draw Single bars with their shadows
            for value in values:
                
                # draw drop shadow, if necessary
                if draw_blur:
                    # Create shadow element
                    shadow = inkex.etree.Element(inkex.addNS("rect", "svg"))
                    # Set chart position to center of document. Make it horizontal or vertical
                    if not rotate:
                        shadow.set('x', str(width/2 + offset + 1))
                        shadow.set('y', str(height/2 - int(value) + 1))
                        shadow.set("width", str(bar_width))
                        shadow.set("height", str(int(value)))
                    else:
                        shadow.set('y', str(width/2 + offset + 1))
                        shadow.set('x', str(height/2 + 1))
                        shadow.set("height", str(bar_width))
                        shadow.set("width", str(int(value)))
                        
                    # Set shadow blur (connect to filter object in xml path)
                    shadow.set("style", "filter:url(#filter)")
                
                # Create rectangle element
                rect = inkex.etree.Element(inkex.addNS('rect', 'svg'))
                
                # Set chart position to center of document.
                if not rotate:
                    rect.set('x', str(width/2 + offset))
                    rect.set('y', str(height/2 - int(value)))
                    rect.set("width", str(bar_width))
                    rect.set("height", str(int(value)))
                else:
                    rect.set('y', str(width/2 + offset))
                    rect.set('x', str(height/2))
                    rect.set("height", str(bar_width))
                    rect.set("width", str(int(value)))
                    
                rect.set("style", "fill:" + colors[color % color_count])
                
                # If keys are given, create text elements
                if keys_present:
                    text = inkex.etree.Element(inkex.addNS('text', 'svg'))
                    if not rotate: #=vertical
                        text.set("transform", "matrix(0,-1,1,0,0,0)")
                        #y after rotation:
                        text.set("x", "-" + str(height/2 + text_offset)) 
                        #x after rotation:
                        text.set("y", str(width/2 + offset + bar_width/2 + font_size/3))
                    else: #=horizontal
                        text.set("y", str(width/2 + offset + bar_width/2 + font_size/3))
                        text.set("x", str(height/2 - text_offset))
                    
                    text.set("style", "font-size:" + str(font_size)\
                           + "px;font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-family:"\
                           + font + ";-inkscape-font-specification:Bitstream Charter;text-align:end;text-anchor:end;fill:"\
                           + font_color)

                    text.text = keys[cnt]

                # Increase Offset and Color
                #offset=offset+bar_width+bar_offset
                color = (color + 1) % 8
                # Connect elements together.
                if draw_blur:
                    layer.append(shadow)
                layer.append(rect)
                if keys_present:
                    layer.append(text)
                    
                if show_values:
                    vtext = inkex.etree.Element(inkex.addNS('text', 'svg'))
                    if not rotate: #=vertical
                        vtext.set("transform", "matrix(0,-1,1,0,0,0)")
                        #y after rotation:
                        vtext.set("x", "-"+str(height/2+text_offset-value-text_offset-text_offset)) 
                        #x after rotation:
                        vtext.set("y", str(width/2+offset+bar_width/2+font_size/3))
                    else: #=horizontal
                        vtext.set("y", str(width/2+offset+bar_width/2+font_size/3))
                        vtext.set("x", str(height/2-text_offset+value+text_offset+text_offset))

                    vtext.set("style", "font-size:"+str(font_size)\
                            + "px;font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-family:"\
                            + font + ";-inkscape-font-specification:Bitstream Charter;text-align:start;text-anchor:start;fill:"\
                            + font_color)

                    vtext.text = str(int(orig_values[cnt]))
                    layer.append(vtext)
                
                cnt = cnt+1
                offset = offset + bar_width + bar_offset
            
            # set x position for heading line
            if not rotate:
                heading_x = width/2 # TODO: adjust
            else:
                heading_x = width/2 # TODO: adjust

                
        elif charttype == "pie":
        #########
        ###PIE###
        #########
            # Iterate all values to draw the different slices
            color = 0
            
            # Create the shadow first (if it should be created):
            if draw_blur:
                shadow = inkex.etree.Element(inkex.addNS("circle", "svg"))
                shadow.set('cx', str(width/2))
                shadow.set('cy', str(height/2))
                shadow.set('r', str(pie_radius))
                shadow.set("style", "filter:url(#filter);fill:#000000")
                layer.append(shadow)
            
            
            # Add a grey background circle with a light stroke
            background = inkex.etree.Element(inkex.addNS("circle", "svg"))
            background.set("cx", str(width/2))
            background.set("cy", str(height/2))
            background.set("r", str(pie_radius))
            background.set("style", "stroke:#ececec;fill:#f9f9f9")
            layer.append(background)
            
            #create value sum in order to divide the slices
            try:
                valuesum = sum(values)
                
            except ValueError:
                valuesum = 0

            if pie_abs:
                valuesum = 100

            num_values = len(values)

            # Set an offsetangle
            offset = 0
            
            # Draw single slices
            for i in range(num_values):
                value = values[i]
                # Calculate the PI-angles for start and end
                angle = (2*3.141592) / valuesum * float(value)
                start = offset
                end = offset + angle
                
                # proper overlapping
                if segment_overlap:
                    if i != num_values-1:
                        end += 0.09 # add a 5° overlap
                    if i == 0:
                        start -= 0.09 # let the first element overlap into the other direction

                #then add the slice
                pieslice = inkex.etree.Element(inkex.addNS("path", "svg"))
                pieslice.set(inkex.addNS('type', 'sodipodi'), 'arc')
                pieslice.set(inkex.addNS('cx', 'sodipodi'), str(width/2))
                pieslice.set(inkex.addNS('cy', 'sodipodi'), str(height/2))
                pieslice.set(inkex.addNS('rx', 'sodipodi'), str(pie_radius))
                pieslice.set(inkex.addNS('ry', 'sodipodi'), str(pie_radius))
                pieslice.set(inkex.addNS('start', 'sodipodi'), str(start))
                pieslice.set(inkex.addNS('end', 'sodipodi'), str(end))
                pieslice.set("style", "fill:"+ colors[color % color_count] + ";stroke:none;fill-opacity:1")
                
                #If text is given, draw short paths and add the text
                if keys_present:
                    path = inkex.etree.Element(inkex.addNS("path", "svg"))
                    path.set("d", "m " 
                                + str((width/2) + pie_radius * math.cos(angle/2 + offset)) + "," 
                                + str((height/2) + pie_radius * math.sin(angle/2 + offset)) + " " 
                                + str((text_offset - 2) * math.cos(angle/2 + offset)) + "," 
                                + str((text_offset - 2) * math.sin(angle/2 + offset)))
                    
                    path.set("style", "fill:none;stroke:" 
                                    + font_color + ";stroke-width:" + str(stroke_width) 
                                    + "px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1")
                    layer.append(path)
                    text = inkex.etree.Element(inkex.addNS('text', 'svg'))
                    text.set("x", str((width/2) + (pie_radius + text_offset) * math.cos(angle/2 + offset)))
                    text.set("y", str((height/2) + (pie_radius + text_offset) * math.sin(angle/2 + offset) + font_size/3))
                    textstyle = "font-size:" + str(font_size) \
                                + "px;font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-family:" \
                                + font + ";-inkscape-font-specification:Bitstream Charter;fill:" + font_color 
                    # check if it is right or left of the Pie
                    if math.cos(angle/2 + offset) > 0:
                        text.set("style", textstyle)
                    else:
                        text.set("style", textstyle + ";text-align:end;text-anchor:end")
                    text.text = keys[cnt]
                    if show_values:
                        text.text = text.text + "(" + str(values[cnt])

                        if pie_abs:
                            text.text = text.text + " %"
                        
                        text.text = text.text + ")"
                    
                    cnt = cnt + 1
                    layer.append(text)
                
                # increase the rotation-offset and the colorcycle-position
                offset = offset + angle
                color = (color + 1) % 8
                
                # append the objects to the extension-layer
                layer.append(pieslice)
                
            # set x position for heading line
            heading_x = width/2 - pie_radius # TODO: adjust
                
        elif charttype == "stbar":
        #################
        ###STACKED BAR###
        #################
            # Iterate over all values to draw the different slices
            color = 0
            
            #create value sum in order to divide the bars
            try:
                valuesum = sum(values)
            except ValueError:
                valuesum = 0.0

            for value in values:
                valuesum = valuesum + float(value)
            
            # Init offset
            offset = 0
            
            if draw_blur:
                # Create rectangle element
                shadow = inkex.etree.Element(inkex.addNS("rect", "svg"))
                # Set chart position to center of document.
                if not rotate:
                    shadow.set('x', str(width/2))
                    shadow.set('y', str(height/2 - bar_height/2)) 
                else:
                    shadow.set('x', str(width/2))
                    shadow.set('y', str(height/2))
                # Set rectangle properties
                if not rotate:
                    shadow.set("width", str(bar_width))
                    shadow.set("height", str(bar_height/2))
                else:
                    shadow.set("width",str(bar_height/2))
                    shadow.set("height", str(bar_width))
                # Set shadow blur (connect to filter object in xml path)
                shadow.set("style", "filter:url(#filter)")
                layer.append(shadow)
            
            i = 0
            # Draw Single bars
            for value in values:
                
                # Calculate the individual heights normalized on 100units
                normedvalue = (bar_height / valuesum) * float(value)
                
                # Create rectangle element
                rect = inkex.etree.Element(inkex.addNS('rect', 'svg'))
                
                # Set chart position to center of document.
                if not rotate:
                    rect.set('x', str(width / 2 ))
                    rect.set('y', str(height / 2 - offset - normedvalue))
                else:
                    rect.set('x', str(width / 2 + offset ))
                    rect.set('y', str(height / 2 ))
                # Set rectangle properties
                if not rotate:
                    rect.set("width", str(bar_width))
                    rect.set("height", str(normedvalue))
                else:
                    rect.set("height", str(bar_width))
                    rect.set("width", str(normedvalue))
                rect.set("style", "fill:" + colors[color % color_count])
                
                #If text is given, draw short paths and add the text
                # TODO: apply overlap workaround for visible gaps in between
                if keys_present:
                    if not rotate:
                        path = inkex.etree.Element(inkex.addNS("path", "svg"))
                        path.set("d","m " + str((width + bar_width)/2) + ","
                                    + str(height/2 - offset - (normedvalue / 2)) + " "
                                    + str(bar_width/2 + text_offset) + ",0")
                        path.set("style", "fill:none;stroke:" + font_color
                                        + ";stroke-width:" + str(stroke_width)
                                        + "px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1")
                        layer.append(path)
                        text = inkex.etree.Element(inkex.addNS('text', 'svg'))
                        text.set("x", str(width/2 + bar_width + text_offset + 1))
                        text.set("y", str(height/ 2 - offset + font_size/3 - (normedvalue/2)))
                        text.set("style", "font-size:" + str(font_size)
                                 + "px;font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-family:"
                                 + font + ";-inkscape-font-specification:Bitstream Charter;fill:" + font_color)
                        text.text = keys[cnt]
                        cnt = cnt + 1
                        layer.append(text)
                    else:
                        path = inkex.etree.Element(inkex.addNS("path", "svg"))
                        path.set("d","m " + str((width)/2 + offset + normedvalue/2) + ","
                                    + str(height / 2 + bar_width/2) + " 0," 
                                    + str(bar_width/2 + (font_size * i) + text_offset)) #line
                        path.set("style", "fill:none;stroke:" + font_color
                                 + ";stroke-width:" + str(stroke_width)
                                 + "px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1")
                        layer.append(path)
                        text = inkex.etree.Element(inkex.addNS('text', 'svg'))
                        text.set("x", str((width)/2 + offset + normedvalue/2 - font_size/3))
                        text.set("y", str((height/2) + bar_width + (font_size * (i + 1)) + text_offset))
                        text.set("style", "font-size:" + str(font_size) 
                                        + "px;font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-family:"
                                        + font + ";-inkscape-font-specification:Bitstream Charter;fill:" + font_color)
                        text.text = keys[color]
                        layer.append(text)
                
                # Increase Offset and Color
                offset = offset + normedvalue
                color = (color + 1) % 8
                
                # Draw rectangle
                layer.append(rect)
                i += 1
            
            # set x position for heading line
            if not rotate:
                heading_x = width/2 + offset + normedvalue # TODO: adjust
            else:
                heading_x = width/2 + offset + normedvalue # TODO: adjust
                
        if headings and input_type == "\"file\"":
            headingtext = inkex.etree.Element(inkex.addNS('text', 'svg'))
            headingtext.set("y", str(height/2 + heading_offset))
            headingtext.set("x", str(heading_x))
            headingtext.set("style", "font-size:" + str(font_size + 4)\
                    + "px;font-style:normal;font-variant:normal;font-weight:bold;font-stretch:normal;font-family:"\
                    + font + ";-inkscape-font-specification:Bitstream Charter;text-align:end;text-anchor:end;fill:"\
                    + font_color)

            headingtext.text = heading
            layer.append(headingtext)
    
    def getUnittouu(self, param):
        try:
            return inkex.unittouu(param)
        except AttributeError:
            return self.unittouu(param)

if __name__ == '__main__':
    # Create effect instance and apply it.
    effect = NiceChart()
    effect.affect()
