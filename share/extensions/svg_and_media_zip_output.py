#!/usr/bin/env python 
"""
svg_and_media_zip_output.py
An extention which collects all images to the documents directory and 
creates a zip archive containing all images and the document

Copyright (C) 2005 Pim Snel, pim@lingewoud.com
this is  the first Python script  ever created
its based on embedimage.py

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

Version 0.3

TODO
- fix bug: not saving existing .zip after a Collect for Output is run
     this bug occurs because after running an effect extention the inkscape:output_extension is reset to svg.inkscape
     the file name is still xxx.zip. after saving again the file xxx.zip is written with a plain .svg which 
     looks like a corrupt zip
- maybe add better extention
"""

import inkex, os.path
import os
import string
import zipfile
import shutil
import sys
import tempfile

class MyEffect(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)

        self.documentDst=None

    def parseTmp(self,file=None):
        """Parse document in specified file or on stdin"""
        reader = inkex.xml.dom.ext.reader.Sax2.Reader()
        try:
            try:
                stream = open(file,'r')
            except:
                stream = open(self.args[-1],'r')
        except:
            stream = sys.stdin
        self.documentDst = reader.fromStream(stream)
        stream.close()

    def output(self):
        pass

    def effect(self):
        
        #get needed info from orig document
        ctx_orig = inkex.xml.xpath.Context.Context(self.document,processorNss=inkex.NSS)

        ttmp_orig = inkex.xml.xpath.Evaluate('/svg',self.document, context=ctx_orig)
        
        docbase = ttmp_orig[0].attributes.getNamedItemNS(inkex.NSS[u'sodipodi'],'docbase')
        docname = ttmp_orig[0].attributes.getNamedItemNS(inkex.NSS[u'sodipodi'],'docname')

        orig_tmpfile = sys.argv[1]

        # create destination zip in same directory as the document
        z = zipfile.ZipFile(docbase.value + '/'+ docname.value + '.zip', 'w')    

        #create os temp dir
        tmp_dir = tempfile.mkdtemp()

        #fixme replace whatever extention
        docstripped = docname.value.replace('.zip', '')
    
        #read tmpdoc and copy all images to temp dir
        for node in inkex.xml.xpath.Evaluate('//image',self.document, context=ctx_orig):
            self.collectAndZipImages(node, tmp_dir, docname, z)

        ##copy tmpdoc to tempdir
        dst_file = os.path.join(tmp_dir, docstripped)
        stream = open(dst_file,'w')
    
        inkex.xml.dom.ext.Print(self.document,stream)
        
        stream.close()
        
        z.write(dst_file.encode("latin-1"),docstripped.encode("latin-1")+'.svg') 
        z.close()

        shutil.move(docbase.value + '/'+ docname.value + '.zip',docbase.value + '/'+ docname.value)
        
        shutil.rmtree(tmp_dir)

    def collectAndZipImages(self, node, tmp_dir, docname, z):
        xlink = node.attributes.getNamedItemNS(inkex.NSS[u'xlink'],'href')
        if (xlink.value[:4]!='data'):
            absref=node.attributes.getNamedItemNS(inkex.NSS[u'sodipodi'],'absref')

            if (os.path.isfile(absref.value)):
                shutil.copy(absref.value,tmp_dir)
                z.write(absref.value.encode("latin-1"),os.path.basename(absref.value).encode("latin-1"))

            elif (os.path.isfile(tmp_dir + '/' + absref.value)):
                shutil.copy(tmp_dir + '/' + absref.value,tmp_dir)
                z.write(tmp_dir + '/' + absref.value.encode("latin-1"),os.path.basename(absref.value).encode("latin-1"))

            xlink.value = os.path.basename(absref.value)
            absref.value = os.path.basename(absref.value)

            
e = MyEffect()
e.affect()
