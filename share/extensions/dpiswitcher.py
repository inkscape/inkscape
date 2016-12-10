#!/usr/bin/env python 
'''
This extension scale or reduce a document to fit diferent SVG DPI -90/96-

Copyright (C) 2012 Jabiertxo Arraiza, jabier.arraiza@marker.es

Version 0.5 - DPI Switcher

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
import re
import string
from lxml import etree
# local libraries
import inkex


# globals
REFERENCED_CONTAINERS = [
    # These container elements - which may be referenced by other
    # elements - do not need to be scaled directly. The referencing
    # elements will be either insided scaled containers or scaled
    # directly as graphics elements in SVG root.
    'defs',
    'glyph',
    'marker',
    'mask',
    'missing-glyph',
    'pattern',
    'symbol',
]
CONTAINER_ELEMENTS = [
    # These element types have graphics elements and other container
    # elements as child elements. They need to be scaled if in SVG root.
    'a',
    'g',
    'switch',
]
GRAPHICS_ELEMENTS = [
    # These element types cause graphics to be drawn. They need to be
    # scaled if in SVG root.
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
# FIXME: instances and referenced elements
# If for example a referenced element in SVG root is directly scaled,
# and its instance (referencing element e.g. <use>) is inside a scaled
# top-level container, the instance in the end will be rendered at an
# incorrect scale relative to the viewport (page area) and the other
# drawing content. Another unsupported case is both the referenced
# element and the instance in SVG root: the clone will in the end be
# rendered with the scale factor applied twice.


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

    def scaleRoot(self, svg):
        widthNumber = re.sub("[a-zA-Z]", "", svg.get('width'))
        heightNumber = re.sub("[a-zA-Z]", "", svg.get('height'))
        widthDoc = str(float(widthNumber) * self.factor_a * self.unitExponent)
        heightDoc = str(float(heightNumber) * self.factor_a * self.unitExponent)
        if svg.get('viewBox'):
            widthNumber = svg.get('viewBox').split(" ")[2]
            heightNumber = svg.get('viewBox').split(" ")[3]
        if svg.get('height'):
            svg.set('height', heightDoc)
        if svg.get('width'):
            svg.set('width', widthDoc)
        if svg.get('viewBox'):
            svg.set('viewBox',"0 0 " + str(float(widthNumber) * self.factor_a) + " " + str(float(heightNumber) * self.factor_a))
        if self.options.switcher == "1":
            self.scaleGuides(svg)
            self.scaleGrid(svg)
        for element in svg:  # iterate all top-level elements of SVGRoot
            box3DSide = element.get(inkex.addNS('box3dsidetype', 'inkscape'))
            if box3DSide:
                continue
            uri, tag = element.tag.split("}")
            width_scale = self.factor_a
            height_scale = self.factor_a
            if tag in GRAPHICS_ELEMENTS or tag in CONTAINER_ELEMENTS:
                if element.get('width') is not None and \
                (re.sub("[0-9]*\.?[0-9]", "", element.get('width')) == "%" or \
                re.sub("[0-9]*\.?[0-9]", "", element.get('width')) == "px"):
                   width_scale = 1.0;
                if element.get('height') is not None and \
                (re.sub("[0-9]*\.?[0-9]", "", element.get('height')) == "%" or \
                re.sub("[0-9]*\.?[0-9]", "", element.get('height')) == "px"):
                   height_scale = 1.0;
                if element.get('x') is not None and \
                re.sub("[0-9]*\.?[0-9]", "", element.get('x')) == "%":
                    xpos = str(float(element.get('x').replace('%','')) * self.factor_b) + '%'
                    element.set('x', xpos)
                if element.get('y') is not None and \
                re.sub("[0-9]*\.?[0-9]", "", element.get('y')) == "%":
                    ypos = str(float(element.get('y').replace('%','')) * self.factor_b) + '%'
                    element.set('y', ypos)
                if element.get('transform'):
                    if "matrix" in str(element.get('transform')) and width_scale != 1.0:
                        result = re.sub(r".*?matrix( \(|\()(.*?)\)", self.matrixElement, str(element.get('transform')))
                        element.set('transform', result)
                    if "scale" in str(element.get('transform')) and width_scale != 1.0:
                        result = re.sub(r".*?scale( \(|\()(.*?)\)", self.scaleElement, str(element.get('transform')))
                        element.set('transform', result)
                    if "translate" in str(element.get('transform')) and width_scale != 1.0:
                        result = re.sub(r".*?translate( \(|\()(.*?)\)", self.translateElement, str(element.get('transform')))
                        element.set('transform', result)
                    if "skew" in str(element.get('transform')) and width_scale != 1.0:
                        result = re.sub(r".*?skew( \(|\()(.*?)\)", self.skewElement, str(element.get('transform')))
                        element.set('transform', result)
                    if "scale" not in str(element.get('transform')) and "matrix" not in str(element.get('transform')):
                        element.set('transform', str(element.get('transform')) + "scale(" + str( width_scale) + ", " + str(height_scale) + ")")
                else:
                    element.set('transform', "scale(" + str(width_scale) + ", " + str(height_scale) + ")")

    #a dictionary of unit to user unit conversion factors
    __uuconv = {'in':96.0, 'pt':1.33333333333, 'px':1.0, 'mm':3.77952755913, 'cm':37.7952755913,
                'm':3779.52755913, 'km':3779527.55913, 'pc':16.0, 'yd':3456.0 , 'ft':1152.0}
    
    __uuconvLegacy = {'in':90.0, 'pt':1.25, 'px':1, 'mm':3.5433070866, 'cm':35.433070866, 'm':3543.3070866,
                      'km':3543307.0866, 'pc':15.0, 'yd':3240 , 'ft':1080}

    def scaleElement(self, m):
        scaleVal = m.group(2).replace(" ","")
        total = scaleVal.count(',')
        if total == 1:
            scaleVal = scaleVal.split(",")
            return "matrix(" + str(float(scaleVal[0]) * self.factor_a) + ",0,0," + str(float(scaleVal[1]) * self.factor_a) + ",0,0)"
        else:
            return "matrix(" + str(float(scaleVal) * self.factor_a) + ",0,0," + str(float(scaleVal) * self.factor_a) + ",0,0)"


    def translateElement(self, m):
        translateVal = m.group(2).replace(" ","")
        total = translateVal.count(',')
        if total == 1:
            translateVal = translateVal.split(",")
            return "matrix(" + str(self.factor_a) + ",0,0," + str(self.factor_a) + "," + str(float(translateVal[0]) * self.factor_a)  + "," + str(float(translateVal[1]) * self.factor_a) + ")"
        else:
            return "matrix(" + str(self.factor_a) + ",0,0," + str(self.factor_a) + "," + str(float(translateVal) * self.factor_a)  + "," + str(float(translateVal) * self.factor_a) + ")"

    def skewElement(self, m):
        skeweVal = m.group(2).replace(" ","")
        total = skewVal.count(',')
        if total == 1:
            skeweVal = skewVal.split(",")
            return "skew(" + str(float(skewVal[0]) * self.factor_a)  + "," + str(float(skewVal[1]) * self.factor_a) + ") matrix(" + str(self.factor_a) + ",0,0," + str(self.factor_a) + ",0,0)"
        else:
            return "skew(" + str(float(skewVal) * self.factor_a)  + ") matrix(" + str(self.factor_a) + ",0,0," + str(self.factor_a) + ",0,0)"

    def matrixElement(self, m):
        matrixVal = m.group(2).replace(" ","")
        total = matrixVal.count(',')
        matrixVal = matrixVal.split(",")
        if total == 5:
            return "matrix(" + str(float(matrixVal[0]) * self.factor_a) + "," + matrixVal[1] + "," + matrixVal[2] + "," + str(float(matrixVal[3]) * self.factor_a) + "," + str(float(matrixVal[4]) * self.factor_a) + "," + str(float(matrixVal[5]) * self.factor_a) + ")"

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
            self.units = re.sub("[0-9]*\.?[0-9]", "", svg.get('width'))
            if self.units and self.units <> "px" and self.units <> "" and self.units <> "%":
                if self.options.switcher == "0":
                    self.unitExponent = 1.0/(self.factor_a/self.__uuconv[self.units])
                else:
                    self.unitExponent = 1.0/(self.factor_a/self.__uuconvLegacy[self.units])
            self.scaleRoot(svg);
        sys.stdout = saveout

effect = DPISwitcher()
effect.affect()
