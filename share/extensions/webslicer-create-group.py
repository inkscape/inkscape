#!/usr/bin/env python
'''
Copyright (C) 2010 Aurelio A. Heckert, aurium (a) gmail dot com

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

import inkex
import gettext

_ = gettext.gettext

def is_empty(val):
    if val is None:
        return True
    else:
        return len(str(val)) == 0

class WebSlicer_CreateGroup(inkex.Effect):

    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--html-id",
                                     action="store", type="string",
                                     dest="html_id",
                                     help="")
        self.OptionParser.add_option("--html-class",
                                     action="store", type="string",
                                     dest="html_class",
                                     help="")
        self.OptionParser.add_option("--width-unity",
                                     action="store", type="string",
                                     dest="width_unity",
                                     help="")
        self.OptionParser.add_option("--height-unity",
                                     action="store", type="string",
                                     dest="height_unity",
                                     help="")

    def effect(self):
        layer = self.document.xpath(
                     '//*[@id="webslicer-layer" and @inkscape:groupmode="layer"]',
                     namespaces=inkex.NSS)[0]
        if len(self.selected) == 0:
            inkex.errormsg(_('You must to select some "Slicer rectangles" or other "Layout groups".'))
            exit(1)
        descendants = get_descendants_in_array(layer)
        for id,node in self.selected.iteritems():
            if node not in descendants:
                inkex.errormsg(_('Opss... The element "%s" is not in the Web Slicer layer') % id)
                exit(2)
        g_parent = self.find_node_parent(node)
        group = inkex.etree.SubElement(g_parent, 'g')
        desc = inkex.etree.SubElement(group, 'desc')
        conf_txt = ''
        if not is_empty(self.options.html_id):
            conf_txt += 'html-id:'    + self.options.html_id +'\n'
        if not is_empty(self.options.html_class):
            conf_txt += 'html-class:' + self.options.html_class +'\n'
        conf_txt += 'width-unity:' + self.options.width_unity +'\n'
        conf_txt += 'height-unity:' + self.options.height_unity
        desc.text = conf_txt
        for id,node in self.selected.iteritems():
            group.insert( 1, node )


    def find_node_parent(self, node):
        #TODO: make it real!
        return self.document.xpath(
                    '//*[@id="webslicer-layer" and @inkscape:groupmode="layer"]',
                    namespaces=inkex.NSS)[0]

def get_descendants_in_array(el):
    descendants = el.getchildren()
    for e in descendants:
        descendants.extend( get_descendants_in_array(e) )
    return descendants

if __name__ == '__main__':
    e = WebSlicer_CreateGroup()
    e.affect()
