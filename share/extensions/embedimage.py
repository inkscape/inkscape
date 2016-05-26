#!/usr/bin/env python 
'''
Copyright (C) 2005,2007 Aaron Spike, aaron@ekips.org

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
# standard library
import base64
import os
import sys
import urllib
import urlparse
# local library
import inkex


class Embedder(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-s", "--selectedonly",
            action="store", type="inkbool", 
            dest="selectedonly", default=False,
            help="embed only selected images")

    def effect(self):
        # if slectedonly is enabled and there is a selection only embed selected
        # images. otherwise embed all images
        if (self.options.selectedonly):
            self.embedSelected(self.document, self.selected)
        else:
            self.embedAll(self.document)

    def embedSelected(self, document, selected):
        self.document=document
        self.selected=selected
        if (self.options.ids):
            for id, node in selected.iteritems():
                if node.tag == inkex.addNS('image','svg'):
                    self.embedImage(node)

    def embedAll(self, document):
        self.document=document #not that nice... oh well
        path = '//svg:image'
        for node in self.document.getroot().xpath(path, namespaces=inkex.NSS):
            self.embedImage(node)

    def embedImage(self, node):
        xlink = node.get(inkex.addNS('href','xlink'))
        if xlink is None or xlink[:5] != 'data:':
            absref=node.get(inkex.addNS('absref','sodipodi'))
            url=urlparse.urlparse(xlink)
            href=urllib.url2pathname(url.path)
            
            path=''
            #path selection strategy:
            # 1. href if absolute
            # 2. realpath-ified href
            # 3. absref, only if the above does not point to a file
            if (href != None):
                path=os.path.realpath(href)
            if (not os.path.isfile(path)):
                if (absref != None):
                    path=absref

            try:
                path=unicode(path, "utf-8")
            except TypeError:
                path=path
                
            if (not os.path.isfile(path)):
                inkex.errormsg(_('No xlink:href or sodipodi:absref attributes found, or they do not point to an existing file! Unable to embed image.'))
                if path:
                    inkex.errormsg(_("Sorry we could not locate %s") % str(path))

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
                elif (file[:4]=='MM\x00\x2a' or file[:4]=='II\x2a\x00'):
                    type='image/tiff'
                #ico files lack any magic... therefore we check the filename instead
                elif(path.endswith('.ico')):
                    type='image/x-icon' #official IANA registered MIME is 'image/vnd.microsoft.icon' tho
                else:
                    embed=False
                if (embed):
                    node.set(inkex.addNS('href','xlink'), 'data:%s;base64,%s' % (type, base64.encodestring(file)))
                    if (absref != None):
                        del node.attrib[inkex.addNS('absref',u'sodipodi')]
                else:
                    inkex.errormsg(_("%s is not of type image/png, image/jpeg, image/bmp, image/gif, image/tiff, or image/x-icon") % path)

if __name__ == '__main__':
    e = Embedder()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
