#!/usr/bin/env python 
'''
Copyright (C) 2006 Aaron Spike, aaron@ekips.org

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
import random, inkex, simplestyle, copy
import gettext
_ = gettext.gettext

class MyEffect(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-m", "--modify",
                        action="store", type="inkbool", 
                        dest="modify", default=False,
                        help="Do not create a copy, modify the markers")
        self.OptionParser.add_option("-t", "--type",
                        action="store", type="string", 
                        dest="fill_type", default="stroke",
                        help="Replace the marker fill with the object stroke or fill color")
        self.OptionParser.add_option("-a", "--alpha",
                        action="store", type="inkbool", 
                        dest="assign_alpha", default=True,
                        help="Assign the object fill and stroke alpha to the marker")

    def effect(self):
        defs = self.xpathSingle('/svg:svg//svg:defs')
        if defs == None:
            defs = inkex.etree.SubElement(self.document.getroot(),inkex.addNS('defs','svg'))
        
        for id, node in self.selected.iteritems():
            mprops = ['marker','marker-start','marker-mid','marker-end']
            try:
                style = simplestyle.parseStyle(node.get('style'))
            except:
                inkex.errormsg(_("No style attribute found for id: %s") % id)
                continue
            
            stroke = style.get('stroke', '#000000')
            stroke_opacity = style.get('stroke-opacity', '1')
            if (self.options.fill_type == "white"):
                fill = "#FFFFFF"
                fill_opacity = "1"
            else:
                fill = style.get('fill', '#000000')
                fill_opacity = style.get('fill-opacity', '1')

            for mprop in mprops:
                if style.has_key(mprop) and style[mprop] != 'none'and style[mprop][:5] == 'url(#':
                    marker_id = style[mprop][5:-1]

                    try:
                        old_mnode = self.xpathSingle('/svg:svg//svg:marker[@id="%s"]' % marker_id)
                        if not self.options.modify:
                            mnode = copy.deepcopy(old_mnode)
                        else:
                            mnode = old_mnode
                    except:
                        inkex.errormsg(_("unable to locate marker: %s") % marker_id)
                        continue
                        
                    new_id = self.uniqueId(marker_id, not self.options.modify)
                    
                    style[mprop] = "url(#%s)" % new_id
                    mnode.set('id', new_id)
                    mnode.set(inkex.addNS('stockid','inkscape'), new_id)
                    defs.append(mnode)
                    
                    children = mnode.xpath('.//*[@style]', namespaces=inkex.NSS)
                    for child in children:
                        cstyle = simplestyle.parseStyle(child.get('style'))
                        if ('stroke' in cstyle and cstyle['stroke'] != 'none') or 'stroke' not in cstyle:
                            cstyle['stroke'] = stroke
                            if (self.options.assign_alpha):
                                cstyle['stroke-opacity'] = stroke_opacity
                        if ('fill' in cstyle and cstyle['fill'] != 'none') or 'fill' not in cstyle:
                            if (self.options.fill_type == "fill" or self.options.fill_type == "white" ):
                                cstyle['fill'] = fill
                                if (self.options.assign_alpha):
                                    cstyle['fill-opacity'] = fill_opacity
                            elif (self.options.fill_type == "stroke"):
                                cstyle['fill'] = stroke
                                if (self.options.assign_alpha):
                                    cstyle['fill-opacity'] = stroke_opacity
                            else:
                                cstyle['fill'] = "none";
                                cstyle['fill-opacity'] = "0"
                        child.set('style',simplestyle.formatStyle(cstyle))
            node.set('style',simplestyle.formatStyle(style))

if __name__ == '__main__':
    e = MyEffect()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
