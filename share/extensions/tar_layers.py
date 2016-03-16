#!/usr/bin/env python
#
# Copyright (C) 2014 Martin Owens, email@doctormo.org
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
"""
An extension to export multiple svg files from a single svg file containing layers.

Each defs is duplicated for each svg outputed.
"""
import os
import sys
import copy
import tarfile
import StringIO
import calendar
import time

# Inkscape Libraries
import inkex
import simplestyle

GROUP = "{http://www.w3.org/2000/svg}g"
LABEL = "{http://www.inkscape.org/namespaces/inkscape}label"
GROUPMODE = "{http://www.inkscape.org/namespaces/inkscape}groupmode"


def oprint(item):
    """DEBUG print an object"""
    for name in dir(item):
        value = getattr(item, name)
        if callable(value):
            yield "%s()" % name
        else:
            yield "%s = %s" % (name, str(value))

def fprint(data):
    """DEBUG print something"""
    if isinstance(data, basestring):
        sys.stderr.write("DEBUG: %s\n" % data)
    else:
        return fprint("%s(%s)\n    " % (str(data), type(data).__name__) \
                + "\n    ".join(oprint(data)))


class LayersOutput(inkex.Effect):
    """Entry point to our layers export"""
    def __init__(self):
        inkex.Effect.__init__(self)
        if os.name == 'nt':
            self.encoding = "cp437"
        else:
            self.encoding = "latin-1"

    def make_template(self):
        """Returns the current document as a new empty document with the same defs"""
        newdoc = copy.deepcopy(self.document)
        for (name, layer) in self.layers(newdoc):
            layer.getparent().remove(layer)
        return newdoc

    def layers(self, document):
        for node in document.getroot().iterchildren():
            if self.is_layer(node):
                name = node.attrib.get(LABEL, None)
                if name:
                    yield (name, node)

    def is_layer(self, node):
        return node.tag == GROUP and node.attrib.get(GROUPMODE,'').lower() == 'layer'

    def io_document(self, name, doc):
        string = StringIO.StringIO()
        doc.write(string)
        string.seek(0)
        info = tarfile.TarInfo(name=name+'.svg')
        info.mtime = calendar.timegm(time.gmtime())
        info.size  = len(string.buf)
        return dict(tarinfo=info, fileobj=string)

    def effect(self):
        # open output tar file as a stream (to stdout)
        tar = tarfile.open(fileobj=sys.stdout, mode='w|')

        # Switch stdout to binary on Windows.
        if sys.platform == "win32":
            import msvcrt
            msvcrt.setmode(sys.stdout.fileno(), os.O_BINARY)

        template = self.make_template()

        previous = None
        for (name, _layer) in self.layers(self.document):
            layer = copy.deepcopy(_layer)
            if previous != None:
                template.getroot().replace(previous, layer)
            else:
                template.getroot().append(layer)
            previous = layer
            
            tar.addfile(**self.io_document(name, template))


if __name__ == '__main__':   #pragma: no cover
    e = LayersOutput()
    e.affect()

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
