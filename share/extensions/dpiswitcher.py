#!/usr/bin/env python 
'''
This extension scale or reduce a document to fit diferent SVG DPI -90/96-

Copyright (C) 2012 Jabiertxo Arraiza, jabier.arraiza@marker.es

Version 0.3 - 96 to 90 DPI

TODO:
Comment Better!!!

CHANGE LOG 
0.1 Start  15/10/2014

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

import inkex, sys, re, string
from lxml import etree

class DPISwitcher(inkex.Effect):

    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--switcher", action="store", 
            type="string", dest="switcher", default="DPI Switch from 90 to 96", 
            help="Select the DPI switch you want")
        self.factor = 0.0;

    def scaleRoot(self, svg):
        widthNumber = re.sub("[a-zA-Z]", "", svg.get('width'))
        heightNumber = re.sub("[a-zA-Z]", "", svg.get('height'))
        if svg.get('viewBox'):
            widthNumber = svg.get('viewBox').split(" ")[2]
            heightNumber = svg.get('viewBox').split(" ")[3]
        widthDoc = str(float(widthNumber) * self.factor)
        heightDoc = str(float(heightNumber) * self.factor)
        if svg.get('height'):
            svg.set('height', heightDoc)
        if svg.get('width'):
            svg.set('width', widthDoc)
        svg.set('viewBox',"0 0 " + widthDoc + " " + heightDoc)
        xpathStr = '//svg:rect | //svg:image | //svg:path | //svg:circle | //svg:ellipse | //svg:text'
        elements = svg.xpath(xpathStr, namespaces=inkex.NSS)
        for element in elements:
            if element.get('transform'):
                if "matrix" in str(element.get('transform')):
                    result = re.sub(r".*?((matrix|MATRIX).*?\))", self.scaleMatrixElement, str(element.get('transform')))
                    element.set('transform', result)
                if "scale" in str(element.get('transform')):
                    result = re.sub(r".*?((scale|SCALE).*?\))", self.scaleElement, str(element.get('transform')))
                    element.set('transform', result)
            else:
                element.set('transform', "scale(" + str(self.factor) + ", " + str(self.factor) + ")")
        self.scaleGuides(svg)
        self.scaleGrid(svg)
    
    def scaleGrid(self, svg):
        xpathStr = '//inkscape:grid'
        grids = svg.xpath(xpathStr, namespaces=inkex.NSS)
        for grid in grids:
            empspacingNumber = float(grid.get("empspacing")) * self.factor
            spacingxNumber = float(grid.get("spacingx").replace("px", "")) * self.factor
            spacingyNumber = float(grid.get("spacingy").replace("px", "")) * self.factor
            originxNumber = float(grid.get("originx").replace("px", "")) * self.factor
            originyNumber = float(grid.get("originy").replace("px", "")) * self.factor
            grid.set("empspacing", str(empspacingNumber))
            grid.set("spacingx", str(spacingxNumber) + "px")
            grid.set("spacingy", str(spacingyNumber) + "px")
            grid.set("originx", str(originxNumber) + "px")
            grid.set("originy", str(originyNumber) + "px")
   
    def scaleGuides(self, svg):
        xpathStr = '//sodipodi:guide'
        guides = svg.xpath(xpathStr, namespaces=inkex.NSS)
        for guide in guides:
            point = string.split(guide.get("position"), ",")
            guide.set("position", str(float(point[0].strip()) * self.factor) + "," + str(float(point[1].strip()) * self.factor))

    #a dictionary of unit to user unit conversion factors
    __uuconv = {'in':96.0, 'pt':1.33333333333, 'px':1.0, 'mm':3.77952755913, 'cm':37.7952755913,
                'm':3779.52755913, 'km':3779527.55913, 'pc':16.0, 'yd':3456.0 , 'ft':1152.0}
    
    __uuconvLegazy = {'in':90.0, 'pt':1.25, 'px':1, 'mm':3.5433070866, 'cm':35.433070866, 'm':3543.3070866,
                      'km':3543307.0866, 'pc':15.0, 'yd':3240 , 'ft':1080}

    def scaleElement(self, m):
      scaleVal = m.group(1).replace("scale","").replace("SCALE","").replace(" ","").replace("(","").replace(")","").split(",")
      return "scale(" + str(float(scaleVal[0]) * self.factor) + "," + str(float(scaleVal[1]) * self.factor) + ")"

    def scaleMatrixElement(self, m):
      scaleMatrixVal = m.group(1).replace("matrix","").replace("MATRIX","").replace(" ","").replace("(","").replace(")","").split(",")
      return "matrix(" + str(float(scaleMatrixVal[0]) * self.factor) + "," + scaleMatrixVal[1] + "," + scaleMatrixVal[2] + "," + str(float(scaleMatrixVal[3]) * self.factor) + "," + scaleMatrixVal[4] + "," + scaleMatrixVal[5] + ")"

    def effect(self):
        self.factor = 90.0/96.0
        if self.options.switcher == "DPI Switch from 90 to 96":
            self.factor = 96.0/90.0
        saveout = sys.stdout
        sys.stdout = sys.stderr
        svg = self.document.getroot()
        namedview = svg.find(inkex.addNS('namedview', 'sodipodi'))
        unit = namedview.get(inkex.addNS('document-units', 'inkscape'))
        unitExponent = 0.0;
        if unit and unit <> "px":
            if self.options.switcher == "DPI Switch from 90 to 96":
                unitExponent = 1.0/(self.factor/self.__uuconv[unit])
            else:
                unitExponent = 1.0/(self.factor/self.__uuconvLegazy[unit])
            self.factor = self.factor * unitExponent
            namedview.set(inkex.addNS('document-units', 'inkscape'), "px")
        self.scaleRoot(svg);
        sys.stdout = saveout

effect = DPISwitcher()
effect.affect()
