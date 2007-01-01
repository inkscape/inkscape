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

import inkex, os, base64

class MyEffect(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)

    def effect(self):
        ctx = inkex.xml.xpath.Context.Context(self.document,processorNss=inkex.NSS)
        
        # if there is a selection only embed selected images
        # otherwise embed all images
        if (self.options.ids):
            for id, node in self.selected.iteritems():
                if node.tagName == 'image':
                    self.embedImage(node)
        else:
            path = '//image'
            for node in inkex.xml.xpath.Evaluate(path,self.document, context=ctx):
                self.embedImage(node)
    def embedImage(self, node):
        xlink = node.attributes.getNamedItemNS(inkex.NSS[u'xlink'],'href')
        if (xlink.value[:4]!='data'):
            absref=node.attributes.getNamedItemNS(inkex.NSS[u'sodipodi'],'absref')
            if (os.path.isfile(absref.value)):
                file = open(absref.value,"rb").read()
                embed=True
                if (file[:4]=='\x89PNG'):
                    type='image/png'
                elif (file[:2]=='\xff\xd8'):
                    type='image/jpeg'
                elif (file[:2]=='BM'):
                    type='image/bmp'
                elif (file[:6]=='GIF87a' or file[:6]=='GIF89a'):
                    type='image/gif'
                #ico files lack any magic... therefore we check the filename instead
                elif(absref.value.endswith('.ico')):
                    type='image/x-icon' #official IANA registered MIME is 'image/vnd.microsoft.icon' tho
                else:
                    embed=False
                if (embed):
                    xlink.value = 'data:%s;base64,%s' % (type, base64.encodestring(file))
                    node.removeAttributeNS(inkex.NSS[u'sodipodi'],'absref')
                else:
                    inkex.debug("%s is not of type image/png, image/jpeg, image/bmp, image/gif or image/x-icon" % absref.value)
            else:
                inkex.debug("Sorry we could not locate %s" % absref.value)
e = MyEffect()
e.affect()
