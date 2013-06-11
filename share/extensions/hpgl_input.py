#!/usr/bin/env python
# coding=utf-8
'''
hpgl_input.py - input a HP Graphics Language file

Copyright (C) 2013 Sebastian Wüst, sebi@timewaster.de, http://www.timewasters-place.com/

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

# standard library
from StringIO import StringIO
# local library
import inkex

inkex.localize()

# parse options
parser = inkex.optparse.OptionParser(usage="usage: %prog [options] HPGLfile", option_class=inkex.InkOption)
parser.add_option("-a", "--resolutionX",
    action="store", type="float", 
    dest="resolutionX", default=1016.0,
    help="Resolution X (dpi)")
parser.add_option("-b", "--resolutionY",
    action="store", type="float", 
    dest="resolutionY", default=1016.0,
    help="Resolution Y (dpi)")
parser.add_option("-c", "--showMovements",
    action="store", type="inkbool", 
    dest="showMovements", default="FALSE",
    help="Show Movements between paths")
(options, args) = parser.parse_args(inkex.sys.argv[1:])

# global vars
scaleX = options.resolutionX / 90.0 # dots/inch to dots/pixels
scaleY = options.resolutionY / 90.0 # dots/inch to dots/pixels
documentWidth = 210.0 * 3.5433070866 # 210mm to pixels
documentHeight = 297.0 * 3.5433070866 # 297mm to pixels
hasUnknownCommands = False

def getData(file):
    # read file (read only one line, there should not be more than one line)
    stream = open(file, 'r')
    data = stream.readline().strip()
    data = data.split(';')
    if len(data) < 2:
        inkex.errormsg(_("No HPGL data found."))
        return False
    if data[-1].strip() == '':
        data.pop()
    return data

def getCoordinates(coord):
    # process coordinates
    (x, y) = coord.split(',')
    x = float(x) / scaleX; # convert to pixels coordinate system
    y = documentHeight - float(y) / scaleY; # convert to pixels coordinate system
    return (x, y)

# prepare document
doc = inkex.etree.parse(StringIO('<svg xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd" width="%s" height="%s"></svg>' % (documentWidth, documentHeight)))
layerDrawing = inkex.etree.SubElement(doc.getroot(), 'g', {inkex.addNS('groupmode','inkscape'):'layer', inkex.addNS('label','inkscape'):'Drawing'})
if options.showMovements:
	layerMovements = inkex.etree.SubElement(doc.getroot(), 'g', {inkex.addNS('groupmode','inkscape'):'layer', inkex.addNS('label','inkscape'):'Movements'})

# load data
data = getData(args[0])
if data != False:
    # parse paths
    oldCoordinates = (0.0, documentHeight) 
    path = ''
    for i, command in enumerate(data):
        if command.strip() != '':
            if command[:2] == 'PU': # if Pen Up command
                if " L" in path:
                    inkex.etree.SubElement(layerDrawing, 'path', {'d':path, 'style':'stroke:#000000; stroke-width:0.2; fill:none;'})
                if options.showMovements and i != len(data) - 1:
                    path = 'M %f,%f' % oldCoordinates
                    path += ' L %f,%f' % getCoordinates(command[2:])
                    inkex.etree.SubElement(layerMovements, 'path', {'d':path, 'style':'stroke:#ff0000; stroke-width:0.2; fill:none;'})
                path = 'M %f,%f' % getCoordinates(command[2:])
            elif command[:2] == 'PD': # if Pen Down command
                path += ' L %f,%f' % getCoordinates(command[2:])
                oldCoordinates = getCoordinates(command[2:])
            elif command[:2] == 'IN': # if Initialize command
                pass
            elif command[:2] == 'SP': # if Select Pen command
                pass
            else:
                hasUnknownCommands = True
    if " L" in path:
        inkex.etree.SubElement(layerDrawing, 'path', {'d':path, 'style':'stroke:#000000; stroke-width:0.2; fill:none;'})

# deliver document to inkscape 
doc.write(inkex.sys.stdout)

# issue warning if unknown commands where found
if hasUnknownCommands:
    inkex.errormsg(_("The HPGL data contained unknown (unsupported) commands, there is a possibility that the drawing is missing some content."))

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
