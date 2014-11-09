#!/usr/bin/env python 
'''
This extension scale or reduce a document to fit diferent SVG DPI -90/96-

Copyright (C) 2012 Jabiertxo Arraiza, jabier.arraiza@marker.es

Version 0.4 - DPI Switcher

TODO:
Comment Better!!!

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
            type="string", dest="switcher", default="0", 
            help="Select the DPI switch you want")
        self.factor = 90.0/96.0
        self.units = "px"
        self.unitExponent = 1.0

    def scaleRoot(self, svg):
        widthNumber = re.sub("[a-zA-Z]", "", svg.get('width'))
        heightNumber = re.sub("[a-zA-Z]", "", svg.get('height'))
        if svg.get('viewBox'):
            widthNumber = svg.get('viewBox').split(" ")[2]
            heightNumber = svg.get('viewBox').split(" ")[3]
        widthDoc = str(float(widthNumber) * self.factor * self.unitExponent)
        heightDoc = str(float(heightNumber) * self.factor * self.unitExponent)
        if svg.get('height'):
            svg.set('height', heightDoc)
        if svg.get('width'):
            svg.set('width', widthDoc)
        svg.set('viewBox',"0 0 " + widthDoc + " " + heightDoc)
        self.scaleGuides(svg)
        self.scaleGrid(svg)
        if self.options.switcher == "1":
            self.factor = self.factor * self.unitExponent
        xpathStr = '//svg:rect | //svg:image | //svg:path | //svg:circle | //svg:ellipse | //svg:text'
        elements = svg.xpath(xpathStr, namespaces=inkex.NSS)
        for element in elements:
            box3DSide = element.get(inkex.addNS('box3dsidetype', 'inkscape'))
            if box3DSide:
                continue
            if element.get('transform'):
                if "matrix" in str(element.get('transform')):
                    result = re.sub(r".*?((matrix).*?\))", self.matrixElement, str(element.get('transform')))
                    element.set('transform', result)
                if "scale" in str(element.get('transform')):
                    result = re.sub(r".*?((scale).*?\))", self.scaleElement, str(element.get('transform')))
                    element.set('transform', result)
                if "translate" in str(element.get('transform')):
                    result = re.sub(r".*?((translate).*?\))", self.translateElement, str(element.get('transform')))
                    element.set('transform', result)
                if "skew" in str(element.get('transform')):
                    result = re.sub(r".*?((translate).*?\))", self.skewElement, str(element.get('transform')))
                    element.set('transform', result)
                if "scale" not in str(element.get('transform')) and "matrix" not in str(element.get('transform')):
                    element.set('transform', str(element.get('transform')) + "scale(" + str(self.factor) + ", " + str(self.factor) + ")")
            else:
                element.set('transform', "scale(" + str(self.factor) + ", " + str(self.factor) + ")")
        xpathStr = '//svg:g'
        elements = svg.xpath(xpathStr, namespaces=inkex.NSS)
        for element in elements:
            if element.get('transform'):
                if "translate" in str(element.get('transform')):
                    result = re.sub(r".*?((translate).*?\))", self.translateElement, str(element.get('transform')))
                    element.set('transform', result)
                if "matrix" in str(element.get('transform')):
                    result = re.sub(r".*?((matrix).*?\))", self.matrixElementNoScale, str(element.get('transform')))
                    element.set('transform', result)
                if "skew" in str(element.get('transform')):
                    result = re.sub(r".*?((translate).*?\))", self.skewElement, str(element.get('transform')))
                    element.set('transform', result)

    def scaleGuides(self, svg):
        xpathStr = '//sodipodi:guide'
        guides = svg.xpath(xpathStr, namespaces=inkex.NSS)
        for guide in guides:
            point = string.split(guide.get("position"), ",")
            guide.set("position", str(float(point[0].strip()) * self.factor ) + "," + str(float(point[1].strip()) * self.factor ))
    
    def scaleGrid(self, svg):
        xpathStr = '//inkscape:grid'
        grids = svg.xpath(xpathStr, namespaces=inkex.NSS)
        for grid in grids:
            if self.options.switcher == "0":
                self.unitExponent = 1.0/(self.factor/self.__uuconv[grid.get("units")])
                self.factor = self.factor * self.unitExponent
            grid.set("units", "px")
            if grid.get("spacingx"):
                spacingx = str(float(re.sub("[a-zA-Z]", "",  grid.get("spacingx"))) * self.factor) + "px"
                grid.set("spacingx", str(spacingx))
            if grid.get("spacingy"):
                spacingy = str(float(re.sub("[a-zA-Z]", "",  grid.get("spacingy"))) * self.factor) + "px"
                grid.set("spacingy", str(spacingy))
            if grid.get("originx"):
                originx = str(float(re.sub("[a-zA-Z]", "",  grid.get("originx"))) * self.factor) + "px"
                grid.set("originx", str(originx))
            if grid.get("originy"):
                originy = str(float(re.sub("[a-zA-Z]", "",  grid.get("originy"))) * self.factor) + "px"
                grid.set("originy", str(originy))

    #a dictionary of unit to user unit conversion factors
    __uuconv = {'in':96.0, 'pt':1.33333333333, 'px':1.0, 'mm':3.77952755913, 'cm':37.7952755913,
                'm':3779.52755913, 'km':3779527.55913, 'pc':16.0, 'yd':3456.0 , 'ft':1152.0}
    
    __uuconvLegazy = {'in':90.0, 'pt':1.25, 'px':1, 'mm':3.5433070866, 'cm':35.433070866, 'm':3543.3070866,
                      'km':3543307.0866, 'pc':15.0, 'yd':3240 , 'ft':1080}

    def scaleElement(self, m):
        scaleVal = m.group(1).replace("scale","").replace(" ","").replace("(","").replace(")","")
        total = scaleVal.count(',')
        if total == 1:
            scaleVal = scaleVal.split(",")
            return "scale(" + str(float(scaleVal[0]) * self.factor) + "," + str(float(scaleVal[1]) * self.factor) + ")"
        else:
            return "scale(" + str(float(scaleVal) * self.factor) + ")"


    def translateElement(self, m):
        translateVal = m.group(1).replace("translate","").replace(" ","").replace("(","").replace(")","")
        total = translateVal.count(',')
        if total == 1:
            translateVal = translateVal.split(",")
            return "translate(" + str(float(translateVal[0]) * self.factor)  + "," + str(float(translateVal[1]) * self.factor) + ")"
        else:
            return "translate(" + str(float(translateVal) * self.factor)  + ")"

    def skewElement(self, m):
        skeweVal = m.group(1).replace("skew","").replace(" ","").replace("(","").replace(")","")
        total = skewVal.count(',')
        if total == 1:
            skeweVal = skewVal.split(",")
            return "skew(" + str(float(skewVal[0]) * self.factor)  + "," + str(float(skewVal[1]) * self.factor) + ")"
        else:
            return "skew(" + str(float(skewVal) * self.factor)  + ")"

    def matrixElement(self, m):
        matrixVal = m.group(1).replace("matrix","").replace(" ","").replace("(","").replace(")","")
        total = matrixVal.count(',')
        matrixVal = matrixVal.split(",")
        if total > 2:
            return "matrix(" + str(float(matrixVal[0]) * self.factor) + "," + str(float(matrixVal[1]) * self.factor) + "," + str(float(matrixVal[2]) * self.factor) + "," + str(float(matrixVal[3]) * self.factor) + "," + str(float(matrixVal[4]) * self.factor) + "," + str(float(matrixVal[5]) * self.factor) + ")"
        else:
            return "matrix(" + str(float(matrixVal[0]) * self.factor) + "," + str(float(matrixVal[1]) * self.factor) + "," + str(float(matrixVal[2]) * self.factor) + ")"

    def matrixElementNoScale(self, m):
        matrixVal = m.group(1).replace("matrix","").replace(" ","").replace("(","").replace(")","")
        total = matrixVal.count(',')
        matrixVal = matrixVal.split(",")
        if total > 2:
            return "matrix(" + matrixVal[0] + "," + str(float(matrixVal[1]) * self.factor) + "," + matrixVal[2] + "," + str(float(matrixVal[3]) * self.factor) + "," + str(float(matrixVal[4]) * self.factor) + "," + str(float(matrixVal[5]) * self.factor) + ")"
        else:
            return "matrix(" + matrixVal[0] + "," + str(float(matrixVal[1]) * self.factor) + "," + str(float(matrixVal[2]) * self.factor) + ")"

    def effect(self):
        if self.options.switcher == "0":
            self.factor = 96.0/90.0
        saveout = sys.stdout
        sys.stdout = sys.stderr
        svg = self.document.getroot()
        namedview = svg.find(inkex.addNS('namedview', 'sodipodi'))
        namedview.set(inkex.addNS('document-units', 'inkscape'), "px")
        self.units = re.sub("[0-9]*\.?[0-9]", "", svg.get('width'))
        if self.units and self.units <> "px":
            if self.options.switcher == "0":
                self.unitExponent = 1.0/(self.factor/self.__uuconv[self.units])
            else:
                self.unitExponent = 1.0/(self.factor/self.__uuconvLegazy[self.units])
        '''
        else:
            self.scaleGuides(svg)
            self.unitExponent = 1.0
            self.scaleGrid(svg)
            sys.stdout = saveout
            return
        '''
        self.scaleRoot(svg);
        sys.stdout = saveout

effect = DPISwitcher()
effect.affect()
