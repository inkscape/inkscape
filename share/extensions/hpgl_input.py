#!/usr/bin/env python
# coding=utf-8
'''
Copyright (C) 2013 Sebastian Wüst, sebi@timewaster.de

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

# standard libraries
import sys
from StringIO import StringIO
# local libraries
import hpgl_decoder
import inkex
import sys
inkex.localize()


# parse options
parser = inkex.optparse.OptionParser(usage='usage: %prog [options] HPGLfile', option_class=inkex.InkOption)
parser.add_option('--resolutionX',   action='store', type='float',   dest='resolutionX',   default=1016.0,  help='Resolution X (dpi)')
parser.add_option('--resolutionY',   action='store', type='float',   dest='resolutionY',   default=1016.0,  help='Resolution Y (dpi)')
parser.add_option('--showMovements', action='store', type='inkbool', dest='showMovements', default='FALSE', help='Show Movements between paths')
(options, args) = parser.parse_args(inkex.sys.argv[1:])

# needed to initialize the document
options.docWidth = 210.0 * 3.5433070866 # 210mm to pixels (DIN A4)
options.docHeight = 297.0 * 3.5433070866 # 297mm to pixels (DIN A4)

# read file
fobj = open(args[0], 'r')
hpglString = []
for line in fobj:
    hpglString.append(line.strip())
fobj.close()
# combine all lines
hpglString = ';'.join(hpglString)

# interpret HPGL data
myHpglDecoder = hpgl_decoder.hpglDecoder(hpglString, options)
try:
    doc, warnings = myHpglDecoder.getSvg()
except Exception as inst:
    if inst.args[0] == 'NO_HPGL_DATA':
        # issue error if no hpgl data found
        inkex.errormsg(_("No HPGL data found."))
        exit(1)
    else:
        type, value, traceback = sys.exc_info()
        raise ValueError, ("", type, value), traceback

# issue warning if unknown commands where found
if 'UNKNOWN_COMMANDS' in warnings:
    inkex.errormsg(_("The HPGL data contained unknown (unsupported) commands, there is a possibility that the drawing is missing some content."))

# deliver document to inkscape
doc.write(inkex.sys.stdout)

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99