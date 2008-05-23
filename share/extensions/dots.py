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
import inkex, simplestyle, simplepath

class Dots(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-d", "--dotsize",
                        action="store", type="string", 
                        dest="dotsize", default="10px",
                        help="Size of the dots placed at path nodes")
        self.OptionParser.add_option("-f", "--fontsize",
                        action="store", type="string", 
                        dest="fontsize", default="20",
                        help="Size of node label numbers")    
    def effect(self):
        for id, node in self.selected.iteritems():
            if node.tag == inkex.addNS('path','svg'):
                self.group = inkex.etree.SubElement(node.getparent(),inkex.addNS('g','svg'))
                new = inkex.etree.SubElement(self.group,inkex.addNS('path','svg'))
                
                try:
                    t = node.get('transform')
                    self.group.set('transform', t)
                except:
                    pass

                s = simplestyle.parseStyle(node.get('style'))
                s['stroke-linecap']='round'
                s['stroke-width']=self.options.dotsize
                new.set('style', simplestyle.formatStyle(s))

                a =[]
                p = simplepath.parsePath(node.get('d'))
                num = 1
                for cmd,params in p:
                    if cmd != 'Z':
                        a.append(['M',params[-2:]])
                        a.append(['L',params[-2:]])
                        self.addText(self.group,params[-2],params[-1],num)
                        num += 1
                new.set('d', simplepath.formatPath(a))
                node.clear()

                
    def addText(self,node,x,y,text):
                new = inkex.etree.SubElement(node,inkex.addNS('text','svg'))
                s = {'font-size': self.options.fontsize, 'fill-opacity': '1.0', 'stroke': 'none',
                    'font-weight': 'normal', 'font-style': 'normal', 'fill': '#000000'}
                new.set('style', simplestyle.formatStyle(s))
                new.set('x', str(x))
                new.set('y', str(y))
                new.text = str(text)

e = Dots()
e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 encoding=utf-8 textwidth=99
