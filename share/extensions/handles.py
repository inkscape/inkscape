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
import inkex, simplepath, simplestyle

class Handles(inkex.Effect):
    def effect(self):
        for id, node in self.selected.iteritems():
            if node.tag == inkex.addNS('path','svg'):
                p = simplepath.parsePath(node.get('d'))
                a =[]
                pen = None
                subPathStart = None
                for cmd,params in p:
                    if cmd == 'C':
                        a.extend([['M', params[:2]], ['L', pen],
                            ['M', params[2:4]], ['L', params[-2:]]])
                    if cmd == 'Q':
                        a.extend([['M', params[:2]], ['L', pen],
                            ['M', params[:2]], ['L', params[-2:]]])
                    
                    if cmd == 'M':
                        subPathStart = params

                    if cmd == 'Z':
                        pen = subPathStart
                    else:
                        pen = params[-2:]
                    
                if len(a) > 0:
                    s = {'stroke-linejoin': 'miter', 'stroke-width': '1.0px', 
                        'stroke-opacity': '1.0', 'fill-opacity': '1.0', 
                        'stroke': '#000000', 'stroke-linecap': 'butt', 
                        'fill': 'none'}
                    attribs = {'style':simplestyle.formatStyle(s),'d':simplepath.formatPath(a)}
                    inkex.etree.SubElement(node.getparent(), inkex.addNS('path','svg'), attribs)
                    
if __name__ == '__main__':
    e = Handles()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
