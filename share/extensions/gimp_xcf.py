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
import inkex
import sys, os, tempfile

class MyEffect(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
    def output(self):
        pass
    def effect(self):
        svg_file = self.args[-1]
        docname = self.xpathSingle('/svg:svg/@sodipodi:docname')[:-4]

        #create os temp dir
        tmp_dir = tempfile.mkdtemp()

        area = '--export-area-canvas'
        pngs = []
        names = []
        path = "/svg:svg/*[name()='g' or @style][@id]"
        for node in self.document.xpath(path, namespaces=inkex.NSS):
            id = node.get('id')
            name = "%s.png" % id
            filename = os.path.join(tmp_dir, name)
            command = "inkscape -i %s -j %s -e %s %s " % (id, area, filename, svg_file)
            _,f,err = os.popen3(command,'r')
            f.read()
            f.close()
            err.close()
            pngs.append(filename)
            names.append(id)

        filelist = '"%s"' % '" "'.join(pngs)
        namelist = '"%s"' % '" "'.join(names)
        xcf = os.path.join(tmp_dir, "%s.xcf" % docname)
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
  (gimp-image-undo-disable img)
  (for-each
    (lambda (names)
      (png-to-layer img (car names) (cdr names))
    )
    (map cons '(%s) '(%s))
  )
  (gimp-image-resize-to-layers img)
  (gimp-image-undo-enable img)
  (gimp-file-save RUN-NONINTERACTIVE img (car (gimp-image-get-active-layer img)) "%s" "%s"))
(gimp-quit 0)
        """ % (filelist, namelist, xcf, xcf)

        junk = os.path.join(tmp_dir, 'junk_from_gimp.txt')
        f = os.popen('gimp -i --batch-interpreter plug-in-script-fu-eval -b - > %s 2>&1' % junk,'w')
        f.write(script_fu)
        f.close()
        # uncomment these lines to see the output from gimp
        #err = open(junk, 'r')
        #inkex.debug(err.read())
        #err.close()

        x = open(xcf, 'r')
        sys.stdout.write(x.read())
        x.close()

e = MyEffect()
e.affect()
