#!/usr/bin/env python
"""
Copyright (C) 2007-2011 Rob Antonishen; rob.antonishen@gmail.com

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
import inkex, os, csv, math

try:
    from subprocess import Popen, PIPE
    bsubprocess = True
except:
    bsubprocess = False

class Restack(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-d", "--direction",
                        action="store", type="string", 
                        dest="direction", default="tb",
                        help="direction to restack")
        self.OptionParser.add_option("-a", "--angle",
                        action="store", type="float", 
                        dest="angle", default=0.0,
                        help="arbitrary angle")
        self.OptionParser.add_option("-x", "--xanchor",
                        action="store", type="string", 
                        dest="xanchor", default="m",
                        help="horizontal point to compare")
        self.OptionParser.add_option("-y", "--yanchor",
                        action="store", type="string", 
                        dest="yanchor", default="m",
                        help="vertical point to compare")
    def effect(self):
        if len( self.selected ) > 0:
            objlist = []
            svg = self.document.getroot()
            parentnode = self.current_layer
            file = self.args[ -1 ]
			
            #get all bounding boxes in file by calling inkscape again with the --query-all command line option
            #it returns a comma seperated list structured id,x,y,w,h
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
                if self.options.direction == "tb" or (self.options.direction == "aa" and self.options.angle == 270):
                    objlist.append([cy,id])
                elif self.options.direction == "bt" or (self.options.direction == "aa" and self.options.angle == 90):
                    objlist.append([-cy,id])
                elif self.options.direction == "lr" or (self.options.direction == "aa" and (self.options.angle == 0 or self.options.angle == 360)):
                    objlist.append([cx,id])
                elif self.options.direction == "rl" or (self.options.direction == "aa" and self.options.angle == 180):
                    objlist.append([-cx,id])
                elif self.options.direction == "aa":
                    distance = math.hypot(cx,cy)*(math.cos(math.radians(-self.options.angle)-math.atan2(cy, cx)))
                    objlist.append([distance,id])
                elif self.options.direction == "ro":
                    distance = math.hypot(midx - cx, midy - cy)
                    objlist.append([distance,id])
                elif self.options.direction == "ri":
                    distance = -math.hypot(midx - cx, midy - cy)
                    objlist.append([distance,id])

            objlist.sort()
            #move them to the top of the object stack in this order.
            for item in objlist:
                parentnode.append( self.selected[item[1]])

if __name__ == '__main__':
    e = Restack()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 encoding=utf-8 textwidth=99
