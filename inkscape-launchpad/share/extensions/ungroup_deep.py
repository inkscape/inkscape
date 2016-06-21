#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
see #inkscape on Freenode and
https://github.com/nikitakit/svg2sif/blob/master/synfig_prepare.py#L370
for an example how to do the transform of parent to children.
"""

__version__ = "0.2"  # Works but in terms of maturity, still unsure

from inkex import addNS
import logging
import simplestyle
import simpletransform
logging.basicConfig(format='%(levelname)s:%(funcName)s:%(message)s',
                    level=logging.INFO)

try:
    import inkex
except ImportError:
    raise ImportError("""No module named inkex in {0}.""".format(__file__))

try:
    from numpy import matrix
except:
    raise ImportError("""Cannot find numpy.matrix in {0}.""".format(__file__))

SVG_NS = "http://www.w3.org/2000/svg"
INKSCAPE_NS = "http://www.inkscape.org/namespaces/inkscape"


class Ungroup(inkex.Effect):

    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-s", "--startdepth",
                                     action="store", type="int",
                                     dest="startdepth", default=0,
                                     help="starting depth for ungrouping")
        self.OptionParser.add_option("-m", "--maxdepth",
                                     action="store", type="int",
                                     dest="maxdepth", default=65535,
                                     help="maximum ungrouping depth")
        self.OptionParser.add_option("-k", "--keepdepth",
                                     action="store", type="int",
                                     dest="keepdepth", default=0,
                                     help="levels of ungrouping to " +
                                     "leave untouched")

    def _get_dimension(s="1024"):
        """Convert an SVG length string from arbitrary units to pixels"""
        if s == "":
            return 0
        try:
            last = int(s[-1])
        except:
            last = None

        if type(last) == int:
            return float(s)
        elif s[-1] == "%":
            return 1024
        elif s[-2:] == "px":
            return float(s[:-2])
        elif s[-2:] == "pt":
            return float(s[:-2]) * 1.33
        elif s[-2:] == "em":
            return float(s[:-2]) * 16
        elif s[-2:] == "mm":
            return float(s[:-2]) * 3.779
        elif s[-2:] == "pc":
            return float(s[:-2]) * 16
        elif s[-2:] == "cm":
            return float(s[:-2]) * 37.79
        elif s[-2:] == "in":
            return float(s[:-2]) * 96
        else:
            return 1024

    def _merge_transform(self, node, transform):
        """Propagate style and transform to remove inheritance
        Originally from
        https://github.com/nikitakit/svg2sif/blob/master/synfig_prepare.py#L370
        """

        # Compose the transformations
        if node.tag == addNS("svg", "svg") and node.get("viewBox"):
            vx, vy, vw, vh = [self._get_dimension(x)
                              for x in node.get("viewBox").split()]
            dw = self._get_dimension(node.get("width", vw))
            dh = self._get_dimension(node.get("height", vh))
            t = ("translate(%f, %f) scale(%f, %f)" %
                (-vx, -vy, dw / vw, dh / vh))
            this_transform = simpletransform.parseTransform(
                t, transform)
            this_transform = simpletransform.parseTransform(
                node.get("transform"), this_transform)
            del node.attrib["viewBox"]
        else:
            this_transform = simpletransform.parseTransform(node.get(
                "transform"), transform)

        # Set the node's transform attrib
        node.set("transform",
                 simpletransform.formatTransform(this_transform))

    def _merge_style(self, node, style):
        """Propagate style and transform to remove inheritance
        Originally from
        https://github.com/nikitakit/svg2sif/blob/master/synfig_prepare.py#L370
        """

        # Compose the style attribs
        this_style = simplestyle.parseStyle(node.get("style", ""))
        remaining_style = {}  # Style attributes that are not propagated

        # Filters should remain on the top ancestor
        non_propagated = ["filter"]
        for key in non_propagated:
            if key in this_style.keys():
                remaining_style[key] = this_style[key]
                del this_style[key]

        # Create a copy of the parent style, and merge this style into it
        parent_style_copy = style.copy()
        parent_style_copy.update(this_style)
        this_style = parent_style_copy

        # Merge in any attributes outside of the style
        style_attribs = ["fill", "stroke"]
        for attrib in style_attribs:
            if node.get(attrib):
                this_style[attrib] = node.get(attrib)
                del node.attrib[attrib]

        if (node.tag == addNS("svg", "svg")
                or node.tag == addNS("g", "svg")
                or node.tag == addNS("a", "svg")
                or node.tag == addNS("switch", "svg")):
            # Leave only non-propagating style attributes
            if len(remaining_style) == 0:
                if "style" in node.keys():
                    del node.attrib["style"]
            else:
                node.set("style", simplestyle.formatStyle(remaining_style))

        else:
            # This element is not a container

            # Merge remaining_style into this_style
            this_style.update(remaining_style)

            # Set the element's style attribs
            node.set("style", simplestyle.formatStyle(this_style))

    def _merge_clippath(self, node, clippathurl):

        if (clippathurl):
            node_transform = simpletransform.parseTransform(
                node.get("transform"))
            if (node_transform):
                # Clip-paths on nodes with a transform have the transform
                # applied to the clipPath as well, which we don't want.  So, we
                # create new clipPath element with references to all existing
                # clippath subelements, but with the inverse transform applied
                inverse_node_transform = simpletransform.formatTransform(
                    self._invert_transform(node_transform))
                new_clippath = inkex.etree.SubElement(
                    self.xpathSingle('//svg:defs'), 'clipPath',
                    {'clipPathUnits': 'userSpaceOnUse',
                     'id': self.uniqueId("clipPath")})
                clippath = self.getElementById(clippathurl[5:-1])
                for c in (clippath.iterchildren()):
                    inkex.etree.SubElement(
                        new_clippath, 'use',
                        {inkex.addNS('href', 'xlink'): '#' + c.get("id"),
                         'transform': inverse_node_transform,
                         'id': self.uniqueId("use")})

                # Set the clippathurl to be the one with the inverse transform
                clippathurl = "url(#" + new_clippath.get("id") + ")"

            # Reference the parent clip-path to keep clipping intersection
            # Find end of clip-path chain and add reference there
            node_clippathurl = node.get("clip-path")
            while (node_clippathurl):
                node = self.getElementById(node_clippathurl[5:-1])
                node_clippathurl = node.get("clip-path")
            node.set("clip-path", clippathurl)

    def _invert_transform(self, transform):
        # duplicate list to avoid modifying it
        return matrix(transform + [[0, 0, 1]]).I.tolist()[0:2]

    # Flatten a group into same z-order as parent, propagating attribs
    def _ungroup(self, node):
        node_parent = node.getparent()
        node_index = list(node_parent).index(node)
        node_style = simplestyle.parseStyle(node.get("style"))
        node_transform = simpletransform.parseTransform(node.get("transform"))
        node_clippathurl = node.get('clip-path')
        for c in reversed(list(node)):
            self._merge_transform(c, node_transform)
            self._merge_style(c, node_style)
            self._merge_clippath(c, node_clippathurl)
            node_parent.insert(node_index, c)
        node_parent.remove(node)

    # Put all ungrouping restrictions here
    def _want_ungroup(self, node, depth, height):
        if (node.tag == addNS("g", "svg") and
                node.getparent() is not None and
                height > self.options.keepdepth and
                depth >= self.options.startdepth and
                depth <= self.options.maxdepth):
            return True
        return False

    def _deep_ungroup(self, node):
        # using iteration instead of recursion to avoid hitting Python
        # max recursion depth limits, which is a problem in converted PDFs

        # Seed the queue (stack) with initial node
        q = [{'node': node,
              'depth': 0,
              'prev': {'height': None},
              'height': None}]

        while q:
            current = q[-1]
            node = current['node']
            depth = current['depth']
            height = current['height']

            # Recursion path
            if (height is None):
                # Don't enter non-graphical portions of the document
                if (node.tag == addNS("namedview", "sodipodi")
                        or node.tag == addNS("defs", "svg")
                        or node.tag == addNS("metadata", "svg")
                        or node.tag == addNS("foreignObject", "svg")):
                    q.pop()

                # Base case: Leaf node
                if (node.tag != addNS("g", "svg") or not len(node)):
                    current['height'] = 0

                # Recursive case: Group element with children
                else:
                    depth += 1
                    for c in node.iterchildren():
                        q.append({'node': c, 'prev': current,
                                 'depth': depth, 'height': None})

            # Return path
            else:
                # Ungroup if desired
                if (self._want_ungroup(node, depth, height)):
                    self._ungroup(node)

                # Propagate (max) height up the call chain
                height += 1
                previous = current['prev']
                prev_height = previous['height']
                if (prev_height is None or prev_height < height):
                    previous['height'] = height

                # Only process each node once
                q.pop()

    def effect(self):
        if len(self.selected):
            for elem in self.selected.itervalues():
                self._deep_ungroup(elem)
        else:
            for elem in self.document.getroot():
                self._deep_ungroup(elem)

if __name__ == '__main__':
    effect = Ungroup()
    effect.affect()
