#!/usr/bin/env python 
'''
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
'''
import inkex, simplestyle, simplepath,sys,cubicsuperpath, bezmisc

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
                        action="store", type="string", 
                        dest="fontsize", default="20",
                        help="Size of node label numbers")    
        self.OptionParser.add_option("-o", "--offset",
                        action="store", type="string", 
                        dest="offset", default="-4",
                        help="The distance above the curve")    
    def effect(self):
        for id, node in self.selected.iteritems():
            if node.tagName == 'path':
                self.group = self.document.createElement('svg:g')
                node.parentNode.appendChild(self.group)
                
                try:
                    t = node.attributes.getNamedItem('transform').value
                    self.group.setAttribute('transform', t)
                except AttributeError:
                    pass

                a =[]
                p = cubicsuperpath.parsePath(node.attributes.getNamedItem('d').value)
                num = 1
                slengths, stotal = csplength(p)
                self.addTextOnPath(self.group,0, 0,str(stotal), id, self.options.offset)


    def addTextOnPath(self,node,x,y,text, id,dy=0):
                new = self.document.createElement('svg:text')
                tp = self.document.createElement('svg:textPath')
                s = {'font-size': self.options.fontsize, 'fill-opacity': '1.0', 'stroke': 'none',
                    'font-weight': 'normal', 'font-style': 'normal', 'fill': '#000000'}
                new.setAttribute('style', simplestyle.formatStyle(s))
                new.setAttribute('x', str(x))
                new.setAttribute('y', str(y))
                tp.setAttributeNS('http://www.w3.org/1999/xlink','xlink:href', '#'+id)
                tp.setAttribute('startOffset', "50%")
                #tp.setAttribute('dy', dy) # dubious merit
                new.appendChild(tp)
                tp.appendChild(self.document.createTextNode(str(text)))
                node.appendChild(new)

e = Length()
e.affect()
