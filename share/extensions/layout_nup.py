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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
'''
import inkex
import sys

try:
    import xml.etree.ElementTree as ElementTree
except:
    try:
        from lxml import etree as ElementTree
    except:
        try:
            from elementtree.ElementTree import ElementTree
        except:
            sys.stderr.write("""Requires ElementTree module, included
in Python 2.5 or supplied by lxml or elementtree modules.

""")

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
        self.pf = self.GenerateNup(
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
        
    def expandTuple(self, unit, x, length = 4):
        try:
            iter(x)
        except:
            return None

        if len(x) != length: x = x*2
        if len(x) != length:
            raise Exception("expandTuple: requires 2 or 4 item tuple")
        try:
            return tuple(map(lambda ev: (self.unittouu(str(eval(str(ev)))+unit)/self.unittouu('1px')), x))
        except:
            return None

    def GenerateNup(self,
                    unit="px",
                    pgSize=("8.5*96","11*96"),
                    pgMargin=(0,0),
                    pgPadding=(0,0),
                    num=(2,2),
                    calculateSize=True,
                    size=None,
                    margin=(0,0),
                    padding=(20,20),
                    show=['default'],
                    container='svg',
                    returnTree = False,
                    ):
        """Generate the SVG.  Inputs are run through 'eval(str(x))' so you can use
    '8.5*72' instead of 612.  Margin / padding dimension tuples can be
    (top & bottom, left & right) or (top, right, bottom, left).

    Keyword arguments:
    pgSize -- page size, width x height
    pgMargin -- extra space around each page
    pgPadding -- added to pgMargin
    n -- rows x cols
    size -- override calculated size, width x height
    margin -- white space around each piece
    padding -- inner padding for each piece
    show -- list of keywords indicating what to show
            - 'crosses' - cutting guides
            - 'inner' - inner boundary
            - 'outer' - outer boundary
    container -- 'svg' or 'g'
    returnTree -- whether to return the ElementTree or the string
    """

        if 'default' in show:
            show = set(show).union(['inner', 'innerbox', 'holder', 'crosses'])

        pgMargin = self.expandTuple(unit, pgMargin)
        pgPadding = self.expandTuple(unit, pgPadding)
        margin = self.expandTuple(unit, margin)
        padding = self.expandTuple(unit, padding)

        pgSize = self.expandTuple(unit, pgSize, length = 2)
    #    num = tuple(map(lambda ev: eval(str(ev)), num))

        pgEdge = map(sum,zip(pgMargin, pgPadding))

        top, right, bottom, left = 0,1,2,3
        width, height = 0,1
        rows, cols = 0,1
        size = self.expandTuple(unit, size, length = 2)
        if size == None or calculateSize == True or len(size) < 2 or size[0] == 0 or size[1] == 0:
            size = ((pgSize[width]
                     - pgEdge[left] - pgEdge[right]
                     - num[cols]*(margin[left] + margin[right])) / num[cols],
                    (pgSize[height]
                     - pgEdge[top] - pgEdge[bottom]
                     - num[rows]*(margin[top] + margin[bottom])) / num[rows]
            )
        else:
            size = self.expandTuple(unit, size, length = 2)

        # sep is separation between same points on pieces
        sep = (size[width]+margin[right]+margin[left],
               size[height]+margin[top]+margin[bottom])

        style = 'stroke:#000000;stroke-opacity:1;fill:none;fill-opacity:1;'

        padbox = 'rect', {
                'x': str(pgEdge[left] + margin[left] + padding[left]),
                'y': str(pgEdge[top] + margin[top] + padding[top]),
                'width': str(size[width] - padding[left] - padding[right]),
                'height': str(size[height] - padding[top] - padding[bottom]),
                'style': style,
                }
        margbox = 'rect', {
                'x': str(pgEdge[left] + margin[left]),
                'y': str(pgEdge[top] + margin[top]),
                'width': str(size[width]),
                'height': str(size[height]),
                'style': style,
                }

        doc = ElementTree.ElementTree(ElementTree.Element(container,
            {'xmlns:inkscape':"http://www.inkscape.org/namespaces/inkscape",
             'xmlns:xlink':"http://www.w3.org/1999/xlink",
             'width':str(pgSize[width]),
             'height':str(pgSize[height]),
             }))

        sub = ElementTree.SubElement

        root = doc.getroot()

        def makeClones(under, to):
            for r in range(0,num[rows]):
                for c in range(0,num[cols]):
                    if r == 0 and c == 0: continue
                    sub(under, 'use', {
                        'xlink:href': '#' + to,
                        'transform': 'translate(%f,%f)' %
                        (c*sep[width], r*sep[height])})

        # guidelayer #####################################################
        if set(['inner', 'outer']).intersection(show):
            layer = sub(root, 'g', {'id':'guidelayer',
                                    'inkscape:groupmode':'layer'})
            if 'inner' in show:
                padbox[1]['id'] = 'innerguide'
                padbox[1]['style'] = padbox[1]['style'].replace('stroke:#000000',
                                                                'stroke:#8080ff')
                sub(layer, *padbox)
                del padbox[1]['id']
                padbox[1]['style'] = padbox[1]['style'].replace('stroke:#8080ff',
                                                                'stroke:#000000')
                makeClones(layer, 'innerguide')
            if 'outer' in show:
                margbox[1]['id'] = 'outerguide'
                margbox[1]['style'] = padbox[1]['style'].replace('stroke:#000000',
                                                                 'stroke:#8080ff')
                sub(layer, *margbox)
                del margbox[1]['id']
                margbox[1]['style'] = padbox[1]['style'].replace('stroke:#8080ff',
                                                                 'stroke:#000000')
                makeClones(layer, 'outerguide')

        # crosslayer #####################################################
        if set(['crosses']).intersection(show):
            layer = sub(root, 'g', {'id':'cutlayer',
                                    'inkscape:groupmode':'layer'})

            if 'crosses' in show:
                crosslen = 12
                group = sub(layer, 'g', id='cross')
                x,y = 0,0
                path = 'M%f %f' % (x+pgEdge[left] + margin[left],
                                   y+pgEdge[top] + margin[top]-crosslen)
                path += ' L%f %f' % (x+pgEdge[left] + margin[left],
                                     y+pgEdge[top] + margin[top]+crosslen)
                path += ' M%f %f' % (x+pgEdge[left] + margin[left]-crosslen,
                                     y+pgEdge[top] + margin[top])
                path += ' L%f %f' % (x+pgEdge[left] + margin[left]+crosslen,
                                     y+pgEdge[top] + margin[top])
                sub(group, 'path', style=style+'stroke-width:0.05',
                    d = path, id = 'crossmarker')
                for r in 0, 1:
                    for c in 0, 1:
                        if r or c:
                            x,y = c*size[width], r*size[height]
                            sub(group, 'use', {
                                'xlink:href': '#crossmarker',
                                'transform': 'translate(%f,%f)' %
                                (x,y)})
                makeClones(layer, 'cross')

        # clonelayer #####################################################
        layer = sub(root, 'g', {'id':'clonelayer', 'inkscape:groupmode':'layer'})
        makeClones(layer, 'main')

        # mainlayer ######################################################
        layer = sub(root, 'g', {'id':'mainlayer', 'inkscape:groupmode':'layer'})
        group = sub(layer, 'g', {'id':'main'})

        if 'innerbox' in show:
            sub(group, *padbox)
        if 'outerbox' in show:
            sub(group, *margbox)
        if 'holder' in show:
            x, y = (pgEdge[left] + margin[left] + padding[left],
                    pgEdge[top] + margin[top] + padding[top])
            w, h = (size[width] - padding[left] - padding[right],
                    size[height] - padding[top] - padding[bottom])
            path = 'M%f %f' % (x + w/2., y)
            path += ' L%f %f' % (x + w, y + h/2.)
            path += ' L%f %f' % (x + w/2., y + h)
            path += ' L%f %f' % (x, y + h/2.)
            path += ' Z'
            sub(group, 'path', style=style, d = path)

        if returnTree:
            return doc
        else:
            return ElementTree.tostring(root)

e = Nup()
e.affect()
