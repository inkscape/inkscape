#!/usr/bin/env python 
'''
Copyright (C) 2006 Aaron Spike, aaron@ekips.org
Copyright (C) 2010-2012 Nicolas Dufour, nicoduf@yahoo.fr
(Windows support and various fixes)

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
import os
import shutil
from subprocess import Popen, PIPE
import sys
import tempfile
# local library
import inkex

# Define extension exceptions
class GimpXCFError(Exception): pass

class GimpXCFExpectedIOError(GimpXCFError): pass

class GimpXCFInkscapeNotInstalled(GimpXCFError):
    def __init__(self):
        inkex.errormsg(_('Inkscape must be installed and set in your path variable.'))

class GimpXCFGimpNotInstalled(GimpXCFError):
    def __init__(self):
        inkex.errormsg(_('Gimp must be installed and set in your path variable.'))

class GimpXCFScriptFuError(GimpXCFError):
    def __init__(self):
        inkex.errormsg(_('An error occurred while processing the XCF file.'))


class MyEffect(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--tab",
                                     action="store", type="string",
                                     dest="tab")
        self.OptionParser.add_option("-d", "--guides",
                                     action="store", type="inkbool",
                                     dest="saveGuides", default=False,
                                     help="Save the Guides with the .XCF")
        self.OptionParser.add_option("-r", "--grid",
                                     action="store", type="inkbool",
                                     dest="saveGrid", default=False,
                                     help="Save the Grid with the .XCF")
        self.OptionParser.add_option("-b", "--background",
                                     action="store", type="inkbool",
                                     dest="layerBackground", default=False,
                                     help="Add background color to each layer")
        self.OptionParser.add_option("-i", "--dpi",
                                     action="store", type="string",
                                     dest="resolution", default="96",
                                     help="File resolution")

    def output(self):
        pass

    def clear_tmp(self):
        shutil.rmtree(self.tmp_dir)

    def effect(self):
        svg_file = self.args[-1]
        ttmp_orig = self.document.getroot()
        docname = ttmp_orig.get(inkex.addNS('docname',u'sodipodi'))
        if docname is None: docname = self.args[-1]

        pageHeight = int(self.unittouu(self.xpathSingle('/svg:svg/@height').split('.')[0]))
        pageWidth = int(self.unittouu(self.xpathSingle('/svg:svg/@width').split('.')[0]))

        # Create os temp dir (to store exported pngs and Gimp log file)
        self.tmp_dir = tempfile.mkdtemp()

        # Guides
        hGuides = []
        vGuides = []
        if self.options.saveGuides:
            # Grab all guide tags in the namedview tag
            guideXpath = "sodipodi:namedview/sodipodi:guide"
            for guideNode in self.document.xpath(guideXpath, namespaces=inkex.NSS):
                ori = guideNode.get('orientation')
                if  ori == '0,1':
                    # This is a horizontal guide
                    pos = int(guideNode.get('position').split(',')[1].split('.')[0])
                    # GIMP doesn't like guides that are outside of the image
                    if pos > 0 and pos < pageHeight:
                        # The origin is at the top in GIMP land
                        hGuides.append(str(pageHeight - pos))
                elif ori == '1,0':
                    # This is a vertical guide
                    pos = int(guideNode.get('position').split(',')[0].split('.')[0])
                    # GIMP doesn't like guides that are outside of the image
                    if pos > 0 and pos < pageWidth:
                        vGuides.append(str(pos))

        hGList = ' '.join(hGuides)
        vGList = ' '.join(vGuides)

        # Grid
        gridSpacingFunc = ''
        gridOriginFunc = '' 
        # GIMP only allows one rectangular grid
        gridXpath = "sodipodi:namedview/inkscape:grid[@type='xygrid' and (not(@units) or @units='px')]"
        if (self.options.saveGrid and self.document.xpath(gridXpath, namespaces=inkex.NSS)):
            gridNode = self.xpathSingle(gridXpath)
            if gridNode != None:
                # These attributes could be nonexistant
                spacingX = gridNode.get('spacingx')
                if spacingX == None: spacingX = '1  '

                spacingY = gridNode.get('spacingy')
                if spacingY == None: spacingY = '1  '

                originX = gridNode.get('originx')
                if originX == None: originX = '0  '

                originY = gridNode.get('originy')
                if originY == None: originY = '0  '

                gridSpacingFunc = '(gimp-image-grid-set-spacing img %s %s)' % (spacingX[:-2], spacingY[:-2])
                gridOriginFunc = '(gimp-image-grid-set-offset img %s %s)'% (originX[:-2], originY[:-2])

        # Layers
        area = '--export-area-page'
        opacity = '--export-background-opacity='
        resolution = '--export-dpi=' + self.options.resolution
        
        if self.options.layerBackground:
            opacity += "1"
        else:
            opacity += "0"
        pngs = []
        names = []
        self.valid = 0
        path = "/svg:svg/*[name()='g' or @style][@id]"
        for node in self.document.xpath(path, namespaces=inkex.NSS):
            if len(node) > 0: # Get rid of empty layers
                self.valid = 1
                id = node.get('id')
                if node.get("{" + inkex.NSS["inkscape"] + "}label"):
                    name = node.get("{" + inkex.NSS["inkscape"] + "}label")
                else:
                    name = id
                filename = os.path.join(self.tmp_dir, "%s.png" % id)
                command = "inkscape -i \"%s\" -j %s %s -e \"%s\" %s %s" % (id, area, opacity, filename, svg_file, resolution)
               
                p = Popen(command, shell=True, stdin=PIPE, stdout=PIPE, stderr=PIPE)
                return_code = p.wait()
                f = p.stdout
                err = p.stderr
                stdin = p.stdin
                f.read()
                f.close()
                err.close()
                stdin.close()

                if return_code != 0:
                    self.clear_tmp()
                    raise GimpXCFInkscapeNotInstalled

                if os.name == 'nt':
                    filename = filename.replace("\\", "/")
                pngs.append(filename)
                names.append(name)

        if (self.valid == 0):
            self.clear_tmp()
            inkex.errormsg(_('This extension requires at least one non empty layer.'))
        else:
            filelist = '"%s"' % '" "'.join(pngs)
            namelist = '"%s"' % '" "'.join(names)
            xcf = os.path.join(self.tmp_dir, "%s.xcf" % docname)
            if os.name == 'nt':
                xcf = xcf.replace("\\", "/")
            script_fu = """
(tracing 1)
(define
  (png-to-layer img png_filename layer_name)
  (let*
    (
      (png (car (file-png-load RUN-NONINTERACTIVE png_filename png_filename)))
      (png_layer (car (gimp-image-get-active-layer png)))
      (xcf_layer (car (gimp-layer-new-from-drawable png_layer img)))
    )
    (gimp-image-add-layer img xcf_layer -1)
    (gimp-drawable-set-name xcf_layer layer_name)
  )
)
(let*
  (
    (img (car (gimp-image-new 200 200 RGB)))
  )
  (gimp-image-set-resolution img %s %s)
  (gimp-image-undo-disable img)
  (for-each
    (lambda (names)
      (png-to-layer img (car names) (cdr names))
    )
    (map cons '(%s) '(%s))
  )

  (gimp-image-resize-to-layers img)

  (for-each
    (lambda (hGuide)
      (gimp-image-add-hguide img hGuide)
    )
    '(%s)
  )

  (for-each
    (lambda (vGuide)
      (gimp-image-add-vguide img vGuide)
    )
    '(%s)
  )

  %s
  %s

  (gimp-image-undo-enable img)
  (gimp-file-save RUN-NONINTERACTIVE img (car (gimp-image-get-active-layer img)) "%s" "%s"))
(gimp-quit 0)
            """ % (self.options.resolution, self.options.resolution, filelist, namelist, hGList, vGList, gridSpacingFunc, gridOriginFunc, xcf, xcf)

            junk = os.path.join(self.tmp_dir, 'junk_from_gimp.txt')
            command = 'gimp -i --batch-interpreter plug-in-script-fu-eval -b - > %s 2>&1' % junk

            p = Popen(command, shell=True, stdin=PIPE, stdout=PIPE, stderr=PIPE)
            f = p.stdin
            out = p.stdout
            err = p.stderr
            f.write(script_fu.encode('utf-8'))
            return_code = p.wait()
            
            if p.returncode != 0:
                self.clear_tmp()
                raise GimpXCFGimpNotInstalled

            f.close()
            err.close()
            out.close()
            # Uncomment these lines to see the output from gimp
            #err = open(junk, 'r')
            #inkex.debug(err.read())
            #err.close()

            try:
                x = open(xcf, 'rb')
            except:
                self.clear_tmp()
                raise GimpXCFScriptFuError

            if os.name == 'nt':
                try:
                    import msvcrt
                    msvcrt.setmode(1, os.O_BINARY)
                except:
                    pass
            try:
                sys.stdout.write(x.read())
            finally:
                x.close()
                self.clear_tmp()


if __name__ == '__main__':
    e = MyEffect()
    e.affect()

