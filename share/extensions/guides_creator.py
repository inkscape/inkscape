#!/usr/bin/env python
'''
Guides Creator v1.0 (21/11/2008)

Copyright (C) 2008 Jonas Termeau jonas.termeau **AT** gmail.com

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
# TODO:  [v2.0]
#        -implement vertical and horizontal golden number proportions (987/610) based guides
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



class Guides_Creator(inkex.Effect):
        
        def __init__(self):
                """
                Constructor.
                Defines options of the script.
                """
                # Call the base class constructor.
                inkex.Effect.__init__(self)
                # Define string option "--vertical_guides" with default value 0.
                self.OptionParser.add_option('--vertical_guides',
                        action = 'store',type = 'string',
                        dest = 'vertical_guides',default = 0,
                        help = 'Vertical guides each:')

                # Define string option "--horizontal_guides" with default value 0.
                self.OptionParser.add_option('--horizontal_guides',
                        action = 'store',type = 'string',
                        dest = 'horizontal_guides',default = 0,
                        help = 'Horizontal guides each:')

                # Define string option "--delete_existing_guides" with default value 0.
                self.OptionParser.add_option('--delete_existing_guides',
                        action = 'store',type = 'inkbool',
                        dest = 'delete_existing_guides',default = False,
                        help = 'Delete existing guides')

                # Define string option "--start_from_edges" with default value 0.
                self.OptionParser.add_option('--start_from_edges',
                        action = 'store',type = 'inkbool',
                        dest = 'start_from_edges',default = False,
                        help = 'Start from edges')
        
        def effect(self):

                # Get script's options value.
                from_edges = self.options.start_from_edges
                delete_existing = self.options.delete_existing_guides
                h_division = int(self.options.horizontal_guides)
                v_division = int(self.options.vertical_guides)

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

                if (from_edges):
                        var = 1
                else:
                        var = 0
                
                # creating vertical guides
                if (v_division > 0):
                        for v in range(0,v_division-1+2*var):
                                # setting up the guide's attributes (id is generated automatically)
                                position = str(round((width / v_division) + (v - var) * (width / v_division),4)) + ",0"
                                orientation = str(round(height,4)) + ",0"
                                
                                # Create a sodipodi:guide node
                                # (look into inkex's namespaces to find 'sodipodi' value in order to make a "sodipodi:guide" tag)
                                # see NSS array in file inkex.py for the other namespaces
                                inkex.etree.SubElement(nv,'{http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd}guide',{'position':position,'orientation':orientation})

                # creating horizontal guides
                if (h_division > 0):
                        for h in range(0,h_division-1+2*var):
                                # setting up the guide's attributes (id is generated automatically)
                                position = "0," + str(round((height / h_division) + (h - var) * (height / h_division),4))
                                orientation = "0," + str(round(width,4))
                                
                                # Create a sodipodi:guide node
                                # (look into inkex's namespaces to find 'sodipodi' value in order to make a "sodipodi:guide" tag)
                                # see NSS array in file inkex.py for the other namespaces
                                inkex.etree.SubElement(nv,'{http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd}guide',{'position':position,'orientation':orientation})

# Create effect instance and apply it.
effect = Guides_Creator()
effect.affect()

## end of file guide_creator.py ##
