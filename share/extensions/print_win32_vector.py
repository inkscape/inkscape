#!/usr/bin/env python 
'''
print_win32_vector.py
This extension will generate vector graphics printout, specifically for Windows GDI32.

Copyright (C) 2012 Alvin Penner, penner@vaxxine.com

This is a modified version of the file dxf_outlines.py by Aaron Spike, aaron@ekips.org
It will write only to the default printer.
The printing preferences dialog will be called.
In order to ensure a pure vector output, use a linewidth < 1 printer pixel

- see http://www.lessanvaezi.com/changing-printer-settings-using-the-windows-api/
- get GdiPrintSample.zip at http://archive.msdn.microsoft.com/WindowsPrintSample

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
# standard library
from ctypes import *
# local library
import inkex
import simplestyle
import simpletransform
import cubicsuperpath

inkex.localize()

if not inkex.sys.platform.startswith('win'):
    exit(_("sorry, this will run only on Windows, exiting..."))

myspool = WinDLL("winspool.drv")
mygdi = WinDLL("gdi32.dll")
LOGBRUSH = c_long*3
DM_IN_PROMPT = 4                        # call printer property sheet 
DM_OUT_BUFFER = 2                       # write to DEVMODE structure

class MyEffect(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.visibleLayers = True       # print only visible layers

    def process_shape(self, node, mat):
        rgb = (0,0,0)                   # stroke color
        fillcolor = None                # fill color
        stroke = 1                      # pen width in printer pixels
        # Very NB : If the pen width is greater than 1 then the output will Not be a vector output !
        style = node.get('style')
        if style:
            style = simplestyle.parseStyle(style)
            if style.has_key('stroke'):
                if style['stroke'] and style['stroke'] != 'none' and style['stroke'][0:3] != 'url':
                    rgb = simplestyle.parseColor(style['stroke'])
            if style.has_key('stroke-width'):
                stroke = self.unittouu(style['stroke-width'])
                stroke = int(stroke*self.scale)
            if style.has_key('fill'):
                if style['fill'] and style['fill'] != 'none' and style['fill'][0:3] != 'url':
                    fill = simplestyle.parseColor(style['fill'])
                    fillcolor = fill[0] + 256*fill[1] + 256*256*fill[2]
        color = rgb[0] + 256*rgb[1] + 256*256*rgb[2]
        if node.tag == inkex.addNS('path','svg'):
            d = node.get('d')
            if not d:
                return
            p = cubicsuperpath.parsePath(d)
        elif node.tag == inkex.addNS('rect','svg'):
            x = float(node.get('x'))
            y = float(node.get('y'))
            width = float(node.get('width'))
            height = float(node.get('height'))
            p = [[[x, y],[x, y],[x, y]]]
            p.append([[x + width, y],[x + width, y],[x + width, y]])
            p.append([[x + width, y + height],[x + width, y + height],[x + width, y + height]])
            p.append([[x, y + height],[x, y + height],[x, y + height]])
            p.append([[x, y],[x, y],[x, y]])
            p = [p]
        else:
            return
        trans = node.get('transform')
        if trans:
            mat = simpletransform.composeTransform(mat, simpletransform.parseTransform(trans))
        simpletransform.applyTransformToPath(mat, p)
        hPen = mygdi.CreatePen(0, stroke, color)
        mygdi.SelectObject(self.hDC, hPen)
        self.emit_path(p)
        if fillcolor is not None:
            brush = LOGBRUSH(0, fillcolor, 0)
            hBrush = mygdi.CreateBrushIndirect(addressof(brush))
            mygdi.SelectObject(self.hDC, hBrush)
            mygdi.BeginPath(self.hDC)
            self.emit_path(p)
            mygdi.EndPath(self.hDC)
            mygdi.FillPath(self.hDC)
        return

    def emit_path(self, p):
        for sub in p:
            mygdi.MoveToEx(self.hDC, int(sub[0][1][0]), int(sub[0][1][1]), None)
            POINTS = c_long*(6*(len(sub)-1))
            points = POINTS()
            for i in range(len(sub)-1):
                points[6*i]     = int(sub[i][2][0])
                points[6*i + 1] = int(sub[i][2][1])
                points[6*i + 2] = int(sub[i + 1][0][0])
                points[6*i + 3] = int(sub[i + 1][0][1])
                points[6*i + 4] = int(sub[i + 1][1][0])
                points[6*i + 5] = int(sub[i + 1][1][1])
            mygdi.PolyBezierTo(self.hDC, addressof(points), 3*(len(sub)-1))
        return

    def process_clone(self, node):
        trans = node.get('transform')
        x = node.get('x')
        y = node.get('y')
        mat = [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0]]
        if trans:
            mat = simpletransform.composeTransform(mat, simpletransform.parseTransform(trans))
        if x:
            mat = simpletransform.composeTransform(mat, [[1.0, 0.0, float(x)], [0.0, 1.0, 0.0]])
        if y:
            mat = simpletransform.composeTransform(mat, [[1.0, 0.0, 0.0], [0.0, 1.0, float(y)]])
        # push transform
        if trans or x or y:
            self.groupmat.append(simpletransform.composeTransform(self.groupmat[-1], mat))
        # get referenced node
        refid = node.get(inkex.addNS('href','xlink'))
        refnode = self.getElementById(refid[1:])
        if refnode is not None:
            if refnode.tag == inkex.addNS('g','svg'):
                self.process_group(refnode)
            elif refnode.tag == inkex.addNS('use', 'svg'):
                self.process_clone(refnode)
            else:
                self.process_shape(refnode, self.groupmat[-1])
        # pop transform
        if trans or x or y:
            self.groupmat.pop()

    def process_group(self, group):
        if group.get(inkex.addNS('groupmode', 'inkscape')) == 'layer':
            style = group.get('style')
            if style:
                style = simplestyle.parseStyle(style)
                if style.has_key('display'):
                    if style['display'] == 'none' and self.visibleLayers:
                        return
        trans = group.get('transform')
        if trans:
            self.groupmat.append(simpletransform.composeTransform(self.groupmat[-1], simpletransform.parseTransform(trans)))
        for node in group:
            if node.tag == inkex.addNS('g','svg'):
                self.process_group(node)
            elif node.tag == inkex.addNS('use', 'svg'):
                self.process_clone(node)
            else:
                self.process_shape(node, self.groupmat[-1])
        if trans:
            self.groupmat.pop()

    def effect(self):
        pcchBuffer = c_long()
        myspool.GetDefaultPrinterA(None, byref(pcchBuffer))     # get length of printer name
        pname = create_string_buffer(pcchBuffer.value)
        myspool.GetDefaultPrinterA(pname, byref(pcchBuffer))    # get printer name
        hPrinter = c_long()
        if myspool.OpenPrinterA(pname.value, byref(hPrinter), None) == 0:
            exit(_("Failed to open default printer"))

        # get printer properties dialog

        pcchBuffer = myspool.DocumentPropertiesA(0, hPrinter, pname, None, None, 0)
        pDevMode = create_string_buffer(pcchBuffer + 100) # allocate extra just in case
        pcchBuffer = myspool.DocumentPropertiesA(0, hPrinter, pname, byref(pDevMode), None, DM_IN_PROMPT + DM_OUT_BUFFER)
        myspool.ClosePrinter(hPrinter)
        if pcchBuffer != 1:             # user clicked Cancel
            exit()

        # initiallize print document

        docname = self.document.getroot().xpath('@sodipodi:docname', namespaces=inkex.NSS)
        if not docname:
            docname = ['New document 1']
        lpszDocName = create_string_buffer('Inkscape ' + docname[0].split('\\')[-1])
        DOCINFO = c_long*5
        docInfo = DOCINFO(20, addressof(lpszDocName), 0, 0, 0)
        self.hDC = mygdi.CreateDCA(None, pname, None, byref(pDevMode))
        if mygdi.StartDocA(self.hDC, byref(docInfo)) < 0:
            exit()                      # user clicked Cancel

        self.scale = (ord(pDevMode[58]) + 256.0*ord(pDevMode[59]))/90    # use PrintQuality from DEVMODE
        self.groupmat = [[[self.scale, 0.0, 0.0], [0.0, self.scale, 0.0]]]
        doc = self.document.getroot()
        self.process_group(doc)
        mygdi.EndDoc(self.hDC)

if __name__ == '__main__':
    e = MyEffect()
    e.affect()

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
