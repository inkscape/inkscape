#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
inkex.py
A helper module for creating Inkscape extensions

Copyright (C) 2005,2010 Aaron Spike <aaron@ekips.org> and contributors

Contributors:
  Aurélio A. Heckert <aurium(a)gmail.com>
  Bulia Byak <buliabyak@users.sf.net>
  Nicolas Dufour, nicoduf@yahoo.fr
  Peter J. R. Moulder <pjrm@users.sourceforge.net>

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
"""
import copy
import gettext
import optparse
import os
import random
import re
import sys
from math import *

# a dictionary of all of the xmlns prefixes in a standard inkscape doc
NSS = {
u'sodipodi' :u'http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd',
u'cc'       :u'http://creativecommons.org/ns#',
u'ccOLD'    :u'http://web.resource.org/cc/',
u'svg'      :u'http://www.w3.org/2000/svg',
u'dc'       :u'http://purl.org/dc/elements/1.1/',
u'rdf'      :u'http://www.w3.org/1999/02/22-rdf-syntax-ns#',
u'inkscape' :u'http://www.inkscape.org/namespaces/inkscape',
u'xlink'    :u'http://www.w3.org/1999/xlink',
u'xml'      :u'http://www.w3.org/XML/1998/namespace'
}


def localize():
    domain = 'inkscape'
    if sys.platform.startswith('win'):
        import locale
        current_locale, encoding = locale.getdefaultlocale()
        os.environ['LANG'] = current_locale
        try:
            localdir = os.environ['INKSCAPE_LOCALEDIR']
            trans = gettext.translation(domain, localdir, [current_locale], fallback=True)
        except KeyError:
            trans = gettext.translation(domain, fallback=True)
    elif sys.platform.startswith('darwin'):
        try:
            localdir = os.environ['INKSCAPE_LOCALEDIR']
            trans = gettext.translation(domain, localdir, fallback=True)
        except KeyError:
            try:
                localdir = os.environ['PACKAGE_LOCALE_DIR']
                trans = gettext.translation(domain, localdir, fallback=True)
            except KeyError:
                trans = gettext.translation(domain, fallback=True)
    else:
        try:
            localdir = os.environ['PACKAGE_LOCALE_DIR']
            trans = gettext.translation(domain, localdir, fallback=True)
        except KeyError:
            trans = gettext.translation(domain, fallback=True)
    #sys.stderr.write(str(localdir) + "\n")
    trans.install()


def debug(what):
    sys.stderr.write(str(what) + "\n")
    return what


def errormsg(msg):
    """Intended for end-user-visible error messages.
    
       (Currently just writes to stderr with an appended newline, but could do
       something better in future: e.g. could add markup to distinguish error
       messages from status messages or debugging output.)
      
       Note that this should always be combined with translation:

         import inkex
         ...
         inkex.errormsg(_("This extension requires two selected paths."))
    """
    if isinstance(msg, unicode):
        sys.stderr.write(msg.encode("utf-8") + "\n")
    else:
        sys.stderr.write((unicode(msg, "utf-8", errors='replace') + "\n").encode("utf-8"))


def are_near_relative(a, b, eps):
    return (a-b <= a*eps) and (a-b >= -a*eps)


# third party library
try:
    from lxml import etree
except ImportError as e:
    localize()
    errormsg(_("The fantastic lxml wrapper for libxml2 is required by inkex.py and therefore this extension."
               "Please download and install the latest version from http://cheeseshop.python.org/pypi/lxml/, "
               "or install it through your package manager by a command like: sudo apt-get install "
               "python-lxml\n\nTechnical details:\n%s" % (e, )))
    sys.exit()


def check_inkbool(option, opt, value):
    if str(value).capitalize() == 'True':
        return True
    elif str(value).capitalize() == 'False':
        return False
    else:
        raise optparse.OptionValueError("option %s: invalid inkbool value: %s" % (opt, value))


def addNS(tag, ns=None):
    val = tag
    if ns is not None and len(ns) > 0 and ns in NSS and len(tag) > 0 and tag[0] != '{':
        val = "{%s}%s" % (NSS[ns], tag)
    return val


class InkOption(optparse.Option):
    TYPES = optparse.Option.TYPES + ("inkbool",)
    TYPE_CHECKER = copy.copy(optparse.Option.TYPE_CHECKER)
    TYPE_CHECKER["inkbool"] = check_inkbool


class Effect:
    """A class for creating Inkscape SVG Effects"""

    def __init__(self, *args, **kwargs):
        self.document = None
        self.original_document = None
        self.ctx = None
        self.selected = {}
        self.doc_ids = {}
        self.options = None
        self.args = None
        self.OptionParser = optparse.OptionParser(usage="usage: %prog [options] SVGfile",
                                                  option_class=InkOption)
        self.OptionParser.add_option("--id",
                        action="append", type="string", dest="ids", default=[], 
                        help="id attribute of object to manipulate")
        self.OptionParser.add_option("--selected-nodes",
                        action="append", type="string", dest="selected_nodes", default=[], 
                        help="id:subpath:position of selected nodes, if any")
        # TODO write a parser for this

    def effect(self):
        """Apply some effects on the document. Extensions subclassing Effect
        must override this function and define the transformations
        in it."""
        pass

    def getoptions(self,args=sys.argv[1:]):
        """Collect command line arguments"""
        self.options, self.args = self.OptionParser.parse_args(args)

    def parse(self, filename=None):
        """Parse document in specified file or on stdin"""

        # First try to open the file from the function argument
        if filename is not None:
            try:
                stream = open(filename, 'r')
            except IOError:
                errormsg(_("Unable to open specified file: %s") % filename)
                sys.exit()

        # If it wasn't specified, try to open the file specified as
        # an object member
        elif self.svg_file is not None:
            try:
                stream = open(self.svg_file, 'r')
            except IOError:
                errormsg(_("Unable to open object member file: %s") % self.svg_file)
                sys.exit()

        # Finally, if the filename was not specified anywhere, use
        # standard input stream
        else:
            stream = sys.stdin

        p = etree.XMLParser(huge_tree=True)
        self.document = etree.parse(stream, parser=p)
        self.original_document = copy.deepcopy(self.document)
        stream.close()

    # defines view_center in terms of document units
    def getposinlayer(self):
        #defaults
        self.current_layer = self.document.getroot()
        self.view_center = (0.0, 0.0)

        layerattr = self.document.xpath('//sodipodi:namedview/@inkscape:current-layer', namespaces=NSS)
        if layerattr:
            layername = layerattr[0]
            layer = self.document.xpath('//svg:g[@id="%s"]' % layername, namespaces=NSS)
            if layer:
                self.current_layer = layer[0]

        xattr = self.document.xpath('//sodipodi:namedview/@inkscape:cx', namespaces=NSS)
        yattr = self.document.xpath('//sodipodi:namedview/@inkscape:cy', namespaces=NSS)
        if xattr and yattr:
            x = self.unittouu(xattr[0] + 'px')
            y = self.unittouu(yattr[0] + 'px')
            doc_height = self.unittouu(self.getDocumentHeight())
            if x and y:
                self.view_center = (float(x), doc_height - float(y))
                # FIXME: y-coordinate flip, eliminate it when it's gone in Inkscape

    def getselected(self):
        """Collect selected nodes"""
        for i in self.options.ids:
            path = '//*[@id="%s"]' % i
            for node in self.document.xpath(path, namespaces=NSS):
                self.selected[i] = node

    def getElementById(self, id):
        path = '//*[@id="%s"]' % id
        el_list = self.document.xpath(path, namespaces=NSS)
        if el_list:
            return el_list[0]
        else:
            return None

    def getParentNode(self, node):
        for parent in self.document.getiterator():
            if node in parent.getchildren():
                return parent

    def getdocids(self):
        docIdNodes = self.document.xpath('//@id', namespaces=NSS)
        for m in docIdNodes:
            self.doc_ids[m] = 1

    def getNamedView(self):
        return self.document.xpath('//sodipodi:namedview', namespaces=NSS)[0]

    def createGuide(self, posX, posY, angle):
        atts = {
          'position': str(posX)+','+str(posY),
          'orientation': str(sin(radians(angle)))+','+str(-cos(radians(angle)))
          }
        guide = etree.SubElement(
                  self.getNamedView(),
                  addNS('guide','sodipodi'), atts)
        return guide

    def output(self):
        """Serialize document into XML on stdout"""
        original = etree.tostring(self.original_document)        
        result = etree.tostring(self.document)        
        if original != result:
            self.document.write(sys.stdout)

    def affect(self, args=sys.argv[1:], output=True):
        """Affect an SVG document with a callback effect"""
        self.svg_file = args[-1]
        localize()
        self.getoptions(args)
        self.parse()
        self.getposinlayer()
        self.getselected()
        self.getdocids()
        self.effect()
        if output:
            self.output()

    def uniqueId(self, old_id, make_new_id=True):
        new_id = old_id
        if make_new_id:
            while new_id in self.doc_ids:
                new_id += random.choice('0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ')
            self.doc_ids[new_id] = 1
        return new_id

    def xpathSingle(self, path):
        try:
            retval = self.document.xpath(path, namespaces=NSS)[0]
        except:
            errormsg(_("No matching node for expression: %s") % path)
            retval = None
        return retval

    # a dictionary of unit to user unit conversion factors
    __uuconv = {'in': 96.0, 'pt': 1.33333333333, 'px': 1.0, 'mm': 3.77952755913, 'cm': 37.7952755913,
                'm': 3779.52755913, 'km': 3779527.55913, 'pc': 16.0, 'yd': 3456.0, 'ft': 1152.0}

    # Fault tolerance for lazily defined SVG
    def getDocumentWidth(self):
        width = self.document.getroot().get('width')
        if width:
            return width
        else:
            viewbox = self.document.getroot().get('viewBox')
            if viewbox:
                return viewbox.split()[2]
            else:
                return '0'

    # Fault tolerance for lazily defined SVG
    def getDocumentHeight(self):
        """Returns a string corresponding to the height of the document, as
        defined in the SVG file. If it is not defined, returns the height
        as defined by the viewBox attribute. If viewBox is not defined,
        returns the string '0'."""
        height = self.document.getroot().get('height')
        if height:
            return height
        else:
            viewbox = self.document.getroot().get('viewBox')
            if viewbox:
                return viewbox.split()[3]
            else:
                return '0'

    def getDocumentUnit(self):
        """Returns the unit used for in the SVG document.
        In the case the SVG document lacks an attribute that explicitly
        defines what units are used for SVG coordinates, it tries to calculate
        the unit from the SVG width and viewBox attributes.
        Defaults to 'px' units."""
        svgunit = 'px'  # default to pixels

        svgwidth = self.getDocumentWidth()
        viewboxstr = self.document.getroot().get('viewBox')
        if viewboxstr:
            unitmatch = re.compile('(%s)$' % '|'.join(self.__uuconv.keys()))
            param = re.compile(r'(([-+]?[0-9]+(\.[0-9]*)?|[-+]?\.[0-9]+)([eE][-+]?[0-9]+)?)')

            p = param.match(svgwidth)
            u = unitmatch.search(svgwidth)

            width = 100  # default
            viewboxwidth = 100  # default
            svgwidthunit = 'px'  # default assume 'px' unit
            if p:
                width = float(p.string[p.start():p.end()])
            else:
                errormsg(_("SVG Width not set correctly! Assuming width = 100"))
            if u:
                svgwidthunit = u.string[u.start():u.end()]

            viewboxnumbers = []
            for t in viewboxstr.split():
                try:
                    viewboxnumbers.append(float(t))
                except ValueError:
                    pass
            if len(viewboxnumbers) == 4:  # check for correct number of numbers
                viewboxwidth = viewboxnumbers[2]

            svgunitfactor = self.__uuconv[svgwidthunit] * width / viewboxwidth

            # try to find the svgunitfactor in the list of units known. If we don't find something, ...
            eps = 0.01  # allow 1% error in factor
            for key in self.__uuconv:
                if are_near_relative(self.__uuconv[key], svgunitfactor, eps):
                    # found match!
                    svgunit = key

        return svgunit

    def unittouu(self, string):
        """Returns userunits given a string representation of units in another system"""
        unit = re.compile('(%s)$' % '|'.join(self.__uuconv.keys()))
        param = re.compile(r'(([-+]?[0-9]+(\.[0-9]*)?|[-+]?\.[0-9]+)([eE][-+]?[0-9]+)?)')

        p = param.match(string)
        u = unit.search(string)    
        if p:
            retval = float(p.string[p.start():p.end()])
        else:
            retval = 0.0
        if u:
            try:
                return retval * (self.__uuconv[u.string[u.start():u.end()]] / self.__uuconv[self.getDocumentUnit()])
            except KeyError:
                pass
        else:  # default assume 'px' unit
            return retval / self.__uuconv[self.getDocumentUnit()]

        return retval

    def uutounit(self, val, unit):
        return val / (self.__uuconv[unit] / self.__uuconv[self.getDocumentUnit()])

    def addDocumentUnit(self, value):
        """Add document unit when no unit is specified in the string """
        try:
            float(value)
            return value + self.getDocumentUnit()
        except ValueError:
            return value

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
