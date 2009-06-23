#! /usr/bin/env python
"""
svg_and_media_zip_output.py
An extention which collects all images to the documents directory and
creates a zip archive containing all images and the document

Copyright (C) 2005 Pim Snel, pim@lingewoud.com
Copyright (C) 2008 Aaron Spike, aaron@ekips.org
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

import inkex, os.path, urlparse, urllib
import os
import string
import zipfile
import shutil
import sys
import tempfile
import gettext
_ = gettext.gettext

class SVG_and_Media_ZIP_Output(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)

    def output(self):
        out = open(self.zip_file,'r')
        sys.stdout.write(out.read())
        out.close()
        self.clear_tmp()

    def clear_tmp(self):
        shutil.rmtree(self.tmp_dir)

    def effect(self):
        ttmp_orig = self.document.getroot()

        docname = ttmp_orig.get(inkex.addNS('docname',u'sodipodi'))
        if docname is None: docname = self.args[-1]

        #create os temp dir
        self.tmp_dir = tempfile.mkdtemp()

        #fixme replace whatever extention
        docstripped = docname.replace('.zip', '')
        docstripped = docstripped.replace('.svg', '')
        docstripped = docstripped.replace('.svgz', '')
        
        # create destination zip in same directory as the document
        self.zip_file = os.path.join(self.tmp_dir, docstripped) + '.zip'
        z = zipfile.ZipFile(self.zip_file, 'w')

        #read tmpdoc and copy all images to temp dir
        for node in self.document.xpath('//svg:image', namespaces=inkex.NSS):
            self.collectAndZipImages(node, docname, z)

        ##copy tmpdoc to tempdir
        dst_file = os.path.join(self.tmp_dir, docstripped)
        stream = open(dst_file,'w')

        self.document.write(stream)

        stream.close()

        z.write(dst_file,docstripped.encode("utf_8")+'.svg')

        z.close()

    def collectAndZipImages(self, node, docname, z):
        xlink = node.get(inkex.addNS('href',u'xlink'))
        if (xlink[:4]!='data'):
            absref=node.get(inkex.addNS('absref',u'sodipodi'))
            url=urlparse.urlparse(xlink)
            href=urllib.unquote(url.path)
            if os.name == 'nt' and href[0] == '/':
                href = href[1:]
            if (href != None):
                absref=os.path.realpath(href)

            if (os.path.isfile(absref)):
                shutil.copy(absref, self.tmp_dir)
                z.write(absref, os.path.basename(absref).encode("utf_8"))
            elif (os.path.isfile(os.path.join(self.tmp_dir, absref))):
                #TODO: please explain why this clause is necessary
                shutil.copy(os.path.join(self.tmp_dir, absref), self.tmp_dir)
                z.write(os.path.join(self.tmp_dir, absref),
                        os.path.basename(absref).encode("utf_8"))
            else:
                inkex.errormsg(_('Could not locate file: %s') % absref)

            node.set(inkex.addNS('href',u'xlink'),os.path.basename(absref))
            node.set(inkex.addNS('absref',u'sodipodi'),os.path.basename(absref))


if __name__ == '__main__':   #pragma: no cover
    e = SVG_and_Media_ZIP_Output()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 encoding=utf-8 textwidth=99
