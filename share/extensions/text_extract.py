#!/usr/bin/env python
"""
Copyright (C) 2011 Nicolas Dufour (jazzynico)
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
from copy import deepcopy
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

class Extract(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-d", "--direction",
                        action="store", type="string",
                        dest="direction", default="tb",
                        help="direction to extract text")
        self.OptionParser.add_option("-x", "--xanchor",
                        action="store", type="string",
                        dest="xanchor", default="m",
                        help="horizontal point to compare")
        self.OptionParser.add_option("-y", "--yanchor",
                        action="store", type="string",
                        dest="yanchor", default="m",
                        help="vertical point to compare")

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
            for item in objlist:
                self.recurse(deepcopy(self.selected[item[1]]))

    def recurse(self, node):
        istext = (node.tag == '{http://www.w3.org/2000/svg}flowPara' or node.tag == '{http://www.w3.org/2000/svg}flowDiv' or node.tag == '{http://www.w3.org/2000/svg}text')
        if node.text != None or node.tail != None:
            for child in node:
                if child.get('{http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd}role'):
                    child.tail = "\n"
            inkex.errormsg(inkex.etree.tostring(node, method='text').strip())
        else:
            for child in node:
                self.recurse(child)


if __name__ == '__main__':
    e = Extract()
    e.affect()

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
