#!/usr/bin/env python 
'''
Author: Jos Hirth, kaioa.com
License: GNU General Public License - http://www.gnu.org/licenses/gpl.html
Warranty: see above
'''

DOCNAME='sodipodi:docname'

import sys, simplestyle
try:
    from xml.dom.minidom import parse
except:
    sys.exit('The export_gpl.py module requires PyXML. Please download the latest version from <http://pyxml.sourceforge.net/>.')

colortags=(u'fill',u'stroke',u'stop-color',u'flood-color',u'lighting-color')
colors={}

def walk(node):
    checkStyle(node)
    if node.hasChildNodes():
        childs=node.childNodes
        for child in childs:
            walk(child)

def checkStyle(node):
    if node.hasAttributes():
        sa=node.getAttribute('style')
        if sa!='':
            styles=simplestyle.parseStyle(sa)
            for c in range(len(colortags)):
                if colortags[c] in styles.keys():
                    addColor(styles[colortags[c]])

def addColor(col):
    if simplestyle.isColor(col):
        c=simplestyle.parseColor(col)
        colors['%3i %3i %3i ' % (c[0],c[1],c[2])]=simplestyle.formatColoria(c).upper()

stream = open(sys.argv[-1:][0],'r')
dom = parse(stream)
stream.close()
walk(dom)
print 'GIMP Palette\nName: %s\n#' % (dom.getElementsByTagName('svg')[0].getAttribute(DOCNAME).split('.')[0])

for k,v in sorted(colors.items()):
    print k+v