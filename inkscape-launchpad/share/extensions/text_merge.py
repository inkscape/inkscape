#!/usr/bin/env python
"""
Copyright (C) 2013 Nicolas Dufour (jazzynico)
Direction code from the Restack extension, by Rob Antonishen

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

"""
# standard library
import chardataeffect
import copy
import csv
import math
import os
import string
try:
    from subprocess import Popen, PIPE
    bsubprocess = True
except:
    bsubprocess = False
# local library
import inkex


class Merge(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-d", "--direction",
                        action="store", type="string", 
                        dest="direction", default="tb",
                        help="direction to merge text")
        self.OptionParser.add_option("-x", "--xanchor",
                        action="store", type="string", 
                        dest="xanchor", default="m",
                        help="horizontal point to compare")
        self.OptionParser.add_option("-y", "--yanchor",
                        action="store", type="string", 
                        dest="yanchor", default="m",
                        help="vertical point to compare")
        self.OptionParser.add_option("-t", "--flowtext",
                        action="store", type="inkbool", 
                        dest="flowtext", default=False,
                        help="use a flow text structure instead of a normal text element")
        self.OptionParser.add_option("-k", "--keepstyle",
                        action="store", type="inkbool", 
                        dest="keepstyle", default=False,
                        help="keep format")
                        
    def effect(self):
        if len(self.selected)==0:
            for node in self.document.xpath('//svg:text | //svg:flowRoot', namespaces=inkex.NSS):
                self.selected[node.get('id')] = node
    
        if len( self.selected ) > 0:
            objlist = []
            svg = self.document.getroot()
            parentnode = self.current_layer
            file = self.args[ -1 ]

            # get all bounding boxes in file by calling inkscape again with the --query-all command line option
            # it returns a comma separated list structured id,x,y,w,h
            if bsubprocess:
                p = Popen('inkscape --query-all "%s"' % (file), shell=True, stdout=PIPE, stderr=PIPE)
                err = p.stderr
                f = p.communicate()[0]
                try:
                    reader=csv.CSVParser().parse_string(f)    #there was a module cvs.py in earlier inkscape that behaved differently
                except:
                    reader=csv.reader(f.split( os.linesep ))
                err.close() 
            else:
                _,f,err = os.popen3('inkscape --query-all "%s"' % ( file ) )
                reader=csv.reader( f )
                err.close()

            #build a dictionary with id as the key
            dimen = dict()
            for line in reader:
                if len(line) > 0:
                    dimen[line[0]] = map( float, line[1:])

            if not bsubprocess: #close file if opened using os.popen3
                f.close

            #find the center of all selected objects **Not the average!
            x,y,w,h = dimen[self.selected.keys()[0]]
            minx = x
            miny = y
            maxx = x + w
            maxy = y + h

            for id, node in self.selected.iteritems():
                # get the bounding box
                x,y,w,h = dimen[id]
                if x < minx:
                    minx = x
                if (x + w) > maxx:
                    maxx = x + w
                if y < miny:
                    miny = y
                if (y + h) > maxy:
                    maxy = y + h

            midx = (minx + maxx) / 2
            midy = (miny + maxy) / 2

            #calculate distances for each selected object
            for id, node in self.selected.iteritems():
                # get the bounding box
                x,y,w,h = dimen[id]

                # calc the comparison coords
                if self.options.xanchor == "l":
                    cx = x
                elif self.options.xanchor == "r":
                    cx = x + w
                else:  # middle
                    cx = x + w / 2

                if self.options.yanchor == "t":
                    cy = y
                elif self.options.yanchor == "b":
                    cy = y + h
                else:  # middle
                    cy = y + h / 2

                #direction chosen
                if self.options.direction == "tb":
                    objlist.append([cy,id])
                elif self.options.direction == "bt":
                    objlist.append([-cy,id])
                elif self.options.direction == "lr":
                    objlist.append([cx,id])
                elif self.options.direction == "rl":
                    objlist.append([-cx,id])

            objlist.sort()
            #move them to the top of the object stack in this order.
            
            if self.options.flowtext:
                self.text_element = "flowRoot"
                self.text_span = "flowPara"
            else:
                self.text_element = "text"
                self.text_span = "tspan"
                
            self.textRoot=inkex.etree.SubElement(parentnode,inkex.addNS(self.text_element,'svg'),{inkex.addNS('space','xml'):'preserve'})
            self.textRoot.set(inkex.addNS('style', ''), 'font-size:20px;font-style:normal;font-weight:normal;line-height:125%;letter-spacing:0px;word-spacing:0px;fill:#000000;fill-opacity:1;stroke:none;')

            for item in objlist:
                self.recurse(self.selected[item[1]], self.textRoot)
                
            if self.options.flowtext:
                self.region=inkex.etree.SubElement(self.textRoot,inkex.addNS('flowRegion','svg'),{inkex.addNS('space','xml'):'preserve'})
                self.rect=inkex.etree.SubElement(self.region,inkex.addNS('rect','svg'),{inkex.addNS('space','xml'):'preserve'})
                self.rect.set(inkex.addNS('height', ''), '200')
                self.rect.set(inkex.addNS('width', ''), '200')
    
    def recurse(self, node, span):
        #istext = (node.tag == '{http://www.w3.org/2000/svg}flowPara' or node.tag == '{http://www.w3.org/2000/svg}flowDiv' or node.tag == '{http://www.w3.org/2000/svg}tspan')
        if node.tag != '{http://www.w3.org/2000/svg}flowRegion':

            newspan=inkex.etree.SubElement(span,inkex.addNS(self.text_span,'svg'),{inkex.addNS('space','xml'):'preserve'})

            if node.get('{http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd}role'):
                newspan.set(inkex.addNS('role', 'sodipodi'), node.get('{http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd}role'))
            if (node.tag == '{http://www.w3.org/2000/svg}text' or node.tag == '{http://www.w3.org/2000/svg}flowPara'):
                newspan.set(inkex.addNS('role', 'sodipodi'), 'line')

            if self.options.keepstyle:
                if node.get('style'):
                    newspan.set(inkex.addNS('style', ''), node.get('style'))

            if node.text != None:
                newspan.text = node.text
            for child in node:
                self.recurse(child, newspan)
            if (node.tail and node.tag != '{http://www.w3.org/2000/svg}text'):
                newspan.tail = node.tail
                

if __name__ == '__main__':
    e = Merge()
    e.affect()

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
