#!/usr/bin/env python
'''
dxf_input.py - input a DXF file >= (AutoCAD Release 13 == AC1012)

Copyright (C) 2008, 2009 Alvin Penner, penner@vaxxine.com
Copyright (C) 2009 Christian Mayer, inkscape@christianmayer.de
- thanks to Aaron Spike for inkex.py and simplestyle.py
- without which this would not have been possible

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

import inkex, simplestyle, math
from StringIO import StringIO

def export_MTEXT():
    # mandatory group codes : (1, 10, 20) (text, x, y)
    if vals[groups['1']] and vals[groups['10']] and vals[groups['20']]:
        x = vals[groups['10']][0]
        y = vals[groups['20']][0]
        # optional group codes : (40, 50) (text height mm, text angle)
        size = 12                       # default fontsize in px
        if vals[groups['40']]:
            size = scale*vals[groups['40']][0]
        attribs = {'x': '%f' % x, 'y': '%f' % y, 'style': 'font-size: %dpx; fill: %s' % (size, color)}
        angle = 0                       # default angle in degrees
        if vals[groups['50']]:
            angle = vals[groups['50']][0]
            attribs.update({'transform': 'rotate (%f %f %f)' % (-angle, x, y)})
        attribs.update({inkex.addNS('linespacing','sodipodi'): '125%'})
        node = inkex.etree.SubElement(layer, 'text', attribs)
        text = vals[groups['1']][0]
        found = text.find('\P')         # new line
        while found > -1:
            tspan = inkex.etree.SubElement(node , 'tspan', {inkex.addNS('role','sodipodi'): 'line'})
            tspan.text = text[:found]
            text = text[(found+2):]
            found = text.find('\P')
        tspan = inkex.etree.SubElement(node , 'tspan', {inkex.addNS('role','sodipodi'): 'line'})
        tspan.text = text

def export_POINT():
    # mandatory group codes : (10, 20) (x, y)
    if vals[groups['10']] and vals[groups['20']]:
        generate_ellipse(vals[groups['10']][0], vals[groups['20']][0], w/2, 0.0, 1.0, 0.0, 0.0)

def export_LINE():
    # mandatory group codes : (10, 11, 20, 21) (x1, x2, y1, y2)
    if vals[groups['10']] and vals[groups['11']] and vals[groups['20']] and vals[groups['21']]:
        path = 'M %f,%f %f,%f' % (vals[groups['10']][0], vals[groups['20']][0], scale*(vals[groups['11']][0] - xmin), - scale*(vals[groups['21']][0] - ymax))
        attribs = {'d': path, 'style': style}
        inkex.etree.SubElement(layer, 'path', attribs)

def export_SPLINE():
    # mandatory group codes : (10, 20, 70) (x, y, flags)
    if vals[groups['10']] and vals[groups['20']] and vals[groups['70']]:
        if not (vals[groups['70']][0] & 3) and len(vals[groups['10']]) == 4 and len(vals[groups['20']]) == 4:
            path = 'M %f,%f C %f,%f %f,%f %f,%f' % (vals[groups['10']][0], vals[groups['20']][0], vals[groups['10']][1], vals[groups['20']][1], vals[groups['10']][2], vals[groups['20']][2], vals[groups['10']][3], vals[groups['20']][3])
            attribs = {'d': path, 'style': style}
            inkex.etree.SubElement(layer, 'path', attribs)
        if not (vals[groups['70']][0] & 3) and len(vals[groups['10']]) == 3 and len(vals[groups['20']]) == 3:
            path = 'M %f,%f Q %f,%f %f,%f' % (vals[groups['10']][0], vals[groups['20']][0], vals[groups['10']][1], vals[groups['20']][1], vals[groups['10']][2], vals[groups['20']][2])
            attribs = {'d': path, 'style': style}
            inkex.etree.SubElement(layer, 'path', attribs)

def export_CIRCLE():
    # mandatory group codes : (10, 20, 40) (x, y, radius)
    if vals[groups['10']] and vals[groups['20']] and vals[groups['40']]:
        generate_ellipse(vals[groups['10']][0], vals[groups['20']][0], scale*vals[groups['40']][0], 0.0, 1.0, 0.0, 0.0)

def export_ARC():
    # mandatory group codes : (10, 20, 40, 50, 51) (x, y, radius, angle1, angle2)
    if vals[groups['10']] and vals[groups['20']] and vals[groups['40']] and vals[groups['50']] and vals[groups['51']]:
        generate_ellipse(vals[groups['10']][0], vals[groups['20']][0], scale*vals[groups['40']][0], 0.0, 1.0, vals[groups['50']][0]*math.pi/180.0, vals[groups['51']][0]*math.pi/180.0)

def export_ELLIPSE():
    # mandatory group codes : (10, 11, 20, 21, 40, 41, 42) (xc, xm, yc, ym, width ratio, angle1, angle2)
    if vals[groups['10']] and vals[groups['11']] and vals[groups['20']] and vals[groups['21']] and vals[groups['40']] and vals[groups['41']] and vals[groups['42']]:
        generate_ellipse(vals[groups['10']][0], vals[groups['20']][0], scale*vals[groups['11']][0], scale*vals[groups['21']][0], vals[groups['40']][0], vals[groups['41']][0], vals[groups['42']][0])

def export_LEADER():
    # mandatory group codes : (10, 20) (x, y)
    if vals[groups['10']] and vals[groups['20']]:
        if len(vals[groups['10']]) > 1 and len(vals[groups['20']]) == len(vals[groups['10']]):
            path = 'M %f,%f' % (vals[groups['10']][0], vals[groups['20']][0])
            for i in range (1, len(vals[groups['10']])):
                path += ' %f,%f' % (vals[groups['10']][i], vals[groups['20']][i])
            attribs = {'d': path, 'style': style}
            inkex.etree.SubElement(layer, 'path', attribs)

def export_LWPOLYLINE():
    # mandatory group codes : (10, 20, 70) (x, y, flags)
    if vals[groups['10']] and vals[groups['20']] and vals[groups['70']]:
        if len(vals[groups['10']]) > 1 and len(vals[groups['20']]) == len(vals[groups['10']]):
            # optional group codes : (42) (bulge)
            iseqs = 0
            ibulge = 0
            while seqs[iseqs] != '20':
                iseqs += 1
            path = 'M %f,%f' % (vals[groups['10']][0], vals[groups['20']][0])
            xold = vals[groups['10']][0]
            yold = vals[groups['20']][0]
            for i in range (1, len(vals[groups['10']])):
                bulge = 0
                iseqs += 1
                while seqs[iseqs] != '20':
                    if seqs[iseqs] == '42':
                        bulge = vals[groups['42']][ibulge]
                        ibulge += 1
                    iseqs += 1
                if bulge:
                    sweep = 0                   # sweep CCW
                    if bulge < 0:
                        sweep = 1               # sweep CW
                        bulge = -bulge
                    large = 0                   # large-arc-flag
                    if bulge > 1:
                        large = 1
                    r = math.sqrt((vals[groups['10']][i] - xold)**2 + (vals[groups['20']][i] - yold)**2)
                    r = 0.25*r*(bulge + 1.0/bulge)
                    path += ' A %f,%f 0.0 %d %d %f,%f' % (r, r, large, sweep, vals[groups['10']][i], vals[groups['20']][i])
                else:
                    path += ' L %f,%f' % (vals[groups['10']][i], vals[groups['20']][i])
                xold = vals[groups['10']][i]
                yold = vals[groups['20']][i]
            if vals[groups['70']][0] == 1:      # closed path
                path += ' z'
            attribs = {'d': path, 'style': style}
            inkex.etree.SubElement(layer, 'path', attribs)

def export_HATCH():
    # mandatory group codes : (10, 20, 72, 93) (x, y, Edge Type, Number of edges)
    if vals[groups['10']] and vals[groups['20']] and vals[groups['72']] and vals[groups['93']]:
        if len(vals[groups['10']]) > 1 and len(vals[groups['20']]) == len(vals[groups['10']]):
            # optional group codes : (11, 21, 40, 50, 51, 73) (x, y, r, angle1, angle2, CCW)
            i10 = 1    # count start points
            i11 = 0    # count line end points
            i40 = 0    # count circles
            i72 = 0    # count edge type flags
            path = ''
            for i in range (0, len(vals[groups['93']])):
                xc = vals[groups['10']][i10]
                yc = vals[groups['20']][i10]
                if vals[groups['72']][i72] == 2:            # arc
                    rm = scale*vals[groups['40']][i40]
                    a1 = vals[groups['50']][i40]
                    path += 'M %f,%f ' % (xc + rm*math.cos(a1*math.pi/180.0), yc + rm*math.sin(a1*math.pi/180.0))
                else:
                    a1 = 0
                    path += 'M %f,%f ' % (xc, yc)
                for j in range(0, vals[groups['93']][i]):
                    if vals[groups['72']][i72] == 2:        # arc
                        xc = vals[groups['10']][i10]
                        yc = vals[groups['20']][i10]
                        rm = scale*vals[groups['40']][i40]
                        a2 = vals[groups['51']][i40]
                        diff = (a2 - a1 + 360) % (360)
                        sweep = 1 - vals[groups['73']][i40] # sweep CCW
                        large = 0                           # large-arc-flag
                        if diff:
                            path += 'A %f,%f 0.0 %d %d %f,%f ' % (rm, rm, large, sweep, xc + rm*math.cos(a2*math.pi/180.0), yc + rm*math.sin(a2*math.pi/180.0))
                        else:
                            path += 'A %f,%f 0.0 %d %d %f,%f ' % (rm, rm, large, sweep, xc + rm*math.cos((a1+180.0)*math.pi/180.0), yc + rm*math.sin((a1+180.0)*math.pi/180.0))
                            path += 'A %f,%f 0.0 %d %d %f,%f ' % (rm, rm, large, sweep, xc + rm*math.cos(a1*math.pi/180.0), yc + rm*math.sin(a1*math.pi/180.0))
                        i40 += 1
                        i72 += 1
                    elif vals[groups['72']][i72] == 1:      # line
                        path += 'L %f,%f ' % (scale*(vals[groups['11']][i11] - xmin), -scale*(vals[groups['21']][i11] - ymax))
                        i11 += 1
                        i72 += 1
                    elif vals[groups['72']][i72] == 0:      # polyline
                        if j > 0:
                            path += 'L %f,%f ' % (vals[groups['10']][i10], vals[groups['20']][i10])
                        if j == vals[groups['93']][i] - 1:
                            i72 += 1
                    i10 += 1
                path += "z "
            style = simplestyle.formatStyle({'fill': '%s' % color})
            attribs = {'d': path, 'style': style}
            inkex.etree.SubElement(layer, 'path', attribs)

def export_DIMENSION():
    # mandatory group codes : (10, 11, 13, 14, 20, 21, 23, 24) (x1..4, y1..4)
    if vals[groups['10']] and vals[groups['11']] and vals[groups['13']] and vals[groups['14']] and vals[groups['20']] and vals[groups['21']] and vals[groups['23']] and vals[groups['24']]:
        dx = abs(vals[groups['10']][0] - vals[groups['13']][0])
        dy = abs(vals[groups['20']][0] - vals[groups['23']][0])
        if (vals[groups['10']][0] == vals[groups['14']][0]) and dx > 0.00001:
            d = dx/scale
            dy = 0
            path = 'M %f,%f %f,%f' % (vals[groups['10']][0], vals[groups['20']][0], vals[groups['13']][0], vals[groups['20']][0])
        elif (vals[groups['20']][0] == vals[groups['24']][0]) and dy > 0.00001:
            d = dy/scale
            dx = 0
            path = 'M %f,%f %f,%f' % (vals[groups['10']][0], vals[groups['20']][0], vals[groups['10']][0], vals[groups['23']][0])
        else:
            return
        attribs = {'d': path, 'style': style + '; marker-start: url(#DistanceX); marker-end: url(#DistanceX)'}
        inkex.etree.SubElement(layer, 'path', attribs)
        x = scale*(vals[groups['11']][0] - xmin)
        y = - scale*(vals[groups['21']][0] - ymax)
        size = 3                    # default fontsize in px
        attribs = {'x': '%f' % x, 'y': '%f' % y, 'style': 'font-size: %dpx; fill: %s' % (size, color)}
        if dx == 0:
            attribs.update({'transform': 'rotate (%f %f %f)' % (-90, x, y)})
        node = inkex.etree.SubElement(layer, 'text', attribs)
        tspan = inkex.etree.SubElement(node , 'tspan', {inkex.addNS('role','sodipodi'): 'line'})
        tspan.text = '%.2f' % d

def generate_ellipse(xc, yc, xm, ym, w, a1, a2):
    rm = math.sqrt(xm*xm + ym*ym)
    a = math.atan2(ym, xm)
    diff = (a2 - a1 + 2*math.pi) % (2*math.pi)
    if diff:                        # open arc
        large = 0                   # large-arc-flag
        if diff > math.pi:
            large = 1
        xt = rm*math.cos(a1)
        yt = w*rm*math.sin(a1)
        x1 = xt*math.cos(a) - yt*math.sin(a)
        y1 = xt*math.sin(a) + yt*math.cos(a)
        xt = rm*math.cos(a2)
        yt = w*rm*math.sin(a2)
        x2 = xt*math.cos(a) - yt*math.sin(a)
        y2 = xt*math.sin(a) + yt*math.cos(a)
        path = 'M %f,%f A %f,%f %f %d 0 %f,%f' % (xc+x1, yc-y1, rm, w*rm, -180.0*a/math.pi, large, xc+x2, yc-y2)
    else:                           # closed arc
        path = 'M %f,%f A %f,%f %f 1 0 %f,%f %f,%f %f 1 0 %f,%f z' % (xc+xm, yc-ym, rm, w*rm, -180.0*a/math.pi, xc-xm, yc+ym, rm, w*rm, -180.0*a/math.pi, xc+xm, yc-ym)
    attribs = {'d': path, 'style': style}
    inkex.etree.SubElement(layer, 'path', attribs)

def get_line():
    return (stream.readline().strip(), stream.readline().strip())

def get_group(group):
    line = get_line()
    if line[0] == group:
        return float(line[1])
    else:
        return 0.0

#   define DXF Entities and specify which Group Codes to monitor

entities = {'MTEXT': export_MTEXT, 'TEXT': export_MTEXT, 'POINT': export_POINT, 'LINE': export_LINE, 'SPLINE': export_SPLINE, 'CIRCLE': export_CIRCLE, 'ARC': export_ARC, 'ELLIPSE': export_ELLIPSE, 'LEADER': export_LEADER, 'LWPOLYLINE': export_LWPOLYLINE, 'HATCH': export_HATCH, 'DIMENSION': export_DIMENSION, 'ENDSEC': ''}
groups = {'1': 0, '8': 1, '10': 2, '11': 3, '13': 4, '14': 5, '20': 6, '21': 7, '23': 8, '24': 9, '40': 10, '41': 11, '42': 12, '50': 13, '51': 14, '62': 15, '70': 16, '72': 17, '73': 18, '93': 19, '370': 20}
colors = {  1: '#FF0000',   2: '#FFFF00',   3: '#00FF00',   4: '#00FFFF',   5: '#0000FF',
            6: '#FF00FF',   8: '#414141',   9: '#808080',  30: '#FF7F00',
          250: '#333333', 251: '#505050', 252: '#696969', 253: '#828282', 254: '#BEBEBE', 255: '#FFFFFF'}

doc = inkex.etree.parse(StringIO('<svg xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"></svg>'))
defs = inkex.etree.SubElement(doc.getroot(), 'defs', {} )
marker = inkex.etree.SubElement(defs, 'marker', {'id': 'DistanceX', 'orient': 'auto', 'refX': '0.0', 'refY': '0.0', 'style': 'overflow:visible'})
inkex.etree.SubElement(marker, 'path', {'d': 'M 3,-3 L -3,3 M 0,-5 L  0,5', 'style': 'stroke:#000000; stroke-width:0.5'})
stream = open(inkex.sys.argv[1], 'r')
xmax = xmin = 0.0
ymax = 297.0                                        # default A4 height in mm
line = get_line()
flag = 0
layer_colors = {}                                   # store colors by layer
layer_nodes = {}                                    # store nodes by layer
while line[0] and line[1] != 'ENTITIES':
    line = get_line()
    if line[1] == '$EXTMIN':
        xmin = get_group('10')
    if line[1] == '$EXTMAX':
        xmax = get_group('10')
        ymax = get_group('20')
    if flag and line[0] == '2':
        name = unicode(line[1], "iso-8859-1")
        attribs = {inkex.addNS('groupmode','inkscape'): 'layer', inkex.addNS('label','inkscape'): '%s' % name}
        layer_nodes[name] = inkex.etree.SubElement(doc.getroot(), 'g', attribs)
    if line[0] == '2' and line[1] == 'LAYER':
        flag = 1
    if flag and line[0] == '62':
        layer_colors[name] = int(line[1])
    if line[0] == '0' and line[1] == 'ENDTAB':
        flag = 0

scale = 90.0/25.4                                   # default convert from mm to pixels
if xmax > xmin:
    scale *= 210.0/(xmax - xmin)                    # scale to A4 width
entity = ''
while line[0] and line[1] != 'ENDSEC':
    line = get_line()
    if entity and groups.has_key(line[0]):
        seqs.append(line[0])                        # list of group codes
        if line[0] == '1' or line[0] == '8':        # text value
            val = line[1].replace('\~', ' ')
            val = inkex.re.sub( '\\\\A.*;', '', val)
            val = inkex.re.sub( '\\\\H.*;', '', val)
            val = inkex.re.sub( '\\\\S.*;', '', val)
            val = inkex.re.sub( '\\\\W.*;', '', val)
            val = unicode(val, "iso-8859-1")
        elif line[0] == '62' or line[0] == '70' or line[0] == '93':
            val = int(line[1])
        elif line[0] == '10' or line[0] == '13' or line[0] == '14': # scaled float x value
            val = scale*(float(line[1]) - xmin)
        elif line[0] == '20' or line[0] == '23' or line[0] == '24': # scaled float y value
            val = - scale*(float(line[1]) - ymax)
        else:                                       # unscaled float value
            val = float(line[1])
        vals[groups[line[0]]].append(val)
    elif entities.has_key(line[1]):
        if entities.has_key(entity):
            color = '#000000'                       # default color
            if vals[groups['8']]:                   # Common Layer Name
                layer = layer_nodes[vals[groups['8']][0]]
                if layer_colors.has_key(vals[groups['8']][0]):
                    if colors.has_key(layer_colors[vals[groups['8']][0]]):
                        color = colors[layer_colors[vals[groups['8']][0]]]
            if vals[groups['62']]:                  # Common Color Number
                if colors.has_key(vals[groups['62']][0]):
                    color = colors[vals[groups['62']][0]]
            style = simplestyle.formatStyle({'stroke': '%s' % color, 'fill': 'none'})
            w = 0.5                                 # default lineweight for POINT
            if vals[groups['370']]:                 # Common Lineweight
                if vals[groups['370']][0] > 0:
                    w = 90.0/25.4*vals[groups['370']][0]/100.0
                    if w < 0.5:
                        w = 0.5
                    style = simplestyle.formatStyle({'stroke': '%s' % color, 'fill': 'none', 'stroke-width': '%.1f' % w})
            entities[entity]()
        entity = line[1]
        vals = [[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[]]
        seqs = []

doc.write(inkex.sys.stdout)

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 encoding=utf-8 textwidth=99
