#!/usr/bin/env python 
'''
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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
'''
import math, inkex, simplestyle, simplepath, bezmisc

class Motion(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-a", "--angle",
                        action="store", type="float", 
                        dest="angle", default=45.0,
                        help="direction of the motion vector")
        self.OptionParser.add_option("-m", "--magnitude",
                        action="store", type="float", 
                        dest="magnitude", default=100.0,
                        help="magnitude of the motion vector")    

    def makeface(self,last,(cmd, params)):
        a = []
        a.append(['M',last[:]])
        a.append([cmd, params[:]])

        #translate path segment along vector
        np = params[:]
        defs = simplepath.pathdefs[cmd]
        for i in range(defs[1]):
            if defs[3][i] == 'x':
                np[i] += self.vx
            elif defs[3][i] == 'y':
                np[i] += self.vy

        a.append(['L',[np[-2],np[-1]]])
        
        #reverse direction of path segment
        np[-2:] = last[0]+self.vx,last[1]+self.vy
        if cmd == 'C':
            c1 = np[:2], np[2:4] = np[2:4], np[:2]
        a.append([cmd,np[:]])
            
        a.append(['Z',[]])
        face = inkex.etree.SubElement(self.facegroup,inkex.addNS('path','svg'),{'d':simplepath.formatPath(a)})
        
    def effect(self):
        self.vx = math.cos(math.radians(self.options.angle))*self.options.magnitude
        self.vy = math.sin(math.radians(self.options.angle))*self.options.magnitude
        for id, node in self.selected.iteritems():
            if node.tag == inkex.addNS('path','svg'):
                group = inkex.etree.SubElement(node.getparent(),inkex.addNS('g','svg'))
                self.facegroup = inkex.etree.SubElement(group, inkex.addNS('g','svg'))
                group.append(node)
                
                t = node.get('transform')
                if t:
                    group.set('transform', t)
                    node.set('transform','')
                    
                s = node.get('style')
                self.facegroup.set('style', s)

                p = simplepath.parsePath(node.get('d'))
                for cmd,params in p:
                    tees = []
                    if cmd == 'C':
                        bez = (last,params[:2],params[2:4],params[-2:])
                        tees = [t for t in bezmisc.beziertatslope(bez,(self.vy,self.vx)) if 0<t<1]
                        tees.sort()

                    segments = []
                    if len(tees) == 0 and cmd in ['L','C']:
                            segments.append([cmd,params[:]])
                    elif len(tees) == 1:
                            one,two = bezmisc.beziersplitatt(bez,tees[0])
                            segments.append([cmd,list(one[1]+one[2]+one[3])])
                            segments.append([cmd,list(two[1]+two[2]+two[3])])
                    elif len(tees) == 2:
                            one,two = bezmisc.beziersplitatt(bez,tees[0])
                            two,three = bezmisc.beziersplitatt(two,tees[1])
                            segments.append([cmd,list(one[1]+one[2]+one[3])])
                            segments.append([cmd,list(two[1]+two[2]+two[3])])
                            segments.append([cmd,list(three[1]+three[2]+three[3])])

                    for seg in segments:
                        self.makeface(last,seg)
                        last = seg[1][-2:]
                    
                    if cmd == 'M':
                        subPathStart = params[-2:]
                    if cmd == 'Z':
                        last = subPathStart
                    else:
                        last = params[-2:]

if __name__ == '__main__':
    e = Motion()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
