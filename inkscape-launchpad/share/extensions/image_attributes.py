#!/usr/bin/env python
'''
image_attributes.py - adjust image attributes which don't have global
GUI options yet

Tool for Inkscape 0.91 to adjust rendering of drawings with linked
or embedded bitmap images created with older versions of Inkscape
or third-party applications.

Copyright (C) 2015, ~suv <suv-sf@users.sf.net>

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

# local library
import inkex
import simplestyle


class SetAttrImage(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        # main options
        self.OptionParser.add_option("--fix_scaling",
                                     action="store", type="inkbool",
                                     dest="fix_scaling", default=True,
                                     help="")
        self.OptionParser.add_option("--fix_rendering",
                                     action="store", type="inkbool",
                                     dest="fix_rendering", default=False,
                                     help="")
        self.OptionParser.add_option("--aspect_ratio",
                                     action="store", type="string",
                                     dest="aspect_ratio", default="none",
                                     help="Value for attribute 'preserveAspectRatio'")
        self.OptionParser.add_option("--aspect_clip",
                                     action="store", type="string",
                                     dest="aspect_clip", default="unset",
                                     help="optional 'meetOrSlice' value")
        self.OptionParser.add_option("--aspect_ratio_scope",
                                     action="store", type="string",
                                     dest="aspect_ratio_scope", default="selected_only",
                                     help="scope within which to edit 'preserveAspectRatio' attribute")
        self.OptionParser.add_option("--image_rendering",
                                     action="store", type="string",
                                     dest="image_rendering", default="unset",
                                     help="Value for attribute 'image-rendering'")
        self.OptionParser.add_option("--image_rendering_scope",
                                     action="store", type="string",
                                     dest="image_rendering_scope", default="selected_only",
                                     help="scope within which to edit 'image-rendering' attribute")
        # tabs
        self.OptionParser.add_option("--tab_main",
                                     action="store", type="string",
                                     dest="tab_main")

    # core method

    def change_attribute(self, node, attribute):
        for key, value in attribute.items():
            if key == 'preserveAspectRatio':
                # set presentation attribute
                if value != "unset":
                    node.set(key, str(value))
                else:
                    if node.get(key):
                        del node.attrib[key]
            elif key == 'image-rendering':
                node_style = simplestyle.parseStyle(node.get('style'))
                if key not in node_style:
                    # set presentation attribute
                    if value != "unset":
                        node.set(key, str(value))
                    else:
                        if node.get(key):
                            del node.attrib[key]
                else:
                    # set style property
                    if value != "unset":
                        node_style[key] = str(value)
                    else:
                        del node_style[key]
                    node.set('style', simplestyle.formatStyle(node_style))
            else:
                pass

    def change_all_images(self, node, attribute):
        path = 'descendant-or-self::svg:image'
        for img in node.xpath(path, namespaces=inkex.NSS):
            self.change_attribute(img, attribute)

    # methods called via dispatcher

    def change_selected_only(self, selected, attribute):
        if selected:
            for node_id, node in selected.iteritems():
                if node.tag == inkex.addNS('image', 'svg'):
                    self.change_attribute(node, attribute)

    def change_in_selection(self, selected, attribute):
        if selected:
            for node_id, node in selected.iteritems():
                self.change_all_images(node, attribute)

    def change_in_document(self, selected, attribute):
        self.change_all_images(self.document.getroot(), attribute)

    def change_on_parent_group(self, selected, attribute):
        if selected:
            for node_id, node in selected.iteritems():
                self.change_attribute(node.getparent(), attribute)

    def change_on_root_only(self, selected, attribute):
        self.change_attribute(self.document.getroot(), attribute)

    # main

    def effect(self):
        attr_val = []
        attr_dict = {}
        cmd_scope = None
        if self.options.tab_main == '"tab_basic"':
            cmd_scope = "in_document"
            attr_dict['preserveAspectRatio'] = ("none" if self.options.fix_scaling else "unset")
            attr_dict['image-rendering'] = ("optimizeSpeed" if self.options.fix_rendering else "unset")
        elif self.options.tab_main == '"tab_aspectRatio"':
            attr_val = [self.options.aspect_ratio]
            if self.options.aspect_clip != "unset":
                attr_val.append(self.options.aspect_clip)
            attr_dict['preserveAspectRatio'] = ' '.join(attr_val)
            cmd_scope = self.options.aspect_ratio_scope
        elif self.options.tab_main == '"tab_image_rendering"':
            attr_dict['image-rendering'] = self.options.image_rendering
            cmd_scope = self.options.image_rendering_scope
        else:  # help tab
            pass
        # dispatcher
        if cmd_scope is not None:
            try:
                change_cmd = getattr(self, 'change_{0}'.format(cmd_scope))
                change_cmd(self.selected, attr_dict)
            except AttributeError:
                inkex.errormsg('Scope "{0}" not supported'.format(cmd_scope))


if __name__ == '__main__':
    e = SetAttrImage()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
