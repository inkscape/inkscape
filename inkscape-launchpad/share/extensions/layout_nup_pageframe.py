#!/usr/bin/env python 
#@+leo-ver=4-thin
#@+node:tbrown.20070622094435.1:@thin pageframe.py
"""Create n-up SVG layouts"""

#@+others
#@+node:tbrown.20070622103716:imports
import sys, inkex

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
#@-node:tbrown.20070622103716:imports
#@+node:tbrown.20070622103716.1:expandTuple
def expandTuple(unit, x, length = 4):
    try:
        iter(x)
    except:
        return None

    if len(x) != length: x = x*2
    if len(x) != length:
        raise Exception("expandTuple: requires 2 or 4 item tuple")
    try:
        return tuple(map(lambda ev: (self.unittouu(str(eval(str(ev)))+unit)), x))
    except:
        return None
#@-node:tbrown.20070622103716.1:expandTuple
#@+node:tbrown.20070622103716.2:GenerateNup
def GenerateNup(unit="px",
                pgSize=("8.5*90","11*90"),
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

    pgMargin = expandTuple(unit, pgMargin)
    pgPadding = expandTuple(unit, pgPadding)
    margin = expandTuple(unit, margin)
    padding = expandTuple(unit, padding)

    pgSize = expandTuple(unit, pgSize, length = 2)
#    num = tuple(map(lambda ev: eval(str(ev)), num))

    pgEdge = map(sum,zip(pgMargin, pgPadding))

    top, right, bottom, left = 0,1,2,3
    width, height = 0,1
    rows, cols = 0,1
    size = expandTuple(unit, size, length = 2)
    if size == None or calculateSize == True or len(size) < 2 or size[0] == 0 or size[1] == 0:
        size = ((pgSize[width]
                 - pgEdge[left] - pgEdge[right]
                 - num[cols]*(margin[left] + margin[right])) / num[cols],
                (pgSize[height]
                 - pgEdge[top] - pgEdge[bottom]
                 - num[rows]*(margin[top] + margin[bottom])) / num[rows]
        )
    else:
        size = expandTuple(unit, size, length = 2)

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

if __name__ == '__main__':
    print GenerateNup(num=(10,3), margin=(5,5), show=['default', 'outer'])
#@-node:tbrown.20070622103716.2:GenerateNup
#@-others
#@-node:tbrown.20070622094435.1:@thin pageframe.py
#@-leo

 	  	 
