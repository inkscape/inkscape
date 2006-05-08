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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
'''
import math, tempfile, copy, cPickle, inkex, simplepath

def findend(d):
    end = []
    subPathStart = []
    for cmd,params in d:
        if cmd == 'M':
            subPathStart = params[-2:]
        if cmd == 'Z':
            end = subPathStart[:]
        else:
            end = params[-2:]
    return end

class Kochify(inkex.Effect):
    def effect(self):
        try:
            f = open(tempfile.gettempdir() + '/kochify.bin', 'r')
            proto = cPickle.load(f)
            f.close()                
        except:
            return False
        for id, node in self.selected.iteritems():
            if node.tagName == 'path':
                d = node.attributes.getNamedItem('d')
                p = simplepath.parsePath(d.value)
                last = p[0][1][-2:]
                subPathStart = []
                cur = []
                new = []
                for i in range(len(p)):
                    rep = copy.deepcopy(proto['path'])
                    cmd, params = p[i]
                    if cmd == 'M':
                        subPathStart = params[-2:]
                        new.append(copy.deepcopy(p[i]))
                        continue
                    if cmd == 'Z':
                        cur = subPathStart[:]
                    else:
                        cur = params[-2:]

                    if last == cur:
                        continue

                    dx = cur[0]-last[0]
                    dy = cur[1]-last[1]
                    length = math.sqrt((dx**2) + (dy**2))
                    angle = math.atan2(dy,dx)
        
                    scale = length / proto['length']
                    rotate = angle - proto['angle']
                    simplepath.scalePath(rep,scale,scale)
                    simplepath.rotatePath(rep, rotate)
                    repend = findend(rep)
                    transx = cur[0] - repend[0]
                    transy = cur[1] - repend[1]
                    simplepath.translatePath(rep, transx, transy)
                    
                    if proto['endsinz']:
                        new.extend(rep[:])
                    else:
                        new.extend(rep[1:])
                    last = cur[:]
                
                d.value = simplepath.formatPath(new)
e = Kochify()
e.affect()
