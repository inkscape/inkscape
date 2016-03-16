#!/usr/bin/env python
'''
dm2svg.py - import a DHW file from ACECAD DigiMemo

Copyright (C) 2009 Kevin Lindsey, https://github.com/thelonious/DM2SVG
Copyright (C) 2011 Nikita Kitaev, https://github.com/nikitakit/DM2SVG
Copyright (C) 2011 Chris Morgan,  https://gist.github.com/1471691

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

import struct

def process_file(filename):
    try:
        f = open(filename, 'rb')
    except IOError as e:
        print >> sys.stderr, 'Unable to open %s: %s' % (filename, e)
        return

    with f:
        height = emit_header(f)

        layer = 'layer1'
        timestamp = 0
        svg_element = '<g inkscape:groupmode="layer" id="%s">' % layer

        while True:
            tag = f.read(1)
            if tag == '':
                break

            if ord(tag) > 128:
                if tag == '\x90':
                    # Emit the current element and close the last layer
                    emit_element(svg_element)
                    emit_element('</g>')

                    # Start a new layer
                    layer = 'layer%d' % (ord(f.read(1)) + 1)
                    timestamp = 0
                    svg_element = '<g inkscape:groupmode="layer" id="%s">' % layer
                elif tag == '\x88':
                    # Read the timestamp next
                    timestamp += ord(f.read(1)) * 20
                else:
                    # Emit the current svg element
                    emit_element(svg_element)

                    coords = []

                    # Pen down
                    while True:
                        coords.append(read_point(f, height))
                        if ord(f.read(1)) >= 128:
                            break
                        f.seek(-1, 1)  # It wasn't the magic value, don't miss it

                    # Pen up
                    coords.append(read_point(f, height))
                    points = ' '.join(','.join(map(str, e)) for e in coords)
                    svg_element = '<polyline points="%s" dm:timestamp="%s" />' % (points, timestamp)
            else:
                print >> sys.stderr, 'Unsupported tag: %s\n' % tag

        # Emit the footer to finish it off
        print '\n</svg>\n'


def read_point(f, ymax):
    x1, x2, y1, y2 = map(ord, f.read(4))
    x = x1 | x2 << 7
    y = y1 | y2 << 7

    return x, ymax - y


def emit_header(f):
    id, version, width, height, page_type = struct.unpack('<32sBHHBxx', f.read(40))

    print '''
<svg viewBox="0 0 %(width)s %(height)s" fill="none" stroke="black" stroke-width="10" stroke-linecap="round" stroke-linejoin="round"
  xmlns="http://www.w3.org/2000/svg"
  xmlns:svg="http://www.w3.org/2000/svg"
  xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
  xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
  xmlns:dm="http://github.com/nikitakit/DM2SVG">
    <metadata>
      <dm:page
        id="%(id)s"
        version="%(version)s"
        width="%(width)s"
        height="%(height)s"
        page_type="%(page_type)s">
      </dm:page>
    </metadata>
    <rect width="%(width)s" height="%(height)s" fill="aliceblue"/>
''' % locals()

    return height


def emit_element(message):
    if message:
        print '%s\n' % message


if __name__ == '__main__':
    import sys

    if len(sys.argv) == 2:
        process_file(sys.argv[1])
    else:
        print >> sys.stderr, 'Usage: %s <dhw-file>' % sys.argv[0]
        
# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
