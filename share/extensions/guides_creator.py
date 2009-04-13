#!/usr/bin/env python
'''
Guides Creator v2.0 (25/11/2008)
http://code.google.com/p/inkscape-guides-creator/

Copyright (C) 2008 Jonas Termeau - jonas.termeau **AT** gmail.com

Thanks to Bernard Gray - bernard.gray **AT** gmail.com

## This basic extension allows you to automatically draw guides in inkscape.

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

# inspired by hello_world turorial by Blackhex and Rubikcube
# (http://wiki.inkscape.org/wiki/index.php/PythonEffectTutorial)

# Making an .INX file : http://wiki.inkscape.org/wiki/index.php/MakingAnINX
# see also http://codespeak.net/lxml/dev/tutorial.html#namespaces for XML namespaces manipulation

# # # # # # # #
# TODO:  See http://code.google.com/p/inkscape-guides-creator/wiki/Roadmap
#
# # # # # # # #
 

# # # extension's begining # # #

# These two lines are only needed if you don't put the script directly into
# the installation directory
import sys
sys.path.append('/usr/share/inkscape/extensions')

# We will use the inkex module with the predefined Effect base class.
import inkex
from simplestyle import *

from xml.etree import ElementTree as ET

# for golden number formulae
from math import sqrt

# for printing debugging output
import gettext
_ = gettext.gettext

def printDebug(string):
        inkex.errormsg(_(string))

def drawVerticalGuides(division,w,h,edges,parent):
        if (division > 0): 
                if (edges):
                        var = 1
                else:
                        var = 0

                for v in range(0,division-1+2*var):
                        # setting up the guide's attributes (id is generated automatically)
                        position = str(round((w / division) + (v - var) * (w / division),4)) + ",0"
                        orientation = str(round(h,4)) + ",0"
                        
                        createGuide(position,orientation,parent)                

def drawHorizontalGuides(division,w,h,edges,parent):
        if (division > 0):
                if (edges):
                        var = 1
                else:
                        var = 0

                for x in range(0,division-1+2*var):
                        # setting up the guide's attributes (id is generated automatically)
                        position = "0," + str(round((h / division) + (x - var) * (h / division),4))
                        orientation = "0," + str(round(w,4))
                        
                        createGuide(position,orientation,parent)

def createGuide(position,orientation,parent):
        # Create a sodipodi:guide node
        # (look into inkex's namespaces to find 'sodipodi' value in order to make a "sodipodi:guide" tag)
        # see NSS array in file inkex.py for the other namespaces
        inkex.etree.SubElement(parent,'{http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd}guide',{'position':position,'orientation':orientation})

def getVerticalDivisionsFromPreset(preset):
        # take a "string1;string2" preset
        # and return "string1"

        str_array = preset.split(';')
        result = int(str_array[0])

        return result

def getHorizontalDivisionsFromPreset(preset):
        # take a "string1;string2" preset
        # and return "string2"
        
        str_array = preset.split(';')
        result = int(str_array[1])

        return result

class Guides_Creator(inkex.Effect):
        
        def __init__(self):
                """
                Constructor.
                Defines options of the script.
                """
                # Call the base class constructor.
                inkex.Effect.__init__(self)

                # Define string option "--preset" with default value 'custom'.
                self.OptionParser.add_option('--preset',
                        action = 'store',type = 'string',
                        dest = 'preset',default = 'custom',
                        help = 'Preset')

                # Define string option "--vertical_guides" with default value '0'.
                self.OptionParser.add_option('--vertical_guides',
                        action = 'store',type = 'string',
                        dest = 'vertical_guides',default = 0,
                        help = 'Vertical guides each:')

                # Define string option "--horizontal_guides" with default value '0'.
                self.OptionParser.add_option('--horizontal_guides',
                        action = 'store',type = 'string',
                        dest = 'horizontal_guides',default = 0,
                        help = 'Horizontal guides each:')

                # Define string option "--start_from_edges" with default value False.
                self.OptionParser.add_option('--start_from_edges',
                        action = 'store',type = 'inkbool',
                        dest = 'start_from_edges',default = False,
                        help = 'Start from edges')

                # Define string option "--delete_existing_guides" with default value False.
                self.OptionParser.add_option('--delete_existing_guides',
                        action = 'store',type = 'inkbool',
                        dest = 'delete_existing_guides',default = False,
                        help = 'Delete existing guides')
        
        def effect(self):

                # Get script's options value.
                from_edges = self.options.start_from_edges
                delete_existing = self.options.delete_existing_guides
                h_division = int(self.options.horizontal_guides)
                v_division = int(self.options.vertical_guides)
                preset = self.options.preset

                # Get access to main SVG document element and get its dimensions.
                svg = self.document.getroot()

                # getting the width and height attributes of the canvas
                width  = inkex.unittouu(svg.get('width'))
                height = inkex.unittouu(svg.attrib['height'])

                # getting the parent tag of the guide
                nv = self.document.xpath('/svg:svg/sodipodi:namedview',namespaces=inkex.NSS)[0]

                if (delete_existing):
                        # getting all the guides
                        children = self.document.xpath('/svg:svg/sodipodi:namedview/sodipodi:guide',namespaces=inkex.NSS)

                        # removing each guides
                        for element in children:
                                nv.remove(element)

                if (preset == 'custom'):

                        # creating vertical guides
                        drawVerticalGuides(v_division,width,height,from_edges,nv)
                                
                        # creating horizontal guides
                        drawHorizontalGuides(h_division,width,height,from_edges,nv)

                elif (preset == 'golden'):

                        gold = (1 + sqrt(5)) / 2
                        
                        # horizontal golden guides
                        position1 = '0,' + str(height / gold)
                        position2 = '0,'+  str(height - (height / gold))
                        h_orientation = '0,' + str(round(width,4))

                        createGuide(position1,h_orientation,nv)
                        createGuide(position2,h_orientation,nv)

                        # vertical golden guides
                        position1 =  str(width / gold) + ',0'
                        position2 = str(width - (width / gold)) + ',0'
                        v_orientation = str(round(height,4)) + ',0'

                        createGuide(position1,v_orientation,nv)
                        createGuide(position2,v_orientation,nv)

                        if (from_edges):
                                # horizontal borders
                                createGuide('0,' + str(height),h_orientation,nv)
                                createGuide(str(height) + ',0',h_orientation,nv)

                                # horizontal borders
                                createGuide('0,' + str(width),v_orientation,nv)
                                createGuide(str(width) + ',0',v_orientation,nv)


                else:

                        v_division = getVerticalDivisionsFromPreset(preset)
                        h_division = getHorizontalDivisionsFromPreset(preset)

                        drawVerticalGuides(v_division,width,height,from_edges,nv)
                        drawHorizontalGuides(h_division,width,height,from_edges,nv)


if __name__ == '__main__':   #pragma: no cover
    # Create effect instance and apply it.
    effect = Guides_Creator()
    effect.affect()

## end of file guide_creator.py ##
