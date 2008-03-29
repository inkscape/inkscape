'''
Copyright (C) 2007 Martin Owens

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

import itertools
import sys
from lxml import etree

class Barcode:
    def __init__(self, param={}):
        self.document = None
        self.x = 0
        self.y = 0

        if param.has_key('document'):
            self.document = param['document']
        if param.has_key('x'):
            self.x = param['x']
        if param.has_key('y'):
            self.y = param['y']

        if param.has_key('height'):
            self.height = param['height']
        else:
            self.height = 30

        self.text   = param['text']
        self.label  = self.text
        self.string = self.encode( self.text )
        if not self.string:
            return
        self.width  = len(self.string)
        self.data   = self.graphicalArray(self.string)

    def generate(self):
        svg_uri = u'http://www.w3.org/2000/svg'
        if not self.string or not self.data:
            return

        data = self.data;
    
        # create an SVG document if required
        # if not self.document:
        #     self.document = UNKNOWN
    
        if not self.document:
            sys.stderr.write("No document defined to add barcode to\n")
            return
        
        # Collect document ids
        doc_ids = {}
        docIdNodes = self.document.xpath('//@id')
        for m in docIdNodes:
            doc_ids[m] = 1

        # We don't have svg documents so lets do something raw:
        name  = 'barcode'

        # Make sure that the id/name is inique
        index = 0
        while (doc_ids.has_key(name)):
            name = 'barcode' + str(index)
            index = index + 1

        # use an svg group element to contain the barcode
        barcode = etree.Element('{%s}%s' % (svg_uri,'g'))
        barcode.set('id', name)
        barcode.set('style', 'fill: black;')

        draw    = 1
        wOffset = int(self.x)
        id      = 1

        for datum in data:
            # Datum 0 tells us what style of bar is to come next
            style = self.getStyle(int(datum[0]))
            # Datum 1 tells us what width in units,
            # style tells us how wide a unit is
            width = int(datum[1]) * int(style['width'])

            if style['write']:
                # Add height for styles such as EA8 where
                # the barcode goes into the text
                
                rect = etree.SubElement(barcode,'{%s}%s' % (svg_uri,'rect'))
                rect.set('x',      str(wOffset))
                rect.set('y',      str(style['top']))
                rect.set('width',  str(width))
                rect.set('height', str(style['height']))
                rect.set('id', name + '_bar' + str(id))
            wOffset = int(wOffset) + int(width)
            id      = id + 1

        barwidth = wOffset - int(self.x)
        # Add text at the bottom of the barcode
        text = etree.SubElement(barcode,'{%s}%s' % (svg_uri,'text'))
        text.set( 'x', str(int(self.x) + int(barwidth / 2)) )
        text.set( 'y', str(int(self.height) + 10 + int(self.y)) )
        text.set( 'style', 'font-size:' + self.fontSize() + 'px;text-align:center;text-anchor:middle;' )
        text.set( '{http://www.w3.org/XML/1998/namespace}space', 'preserve' )
        text.set( 'id', name + '_bottomtext' )

        text.text = str(self.label)

        return barcode

    # Converts black and white markers into a space array
    def graphicalArray(self, code):
        return [(x,len(list(y))) for x, y in itertools.groupby(code)]

    def getStyle(self, index):
        result = { 'width' : 1, 'top' : int(self.y), 'write' : False }
        if index==1: # Black Bar
            result['height'] = int(self.height)
            result['write']  = True
        return result

    def fontSize(self):
        return '9'
