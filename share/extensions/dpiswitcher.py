#!/usr/bin/env python 
'''
This extension scales a document to fit different SVG DPI -90/96-

Copyright (C) 2012 Jabiertxo Arraiza, jabier.arraiza@marker.es
Copyright (C) 2016 su_v, <suv-sf@users.sf.net>

Version 0.6 - DPI Switcher

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


Changes since v0.5:
    - transform all top-level containers and graphics elements
    - support scientific notation in SVG lengths
    - fix scaling with existing matrix() (use functions from simpletransform.py)
    - support different units for document width, height attributes
    - improve viewBox support (syntax, offset)
    - support common cases of text-put-on-path in SVG root
    - support common cases of <use> references in SVG root
    - examples from http://tavmjong.free.fr/INKSCAPE/UNITS/ tested

TODO:
    - check grids/guides created with 0.91:
      http://tavmjong.free.fr/INKSCAPE/UNITS/units_mm_nv_90dpi.svg
    - check <symbol> instances
    - check more <use> and text-on-path cases (reverse scaling needed?)
    - scale perspective of 3dboxes

'''
# standard libraries
import sys
import re
import string
import math
from lxml import etree
# local libraries
import inkex
import simpletransform
import simplestyle


# globals
SKIP_CONTAINERS = [
    'defs',
    'glyph',
    'marker',
    'mask',
    'missing-glyph',
    'pattern',
    'symbol',
]
CONTAINER_ELEMENTS = [
    'a',
    'g',
    'switch',
]
GRAPHICS_ELEMENTS = [
    'circle',
    'ellipse',
    'image',
    'line',
    'path',
    'polygon',
    'polyline',
    'rect',
    'text',
    'use',
]

def is_3dbox(element):
    """Check whether element is an Inkscape 3dbox type."""
    return element.get(inkex.addNS('type', 'sodipodi')) == 'inkscape:box3d'


def is_use(element):
    """Check whether element is of type <text>."""
    return element.tag == inkex.addNS('use', 'svg')


def is_text(element):
    """Check whether element is of type <text>."""
    return element.tag == inkex.addNS('text', 'svg')


def is_text_on_path(element):
    """Check whether text element is put on a path."""
    if is_text(element):
        text_path = element.find(inkex.addNS('textPath', 'svg'))
        if text_path is not None and len(text_path):
            return True
    return False


def is_sibling(element1, element2):
    """Check whether element1 and element2 are siblings of same parent."""
    return element2 in element1.getparent()


def is_in_defs(doc, element):
    """Check whether element is in defs."""
    if element is not None:
        defs = doc.find('defs', namespaces=inkex.NSS)
        if defs is not None:
            return linked_node in defs.iterdescendants()
    return False


def get_linked(doc, element):
    """Return linked element or None."""
    if element is not None:
        href = element.get(inkex.addNS('href', 'xlink'), None)
    if href is not None:
        linked_id = href[href.find('#')+1:]
        path = '//*[@id="%s"]' % linked_id
        el_list = doc.xpath(path, namespaces=inkex.NSS)
        if isinstance(el_list, list) and len(el_list):
            return el_list[0]
        else:
            return None


def check_3dbox(svg, element, scale_x, scale_y):
    """Check transformation for 3dbox element."""
    skip = False
    if skip:
        # 3dbox elements ignore preserved transforms
        # FIXME: manually update geometry of 3dbox?
        pass
    return skip


def check_text_on_path(svg, element, scale_x, scale_y):
    """Check whether to skip scaling a text put on a path."""
    skip = False
    path = get_linked(svg, element.find(inkex.addNS('textPath', 'svg')))
    if not is_in_defs(svg, path):
        if is_sibling(element, path):
            # skip common element scaling if both text and path are siblings
            skip = True
            # scale offset
            if 'transform' in element.attrib:
                mat = simpletransform.parseTransform(element.get('transform'))
                mat[0][2] *= scale_x
                mat[1][2] *= scale_y
                element.set('transform', simpletransform.formatTransform(mat))
            # scale font size
            mat = simpletransform.parseTransform(
                'scale({},{})'.format(scale_x, scale_y))
            det = abs(mat[0][0]*mat[1][1] - mat[0][1]*mat[1][0])
            descrim = math.sqrt(abs(det))
            prop = 'font-size'
            # outer text
            sdict = simplestyle.parseStyle(element.get('style'))
            if prop in sdict:
                sdict[prop] = float(sdict[prop]) * descrim
                element.set('style', simplestyle.formatStyle(sdict))
            # inner tspans
            for child in element.iterdescendants():
                if child.tag == inkex.addNS('tspan', 'svg'): 
                    sdict = simplestyle.parseStyle(child.get('style'))
                    if prop in sdict:
                        sdict[prop] = float(sdict[prop]) * descrim
                        child.set('style', simplestyle.formatStyle(sdict))
    return skip


def check_use(svg, element, scale_x, scale_y):
    """Check whether to skip scaling an instanciated element (<use>)."""
    skip = False
    path = get_linked(svg, element)
    if not is_in_defs(svg, path):
        if is_sibling(element, path):
            skip = True
            # scale offset
            if 'transform' in element.attrib:
                mat = simpletransform.parseTransform(element.get('transform'))
                mat[0][2] *= scale_x
                mat[1][2] *= scale_y
                element.set('transform', simpletransform.formatTransform(mat))
    return skip


class DPISwitcher(inkex.Effect):

    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--switcher", action="store", 
            type="string", dest="switcher", default="0", 
            help="Select the DPI switch you want")
        self.OptionParser.add_option("--action", action="store",
            type="string", dest="action",
            default=None, help="")
        self.factor_a = 90.0/96.0
        self.factor_b = 96.0/90.0
        self.units = "px"
        self.unitExponent = 1.0

    # dictionaries of unit to user unit conversion factors
    __uuconvLegacy = {
        'in': 90.0,
        'pt': 1.25,
        'px': 1.0,
        'mm': 3.5433070866,
        'cm': 35.433070866,
        'm': 3543.3070866,
        'km': 3543307.0866,
        'pc': 15.0,
        'yd': 3240.0,
        'ft': 1080.0,
    }
    __uuconv = {
        'in': 96.0,
        'pt': 1.33333333333,
        'px': 1.0,
        'mm': 3.77952755913,
        'cm': 37.7952755913,
        'm': 3779.52755913,
        'km': 3779527.55913,
        'pc': 16.0,
        'yd': 3456.0,
        'ft': 1152.0,
    }

    def parse_length(self, length, percent=False):
        """Parse SVG length."""
        if self.options.switcher == "0":  # dpi90to96
            known_units = self.__uuconvLegacy.keys()
        else:  # dpi96to90
            known_units = self.__uuconv.keys()
        if percent:
            unitmatch = re.compile('(%s)$' % '|'.join(known_units + ['%']))
        else:
            unitmatch = re.compile('(%s)$' % '|'.join(known_units))
        param = re.compile(r'(([-+]?[0-9]+(\.[0-9]*)?|[-+]?\.[0-9]+)([eE][-+]?[0-9]+)?)')
        p = param.match(length)
        u = unitmatch.search(length)
        val = 100       # fallback: assume default length of 100
        unit = 'px'     # fallback: assume 'px' unit
        if p:
            val = float(p.string[p.start():p.end()])
        if u:
            unit = u.string[u.start():u.end()]
        return (val, unit)

    def convert_length(self, val, unit):
        """Convert length to self.units if unit differs."""
        doc_unit = self.units or 'px'
        if unit != doc_unit:
            if self.options.switcher == "0":  # dpi90to96
                val_px = val * self.__uuconvLegacy[unit]
                val = val_px / (self.__uuconvLegacy[doc_unit] / self.__uuconvLegacy['px'])
                unit = doc_unit
            else:  # dpi96to90
                val_px = val * self.__uuconv[unit]
                val = val_px / (self.__uuconv[doc_unit] / self.__uuconv['px']) 
                unit = doc_unit
        return (val, unit)

    def check_attr_unit(self, element, attr, unit_list):
        """Check unit of attribute value, match to units in *unit_list*."""
        if attr in element.attrib:
            unit = self.parse_length(element.get(attr), percent=True)[1]
            return unit in unit_list

    def scale_attr_val(self, element, attr, unit_list, factor):
        """Scale attribute value if unit matches one in *unit_list*."""
        if attr in element.attrib:
            val, unit = self.parse_length(element.get(attr), percent=True)
            if unit in unit_list:
                element.set(attr, '{}{}'.format(val * factor, unit))

    def scaleRoot(self, svg):
        """Scale all top-level elements in SVG root."""

        # update viewport
        widthNumber = self.parse_length(svg.get('width'))[0]
        heightNumber = self.convert_length(*self.parse_length(svg.get('height')))[0]
        widthDoc = widthNumber * self.factor_a * self.unitExponent
        heightDoc = heightNumber * self.factor_a * self.unitExponent

        if svg.get('height'):
            svg.set('height', str(heightDoc))
        if svg.get('width'):
            svg.set('width', str(widthDoc))

        # update viewBox
        if svg.get('viewBox'):
            viewboxstring = re.sub(' +|, +|,',' ', svg.get('viewBox'))
            viewboxlist = [float(i) for i in viewboxstring.strip().split(' ', 4)]
            svg.set('viewBox','{} {} {} {}'.format(*[(val * self.factor_a) for val in viewboxlist]))

        # update guides, grids
        if self.options.switcher == "1":
            # FIXME: dpi96to90 only?
            self.scaleGuides(svg)
            self.scaleGrid(svg)

        for element in svg:  # iterate all top-level elements of SVGRoot

            # init variables
            tag = etree.QName(element).localname
            width_scale = self.factor_a
            height_scale = self.factor_a

            if tag in GRAPHICS_ELEMENTS or tag in CONTAINER_ELEMENTS:

                # test for specific elements to skip from scaling
                if is_3dbox(element):
                    if check_3dbox(svg, element, width_scale, height_scale):
                        continue
                if is_text_on_path(element):
                    if check_text_on_path(svg, element, width_scale, height_scale):
                        continue
                if is_use(element):
                    if check_use(svg, element, width_scale, height_scale):
                        continue

                # relative units ('%') in presentation attributes
                for attr in ['width', 'height']:
                    self.scale_attr_val(element, attr, ['%'], 1.0 / self.factor_a)
                for attr in ['x', 'y']:
                    self.scale_attr_val(element, attr, ['%'], 1.0 / self.factor_a)

                # set preserved transforms on top-level elements
                if width_scale != 1.0 and height_scale != 1.0:
                    mat = simpletransform.parseTransform(
                        'scale({},{})'.format(width_scale, height_scale))
                    simpletransform.applyTransformToNode(mat, element)

    def scaleElement(self, m):
        pass  # TODO: optionally scale graphics elements only?

    def scaleGuides(self, svg):
        xpathStr = '//sodipodi:guide'
        guides = svg.xpath(xpathStr, namespaces=inkex.NSS)
        for guide in guides:
            point = string.split(guide.get("position"), ",")
            guide.set("position", str(float(point[0].strip()) * self.factor_a ) + "," + str(float(point[1].strip()) * self.factor_a ))
    
    def scaleGrid(self, svg):
        xpathStr = '//inkscape:grid'
        grids = svg.xpath(xpathStr, namespaces=inkex.NSS)
        for grid in grids:
            grid.set("units", "px")
            if grid.get("spacingx"):
                spacingx = str(float(re.sub("[a-zA-Z]", "",  grid.get("spacingx"))) * self.factor_a) + "px"
                grid.set("spacingx", str(spacingx))
            if grid.get("spacingy"):
                spacingy = str(float(re.sub("[a-zA-Z]", "",  grid.get("spacingy"))) * self.factor_a) + "px"
                grid.set("spacingy", str(spacingy))
            if grid.get("originx"):
                originx = str(float(re.sub("[a-zA-Z]", "",  grid.get("originx"))) * self.factor_a) + "px"
                grid.set("originx", str(originx))
            if grid.get("originy"):
                originy = str(float(re.sub("[a-zA-Z]", "",  grid.get("originy"))) * self.factor_a) + "px"
                grid.set("originy", str(originy))

    def effect(self):
        saveout = sys.stdout
        sys.stdout = sys.stderr
        svg = self.document.getroot()
        if self.options.action == '"page_info"':
            print ":::SVG document related info:::"
            print "version: " + str(svg.get(inkex.addNS('version',u'inkscape')))
            width = svg.get('width')
            if width:
                print "width: " + width
            height = svg.get('height')
            if height:
                print "height: " + height
            viewBox = svg.get('viewBox')
            if viewBox:
                print "viewBox: " + viewBox
            namedview = svg.find(inkex.addNS('namedview', 'sodipodi'))
            docunits= namedview.get(inkex.addNS('document-units', 'inkscape'))
            if docunits:
                print "document-units: " + docunits
            units = namedview.get('units')
            if units:
                print "units: " + units
            xpathStr = '//sodipodi:guide'
            guides = svg.xpath(xpathStr, namespaces=inkex.NSS)
            xpathStr = '//inkscape:grid'
            if guides:
                numberGuides = len(guides)
                print "Document has " + str(numberGuides) + " guides"
            grids = svg.xpath(xpathStr, namespaces=inkex.NSS)
            i = 1
            for grid in grids:
                print "Grid number " + str(i) + ": Units: " + grid.get("units")
                i = i+1
        else:
            if self.options.switcher == "0":
                self.factor_a = 96.0/90.0
                self.factor_b = 90.0/96.0
            namedview = svg.find(inkex.addNS('namedview', 'sodipodi'))
            namedview.set(inkex.addNS('document-units', 'inkscape'), "px")
            self.units = self.parse_length(svg.get('width'))[1]
            if self.units and self.units <> "px" and self.units <> "" and self.units <> "%":
                if self.options.switcher == "0":
                    self.unitExponent = 1.0/(self.factor_a/self.__uuconv[self.units])
                else:
                    self.unitExponent = 1.0/(self.factor_a/self.__uuconvLegacy[self.units])
            self.scaleRoot(svg);
        sys.stdout = saveout


if __name__ == '__main__':
    effect = DPISwitcher()
    effect.affect()

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
