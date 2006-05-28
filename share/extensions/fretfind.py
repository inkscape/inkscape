#!/usr/bin/env python 
'''
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
'''
import sys
import inkex, simplestyle, simplepath
import ffscale, ffproc

def seg2path(seg):
    return "M%s,%sL%s,%s" % (seg[0]['x'],seg[0]['y'],seg[1]['x'],seg[1]['y'])

class FretFind(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.doScala = False
        self.doMultiScale = False
        self.OptionParser.add_option("--scalafile",
                        action="store", type="string", 
                        dest="scalafile", default=None,
                        help="")
        self.OptionParser.add_option("--scalascale",
                        action="store", type="string", 
                        dest="scalascale", default=None,
                        help="")
        self.OptionParser.add_option("--etbase",
                        action="store", type="float", 
                        dest="etbase", default=2.0,
                        help="")
        self.OptionParser.add_option("--etroot",
                        action="store", type="float", 
                        dest="etroot", default=12.0,
                        help="")
        self.OptionParser.add_option("--tuning",
                        action="store", type="string", 
                        dest="tuning", default="0;0;0;0;0;0",
                        help="")
        self.OptionParser.add_option("--perpdist",
                        action="store", type="float", 
                        dest="perpdist", default=0.5,
                        help="")
        self.OptionParser.add_option("--offset",
                        action="store", type="float", 
                        dest="offset", default=0.0,
                        help="")
        self.OptionParser.add_option("--scalelength",
                        action="store", type="float", 
                        dest="scalelength", default=25.0,
                        help="")
        self.OptionParser.add_option("--firstscalelength",
                        action="store", type="float", 
                        dest="firstscalelength", default=None,
                        help="")
        self.OptionParser.add_option("--lastscalelength",
                        action="store", type="float", 
                        dest="lastscalelength", default=None,
                        help="")
        self.OptionParser.add_option("--nutwidth",
                        action="store", type="float", 
                        dest="nutwidth", default=1.5,
                        help="")
        self.OptionParser.add_option("--bridgewidth",
                        action="store", type="float", 
                        dest="bridgewidth", default=2.5,
                        help="")
        self.OptionParser.add_option("--frets",
                        action="store", type="int", 
                        dest="frets", default=24,
                        help="")
        self.OptionParser.add_option("--strings",
                        action="store", type="int", 
                        dest="strings", default=6,
                        help="")
        self.OptionParser.add_option("--fbedges",
                        action="store", type="float", 
                        dest="fbedges", default=0.0975,
                        help="")
        self.OptionParser.add_option("--pxperunit",
                        action="store", type="float", 
                        dest="pxperunit", default=90,
                        help="")
    def checkopts(self):
        #scale type
        if self.options.scalascale is not None:
            self.doScala = True
        if self.options.scalafile is not None:
            if self.doScala:
                sys.stderr.write('Mutually exclusive options: if found scalafile will override scalascale.')
            else:
                self.options.scalascale = ""
            self.doScala = True
            try:
                f = open(self.options.scalafile,'r')
                self.options.scalascale = f.read()
            except: 
                sys.exit('Scala scale description file expected.')
        #scale
        if self.doScala:
            self.scale = ffscale.ScalaScale(self.options.scalascale)
        else:
            self.scale = ffscale.ETScale(self.options.etroot,self.options.etbase)
    
        #string length
        first = self.options.firstscalelength is not None
        last = self.options.lastscalelength is not None
        if first and last:
            self.doMultiScale = True
        elif first:
            sys.stderr.write('Missing lastscalelength: overriding scalelength with firstscalelength.')
            self.options.scalelength = self.options.firstscalelength
        elif last:
            sys.stderr.write('Missing firstscalelength: overriding scalelength with lastscalelength.')
            self.options.scalelength = self.options.lastscalelength
        
        #tuning
        self.tuning = [int(t.strip()) for t in self.options.tuning.split(";")]
        self.tuning.extend([0 for i in range(self.options.strings - len(self.tuning))])

    def effect(self):
        self.checkopts()

        o = self.options.fbedges
        oNF,oBF,oNL,oBL = o,o,o,o

        if self.doMultiScale:
            strings, meta = ffproc.FindStringsMultiScale(self.options.strings,self.options.firstscalelength,
                self.options.lastscalelength,self.options.nutwidth,self.options.bridgewidth,
                self.options.perpdist,oNF,oBF,oNL,oBL)
        else:
            strings, meta = ffproc.FindStringsSingleScale(self.options.strings,self.options.scalelength,
                self.options.nutwidth,self.options.bridgewidth,
                oNF,oBF,oNL,oBL)
        
        frets = ffproc.FindFrets(strings, meta, self.scale, self.tuning, self.options.frets)
        
        edgepath = seg2path(meta[0]) + seg2path(meta[-1])
        stringpath = "".join([seg2path(s) for s in strings])
        fretpath = "".join(["".join([seg2path(f) for f in s]) for s in frets])
    
        group = self.document.createElement('svg:g')
        group.setAttribute('transform',"scale(%s,%s)" % (self.options.pxperunit,self.options.pxperunit))
        self.current_layer.appendChild(group)
        
        edge = self.document.createElement('svg:path')
        s = {'stroke-linejoin': 'miter', 'stroke-width': '0.01px', 
            'stroke-opacity': '1.0', 'fill-opacity': '1.0', 
            'stroke': '#0000FF', 'stroke-linecap': 'butt', 
            'fill': 'none'}
        edge.setAttribute('style', simplestyle.formatStyle(s))
        edge.setAttribute('d', edgepath)

        string = self.document.createElement('svg:path')
        s = {'stroke-linejoin': 'miter', 'stroke-width': '0.01px', 
            'stroke-opacity': '1.0', 'fill-opacity': '1.0', 
            'stroke': '#FF0000', 'stroke-linecap': 'butt', 
            'fill': 'none'}
        string.setAttribute('style', simplestyle.formatStyle(s))
        string.setAttribute('d', stringpath)

        fret = self.document.createElement('svg:path')
        s = {'stroke-linejoin': 'miter', 'stroke-width': '0.01px', 
            'stroke-opacity': '1.0', 'fill-opacity': '1.0', 
            'stroke': '#000000', 'stroke-linecap': 'butt', 
            'fill': 'none'}
        fret.setAttribute('style', simplestyle.formatStyle(s))
        fret.setAttribute('d', fretpath)

        group.appendChild(edge)
        group.appendChild(string)
        group.appendChild(fret)

e = FretFind()
e.affect()