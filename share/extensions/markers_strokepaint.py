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
import random, inkex, simplestyle

class MyEffect(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
    def xpathSingle(self, path):
        try:
            retval = inkex.xml.xpath.Evaluate(path,self.document,context=self.ctx)[0]
        except:
            inkex.debug("No matching node for expression: %s" % path)
            retval = None
        return retval
        
    def effect(self):
        self.ctx = inkex.xml.xpath.Context.Context(self.document,processorNss=inkex.NSS)
        id_characters = '0123456789abcdefghijklmnopqrstuvwkyzABCDEFGHIJKLMNOPQRSTUVWXYZ'
        
        defs = self.xpathSingle('/svg//defs')
        if not defs:
            defs = self.document.createElement('svg:defs')
            self.document.documentElement.appendChile(defs)
        
        doc_ids = {}
        docIdNodes = inkex.xml.xpath.Evaluate('//@id',self.document,context=self.ctx)
        for m in docIdNodes:
            doc_ids[m.value] = 1
        
        for id, node in self.selected.iteritems():
            mprops = ['marker','marker-start','marker-mid','marker-end']
            try:
                style = simplestyle.parseStyle(node.attributes.getNamedItem('style').value)
            except:
                inkex.debug("No style attribute found for id: %s" % id)
                continue
            
            stroke = style.get('stroke', '#000000')
            
            for mprop in mprops:
                if style.has_key(mprop) and style[mprop] != 'none'and style[mprop][:5] == 'url(#':
                    marker_id = style[mprop][5:-1]
                    try:
                        old_mnode = self.xpathSingle('/svg//marker[@id="%s"]' % marker_id)
                        mnode = old_mnode.cloneNode(True)
                    except:
                        inkex.debug("unable to locate marker: %s" % marker_id)
                        continue
                        
                    #generate a unique id
                    new_id = marker_id
                    while new_id in doc_ids:
                        new_id = "%s%s" % (new_id,random.choice(id_characters))
                    doc_ids[new_id] = 1
                    
                    style[mprop] = "url(#%s)" % new_id
                    mnode.attributes.getNamedItem('id').value = new_id
                    defs.appendChild(mnode)
                    
                    children = inkex.xml.xpath.Evaluate('/svg//marker[@id="%s"]//*[@style]' % new_id,self.document,context=self.ctx)
                    for child in children:
                        cstyle = simplestyle.parseStyle(child.attributes.getNamedItem('style').value)
                        if ('stroke' in cstyle and cstyle['stroke'] != 'none') or 'stroke' not in cstyle:
                                cstyle['stroke'] = stroke
                        if ('fill' in cstyle and cstyle['fill'] != 'none') or 'fill' not in cstyle:
                                cstyle['fill'] = stroke
                        child.attributes.getNamedItem('style').value = simplestyle.formatStyle(cstyle)
            node.attributes.getNamedItem('style').value = simplestyle.formatStyle(style)
e = MyEffect()
e.affect()
