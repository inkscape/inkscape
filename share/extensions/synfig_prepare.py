#!/usr/bin/env python
"""
synfig_prepare.py
Simplifies SVG files in preparation for sif export.

Copyright (C) 2011 Nikita Kitaev

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
"""

import os, tempfile

import inkex
from inkex import NSS, addNS, etree, errormsg
import simplepath, simplestyle, simpletransform

###### Utility Classes ####################################

class MalformedSVGError(Exception):
    """Raised when the SVG document is invalid or contains unsupported features"""
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return """SVG document is invalid or contains unsupported features

Error message: %s

The SVG to Synfig converter is designed to handle SVG files that were created using Inkscape. Unsupported features are most likely to occur in SVG files written by other programs.
"""     % repr(self.value)

try:
    from subprocess import Popen, PIPE
    bsubprocess = True
except:
    bsubprocess = False

class InkscapeActionGroup(object):
    """A class for calling Inkscape to perform operations on a document"""
    def __init__(self, svg_document=None):
        self.command = ""
        self.init_args = ""
        self.has_selection = False
        self.has_action = False
        self.svg_document = svg_document

    def set_svg_document(self, svg_document):
        """Set the SVG document that Inkscape will operate on"""
        self.svg_document = svg_document

    def set_init_args(self, cmd):
        """Set the initial arguments to Inkscape subprocess

        Can be used to pass additional arguments to Inkscape, or an initializer
        command (e.g. unlock all objects before proceeding).
        """
        self.init_args = cmd

    def clear(self):
        """Clear all actions"""
        self.command = ""
        self.has_action = False
        self.has_selection = False

    def verb(self, verb):
        """Run an Inkscape verb

        For a list of verbs, run `inkscape --verb-list`
        """
        if self.has_selection:
            self.command += "--verb=%s " % (verb)

            if not self.has_action:
                self.has_action = True

    def select_id(self, object_id):
        """Select object with given id"""
        self.command += "--select=%s " % (object_id)
        if not self.has_selection:
            self.has_selection = True

    def select_node(self, node):
        """Select the object represented by the SVG node

        Selection will fail if node has no id attribute
        """
        node_id = node.get("id", None)
        if node_id is None:
            raise MalformedSVGError, "Node has no id"
        self.select_id(node_id)

    def select_nodes(self, nodes):
        """Select objects represented by SVG nodes

        Selection will fail if any node has no id attribute
        """
        for node in nodes:
            self.select_node(node)

    def select_xpath(self, xpath, namespaces=NSS):
        """Select objects matching a given XPath expression

        Selection will fail if any matching node has no id attribute
        """
        nodes = self.svg_document.xpath(xpath, namespaces=namespaces)

        self.select_nodes(nodes)

    def deselect(self):
        """Deselect all objects"""
        if self.has_selection:
            self.verb("EditDeselect")
            self.has_selection = False

    def run_file(self, filename):
        """Run the actions on a specific file"""
        if not self.has_action:
            return

        cmd = self.init_args + " " + self.command + "--verb=FileSave --verb=FileQuit"
        if bsubprocess:
            p = Popen('inkscape "%s" %s' % (filename, cmd), shell=True, stdout=PIPE, stderr=PIPE)
            rc = p.wait()
            f = p.stdout
            err = p.stderr
        else:
            _, f, err = os.popen3( "inkscape %s %s" % ( filename, cmd ) )

        f.close()
        err.close()

    def run_document(self):
        """Run the actions on the svg xml tree"""
        if not self.has_action:
            return self.svg_document

        # First save the document
        svgfile = tempfile.mktemp(".svg")
        self.svg_document.write(svgfile)

        # Run the action on the document
        self.run_file(svgfile)

        # Open the resulting file
        stream = open(svgfile, 'r')
        new_svg_doc = etree.parse(stream)
        stream.close()

        # Clean up.
        try:
            os.remove(svgfile)
        except Exception:
            pass

        # Set the current SVG document
        self.svg_document = new_svg_doc

        # Return the new document
        return new_svg_doc

class SynfigExportActionGroup(InkscapeActionGroup):
    """An action group with stock commands designed for Synfig exporting"""
    def __init__(self, svg_document=None):
        InkscapeActionGroup.__init__(self, svg_document)
        self.set_init_args("--verb=UnlockAllInAllLayers")
        self.objects_to_paths()
        self.unlink_clones()

    def objects_to_paths(self):
        """Convert unsupported objects to paths"""
        # Flow roots contain rectangles inside them, so they need to be
        # converted to paths separately from other shapes
        self.select_xpath("//svg:flowRoot", namespaces=NSS)
        self.verb("ObjectToPath")
        self.deselect()

        non_paths = [
            "svg:rect",
            "svg:circle",
            "svg:ellipse",
            "svg:line",
            "svg:polyline",
            "svg:polygon",
            "svg:text"
            ]

        # Build an xpath command to select these nodes
        xpath_cmd = " | ".join(["//" + np for np in non_paths])

        # Select all of these elements
        # Note: already selected elements are not deselected
        self.select_xpath(xpath_cmd, namespaces=NSS)

        # Convert them to paths
        self.verb("ObjectToPath")
        self.deselect()

    def unlink_clones(self):
        """Unlink clones (remove <svg:use> elements)"""
        self.select_xpath("//svg:use", namespaces=NSS)
        self.verb("EditUnlinkClone")
        self.deselect()

###### Utility Functions ##################################

### Path related

def fuse_subpaths(path_node):
    """Fuse subpaths of a path. Should only be used on unstroked paths"""
    path_d = path_node.get("d", None)
    path = simplepath.parsePath(path_d)

    if len(path) == 0:
        return

    i = 0
    initial_point = [ path[i][1][-2], path[i][1][-1] ]
    return_stack = []
    while i < len(path):
        # Remove any terminators: they are redundant
        if path[i][0] == "Z":
            path.remove(["Z", []])
            continue

        # Skip all elements that do not begin a new path
        if i == 0 or path[i][0] != "M":
            i += 1
            continue

        # This element begins a new path - it should be a moveto
        assert(path[i][0] == 'M')

        # Swap it for a lineto
        path[i][0] = 'L'

        # If the old subpath has not been closed yet, close it
        if path[i-1][1][-2] != initial_point[0] or path[i-1][1][-2] != initial_point[1]:
            path.insert(i, ['L', initial_point])
            i += 1

        # Set the initial point of this subpath
        initial_point = [ path[i-1][1][-2], path[i-1][1][-1] ]

        # Append this point to the return stack
        return_stack.append(initial_point)
    #end while

    # Now pop the entire return stack
    while return_stack != []:
        el = ['L', return_stack.pop()]
        path.insert(i, el)
        i += 1


    path_d = simplepath.formatPath(path)
    path_node.set("d", path_d)

def split_fill_and_stroke(path_node):
    """Split a path into two paths, one filled and one stroked

    Returns a the list [fill, stroke], where each is the XML element of the
    fill or stroke, or None.
    """
    style = simplestyle.parseStyle(path_node.get("style", ""))

    # If there is only stroke or only fill, don't split anything
    if "fill" in style.keys() and style["fill"] == "none":
        if "stroke" not in style.keys() or style["stroke"] == "none":
            return [None, None] # Path has neither stroke nor fill
        else:
            return [None, path_node]
    if "stroke" not in style.keys() or style["stroke"] == "none":
        return [path_node, None]

    group = path_node.makeelement(addNS("g", "svg"))
    fill = etree.SubElement(group, addNS("path", "svg"))
    stroke = etree.SubElement(group, addNS("path", "svg"))

    attribs = path_node.attrib

    if "d" in attribs.keys():
        d = attribs["d"]
        del attribs["d"]
    else:
        raise AssertionError, "Cannot split stroke and fill of non-path element"

    if addNS("nodetypes", "sodipodi") in attribs.keys():
        nodetypes = attribs[addNS("nodetypes", "sodipodi")]
        del attribs[addNS("nodetypes", "sodipodi")]
    else:
        nodetypes = None

    if "id" in attribs.keys():
        path_id = attribs["id"]
        del attribs["id"]
    else:
        path_id = str(id(path_node))

    if "style" in attribs.keys():
        del attribs["style"]

    if "transform" in attribs.keys():
        transform = attribs["transform"]
        del attribs["transform"]
    else:
        transform = None

    # Pass along all remaining attributes to the group
    for attrib_name in attribs.keys():
        group.set(attrib_name, attribs[attrib_name])

    group.set("id", path_id)

    # Next split apart the style attribute
    style_group = {}
    style_fill = {"stroke":"none", "fill":"#000000"}
    style_stroke = {"fill":"none", "stroke":"none"}

    for key in style.keys():
        if key.startswith("fill"):
            style_fill[key] = style[key]
        elif key.startswith("stroke"):
            style_stroke[key] = style[key]
        elif key.startswith("marker"):
            style_stroke[key] = style[key]
        elif key.startswith("filter"):
            style_group[key] = style[key]
        else:
            style_fill[key] = style[key]
            style_stroke[key] = style[key]

    if len(style_group) != 0:
        group.set("style", simplestyle.formatStyle(style_group))

    fill.set("style", simplestyle.formatStyle(style_fill))
    stroke.set("style", simplestyle.formatStyle(style_stroke))

    # Finalize the two paths
    fill.set("d", d)
    stroke.set("d", d)
    if nodetypes is not None:
        fill.set(addNS("nodetypes", "sodipodi"), nodetypes)
        stroke.set(addNS("nodetypes", "sodipodi"), nodetypes)
    fill.set("id", path_id+"-fill")
    stroke.set("id", path_id+"-stroke")
    if transform is not None:
        fill.set("transform", transform)
        stroke.set("transform", transform)


    # Replace the original node with the group
    path_node.getparent().replace(path_node, group)

    return [fill, stroke]

### Object related

def propagate_attribs(node, parent_style={}, parent_transform=[[1.0, 0.0, 0.0], [0.0, 1.0, 0.0]]):
    """Propagate style and transform to remove inheritance"""

    # Don't enter non-graphical portions of the document
    if (node.tag == addNS("namedview", "sodipodi")
        or node.tag == addNS("defs", "svg")
        or node.tag == addNS("metadata", "svg")
        or node.tag == addNS("foreignObject", "svg")):
        return


    # Compose the transformations
    if node.tag == addNS("svg", "svg") and node.get("viewBox"):
        vx, vy, vw, vh = [get_dimension(x) for x in node.get("viewBox").split()]
        dw = get_dimension(node.get("width", vw))
        dh = get_dimension(node.get("height", vh))
        t = "translate(%f, %f) scale(%f, %f)" % (-vx, -vy, dw/vw, dh/vh)
        this_transform = simpletransform.parseTransform(t, parent_transform)
        this_transform = simpletransform.parseTransform(node.get("transform"), this_transform)
        del node.attrib["viewBox"]
    else:
        this_transform = simpletransform.parseTransform(node.get("transform"), parent_transform)

    # Compose the style attribs
    this_style = simplestyle.parseStyle(node.get("style", ""))
    remaining_style = {} # Style attributes that are not propagated

    non_propagated = ["filter"] # Filters should remain on the topmost ancestor
    for key in non_propagated:
        if key in this_style.keys():
            remaining_style[key] = this_style[key]
            del this_style[key]

    # Create a copy of the parent style, and merge this style into it
    parent_style_copy = parent_style.copy()
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

        # Remove the transform attribute
        if "transform" in node.keys():
            del node.attrib["transform"]

        # Continue propagating on subelements
        for c in node.iterchildren():
            propagate_attribs(c, this_style, this_transform)
    else:
        # This element is not a container

        # Merge remaining_style into this_style
        this_style.update(remaining_style)

        # Set the element's style and transform attribs
        node.set("style", simplestyle.formatStyle(this_style))
        node.set("transform", simpletransform.formatTransform(this_transform))

### Style related

def get_dimension(s="1024"):
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
        return float(s[:-2])*1.333
    elif s[-2:] == "em":
        return float(s[:-2])*16
    elif s[-2:] == "mm":
        return float(s[:-2])*3.779
    elif s[-2:] == "pc":
        return float(s[:-2])*16
    elif s[-2:] == "cm":
        return float(s[:-2])*37.79
    elif s[-2:] == "in":
        return float(s[:-2])*96
    else:
        return 1024

###### Main Class #########################################
class SynfigPrep(inkex.Effect):
    def effect(self):
        """Transform document in preparation for exporting it into the Synfig format"""

        a = SynfigExportActionGroup(self.document)
        self.document = a.run_document()

        # Remove inheritance of attributes
        propagate_attribs(self.document.getroot())

        # Fuse multiple subpaths in fills
        for node in self.document.xpath('//svg:path', namespaces=NSS):
            if node.get("d", "").lower().count("m") > 1:
                # There are multiple subpaths
                fill = split_fill_and_stroke(node)[0]
                if fill is not None:
                    fuse_subpaths(fill)

if __name__ == '__main__':
    try:
        e = SynfigPrep()
        e.affect()
    except MalformedSVGError, e:
        errormsg(e)


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
