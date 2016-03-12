#!/usr/bin/env python

# Written by Jabiertxof
# V.06

import inkex, sys, re, os
from lxml import etree

class C(inkex.Effect):
  def __init__(self):
    inkex.Effect.__init__(self)
    self.OptionParser.add_option("-w", "--width",  action="store", type="int",    dest="desktop_width",  default="100", help="Custom width")
    self.OptionParser.add_option("-z", "--height", action="store", type="int",    dest="desktop_height", default="100", help="Custom height")

  def effect(self):
    saveout = sys.stdout
    sys.stdout = sys.stderr
    width  = self.options.desktop_width
    height = self.options.desktop_height
    if height == 0 | width == 0:
        return;
    factor = float(width)/float(height)
    path = os.path.dirname(os.path.realpath(__file__))
    self.document = etree.parse(os.path.join(path, "seamless_pattern.svg"))
    root = self.document.getroot()
    root.set("id", "SVGRoot")
    root.set("width",  str(width))
    root.set("height", str(height))
    root.set("viewBox", "0 0 " + str(width) + " " + str(height) )

    xpathStr = '//svg:rect[@id="clipPathRect"]'
    clipPathRect = root.xpath(xpathStr, namespaces=inkex.NSS)
    if clipPathRect != []:
        clipPathRect[0].set("width", str(width))
        clipPathRect[0].set("height", str(height))

    xpathStr = '//svg:pattern[@id="Checkerboard"]'
    designZoneData = root.xpath(xpathStr, namespaces=inkex.NSS)
    if designZoneData != []:
        if factor <= 1:
            designZoneData[0].set("patternTransform", "scale(" + str(10.0) + "," + str(factor * 10) + ")")
        else:
            designZoneData[0].set("patternTransform", "scale(" + str(10.0/factor) + "," + str(10.0) + ")")

    xpathStr = '//svg:g[@id="designTop"] | //svg:g[@id="designBottom"]'
    designZone = root.xpath(xpathStr, namespaces=inkex.NSS)
    if designZone != []:
        designZone[0].set("transform", "scale(" + str(width/100.0) + "," + str(height/100.0) + ")")
        designZone[1].set("transform", "scale(" + str(width /100.0) + "," + str(height/100.0) + ")")

    xpathStr = '//svg:g[@id="designTop"]/child::*'
    designZoneData = root.xpath(xpathStr, namespaces=inkex.NSS)
    if designZoneData != []:
        if factor <= 1:
            designZoneData[0].set("transform", "scale(1," + str(factor) + ")")
            designZoneData[1].set("transform", "scale(1," + str(factor) + ")")
            designZoneData[2].set("transform", "scale(1," + str(factor) + ")")
        else:
            designZoneData[0].set("transform", "scale(" + str(1.0/factor) + ",1)")
            designZoneData[1].set("transform", "scale(" + str(1.0/factor) + ",1)")
            designZoneData[2].set("transform", "scale(" + str(1.0/factor) + ",1)")

    xpathStr = '//svg:g[@id="textPreview"]'
    previewText = root.xpath(xpathStr, namespaces=inkex.NSS)
    if previewText != []:
        if factor <= 1:
            previewText[0].set("transform", "translate(" + str(width * 2) + ",0) scale(" + str(width/100.0) + "," + str((height/100.0) * factor) + ")")
        else:
            previewText[0].set("transform", "translate(" + str(width * 2) + ",0) scale(" + str((width/100.0)/factor) + "," + str(height/100.0) + ")")

    xpathStr = '//svg:g[@id="infoGroup"]'
    infoGroup = root.xpath(xpathStr, namespaces=inkex.NSS)
    if infoGroup != []:
        if factor <= 1:
            infoGroup[0].set("transform", "scale(" + str(width/100.0) + "," + str((height/100.0) * factor) + ")")
        else:
            infoGroup[0].set("transform", "scale(" + str(width/1000.0) + "," + str((height/1000.0) * factor) + ")")

    xpathStr = '//svg:use[@id="top1"] | //svg:use[@id="bottom1"]'
    pattern1 = root.xpath(xpathStr, namespaces=inkex.NSS)
    if pattern1 != []:
        pattern1[0].set("transform", "translate(" + str(-width) + "," + str(-height) + ")")
        pattern1[1].set("transform", "translate(" + str(-width) + "," + str(-height) + ")")

    xpathStr = '//svg:use[@id="top2"] | //svg:use[@id="bottom2"]'
    pattern2 = root.xpath(xpathStr, namespaces=inkex.NSS)
    if pattern2 != []:
        pattern2[0].set("transform", "translate(0," + str(-height) +")" )
        pattern2[1].set("transform", "translate(0," + str(-height) +")" )

    xpathStr = '//svg:use[@id="top3"] | //svg:use[@id="bottom3"]'
    pattern3 = root.xpath(xpathStr, namespaces=inkex.NSS)
    if pattern3 != []:
        pattern3[0].set("transform", "translate(" + str(width) + "," + str(-height) + ")" )
        pattern3[1].set("transform", "translate(" + str(width) + "," + str(-height) + ")" )

    xpathStr = '//svg:use[@id="top4"] | //svg:use[@id="bottom4"]'
    pattern4 = root.xpath(xpathStr, namespaces=inkex.NSS)
    if pattern4 != []:
        pattern4[0].set("transform", "translate(" + str(-width) + ",0)" )
        pattern4[1].set("transform", "translate(" + str(-width) + ",0)" )

    xpathStr = '//svg:use[@id="top5"] | //svg:use[@id="bottom5"]'
    pattern5 = root.xpath(xpathStr, namespaces=inkex.NSS)
    if pattern5 != []:
        pattern5[0].set("transform", "translate(0,0)" )
        pattern5[1].set("transform", "translate(0,0)" )

    xpathStr = '//svg:use[@id="top6"] | //svg:use[@id="bottom6"]'
    pattern6 = root.xpath(xpathStr, namespaces=inkex.NSS)
    if pattern6 != []:
        pattern6[0].set("transform", "translate(" + str(width) + ",0)" )
        pattern6[1].set("transform", "translate(" + str(width) + ",0)" )

    xpathStr = '//svg:use[@id="top7"] | //svg:use[@id="bottom7"]'
    pattern7 = root.xpath(xpathStr, namespaces=inkex.NSS)
    if pattern7 != []:
        pattern7[0].set("transform", "translate(" + str(-width) + "," + str(height) + ")" )
        pattern7[1].set("transform", "translate(" + str(-width) + "," + str(height) + ")" )

    xpathStr = '//svg:use[@id="top8"] | //svg:use[@id="bottom8"]'
    pattern8 = root.xpath(xpathStr, namespaces=inkex.NSS)
    if pattern8 != []:
        pattern8[0].set("transform", "translate(0," + str(height) + ")" )
        pattern8[1].set("transform", "translate(0," + str(height) + ")" )

    xpathStr = '//svg:use[@id="top9"] | //svg:use[@id="bottom9"]'
    pattern9 = root.xpath(xpathStr, namespaces=inkex.NSS)
    if pattern9 != []:
        pattern9[0].set("transform", "translate(" + str(width) + "," + str(height) + ")" )
        pattern9[1].set("transform", "translate(" + str(width) + "," + str(height) + ")" )

    xpathStr = '//svg:use[@id="clonePreview1"]'
    clonePreview1 = root.xpath(xpathStr, namespaces=inkex.NSS)
    if clonePreview1 != []:
        clonePreview1[0].set("transform", "translate(0," + str(height) + ")" )

    xpathStr = '//svg:use[@id="clonePreview2"]'
    clonePreview2 = root.xpath(xpathStr, namespaces=inkex.NSS)
    if clonePreview2 != []:
        clonePreview2[0].set("transform", "translate(0," + str(height * 2) + ")" )

    xpathStr = '//svg:use[@id="clonePreview3"]'
    clonePreview3 = root.xpath(xpathStr, namespaces=inkex.NSS)
    if clonePreview3 != []:
        clonePreview3[0].set("transform", "translate(" + str(width) + ",0)" )

    xpathStr = '//svg:use[@id="clonePreview4"]'
    clonePreview4 = root.xpath(xpathStr, namespaces=inkex.NSS)
    if clonePreview4 != []:
        clonePreview4[0].set("transform", "translate(" + str(width) + "," + str(height) + ")" )

    xpathStr = '//svg:use[@id="clonePreview5"]'
    clonePreview5 = root.xpath(xpathStr, namespaces=inkex.NSS)
    if clonePreview5 != []:
        clonePreview5[0].set("transform", "translate(" + str(width) + "," + str(height * 2) + ")" )

    xpathStr = '//svg:use[@id="clonePreview6"]'
    clonePreview6 = root.xpath(xpathStr, namespaces=inkex.NSS)
    if clonePreview6 != []:
        clonePreview6[0].set("transform", "translate(" + str(width*2) + ", 0)" )

    xpathStr = '//svg:use[@id="clonePreview7"]'
    clonePreview7 = root.xpath(xpathStr, namespaces=inkex.NSS)
    if clonePreview7 != []:
        clonePreview7[0].set("transform", "translate(" + str(width*2) + "," + str(height) + ")" )

    xpathStr = '//svg:use[@id="clonePreview8"]'
    clonePreview8 = root.xpath(xpathStr, namespaces=inkex.NSS)
    if clonePreview8 != []:
        clonePreview8[0].set("transform", "translate(" + str(width*2) + "," + str(height*2) + ")" )

    xpathStr = '//svg:use[@id="fullPatternClone"]'
    patternGenerator = root.xpath(xpathStr, namespaces=inkex.NSS)
    if patternGenerator != []:
        patternGenerator[0].set("transform", "translate(" + str(width * 2) + ",-" + str(height) + ")" )
        patternGenerator[0].set("{http://www.inkscape.org/namespaces/inkscape}tile-cx", str(width/2))
        patternGenerator[0].set("{http://www.inkscape.org/namespaces/inkscape}tile-cy", str(height/2))
        patternGenerator[0].set("{http://www.inkscape.org/namespaces/inkscape}tile-w", str(width))
        patternGenerator[0].set("{http://www.inkscape.org/namespaces/inkscape}tile-h", str(height))
        patternGenerator[0].set("{http://www.inkscape.org/namespaces/inkscape}tile-x0", str(width))
        patternGenerator[0].set("{http://www.inkscape.org/namespaces/inkscape}tile-y0", str(height))
        patternGenerator[0].set("width", str(width))
        patternGenerator[0].set("height", str(height))

    namedview = root.find(inkex.addNS('namedview', 'sodipodi'))
    if namedview is None:
        namedview = inkex.etree.SubElement( root, inkex.addNS('namedview', 'sodipodi') );
     
    namedview.set(inkex.addNS('document-units', 'inkscape'), 'px')

    namedview.set(inkex.addNS('cx',        'inkscape'), str((width*5.5)/2.0) )
    namedview.set(inkex.addNS('cy',        'inkscape'), "0" )
    namedview.set(inkex.addNS('zoom',        'inkscape'), str(1.0 / (width/100.00)) )
    sys.stdout = saveout

c = C()
c.affect()
