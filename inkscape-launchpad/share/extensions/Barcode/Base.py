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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA.
#
"""
Base module for rendering barcodes for Inkscape.
"""

import itertools
import sys
from lxml import etree

(TEXT_POS_BOTTOM, TEXT_POS_TOP) = range(2)
(WHITE_BAR, BLACK_BAR, TALL_BAR) = range(3)
TEXT_TEMPLATE = 'font-size:%dpx;text-align:center;text-anchor:middle;'
SVG_URI = u'http://www.w3.org/2000/svg'

# pylint: disable=abstract-class-not-used
class Barcode(object):
    """Provide a base class for all barcode renderers"""
    default_height = 30
    font_size = 9
    name = None

    def error(self, text, msg):
        """Cause an error to be reported"""
        sys.stderr.write(
            "Error encoding '%s' as %s barcode: %s\n" % (text, self.name, msg))
        return "ERROR"

    def encode(self, text):
        """
        Replace this with the encoding function, it should return
        a string of ones and zeros
        """
        raise NotImplementedError("You need to write an encode() function.")

    def __init__(self, param):
        param = param or {}
        self.document = param.get('document', None)
        self.known_ids = []
        self._extra = []

        self.pos_x = int(param.get('x', 0))
        self.pos_y = int(param.get('y', 0))
        self.text = param.get('text', None)
        self.scale = param.get('scale', 1)
        self.height = param.get('height', self.default_height)
        self.pos_text = param.get('text_pos', TEXT_POS_BOTTOM)

        if self.document:
            self.known_ids = list(self.document.xpath('//@id'))

        if not self.text:
            raise ValueError("No string specified for barcode.")

    def get_id(self, name='element'):
        """Get the next useful id (and claim it)"""
        index = 0
        while name in self.known_ids:
            index += 1
            name = 'barcode%d' % index
        self.known_ids.append(name)
        return name

    def add_extra_barcode(self, barcode, **kw):
        """Add an extra barcode along side this one, used for ean13 extras"""
        from . import getBarcode
        kw['height'] = self.height
        kw['document'] = self.document
        kw['scale'] = None
        self._extra.append(getBarcode(barcode, **kw).generate())

    def generate(self):
        """Generate the actual svg from the coding"""
        string = self.encode(self.text)

        if string == 'ERROR':
            return

        name = self.get_id('barcode')

        # use an svg group element to contain the barcode
        barcode = etree.Element('{%s}g' % SVG_URI)
        barcode.set('id', name)
        barcode.set('style', 'fill: black;')
        if self.scale:
            barcode.set('transform', 'translate(%d,%d) scale(%f)' % (
                self.pos_x, self.pos_y, self.scale))
        else:
            barcode.set('transform', 'translate(%d,%d)' % (
                self.pos_x, self.pos_y))

        bar_id = 1
        bar_offset = 0
        tops = set()

        for datum in self.graphical_array(string):
            # Datum 0 tells us what style of bar is to come next
            style = self.get_style(int(datum[0]))
            # Datum 1 tells us what width in units,
            # style tells us how wide a unit is
            width = int(datum[1]) * int(style['width'])

            if style['write']:
                tops.add(style['top'])
                rect = etree.SubElement(barcode, '{%s}rect' % SVG_URI)
                rect.set('x', str(bar_offset))
                rect.set('y', str(style['top']))
                if self.pos_text == TEXT_POS_TOP:
                    rect.set('y', str(style['top'] + self.font_size))
                rect.set('id', "%s_bar%d" % (name, bar_id))
                rect.set('width', str(width))
                rect.set('height', str(style['height']))
            bar_offset += width
            bar_id += 1

        for extra in self._extra:
            if extra is not None:
                barcode.append(extra)

        bar_width = bar_offset
        # Add text at the bottom of the barcode
        text = etree.SubElement(barcode, '{%s}text' % SVG_URI)
        text.set('x', str(int(bar_width / 2)))
        text.set('y', str(min(tops) + self.font_size - 1))
        if self.pos_text == TEXT_POS_BOTTOM:
            text.set('y', str(self.height + max(tops) + self.font_size))
        text.set('style', TEXT_TEMPLATE % self.font_size)
        text.set('{http://www.w3.org/XML/1998/namespace}space', 'preserve')
        text.set('id', '%s_text' % name)
        text.text = str(self.text)
        return barcode

    def graphical_array(self, code):
        """Converts black and white markets into a space array"""
        return [(x, len(list(y))) for x, y in itertools.groupby(code)]

    def get_style(self, index):
        """Returns the styles that should be applied to each bar"""
        result = {'width' : 1, 'top' : 0, 'write' : True}
        if index == BLACK_BAR:
            result['height'] = int(self.height)
        if index == TALL_BAR:
            result['height'] = int(self.height) + int(self.font_size / 2)
        if index == WHITE_BAR:
            result['write'] = False
        return result

