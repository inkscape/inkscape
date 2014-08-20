#!/usr/bin/env python 
'''
Copyright (C) 2007 Terry Brown, terry_n_brown@yahoo.com

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
import sys

import layout_nup_pageframe

class Nup(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        opts = [('', '--unit', 'string', 'unit', 'px', ''),
                ('', '--rows', 'int', 'rows', '2', ''),
                ('', '--cols', 'int', 'cols', '2', ''),
                ('', '--paddingTop', 'string', 'paddingTop', '', ''),
                ('', '--paddingBottom', 'string', 'paddingBottom', '', ''),
                ('', '--paddingLeft', 'string', 'paddingLeft', '', ''),
                ('', '--paddingRight', 'string', 'paddingRight', '', ''),
                ('', '--marginTop', 'string', 'marginTop', '', ''),
                ('', '--marginBottom', 'string', 'marginBottom', '', ''),
                ('', '--marginLeft', 'string', 'marginLeft', '', ''),
                ('', '--marginRight', 'string', 'marginRight', '', ''),
                ('', '--pgSizeX', 'string', 'pgSizeX', '', ''),
                ('', '--pgSizeY', 'string', 'pgSizeY', '', ''),
                ('', '--sizeX', 'string', 'sizeX', '', ''),
                ('', '--sizeY', 'string', 'sizeY', '', ''),
                ('', '--calculateSize', 'inkbool', 'calculateSize', True, ''),
                ('', '--pgMarginTop', 'string', 'pgMarginTop', '', ''),
                ('', '--pgMarginBottom', 'string', 'pgMarginBottom', '', ''),
                ('', '--pgMarginLeft', 'string', 'pgMarginLeft', '', ''),
                ('', '--pgMarginRight', 'string', 'pgMarginRight', '', ''),
                ('', '--showHolder', 'inkbool', 'showHolder', True, ''),
                ('', '--showCrosses', 'inkbool', 'showCrosses', True, ''),
                ('', '--showInner', 'inkbool', 'showInner', True, ''),
                ('', '--showOuter', 'inkbool', 'showOuter', False, ''),
                ('', '--showInnerBox', 'inkbool', 'showInnerBox', False, ''),
                ('', '--showOuterBox', 'inkbool', 'showOuterBox', False, ''),
                ('', '--tab', 'string', 'tab', '', ''),
                ]
        for o in opts:
            self.OptionParser.add_option(o[0], o[1], action="store", type=o[2],
                                         dest=o[3], default=o[4], help=o[5])


    def effect(self):
        showList = []
        for i in ['showHolder','showCrosses','showInner','showOuter',
                  'showInnerBox','showOuterBox',]:
            if getattr(self.options, i):
                showList.append(i.lower().replace('show', ''))
        o = self.options
        self.pf = layout_nup_pageframe.GenerateNup(
            unit=o.unit,
            pgSize=(o.pgSizeX,o.pgSizeY),
            pgMargin=(o.pgMarginTop,o.pgMarginRight,o.pgMarginBottom,o.pgMarginLeft),
            num=(o.rows,o.cols),
            calculateSize = o.calculateSize,
            size=(o.sizeX,o.sizeY),
            margin=(o.marginTop,o.marginRight,o.marginBottom,o.marginLeft),
            padding=(o.paddingTop,o.paddingRight,o.paddingBottom,o.paddingLeft),
            show=showList,
            )

    def setAttr(self, node, name, value):
        attr = node.ownerDocument.createAttribute(name)
        attr.value = value
        node.attributes.setNamedItem(attr)

    def output(self):
        sys.stdout.write(self.pf)
        
e = Nup()
e.affect()

 	  	 
