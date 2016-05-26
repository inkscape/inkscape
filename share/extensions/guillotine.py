#!/usr/bin/env python
'''
guillotine.py

Copyright (C) 2010 Craig Marshall, craig9 [at] gmail.com

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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

-----------------------

This script slices an inkscape drawing along the guides, similarly to
the GIMP plugin called "guillotine". It can optionally export to the
same directory as the SVG file with the same name, but with a number
suffix. e.g.

/home/foo/drawing.svg

will export to:

/home/foo/drawing0.png
/home/foo/drawing1.png
/home/foo/drawing2.png
/home/foo/drawing3.png

etc.

'''
# standard library
import locale
import os
import sys
try:
    from subprocess import Popen, PIPE
    bsubprocess = True
except:
    bsubprocess = False
# local library
import inkex
import simplestyle

locale.setlocale(locale.LC_ALL, '')

def float_sort(a, b):
    '''
    This is used to sort the horizontal and vertical guide positions,
    which are floating point numbers, but which are held as text.
    '''
    return cmp(float(a), float(b))

class Guillotine(inkex.Effect):
    """Exports slices made using guides"""
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--directory", action="store",
                                        type="string", dest="directory",
                                        default=None, help="")

        self.OptionParser.add_option("--image", action="store",
                                        type="string", dest="image",
                                        default=None, help="")

        self.OptionParser.add_option("--ignore", action="store",
                                        type="inkbool", dest="ignore",
                                        default=None, help="")

    def get_guides(self):
        '''
        Returns all guide elements as an iterable collection
        '''
        root = self.document.getroot()
        guides = []
        xpath = self.document.xpath("//sodipodi:guide",
                                    namespaces=inkex.NSS)
        for g in xpath:
            guide = {}
            (x, y) = g.attrib['position'].split(',')
            if g.attrib['orientation'][:2] == '0,':
                guide['orientation'] = 'horizontal'
                guide['position'] = y
                guides.append(guide)
            elif g.attrib['orientation'][-2:] == ',0':
                guide['orientation'] = 'vertical'
                guide['position'] = x
                guides.append(guide)
        return guides

    def get_all_horizontal_guides(self):
        '''
        Returns all horizontal guides as a list of floats stored as
        strings. Each value is the position from 0 in pixels.
        '''
        guides = []
        for g in self.get_guides():
            if g['orientation'] == 'horizontal':
                guides.append(g['position'])
        return guides

    def get_all_vertical_guides(self):
        '''
        Returns all vertical guides as a list of floats stored as
        strings. Each value is the position from 0 in pixels.
        '''
        guides = []
        for g in self.get_guides():
            if g['orientation'] == 'vertical':
                guides.append(g['position'])
        return guides

    def get_horizontal_slice_positions(self):
        '''
        Make a sorted list of all horizontal guide positions,
        including 0 and the document height, but not including
        those outside of the canvas
        '''
        root = self.document.getroot()
        horizontals = ['0']
        height = self.unittouu(root.attrib['height'])
        for h in self.get_all_horizontal_guides():
            if h >= 0 and float(h) <= float(height):
                horizontals.append(h)
        horizontals.append(height)
        horizontals.sort(cmp=float_sort)
        return horizontals

    def get_vertical_slice_positions(self):
        '''
        Make a sorted list of all vertical guide positions,
        including 0 and the document width, but not including
        those outside of the canvas.
        '''
        root = self.document.getroot()
        verticals = ['0']
        width = self.unittouu(root.attrib['width'])
        for v in self.get_all_vertical_guides():
            if v >= 0 and float(v) <= float(width):
                verticals.append(v)
        verticals.append(width)
        verticals.sort(cmp=float_sort)
        return verticals

    def get_slices(self):
        '''
        Returns a list of all "slices" as denoted by the guides
        on the page. Each slice is really just a 4 element list of
        floats (stored as strings), consisting of the X and Y start
        position and the X and Y end position.
        '''
        hs = self.get_horizontal_slice_positions()
        vs = self.get_vertical_slice_positions()
        slices = []
        for i in range(len(hs)-1):
            for j in range(len(vs)-1):
                slices.append([vs[j], hs[i], vs[j+1], hs[i+1]])
        return slices

    def get_filename_parts(self):
        '''
        Attempts to get directory and image as passed in by the inkscape
        dialog. If the boolean ignore flag is set, then it will ignore
        these settings and try to use the settings from the export
        filename.
        '''

        if self.options.ignore == False:
            if self.options.image == "" or self.options.image is None:
                inkex.errormsg("Please enter an image name")
                sys.exit(0)
            return (self.options.directory, self.options.image)
        else:
            '''
            First get the export-filename from the document, if the
            document has been exported before (TODO: Will not work if it
            hasn't been exported yet), then uses this to return a tuple
            consisting of the directory to export to, and the filename
            without extension.
            '''
            svg = self.document.getroot()
            att = '{http://www.inkscape.org/namespaces/inkscape}export-filename'
            try:
                export_file = svg.attrib[att]
            except KeyError:
                inkex.errormsg("To use the export hints option, you " +
                "need to have previously exported the document. " +
                "Otherwise no export hints exist!")
                sys.exit(-1)
            dirname, filename = os.path.split(export_file)
            filename = filename.rsplit(".", 1)[0] # Without extension
            return (dirname, filename)

    def check_dir_exists(self, dir):
        if not os.path.isdir(dir):
            os.makedirs(dir)

    def get_localised_string(self, str):
        return locale.format("%.f", float(str), 0)

    def export_slice(self, s, filename):
        '''
        Runs inkscape's command line interface and exports the image
        slice from the 4 coordinates in s, and saves as the filename
        given.
        '''
        svg_file = self.args[-1]
        command = "inkscape -a %s:%s:%s:%s -e \"%s\" \"%s\" " % (self.get_localised_string(s[0]), self.get_localised_string(s[1]), self.get_localised_string(s[2]), self.get_localised_string(s[3]), filename, svg_file)
        if bsubprocess:
            p = Popen(command, shell=True, stdout=PIPE, stderr=PIPE)
            return_code = p.wait()
            f = p.stdout
            err = p.stderr
        else:
            _, f, err = os.open3(command)
        f.close()

    def export_slices(self, slices):
        '''
        Takes the slices list and passes each one with a calculated
        filename/directory into export_slice.
        '''
        dirname, filename = self.get_filename_parts()
        output_files = list()
        if dirname == '' or dirname == None:
            dirname = './'

        dirname = os.path.expanduser(dirname)
        dirname = os.path.expandvars(dirname)
        dirname = os.path.abspath(dirname)
        if dirname[-1] != os.path.sep:
            dirname += os.path.sep
        self.check_dir_exists(dirname)
        i = 0
        for s in slices:
            f = dirname + filename + str(i) + ".png"
            output_files.append(f)
            self.export_slice(s, f)
            i += 1
        inkex.errormsg(_("The sliced bitmaps have been saved as:") + "\n\n" + "\n".join(output_files))

    def effect(self):
        slices = self.get_slices()
        self.export_slices(slices)

if __name__ == "__main__":
    e = Guillotine()
    e.affect()
