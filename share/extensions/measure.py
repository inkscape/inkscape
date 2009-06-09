#!/usr/bin/env python 
'''
This extension module can measure arbitrary path and object length
It adds a text to the selected path containing the length in a
given unit.

Copyright (C) 2006 Georg Wiora
Copyright (C) 2006 Nathan Hurst
Copyright (C) 2005 Aaron Spike, aaron@ekips.org

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

TODO:
 * should use the standard attributes for text
 * Implement option to keep text orientation upright 
    1. Find text direction i.e. path tangent,
    2. check direction >90 or <-90 Degrees
    3. rotate by 180 degrees around text center
'''
import inkex, simplestyle, simplepath, sys, cubicsuperpath, bezmisc, locale
# Set current system locale
locale.setlocale(locale.LC_ALL, '')

def numsegs(csp):
    return sum([len(p)-1 for p in csp])
def interpcoord(v1,v2,p):
    return v1+((v2-v1)*p)
def interppoints(p1,p2,p):
    return [interpcoord(p1[0],p2[0],p),interpcoord(p1[1],p2[1],p)]
def pointdistance((x1,y1),(x2,y2)):
    return math.sqrt(((x2 - x1) ** 2) + ((y2 - y1) ** 2))
def bezlenapprx(sp1, sp2):
    return pointdistance(sp1[1], sp1[2]) + pointdistance(sp1[2], sp2[0]) + pointdistance(sp2[0], sp2[1])
def tpoint((x1,y1), (x2,y2), t = 0.5):
    return [x1+t*(x2-x1),y1+t*(y2-y1)]
def cspbezsplit(sp1, sp2, t = 0.5):
    m1=tpoint(sp1[1],sp1[2],t)
    m2=tpoint(sp1[2],sp2[0],t)
    m3=tpoint(sp2[0],sp2[1],t)
    m4=tpoint(m1,m2,t)
    m5=tpoint(m2,m3,t)
    m=tpoint(m4,m5,t)
    return [[sp1[0][:],sp1[1][:],m1], [m4,m,m5], [m3,sp2[1][:],sp2[2][:]]]
def cspbezsplitatlength(sp1, sp2, l = 0.5, tolerance = 0.001):
    bez = (sp1[1][:],sp1[2][:],sp2[0][:],sp2[1][:])
    t = bezmisc.beziertatlength(bez, l, tolerance)
    return cspbezsplit(sp1, sp2, t)
def cspseglength(sp1,sp2, tolerance = 0.001):
    bez = (sp1[1][:],sp1[2][:],sp2[0][:],sp2[1][:])
    return bezmisc.bezierlength(bez, tolerance)    
def csplength(csp):
    total = 0
    lengths = []
    for sp in csp:
        lengths.append([])
        for i in xrange(1,len(sp)):
            l = cspseglength(sp[i-1],sp[i])
            lengths[-1].append(l)
            total += l            
    return lengths, total

class Length(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-f", "--fontsize",
                        action="store", type="int", 
                        dest="fontsize", default=20,
                        help="Size of length lable text in px")
        self.OptionParser.add_option("-o", "--offset",
                        action="store", type="float", 
                        dest="offset", default=-6,
                        help="The distance above the curve")
        self.OptionParser.add_option("-u", "--unit",
                        action="store", type="string", 
                        dest="unit", default="mm",
                        help="The unit of the measurement")
        self.OptionParser.add_option("-p", "--precision",
                        action="store", type="int", 
                        dest="precision", default=2,
                        help="Number of significant digits after decimal point")
        self.OptionParser.add_option("-s", "--scale",
                        action="store", type="float", 
                        dest="scale", default=1,
                        help="The distance above the curve")
        self.OptionParser.add_option("-r", "--orient",
                        action="store", type="inkbool", 
                        dest="orient", default=True,
                        help="Keep orientation of text upright")
        self.OptionParser.add_option("--tab",
                        action="store", type="string", 
                        dest="tab", default="sampling",
                        help="The selected UI-tab when OK was pressed") 
        self.OptionParser.add_option("--measurehelp",
                        action="store", type="string", 
                        dest="measurehelp", default="",
                        help="dummy") 
                        
    def effect(self):
        # get number of digits
        prec = int(self.options.precision)
        # loop over all selected paths
        for id, node in self.selected.iteritems():
            if node.tag == inkex.addNS('path','svg'):
                self.group = inkex.etree.SubElement(node.getparent(),inkex.addNS('text','svg'))
                
                t = node.get('transform')
                if t:
                    self.group.set('transform', t)


                a =[]
                p = cubicsuperpath.parsePath(node.get('d'))
                num = 1
                slengths, stotal = csplength(p)
                ''' Wio: Umrechnung in unit '''
                if self.options.unit=="mm":
                    factor=25.4/90.0        # px->mm
                elif self.options.unit=="pt":
                    factor=0.80             # px->pt
                elif self.options.unit=="cm":
                    factor=25.4/900.0       # px->cm
                elif self.options.unit=="m":
                    factor=25.4/90000.0     # px->m
                elif self.options.unit=="km":
                    factor=25.4/90000000.0  # px->km
                elif self.options.unit=="in":
                    factor=1.0/90.0         # px->in
                elif self.options.unit=="ft":
                    factor=1.0/90.0/12.0    # px->ft
                elif self.options.unit=="yd":
                    factor=1.0/90.0/36.0    # px->yd
                else :
                    ''' Default unit is px'''
                    factor=1
                    self.options.unit="px"
                    
                # Format the length as string
                lenstr = locale.format("%(len)25."+str(prec)+"f",{'len':round(stotal*factor*self.options.scale,prec)}).strip()
                self.addTextOnPath(self.group,0, 0,lenstr+' '+self.options.unit, id, self.options.offset)


    def addTextOnPath(self,node,x,y,text, id,dy=0):
                new = inkex.etree.SubElement(node,inkex.addNS('textPath','svg'))
                s = {'text-align': 'center', 'vertical-align': 'bottom',
                    'text-anchor': 'middle', 'font-size': str(self.options.fontsize),
                    'fill-opacity': '1.0', 'stroke': 'none',
                    'font-weight': 'normal', 'font-style': 'normal', 'fill': '#000000'}
                new.set('style', simplestyle.formatStyle(s))
                new.set(inkex.addNS('href','xlink'), '#'+id)
                new.set('startOffset', "50%")
                new.set('dy', str(dy)) # dubious merit
                #new.append(tp)
                new.text = str(text)
                #node.set('transform','rotate(180,'+str(-x)+','+str(-y)+')')
                node.set('x', str(x))
                node.set('y', str(y))

if __name__ == '__main__':
    e = Length()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 encoding=utf-8 textwidth=99
