#!/usr/bin/env python
'''
svg_and_media_zip_output.py
An extention which collects all images to the documents directory and
creates a zip archive containing all images and the document

Copyright (C) 2005 Pim Snel, pim@lingewoud.com
Copyright (C) 2008 Aaron Spike, aaron@ekips.org
Copyright (C) 2011 Nicolas Dufour, nicoduf@yahoo.fr
    * Fix for a bug related to special caracters in the path (LP #456248).
    * Fix for Windows support (LP #391307 ).
    * Font list and image directory features.

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

TODOs
- fix bug: not saving existing .zip after a Collect for Output is run
     this bug occurs because after running an effect extention the inkscape:output_extension is reset to svg.inkscape
     the file name is still xxx.zip. after saving again the file xxx.zip is written with a plain .svg which
     looks like a corrupt zip
- maybe add better extention
- consider switching to lzma in order to allow cross platform compression with no encoding problem...
'''
# standard library
import urlparse
import urllib
import os, os.path
import string
import zipfile
import shutil
import sys
import tempfile
import locale
# local library
import inkex
import simplestyle

locale.setlocale(locale.LC_ALL, '')
inkex.localize()  # TODO: test if it's still needed now that localize is called from inkex.

class CompressedMediaOutput(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        if os.name == 'nt':
            self.encoding = "cp437"
        else:
            self.encoding = "latin-1"
        self.text_tags = ['{http://www.w3.org/2000/svg}tspan',
                            '{http://www.w3.org/2000/svg}text',
                            '{http://www.w3.org/2000/svg}flowRoot',
                            '{http://www.w3.org/2000/svg}flowPara',
                            '{http://www.w3.org/2000/svg}flowSpan']
        self.OptionParser.add_option("--image_dir",
                                     action="store", type="string",
                                     dest="image_dir", default="",
                                     help="Image directory")
        self.OptionParser.add_option("--font_list",
                                     action="store", type="inkbool",
                                     dest="font_list", default=False,
                                     help="Add font list")
        self.OptionParser.add_option("--tab",
                                     action="store", type="string",
                                     dest="tab",
                                     help="The selected UI-tab when OK was pressed")

    def output(self):
        '''
        Writes the temporary compressed file to its destination
        and removes the temporary directory.
        '''
        out = open(self.zip_file,'rb')
        if os.name == 'nt':
            try:
                import msvcrt
                msvcrt.setmode(1, os.O_BINARY)
            except:
                pass
        sys.stdout.write(out.read())
        out.close()
        shutil.rmtree(self.tmp_dir)

    def collect_images(self, docname, z):
        '''
        Collects all images in the document
        and copy them to the temporary directory.
        '''
        if locale.getpreferredencoding():
          dir_locale = locale.getpreferredencoding()
        else:
          dir_locale = "UTF-8"
        dir = unicode(self.options.image_dir, dir_locale)
        for node in self.document.xpath('//svg:image', namespaces=inkex.NSS):
            xlink = node.get(inkex.addNS('href',u'xlink'))
            if (xlink[:4] != 'data'):
                absref = node.get(inkex.addNS('absref',u'sodipodi'))
                url = urlparse.urlparse(xlink)
                href = urllib.url2pathname(url.path)
                
                if (href != None and os.path.isfile(href)):
                    absref = os.path.realpath(href)

                absref = unicode(absref, "utf-8")
                image_path = os.path.join(dir, os.path.basename(absref))
                
                if (os.path.isfile(absref)):
                    shutil.copy(absref, self.tmp_dir)
                    z.write(absref, image_path.encode(self.encoding))
                elif (os.path.isfile(os.path.join(self.tmp_dir, absref))):
                    # TODO: please explain why this clause is necessary
                    shutil.copy(os.path.join(self.tmp_dir, absref), self.tmp_dir)
                    z.write(os.path.join(self.tmp_dir, absref), image_path.encode(self.encoding))
                else:
                    inkex.errormsg(_('Could not locate file: %s') % absref)

                node.set(inkex.addNS('href',u'xlink'), image_path)
                #node.set(inkex.addNS('absref',u'sodipodi'), image_path)

    def collect_SVG(self, docstripped, z):
        '''
        Copy SVG document to the temporary directory
        and add it to the temporary compressed file
        '''
        dst_file = os.path.join(self.tmp_dir, docstripped)
        stream = open(dst_file,'w')
        self.document.write(stream)
        stream.close()
        z.write(dst_file,docstripped.encode(self.encoding)+'.svg')

    def is_text(self, node):
        '''
        Returns true if the tag in question is an element that
        can hold text.
        '''
        return node.tag in self.text_tags

    def get_fonts(self, node):
        '''
        Given a node, returns a list containing all the fonts that
        the node is using.
        '''
        fonts = []
        s = ''
        if 'style' in node.attrib:
            s = simplestyle.parseStyle(node.attrib['style'])
        if not s:
            return fonts
            
        if s.has_key('font-family'):
            if s.has_key('font-weight'):
                fonts.append(s['font-family'] + ' ' + s['font-weight'])
            else:
                fonts.append(s['font-family'])
        elif s.has_key('-inkscape-font-specification'):
            fonts.append(s['-inkscape-font-specification'])
        return fonts

    def list_fonts(self, z):
        '''
        Walks through nodes, building a list of all fonts found, then
        reports to the user with that list.
        Based on Craig Marshall's replace_font.py
        '''
        items = []
        nodes = []
        items = self.document.getroot().getiterator()
        nodes.extend(filter(self.is_text, items))
        fonts_found = []
        for node in nodes:
            for f in self.get_fonts(node):
                if not f in fonts_found:
                    fonts_found.append(f)
        findings = sorted(fonts_found)
        # Write list to the temporary compressed file
        filename = 'fontlist.txt'
        dst_file = os.path.join(self.tmp_dir, filename)
        stream = open(dst_file,'w')
        if len(findings) == 0:
            stream.write(_("Didn't find any fonts in this document/selection."))
        else:
            if len(findings) == 1:
                stream.write(_("Found the following font only: %s") % findings[0])
            else:
                stream.write(_("Found the following fonts:\n%s") % '\n'.join(findings))
        stream.close()
        z.write(dst_file, filename)


    def effect(self):
        docroot = self.document.getroot()
        docname = docroot.get(inkex.addNS('docname',u'sodipodi'))
        #inkex.errormsg(_('Locale: %s') % locale.getpreferredencoding())
        if docname is None:
            docname = self.args[-1]
        # TODO: replace whatever extention
        docstripped = os.path.basename(docname.replace('.zip', ''))
        docstripped = docstripped.replace('.svg', '')
        docstripped = docstripped.replace('.svgz', '')
        # Create os temp dir
        self.tmp_dir = tempfile.mkdtemp()
        # Create destination zip in same directory as the document
        self.zip_file = os.path.join(self.tmp_dir, docstripped) + '.zip'
        z = zipfile.ZipFile(self.zip_file, 'w')

        self.collect_images(docname, z)
        self.collect_SVG(docstripped, z)
        if self.options.font_list == True:
            self.list_fonts(z)
        z.close()


if __name__ == '__main__':   #pragma: no cover
    e = CompressedMediaOutput()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
