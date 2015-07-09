#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys, inkex
from scour import scourString

class ScourInkscape (inkex.Effect):

    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--tab",
            action="store", type="string",
            dest="tab")
        self.OptionParser.add_option("--simplify-colors", type="inkbool",
            action="store", dest="simple_colors", default=True,
            help="won't convert all colors to #RRGGBB format")
        self.OptionParser.add_option("--style-to-xml", type="inkbool",
            action="store", dest="style_to_xml", default=True,
            help="won't convert styles into XML attributes")
        self.OptionParser.add_option("--group-collapsing", type="inkbool",
            action="store", dest="group_collapse", default=True,
            help="won't collapse <g> elements")
        self.OptionParser.add_option("--create-groups", type="inkbool",
            action="store", dest="group_create", default=False,
            help="create <g> elements for runs of elements with identical attributes")
        self.OptionParser.add_option("--enable-id-stripping", type="inkbool",
            action="store", dest="strip_ids", default=False,
            help="remove all un-referenced ID attributes")
        self.OptionParser.add_option("--shorten-ids", type="inkbool",
            action="store", dest="shorten_ids", default=False,
            help="shorten all ID attributes to the least number of letters possible")
        self.OptionParser.add_option("--embed-rasters", type="inkbool",
            action="store", dest="embed_rasters", default=True,
            help="won't embed rasters as base64-encoded data")
        self.OptionParser.add_option("--keep-editor-data", type="inkbool",
            action="store", dest="keep_editor_data", default=False,
            help="won't remove Inkscape, Sodipodi or Adobe Illustrator elements and attributes")
        self.OptionParser.add_option("--remove-metadata", type="inkbool",
            action="store", dest="remove_metadata", default=False,
            help="remove <metadata> elements (which may contain license metadata etc.)")
        self.OptionParser.add_option("--strip-xml-prolog", type="inkbool",
            action="store", dest="strip_xml_prolog", default=False,
            help="won't output the <?xml ?> prolog")
        self.OptionParser.add_option("-p", "--set-precision",
            action="store", type=int, dest="digits", default=5,
            help="set number of significant digits (default: %default)")
        self.OptionParser.add_option("--indent",
            action="store", type="string", dest="indent_type", default="space",
            help="indentation of the output: none, space, tab (default: %default)")
        self.OptionParser.add_option("--protect-ids-noninkscape", type="inkbool",
            action="store", dest="protect_ids_noninkscape", default=False,
            help="don't change IDs not ending with a digit")
        self.OptionParser.add_option("--protect-ids-list",
            action="store", type="string", dest="protect_ids_list", default=None,
            help="don't change IDs given in a comma-separated list")
        self.OptionParser.add_option("--protect-ids-prefix",
            action="store", type="string", dest="protect_ids_prefix", default=None,
            help="don't change IDs starting with the given prefix")
        self.OptionParser.add_option("--enable-viewboxing", type="inkbool",
            action="store", dest="enable_viewboxing", default=False,
            help="changes document width/height to 100%/100% and creates viewbox coordinates")
        self.OptionParser.add_option("--enable-comment-stripping", type="inkbool",
            action="store", dest="strip_comments", default=False,
            help="remove all <!-- --> comments")
        self.OptionParser.add_option("--renderer-workaround", type="inkbool",
            action="store", dest="renderer_workaround", default=False,
            help="work around various renderer bugs (currently only librsvg)")

    def effect(self):
        input = file(self.args[0], "r")
        self.options.infilename=self.args[0]
        sys.stdout.write(scourString(input.read(), self.options).encode("UTF-8"))
        input.close()
        sys.stdout.close()


if __name__ == '__main__':
    e = ScourInkscape()
    e.affect(output=False)
