# This script produces a sample SVG demonstrating all filters in a filters file.
#
# It takes two inputs: the sample file with the object that will be cloned and filtered, and
# the file with filters (such as Inkscape's share/filters/filters.svg).
#
# Run it thus:
#
#    python samplify.py sample.svg filters.svg > out.svg
#
# It requires 'inkscape' in executable path for dimension queries.

import sys, os, string
from lxml import etree

if len(sys.argv) < 3:
    sys.stderr.write ("Usage: python samplify.py sample.svg filters.svg > out.svg\n")
    sys.exit(1)

# namespaces we need to be aware of
NSS = {
u'sodipodi' :u'http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd',
u'cc'       :u'http://web.resource.org/cc/',
u'svg'      :u'http://www.w3.org/2000/svg',
u'dc'       :u'http://purl.org/dc/elements/1.1/',
u'rdf'      :u'http://www.w3.org/1999/02/22-rdf-syntax-ns#',
u'inkscape' :u'http://www.inkscape.org/namespaces/inkscape',
u'xlink'    :u'http://www.w3.org/1999/xlink',
u'xml'      :u'http://www.w3.org/XML/1998/namespace'
}

# helper function to add namespace URI to a name
def addNS(tag, ns=None):
    val = tag
    if ns!=None and len(ns)>0 and NSS.has_key(ns) and len(tag)>0 and tag[0]!='{':
        val = "{%s}%s" % (NSS[ns], tag)
    return val

# attributes and elements we will use, prepared with their namespace
a_href = addNS('href', 'xlink')
a_menu = addNS('menu', 'inkscape')
a_tooltip = addNS('menu-tooltip', 'inkscape')
a_label = addNS('label', 'inkscape')
e_text = addNS('text', 'svg')
e_tspan = addNS('tspan', 'svg')
e_flowRoot = addNS('flowRoot', 'svg')
e_flowPara = addNS('flowPara', 'svg')
e_flowSpan = addNS('flowSpan', 'svg')
e_g = addNS('g', 'svg')
e_use = addNS('use', 'svg')
e_defs = addNS('defs', 'svg')
e_filter = addNS('filter', 'svg')
e_rect = addNS('rect', 'svg')
e_svg = addNS('svg', 'svg')
e_switch = addNS('switch', 'svg')


tstream = open(sys.argv[1], 'rb')
tdoc = etree.parse(tstream)

fstream = open(sys.argv[2], 'rb')
fdoc = etree.parse(fstream)

menus = []

for defs in fdoc.getroot().getchildren():
    for fi in defs.getchildren():
        if fi.tag == e_filter and fi.attrib[a_menu] not in menus:
            menus.append(fi.attrib[a_menu])

menu_shifts = {}
for m in menus:
    menu_shifts[m] = 0

menus.sort()

#print menus

def copy_element (a):
    b = etree.Element(a.tag, nsmap=NSS)
    for i in a.items():
        b.set(i[0], i[1])
    b.text = a.text
    b.tail = a.tail
    return b

#query inkscape about the bounding box of obj
q = {'x':0,'y':0,'width':0,'height':0}
file = sys.argv[1]
id = tdoc.getroot().attrib["id"]
for query in q.keys():
    f,err = os.popen3('inkscape --query-%s --query-id=%s "%s"' % (query,id,file))[1:]
    q[query] = float(f.read())
    f.close()
    err.close()

# add some margins
q['width'] = q['width'] * 1.3
q['height'] = q['height'] * 1.3

#print q    

root = tdoc.getroot()
tout = etree.ElementTree(copy_element(root))
newroot = tout.getroot()
for ch in root.getchildren():
    chcopy = ch.__deepcopy__(-1)
    newroot.append(chcopy)
    if ch.tag == e_defs:
        for defs in fdoc.getroot().getchildren():
            for fi in defs.getchildren():
                ficopy = fi.__deepcopy__(-1)
                newroot.getchildren()[-1].append(ficopy)
    if ch.tag == e_g:
        newroot.getchildren()[-1].attrib["id"] = "original"
        for menu in menus:
            text = etree.Element(e_text, nsmap=NSS)
            text.attrib['x']=str(q['x'] - q['width'] * 0.2)
            text.attrib['y']=str( q['y'] + q['height'] * (menus.index(menu) + 1.4) )
            text.attrib['style']="font-size:%d;text-anchor:end;" % (q['height']*0.2)
            text.text = menu
            newroot.append(text)
        for defs in fdoc.getroot().getchildren():
            for fi in defs.getchildren():
                if fi.tag != e_filter:
                    continue
                clone = etree.Element(e_use, nsmap=NSS)
                clone.attrib[a_href]='#original'
                clone.attrib["style"]='filter:url(#'+fi.attrib["id"]+')'
                menu = fi.attrib[a_menu]
                clone.attrib["transform"] = 'translate('+str( q['width'] * menu_shifts[menu] )+', '+str( q['height'] * (menus.index(menu) + 1) )+')'
                newroot.append(clone)

                text = etree.Element(e_text, nsmap=NSS)
                text.attrib['x']=str( q['x'] + q['width'] * (menu_shifts[menu] + 0.5) )
                text.attrib['y']=str( q['y'] + q['height'] * (menus.index(menu) + 1.86) )
                text.attrib['style']="font-size:%d;text-anchor:middle;" % (q['height']*0.08)
                text.text = fi.attrib[a_label]
                newroot.append(text)

                if a_tooltip not in fi.keys():
                    print "no menu-tooltip for", fi.attrib["id"]
                    sys.exit()

                text = etree.Element(e_text, nsmap=NSS)
                text.attrib['x']=str( q['x'] + q['width'] * (menu_shifts[menu] + 0.5) )
                text.attrib['y']=str( q['y'] + q['height'] * (menus.index(menu) + 1.92) )
                text.attrib['style']="font-size:%d;text-anchor:middle;" % (q['height']*0.04)
                text.text = fi.attrib[a_tooltip]
                newroot.append(text)

                menu_shifts[menu] = menu_shifts[menu] + 1
        break

total_width = max(menu_shifts.values()) * q['width']    
total_height = (len(menus) + 1) * q['height']
tout.getroot().attrib['width'] = str(total_width)
tout.getroot().attrib['height'] = str(total_height)

print etree.tostring(tout, encoding='UTF-8')

