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
import math, tempfile, cPickle, inkex, simplepath

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

class LoadKochify(inkex.Effect):
    def effect(self):
        for id, node in self.selected.iteritems():
            if node.tagName == 'path':
                d = simplepath.parsePath(node.attributes.getNamedItem('d').value)
                start = d[0][1][-2:]
                end = findend(d)
                while start == end and len(d):
                    d = d[:-1]
                    end = findend(d)
                if not end: 
                    break
                dx = end[0]-start[0]
                dy = end[1]-start[1]
                length = math.sqrt((dx**2) + (dy**2))
                angle = math.atan2(dy,dx)
                endsinz = False
                if d[-1][0]=='Z':
                    endsinz = True
                path = {'start': start, 'end': end, 'endsinz': endsinz,
                    'length': length, 'angle': angle, 'path': d}
                f = open(tempfile.gettempdir() + '/kochify.bin', 'w')
                cPickle.dump(path, f)
                f.close()                
                break

e = LoadKochify()
e.affect()
