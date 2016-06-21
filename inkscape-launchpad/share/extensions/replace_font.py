#!/usr/bin/env python
'''
replace_font.py

Copyright (C) 2010 Craig Marshall, craig9 [at] gmail.com

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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

-----------------------

This script finds all fonts in the current drawing that match the
specified find font, and replaces them with the specified replacement
font.

It can also replace all fonts indiscriminately, and list all fonts
currently being used.
'''
# standard library
import os
import sys
# local library
import inkex
import simplestyle

text_tags = ['{http://www.w3.org/2000/svg}tspan',
                            '{http://www.w3.org/2000/svg}text',
                            '{http://www.w3.org/2000/svg}flowRoot',
                            '{http://www.w3.org/2000/svg}flowPara',
                            '{http://www.w3.org/2000/svg}flowSpan']
font_attributes = ['font-family', '-inkscape-font-specification']

def set_font(node, new_font, style=None):
    '''
    Sets the font attribute in the style attribute of node, using the
    font name stored in new_font. If the style dict is open already,
    it can be passed in, otherwise it will be optned anyway.

    Returns a dirty boolean flag
    '''
    dirty = False
    if not style:
        style = get_style(node)
    if style:
        for att in font_attributes:
            if att in style:
                style[att] = new_font
                set_style(node, style)
                dirty = True
    return dirty

def find_replace_font(node, find, replace):
    '''
    Searches the relevant font attributes/styles of node for find, and
    replaces them with replace.

    Returns a dirty boolean flag
    '''
    dirty = False
    style = get_style(node)
    if style:
        for att in font_attributes:
            if att in style and style[att].strip().lower() == find:
                set_font(node, replace, style)
                dirty = True
    return dirty

def is_styled_text(node):
    '''
    Returns true if the tag in question is a "styled" element that
    can hold text.
    '''
    return node.tag in text_tags and 'style' in node.attrib

def is_text(node):
    '''
    Returns true if the tag in question is an element that
    can hold text.
    '''
    return node.tag in text_tags


def get_style(node):
    '''
    Sugar coated way to get style dict from a node
    '''
    if 'style' in node.attrib:
        return simplestyle.parseStyle(node.attrib['style'])

def set_style(node, style):
    '''
    Sugar coated way to set the style dict, for node
    '''
    node.attrib['style'] = simplestyle.formatStyle(style)

def get_fonts(node):
    '''
    Given a node, returns a list containing all the fonts that
    the node is using.
    '''
    fonts = []
    s = get_style(node)
    if not s:
        return fonts
    for a in font_attributes:
        if a in s:
            fonts.append(s[a])
    return fonts

def die(msg = "Dying!"):
    inkex.errormsg(msg)
    sys.exit(0)

def report_replacements(num):
    '''
    Sends a message to the end user showing success of failure
    of the font replacement
    '''
    if num == 0:
        die(_('Couldn\'t find anything using that font, please ensure the spelling and spacing is correct.'))

def report_findings(findings):
    '''
    Tells the user which fonts were found, if any
    '''
    if len(findings) == 0:
        inkex.errormsg(_("Didn't find any fonts in this document/selection."))
    else:
        if len(findings) == 1:
            inkex.errormsg(_("Found the following font only: %s") % findings[0])
        else:
            inkex.errormsg(_("Found the following fonts:\n%s") % '\n'.join(findings))

class ReplaceFont(inkex.Effect):
    '''
    Replaces all instances of one font with another
    '''
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--fr_find", action="store",
                                        type="string", dest="fr_find",
                                        default=None, help="")

        self.OptionParser.add_option("--fr_replace", action="store",
                                        type="string", dest="fr_replace",
                                        default=None, help="")

        self.OptionParser.add_option("--r_replace", action="store",
                                        type="string", dest="r_replace",
                                        default=None, help="")

        self.OptionParser.add_option("--action", action="store",
                                        type="string", dest="action",
                                        default=None, help="")

        self.OptionParser.add_option("--scope", action="store",
                                        type="string", dest="scope",
                                        default=None, help="")

    def find_child_text_items(self, node):
        '''
        Recursive method for appending all text-type elements
        to self.selected_items
        '''
        if is_text(node):
            self.selected_items.append(node)
            for child in node:
                self.find_child_text_items(child)

    def relevant_items(self, scope):
        '''
        Depending on the scope, returns all text elements, or all
        selected text elements including nested children
        '''
        items = []
        to_return = []
        if scope == "selection_only":
            self.selected_items = []
            for item in self.selected.iteritems():
                self.find_child_text_items(item[1])
            items = self.selected_items
            if len(items) == 0:
                die(_("There was nothing selected"))
        else:
            items = self.document.getroot().getiterator()
        to_return.extend(filter(is_text, items))
        return to_return

    def find_replace(self, nodes, find, replace):
        '''
        Walks through nodes, replacing fonts as it goes according
        to find and replace
        '''
        replacements = 0
        for node in nodes:
            if find_replace_font(node, find, replace):
                replacements += 1
        report_replacements(replacements)

    def replace_all(self, nodes, replace):
        '''
        Walks through nodes, setting fonts indiscriminately.
        '''
        replacements = 0
        for node in nodes:
            if set_font(node, replace):
                replacements += 1
        report_replacements(replacements)

    def list_all(self, nodes):
        '''
        Walks through nodes, building a list of all fonts found, then
        reports to the user with that list
        '''
        fonts_found = []
        for node in nodes:
            for f in get_fonts(node):
                if not f in fonts_found:
                    fonts_found.append(f)
        report_findings(sorted(fonts_found))

    def effect(self):
        action = self.options.action.strip("\"") # TODO Is this a bug? (Extra " characters)
        scope = self.options.scope

        relevant_items = self.relevant_items(scope)

        if action == "find_replace":
            find = self.options.fr_find
            if find is None or find == "":
                die(_("Please enter a search string in the find box."));
            find = find.strip().lower()
            replace = self.options.fr_replace
            if replace is None or replace == "":
                die(_("Please enter a replacement font in the replace with box."));
            self.find_replace(relevant_items, find, replace)
        elif action == "replace_all":
            replace = self.options.r_replace
            if replace is None or replace == "":
                die(_("Please enter a replacement font in the replace all box."));
            self.replace_all(relevant_items, replace)
        elif action == "list_only":
            self.list_all(relevant_items)
            sys.exit(0)

if __name__ == "__main__":
    e = ReplaceFont()
    e.affect()
