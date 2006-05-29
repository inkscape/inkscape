#!/usr/bin/env python
"""
inkex.py
A helper module for creating Inkscape extensions

Copyright (C) 2005 Aaron Spike, aaron@ekips.org

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
import sys, copy, optparse

#a dictionary of all of the xmlns prefixes in a standard inkscape doc
NSS = {
u'sodipodi'    :u'http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd',
u'cc'        :u'http://web.resource.org/cc/',
u'svg'        :u'http://www.w3.org/2000/svg',
u'dc'        :u'http://purl.org/dc/elements/1.1/',
u'rdf'        :u'http://www.w3.org/1999/02/22-rdf-syntax-ns#',
u'inkscape'    :u'http://www.inkscape.org/namespaces/inkscape',
u'xlink'    :u'http://www.w3.org/1999/xlink'
}

try:
    import xml.dom.ext
    import xml.dom.ext.reader.Sax2
    import xml.xpath
except:
    sys.exit('The inkex.py module requires PyXML. Please download the latest version from <http://pyxml.sourceforge.net/>.')

def debug(what):
    sys.stderr.write(str(what) + "\n")
    return what

def check_inkbool(option, opt, value):
    if str(value).capitalize() == 'True':
        return True
    elif str(value).capitalize() == 'False':
        return False
    else:
        raise OptionValueError("option %s: invalid inkbool value: %s" % (opt, value))

class InkOption(optparse.Option):
    TYPES = optparse.Option.TYPES + ("inkbool",)
    TYPE_CHECKER = copy.copy(optparse.Option.TYPE_CHECKER)
    TYPE_CHECKER["inkbool"] = check_inkbool


class Effect:
    """A class for creating Inkscape SVG Effects"""
    def __init__(self):
        self.document=None
        self.selected={}
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
        reader = xml.dom.ext.reader.Sax2.Reader()
        try:
            try:
                stream = open(file,'r')
            except:
                stream = open(self.args[-1],'r')
        except:
            stream = sys.stdin
        self.document = reader.fromStream(stream)
        stream.close()
    def getposinlayer(self):
        ctx = xml.xpath.Context.Context(self.document,processorNss=NSS)
        #defaults
        self.current_layer = self.document.documentElement
        self.view_center = (0.0,0.0)

        layerattr = xml.xpath.Evaluate('//sodipodi:namedview/@inkscape:current-layer',self.document,context=ctx)
        if layerattr:
            layername = layerattr[0].value
            layer = xml.xpath.Evaluate('//g[@id="%s"]' % layername,self.document,context=ctx)[0]
            if layer:
                self.current_layer = layer

        xattr = xml.xpath.Evaluate('//sodipodi:namedview/@inkscape:cx',self.document,context=ctx)
        yattr = xml.xpath.Evaluate('//sodipodi:namedview/@inkscape:cy',self.document,context=ctx)
        if xattr and yattr:
            x = xattr[0].value
            y = yattr[0].value
            if x and y:
                self.view_center = (float(x),float(y))
    def getselected(self):
        """Collect selected nodes"""
        for id in self.options.ids:
            path = '//*[@id="%s"]' % id
            for node in xml.xpath.Evaluate(path,self.document):
                self.selected[id] = node
    def output(self):
        """Serialize document into XML on stdout"""
        xml.dom.ext.Print(self.document)
    def affect(self):
        """Affect an SVG document with a callback effect"""
        self.getoptions()
        self.parse()
        self.getposinlayer()
        self.getselected()
        self.effect()
        self.output()
