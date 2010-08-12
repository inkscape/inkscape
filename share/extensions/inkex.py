#!/usr/bin/env python
"""
inkex.py
A helper module for creating Inkscape extensions

Copyright (C) 2005,2007 Aaron Spike, aaron@ekips.org

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
"""
import sys, copy, optparse, random, re
import gettext
from math import *
_ = gettext.gettext

#a dictionary of all of the xmlns prefixes in a standard inkscape doc
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

#a dictionary of unit to user unit conversion factors
uuconv = {'in':90.0, 'pt':1.25, 'px':1, 'mm':3.5433070866, 'cm':35.433070866, 'm':3543.3070866,
          'km':3543307.0866, 'pc':15.0, 'yd':3240 , 'ft':1080}
def unittouu(string):
    '''Returns userunits given a string representation of units in another system'''
    unit = re.compile('(%s)$' % '|'.join(uuconv.keys()))
    param = re.compile(r'(([-+]?[0-9]+(\.[0-9]*)?|[-+]?\.[0-9]+)([eE][-+]?[0-9]+)?)')

    p = param.match(string)
    u = unit.search(string)    
    if p:
        retval = float(p.string[p.start():p.end()])
    else:
        retval = 0.0
    if u:
        try:
            return retval * uuconv[u.string[u.start():u.end()]]
        except KeyError:
            pass
    return retval

def uutounit(val, unit):
    return val/uuconv[unit]

try:
    from lxml import etree
except Exception, e:
    sys.exit(_("The fantastic lxml wrapper for libxml2 is required by inkex.py and therefore this extension. Please download and install the latest version from http://cheeseshop.python.org/pypi/lxml/, or install it through your package manager by a command like: sudo apt-get install python-lxml\n\nTechnical details:\n%s" % (e,)))
 

def debug(what):
    sys.stderr.write(str(what) + "\n")
    return what

def errormsg(msg):
    """Intended for end-user-visible error messages.
    
       (Currently just writes to stderr with an appended newline, but could do
       something better in future: e.g. could add markup to distinguish error
       messages from status messages or debugging output.)
      
       Note that this should always be combined with translation:

         import gettext
         _ = gettext.gettext
         ...
         inkex.errormsg(_("This extension requires two selected paths."))
    """
    sys.stderr.write((unicode(msg) + "\n").encode("UTF-8"))

def check_inkbool(option, opt, value):
    if str(value).capitalize() == 'True':
        return True
    elif str(value).capitalize() == 'False':
        return False
    else:
        raise optparse.OptionValueError("option %s: invalid inkbool value: %s" % (opt, value))

def addNS(tag, ns=None):
    val = tag
    if ns!=None and len(ns)>0 and NSS.has_key(ns) and len(tag)>0 and tag[0]!='{':
        val = "{%s}%s" % (NSS[ns], tag)
    return val

class InkOption(optparse.Option):
    TYPES = optparse.Option.TYPES + ("inkbool",)
    TYPE_CHECKER = copy.copy(optparse.Option.TYPE_CHECKER)
    TYPE_CHECKER["inkbool"] = check_inkbool

class Effect:
    """A class for creating Inkscape SVG Effects"""

    def __init__(self, *args, **kwargs):
        self.document=None
        self.ctx=None
        self.selected={}
        self.doc_ids={}
        self.options=None
        self.args=None
        self.OptionParser = optparse.OptionParser(usage="usage: %prog [options] SVGfile",option_class=InkOption)
        self.OptionParser.add_option("--id",
                        action="append", type="string", dest="ids", default=[], 
                        help="id attribute of object to manipulate")

    def effect(self):
        pass

    def getoptions(self,args=sys.argv[1:]):
        """Collect command line arguments"""
        self.options, self.args = self.OptionParser.parse_args(args)

    def parse(self,file=None):
        """Parse document in specified file or on stdin"""
        try:
            try:
                stream = open(file,'r')
            except:
                stream = open(self.svg_file,'r')
        except:
            stream = sys.stdin
        self.document = etree.parse(stream)
        stream.close()

    def getposinlayer(self):
        #defaults
        self.current_layer = self.document.getroot()
        self.view_center = (0.0,0.0)

        layerattr = self.document.xpath('//sodipodi:namedview/@inkscape:current-layer', namespaces=NSS)
        if layerattr:
            layername = layerattr[0]
            layer = self.document.xpath('//svg:g[@id="%s"]' % layername, namespaces=NSS)
            if layer:
                self.current_layer = layer[0]

        xattr = self.document.xpath('//sodipodi:namedview/@inkscape:cx', namespaces=NSS)
        yattr = self.document.xpath('//sodipodi:namedview/@inkscape:cy', namespaces=NSS)
        doc_height = unittouu(self.document.getroot().get('height'))
        if xattr and yattr:
            x = xattr[0]
            y = yattr[0]
            if x and y:
                self.view_center = (float(x), doc_height - float(y)) # FIXME: y-coordinate flip, eliminate it when it's gone in Inkscape

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
                break


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
                  addNS('guide','sodipodi'), atts )
        return guide

    def output(self):
        """Serialize document into XML on stdout"""
        self.document.write(sys.stdout)

    def affect(self, args=sys.argv[1:], output=True):
        """Affect an SVG document with a callback effect"""
        self.svg_file = args[-1]
        self.getoptions(args)
        self.parse()
        self.getposinlayer()
        self.getselected()
        self.getdocids()
        self.effect()
        if output: self.output()

    def uniqueId(self, old_id, make_new_id = True):
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
            

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 encoding=utf-8 textwidth=99
