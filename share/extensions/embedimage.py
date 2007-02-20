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
        self.OptionParser.add_option("-s", "--selectedonly",
            action="store", type="inkbool", 
            dest="selectedonly", default=False,
            help="embed only selected images")

    def effect(self):
        ctx = inkex.xml.xpath.Context.Context(self.document,processorNss=inkex.NSS)

        # if slectedonly is enabled and there is a selection only embed selected
        # images. otherwise embed all images
        if (self.options.selectedonly):
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
            href=node.attributes.getNamedItemNS(inkex.NSS[u'xlink'],'href')
            svg=self.document.getElementsByTagName('svg')[0]
            docbase=svg.attributes.getNamedItemNS(inkex.NSS[u'sodipodi'],'docbase')

            path=''
            #path selection strategy:
            # 1. href if absolute
            # 2. sodipodi:docbase + href
            # 3. realpath-ified href
            # 4. absref, only if the above does not point to a file
            if (href != None):
                if (os.path.isabs(href.value)):
                    path=os.path.realpath(href.value)
                elif (docbase != None):
                    path=os.path.join(docbase.value,href.value)
                else:
                    path=os.path.realpath(href.value)
            if (not os.path.isfile(path)):
                if (absref != None):
                    path=absref.value
            if (not os.path.isfile(path)):
                inkex.debug('No xlink:href or sodipodi:absref attributes found, or they do not point to an existing file! Unable to embed image.')
            
            if (os.path.isfile(path)):
                file = open(path,"rb").read()
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
                elif(path.endswith('.ico')):
                    type='image/x-icon' #official IANA registered MIME is 'image/vnd.microsoft.icon' tho
                else:
                    embed=False
                if (embed):
                    xlink.value = 'data:%s;base64,%s' % (type, base64.encodestring(file))
                    node.removeAttributeNS(inkex.NSS[u'sodipodi'],'absref')
                else:
                    inkex.debug("%s is not of type image/png, image/jpeg, image/bmp, image/gif or image/x-icon" % path)
            else:
                inkex.debug("Sorry we could not locate %s" % path)
e = MyEffect()
e.affect()
