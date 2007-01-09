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

import inkex, base64, os

class MyEffect(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--desc")
        self.OptionParser.add_option("--filepath",
                        action="store", type="string", 
                        dest="filepath", default=None,
                        help="")
    def effect(self):
        mimesubext={
            'png' :'.png',
            'bmp' :'.bmp',
            'jpeg':'.jpg',
            'jpg' :'.jpg', #bogus mime
            'icon':'.ico',
            'gif' :'.gif'
        }
        #ctx = inkex.xml.xpath.Context.Context(self.document,processorNss=inkex.NSS)
        
        # exbed the first embedded image
        path = self.options.filepath
        if (path != ''):
            if (self.options.ids):
                for id, node in self.selected.iteritems():
                    if node.tagName == 'image':
                        xlink = node.attributes.getNamedItemNS(inkex.NSS[u'xlink'],'href')
                        if (xlink.value[:4]=='data'):
                            comma = xlink.value.find(',')
                            if comma>0:
                                #get extension
                                fileext=''
                                semicolon = xlink.value.find(';')
                                if semicolon>0:
                                    for sub in mimesubext.keys():
                                        if sub in xlink.value[5:semicolon]:
                                            fileext=mimesubext[sub]
                                            path=path+fileext;
                                            break
                                #save
                                data = base64.decodestring(xlink.value[comma:])
                                open(path,'wb').write(data)
                                xlink.value = os.path.realpath(path) #absolute for making in-mem cycles work
                            else:
                                inkex.debug('Difficulty finding the image data.')
                            break

e = MyEffect()
e.affect()
