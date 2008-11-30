#!/usr/bin/env python
'''
dxf_input.py - input a DXF file >= (AutoCAD Release 13 == AC1012)

Copyright (C) 2008 Alvin Penner, penner@vaxxine.com
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
        attribs.update({'sodipodi:linespacing': '125%'})
        node = inkex.etree.SubElement(doc.getroot(), 'text', attribs)
        text = vals[groups['1']][0]
        found = text.find('\P')         # new line
        while found > -1:
            tspan = inkex.etree.SubElement(node , 'tspan', {'sodipodi:role': 'line'})
            tspan.text = text[:found]
            text = text[(found+2):]
            found = text.find('\P')
        tspan = inkex.etree.SubElement(node , 'tspan', {'sodipodi:role': 'line'})
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
        inkex.etree.SubElement(doc.getroot(), 'path', attribs)

def export_SPLINE():
    # mandatory group codes : (10, 20, 70) (x, y, flags)
    if vals[groups['10']] and vals[groups['20']] and vals[groups['70']]:
        if vals[groups['70']][0] == 8 and len(vals[groups['10']]) == 4 and len(vals[groups['20']]) == 4:
            path = 'M %f,%f C %f,%f %f,%f %f,%f' % (vals[groups['10']][0], vals[groups['20']][0], vals[groups['10']][1], vals[groups['20']][1], vals[groups['10']][2], vals[groups['20']][2], vals[groups['10']][3], vals[groups['20']][3])
            attribs = {'d': path, 'style': style}
            inkex.etree.SubElement(doc.getroot(), 'path', attribs)

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
            inkex.etree.SubElement(doc.getroot(), 'path', attribs)

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
            inkex.etree.SubElement(doc.getroot(), 'path', attribs)

def generate_ellipse(xc, yc, xm, ym, w, a1, a2):
    rm = math.sqrt(xm*xm + ym*ym)
    a = math.atan(ym/xm)
    if xm < 0:
        a += math.pi
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
    inkex.etree.SubElement(doc.getroot(), 'path', attribs)

def get_line():
    return (stream.readline().strip(), stream.readline().strip())

def get_group(group):
    line = get_line()
    if line[0] == group:
        return float(line[1])
    else:
        return 0.0

#   define DXF Entities and specify which Group Codes to monitor

entities = {'MTEXT': export_MTEXT, 'TEXT': export_MTEXT, 'POINT': export_POINT, 'LINE': export_LINE, 'SPLINE': export_SPLINE, 'CIRCLE': export_CIRCLE, 'ARC': export_ARC, 'ELLIPSE': export_ELLIPSE, 'LEADER': export_LEADER, 'LWPOLYLINE': export_LWPOLYLINE, 'ENDSEC': ''}
groups = {'1': 0, '8': 1, '10': 2, '11': 3, '20': 4, '21': 5, '40': 6, '41': 7, '42': 8, '50': 9, '51': 10, '62': 11, '70': 12, '370': 13}
colors = {  1: '#FF0000',   2: '#FFFF00',   3: '#00FF00',   4: '#00FFFF',   5: '#0000FF',
            6: '#FF00FF',   8: '#414141',   9: '#808080',  30: '#FF7F00',
          250: '#333333', 251: '#505050', 252: '#696969', 253: '#828282', 254: '#BEBEBE', 255: '#FFFFFF'}

doc = inkex.etree.parse(StringIO('<svg xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"></svg>'))
stream = open(inkex.sys.argv[1], 'r')
xmax = xmin = 0.0
ymax = 297.0                                        # default A4 height in mm
line = get_line()
flag = 0
layers = {}                                         # store colors by layer
while line[0] and line[1] != 'ENTITIES':
    line = get_line()
    if line[1] == '$EXTMIN':
        xmin = get_group('10')
    if line[1] == '$EXTMAX':
        xmax = get_group('10')
        ymax = get_group('20')
    if line[0] == '2':
        name = line[1]
    if line[0] == '2' and line[1] == 'LAYER':
        flag = 1
    if flag and line[0] == '62':
        layers[name] = int(line[1])
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
        elif line[0] == '62' or line[0] == '70':    # unscaled integer value
            val = int(line[1])
        elif line[0] == '10':                       # scaled float x value
            val = scale*(float(line[1]) - xmin)
        elif line[0] == '20':                       # scaled float y value
            val = - scale*(float(line[1]) - ymax)
        else:                                       # unscaled float value
            val = float(line[1])
        vals[groups[line[0]]].append(val)
    elif entities.has_key(line[1]):
        if entities.has_key(entity):
            color = '#000000'                       # default color
            if vals[groups['8']]:                   # Common Layer Name
                if layers.has_key(vals[groups['8']][0]):
                    if colors.has_key(layers[vals[groups['8']][0]]):
                        color = colors[layers[vals[groups['8']][0]]]
            if vals[groups['62']]:                  # Common Color Number
                if colors.has_key(vals[groups['62']][0]):
                    color = colors[vals[groups['62']][0]]
            style = simplestyle.formatStyle({'stroke': '%s' % color, 'fill': 'none'})
            w = 0.5                                 # default lineweight for POINT
            if vals[groups['370']]:                 # Common Lineweight
                if vals[groups['370']][0] > 0:
                    w = scale*vals[groups['370']][0]/100.0
                    style = simplestyle.formatStyle({'stroke': '%s' % color, 'fill': 'none', 'stroke-width': '%.1f' % w})
            entities[entity]()
        entity = line[1]
        vals = [[],[],[],[],[],[],[],[],[],[],[],[],[],[]]
        seqs = []

doc.write(inkex.sys.stdout)

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 encoding=utf-8 textwidth=99
