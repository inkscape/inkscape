#
# Copyright (C) 2010 Martin Owens
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
"""
Base module for rendering barcodes for Inkscape.
"""

import itertools
import sys
from lxml import etree

(WHITE_BAR, BLACK_BAR, TALL_BAR) = range(3)
TEXT_TEMPLATE = 'font-size:%dpx;text-align:center;text-anchor:middle;'

class Barcode(object):
    """Provide a base class for all barcode renderers"""
    name = None

    def error(self, bar, msg):
        """Cause an error to be reported"""
        sys.stderr.write(
            "Error encoding '%s' as %s barcode: %s\n" % (bar, self.name, msg))
        return "ERROR"

    def __init__(self, param={}):
        self.document = param.get('document', None)
        self.x        = int(param.get('x', 0))
        self.y        = int(param.get('y', 0))
        self.scale    = param.get('scale', 1)
        self.height   = param.get('height', 30)
        self.label    = param.get('text', None)
        self.string   = self.encode( self.label )

        if not self.string:
            return

        self.width  = len(self.string)
        self.data   = self.graphicalArray(self.string)

    def get_id(self):
        """Get the next useful id"""
        if not self.document:
            return "barcode"
        doc_ids = {}
        docIdNodes = self.document.xpath('//@id')
        for m in docIdNodes:
            doc_ids[m] = 1

        name  = 'barcode'

        index = 0
        while (doc_ids.has_key(name)):
            index += 1
            name = 'barcode%d' % index
        return name

    def generate(self):
        """Generate the actual svg from the coding"""
        svg_uri = u'http://www.w3.org/2000/svg'
        if self.string == 'ERROR':
            return
        if not self.string or not self.data:
            raise ValueError("No string specified for barcode.")

        data = self.data
        name = self.get_id()

        # use an svg group element to contain the barcode
        barcode = etree.Element('{%s}%s' % (svg_uri,'g'))
        barcode.set('id', name)
        barcode.set('style', 'fill: black;')
        barcode.set('transform', 'translate(%d,%d) scale(%f)' % (self.x, self.y, self.scale))

        bar_offset = 0
        bar_id     = 1

        for datum in data:
            # Datum 0 tells us what style of bar is to come next
            style = self.getStyle(int(datum[0]))
            # Datum 1 tells us what width in units,
            # style tells us how wide a unit is
            width = int(datum[1]) * int(style['width'])

            if style['write']:
                rect = etree.SubElement(barcode,'{%s}%s' % (svg_uri,'rect'))
                rect.set('x',      str(bar_offset))
                rect.set('y',      str(style['top']))
                rect.set('width',  str(width))
                rect.set('height', str(style['height']))
                rect.set('id',     "%s_bar%d" % (name, bar_id))
            bar_offset += width
            bar_id += 1

        bar_width = bar_offset
        # Add text at the bottom of the barcode
        text = etree.SubElement(barcode,'{%s}%s' % (svg_uri,'text'))
        text.set( 'x', str(int(bar_width / 2)))
        text.set( 'y', str(self.height + self.fontSize() ))
        text.set( 'style', TEXT_TEMPLATE % self.fontSize() )
        text.set( '{http://www.w3.org/XML/1998/namespace}space', 'preserve' )
        text.set( 'id', '%s_text' % name )
        text.text = str(self.label)
        return barcode

    def graphicalArray(self, code):
        """Converts black and white markets into a space array"""
        return [(x,len(list(y))) for x, y in itertools.groupby(code)]

    def getStyle(self, index):
        """Returns the styles that should be applied to each bar"""
        result = { 'width' : 1, 'top' : 0, 'write' : True }
        if index == BLACK_BAR:
            result['height'] = int(self.height)
        if index == TALL_BAR:
            result['height'] = int(self.height) + int(self.fontSize() / 2)
        if index == WHITE_BAR:
            result['write'] = False
        return result

    def fontSize(self):
        """Return the ideal font size, defaults to 9px"""
        return 9
