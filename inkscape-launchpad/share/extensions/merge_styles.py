#!/usr/bin/env python
#
# Copyright (C) 2014 Martin Owens
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
Merges styles into class based styles and removes.
"""

import inkex
import sys

from collections import defaultdict

class Style(dict):
    """Controls the style/css mechanics for this effect"""
    def __init__(self, attr=None):
        self.weights = defaultdict(int)
        self.total   = []
        if attr:
            self.parse(attr)

    def parse(self, attr):
        for name,value in [ a.split(':',1) for a in attr.split(';') if ':' in a ]:
            self[name.strip()] = value.strip()

    def entries(self):
        return [ "%s:%s;" % (n,v) for (n,v) in self.iteritems() ]

    def to_str(self, sep="\n    "):
        return "    " + "\n    ".join(self.entries())

    def __str__(self):
        return self.to_str()

    def css(self, cls):
        return ".%s {\n%s\n}" % (cls, str(self))

    def remove(self, keys):
        for key in keys:
            self.pop(key, None)

    def add(self, c, el):
        self.total.append( (c, el) )
        for name,value in c.iteritems():
            if not self.has_key(name):
                self[name] = value
            if self[name] == value:
                self.weights[name] += 1

    def clean(self, threshold):
        """Removes any elements that aren't the same using a weighted threshold"""
        for attr in self.keys():
            if self.weights[attr] < len(self.total) - threshold:
                self.pop(attr)

    def all_matches(self):
        """Returns an iter for each added element who's style matches this style"""
        for (c, el) in self.total:
            if self == c:
                yield (c, el)

    def __eq__(self, o):
        """Not equals, prefer to overload 'in' but that doesn't seem possible"""
        for arg in self.keys():
            if o.has_key(arg) and self[arg] != o[arg]:
                return False
        return True


def get_styles(document):
    nodes = []
    for node in document.getroot().iterchildren():
        if node.tag == inkex.addNS('style', 'svg'):
            return node
        nodes.append(node)
    ret = inkex.etree.SubElement(document.getroot(), 'style', {})
    # Reorder to make the style element FIRST
    for node in nodes:
        document.getroot().append(node)
    return ret


class MergeStyles(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-n", "--name",
                        action="store", type="string",
                        dest="name", default='',
                        help="Name of selected element's common class")

    def effect(self):
        newclass = self.options.name
        elements = self.selected.values()
        common = Style()
        threshold = 1

        for el in elements:
            common.add(Style(el.attrib['style']), el)
        common.clean(threshold)

        if not common:
            raise KeyError("There are no common styles between these elements.")

        styles = get_styles(self.document)
        styles.text = (styles.text or "") + "\n" + common.css( newclass )

        for (st, el) in common.all_matches():
            st.remove(common.keys())
            el.attrib['style'] = st.to_str("")
            
            olds = el.attrib.has_key('class') and el.attrib['class'].split() or []
            if newclass not in olds:
                olds.append(newclass)
            el.attrib['class'] = ' '.join(olds)


if __name__ == '__main__':
        e = MergeStyles()
        e.affect()

