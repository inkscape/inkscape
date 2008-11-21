#!/usr/bin/env python
'''
dxf_input.py - input a DXF file >= (AutoCAD Release 13 == AC1012)

Copyright (C) 2008 Alvin Penner, penner@vaxxine.com

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
        attribs = {'x': '%f' % x, 'y': '%f' % y, 'style': 'font-size: %dpx' % size}
        angle = 0                       # default angle in degrees
        if vals[groups['50']]:
            angle = vals[groups['50']][0]
            attribs.update({'transform': 'rotate (%f %f %f)' % (-angle, x, y)})
        node = inkex.etree.SubElement(doc.getroot(), 'text', attribs)
        node.text = vals[groups['1']][0]

def export_POINT():
    # mandatory group codes : (10, 20) (x, y)
    if vals[groups['10']] and vals[groups['20']]:
        generate_ellipse(vals[groups['10']][0], vals[groups['20']][0], w/2, 0.0, 1.0, 0.0, 0.0)

def export_LINE():
    # mandatory group codes : (10, 11, 20, 21) (x1, x2, y1, y2)
    if vals[groups['10']] and vals[groups['11']] and vals[groups['20']] and vals[groups['21']]:
        path = 'M %f,%f %f,%f' % (vals[groups['10']][0], vals[groups['20']][0], scale*vals[groups['11']][0], height - scale*vals[groups['21']][0])
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

#   define DXF Entities and specify which Group Codes to monitor

entities = {'MTEXT': export_MTEXT, 'POINT': export_POINT, 'LINE': export_LINE, 'SPLINE': export_SPLINE, 'CIRCLE': export_CIRCLE, 'ARC': export_ARC, 'ELLIPSE': export_ELLIPSE, 'LEADER': export_LEADER, 'LWPOLYLINE': export_LWPOLYLINE, 'ENDSEC': ''}
groups = {'1': 0, '10': 1, '11': 2, '20': 3, '21': 4, '40': 5, '41': 6, '42': 7, '50': 8, '51': 9, '70': 10, '370': 11}
scale = 90.0/25.4                                   # convert from mm to pixels
height = 90.0*11.0                                  # 11 inch height in pixels
entity = ''

doc = inkex.etree.parse(StringIO('<svg></svg>'))
stream = open(inkex.sys.argv[1], 'r')
line = get_line()
while line[0] and line[1] != 'ENTITIES':
    line = get_line()

while line[0] and line[1] != 'ENDSEC':
    line = get_line()
    if entity and groups.has_key(line[0]):
        seqs.append(line[0])                        # list of group codes
        if line[0] == '1':                          # text value
            val = line[1].replace('\~', ' ')
        elif line[0] == '70':                       # unscaled integer value
            val = int(line[1])
        elif line[0] == '10':                       # scaled float x value
            val = scale*float(line[1])
        elif line[0] == '20':                       # scaled float y value
            val = height - scale*float(line[1])
        else:                                       # unscaled float value
            val = float(line[1])
        vals[groups[line[0]]].append(val)
    elif entities.has_key(line[1]):
        if entities.has_key(entity):
            style = simplestyle.formatStyle({'stroke': '#000000', 'fill': 'none'})
            w = 0.5                                 # default lineweight for POINT
            if vals[groups['370']]:                 # Common Lineweight
                if vals[groups['370']][0] > 0:
                    w = scale*vals[groups['370']][0]/100.0
                    style = simplestyle.formatStyle({'stroke': '#000000', 'fill': 'none', 'stroke-width': '%.1f' % w})
            entities[entity]()
        entity = line[1]
        vals = [[],[],[],[],[],[],[],[],[],[],[],[]]
        seqs = []

doc.write(inkex.sys.stdout)

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 encoding=utf-8 textwidth=99
