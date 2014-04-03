#!/usr/bin/env python 
'''
This extension module can measure arbitrary path and object length
It adds text to the selected path containing the length in a given unit.
Area and Center of Mass calculated using Green's Theorem:
http://mathworld.wolfram.com/GreensTheorem.html

Copyright (C) 2010 Alvin Penner
Copyright (C) 2006 Georg Wiora
Copyright (C) 2006 Nathan Hurst
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

TODO:
 * should use the standard attributes for text
 * Implement option to keep text orientation upright 
    1. Find text direction i.e. path tangent,
    2. check direction >90 or <-90 Degrees
    3. rotate by 180 degrees around text center
'''
# standard library
import locale
# local library
import inkex
import simplestyle
import simpletransform
import cubicsuperpath
import bezmisc

inkex.localize()
locale.setlocale(locale.LC_ALL, '')

# third party
try:
    import numpy
except:
    inkex.errormsg(_("Failed to import the numpy modules. These modules are required by this extension. Please install them and try again.  On a Debian-like system this can be done with the command, sudo apt-get install python-numpy."))
    exit()

mat_area   = numpy.matrix([[  0,  2,  1, -3],[ -2,  0,  1,  1],[ -1, -1,  0,  2],[  3, -1, -2,  0]])
mat_cofm_0 = numpy.matrix([[  0, 35, 10,-45],[-35,  0, 12, 23],[-10,-12,  0, 22],[ 45,-23,-22,  0]])
mat_cofm_1 = numpy.matrix([[  0, 15,  3,-18],[-15,  0,  9,  6],[ -3, -9,  0, 12],[ 18, -6,-12,  0]])
mat_cofm_2 = numpy.matrix([[  0, 12,  6,-18],[-12,  0,  9,  3],[ -6, -9,  0, 15],[ 18, -3,-15,  0]])
mat_cofm_3 = numpy.matrix([[  0, 22, 23,-45],[-22,  0, 12, 10],[-23,-12,  0, 35],[ 45,-10,-35,  0]])

def numsegs(csp):
    return sum([len(p)-1 for p in csp])
def interpcoord(v1,v2,p):
    return v1+((v2-v1)*p)
def interppoints(p1,p2,p):
    return [interpcoord(p1[0],p2[0],p),interpcoord(p1[1],p2[1],p)]
def pointdistance((x1,y1),(x2,y2)):
    return math.sqrt(((x2 - x1) ** 2) + ((y2 - y1) ** 2))
def bezlenapprx(sp1, sp2):
    return pointdistance(sp1[1], sp1[2]) + pointdistance(sp1[2], sp2[0]) + pointdistance(sp2[0], sp2[1])
def tpoint((x1,y1), (x2,y2), t = 0.5):
    return [x1+t*(x2-x1),y1+t*(y2-y1)]
def cspbezsplit(sp1, sp2, t = 0.5):
    m1=tpoint(sp1[1],sp1[2],t)
    m2=tpoint(sp1[2],sp2[0],t)
    m3=tpoint(sp2[0],sp2[1],t)
    m4=tpoint(m1,m2,t)
    m5=tpoint(m2,m3,t)
    m=tpoint(m4,m5,t)
    return [[sp1[0][:],sp1[1][:],m1], [m4,m,m5], [m3,sp2[1][:],sp2[2][:]]]
def cspbezsplitatlength(sp1, sp2, l = 0.5, tolerance = 0.001):
    bez = (sp1[1][:],sp1[2][:],sp2[0][:],sp2[1][:])
    t = bezmisc.beziertatlength(bez, l, tolerance)
    return cspbezsplit(sp1, sp2, t)
def cspseglength(sp1,sp2, tolerance = 0.001):
    bez = (sp1[1][:],sp1[2][:],sp2[0][:],sp2[1][:])
    return bezmisc.bezierlength(bez, tolerance)    
def csplength(csp):
    total = 0
    lengths = []
    for sp in csp:
        lengths.append([])
        for i in xrange(1,len(sp)):
            l = cspseglength(sp[i-1],sp[i])
            lengths[-1].append(l)
            total += l
    return lengths, total
def csparea(csp):
    area = 0.0
    for sp in csp:
        if len(sp) < 2: continue
        for i in range(len(sp)):            # calculate polygon area
            area += 0.5*sp[i-1][1][0]*(sp[i][1][1] - sp[i-2][1][1])
        for i in range(1, len(sp)):         # add contribution from cubic Bezier
            vec_x = numpy.matrix([sp[i-1][1][0], sp[i-1][2][0], sp[i][0][0], sp[i][1][0]])
            vec_y = numpy.matrix([sp[i-1][1][1], sp[i-1][2][1], sp[i][0][1], sp[i][1][1]])
            area += 0.15*(vec_x*mat_area*vec_y.T)[0,0]
    return -area                            # require positive area for CCW
def cspcofm(csp):
    area = csparea(csp)
    xc = 0.0
    yc = 0.0
    if abs(area) < 1.e-8:
        inkex.errormsg(_("Area is zero, cannot calculate Center of Mass"))
        return 0, 0
    for sp in csp:
        for i in range(len(sp)):            # calculate polygon moment
            xc += sp[i-1][1][1]*(sp[i-2][1][0] - sp[i][1][0])*(sp[i-2][1][0] + sp[i-1][1][0] + sp[i][1][0])/6
            yc += sp[i-1][1][0]*(sp[i][1][1] - sp[i-2][1][1])*(sp[i-2][1][1] + sp[i-1][1][1] + sp[i][1][1])/6
        for i in range(1, len(sp)):         # add contribution from cubic Bezier
            vec_x = numpy.matrix([sp[i-1][1][0], sp[i-1][2][0], sp[i][0][0], sp[i][1][0]])
            vec_y = numpy.matrix([sp[i-1][1][1], sp[i-1][2][1], sp[i][0][1], sp[i][1][1]])
            vec_t = numpy.matrix([(vec_x*mat_cofm_0*vec_y.T)[0,0], (vec_x*mat_cofm_1*vec_y.T)[0,0], (vec_x*mat_cofm_2*vec_y.T)[0,0], (vec_x*mat_cofm_3*vec_y.T)[0,0]])
            xc += (vec_x*vec_t.T)[0,0]/280
            yc += (vec_y*vec_t.T)[0,0]/280
    return -xc/area, -yc/area
def appendSuperScript(node, text):
    super = inkex.etree.SubElement(node, inkex.addNS('tspan', 'svg'), {'style': 'font-size:65%;baseline-shift:super'})
    super.text = text
    
class Length(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--type",
                        action="store", type="string", 
                        dest="type", default="length",
                        help="Type of measurement")
        self.OptionParser.add_option("--format",
                        action="store", type="string", 
                        dest="format", default="textonpath",
                        help="Text Orientation")
        self.OptionParser.add_option("--angle",
                        action="store", type="float", 
                        dest="angle", default=0,
                        help="Angle")             
        self.OptionParser.add_option("-f", "--fontsize",
                        action="store", type="int", 
                        dest="fontsize", default=20,
                        help="Size of length lable text in px")
        self.OptionParser.add_option("-o", "--offset",
                        action="store", type="float", 
                        dest="offset", default=-6,
                        help="The distance above the curve")
        self.OptionParser.add_option("-u", "--unit",
                        action="store", type="string", 
                        dest="unit", default="mm",
                        help="The unit of the measurement")
        self.OptionParser.add_option("-p", "--precision",
                        action="store", type="int", 
                        dest="precision", default=2,
                        help="Number of significant digits after decimal point")
        self.OptionParser.add_option("-s", "--scale",
                        action="store", type="float", 
                        dest="scale", default=1,
                        help="Scale Factor (Drawing:Real Length)")
        self.OptionParser.add_option("-r", "--orient",
                        action="store", type="inkbool", 
                        dest="orient", default=True,
                        help="Keep orientation of text upright")
        self.OptionParser.add_option("--tab",
                        action="store", type="string", 
                        dest="tab", default="sampling",
                        help="The selected UI-tab when OK was pressed") 
        self.OptionParser.add_option("--measurehelp",
                        action="store", type="string", 
                        dest="measurehelp", default="",
                        help="dummy")

    def effect(self):
        # get number of digits
        prec = int(self.options.precision)
        scale = self.unittouu('1px')    # convert to document units
        self.options.offset *= scale
        factor = 1.0
        doc = self.document.getroot()
        if doc.get('viewBox'):
            [viewx, viewy, vieww, viewh] = doc.get('viewBox').split(' ')
            factor = float(doc.get('width'))/float(vieww)
            if float(doc.get('height'))/float(viewh) < factor:
                factor = float(doc.get('height'))/float(viewh)
            self.options.fontsize /= factor
        # loop over all selected paths
        for id, node in self.selected.iteritems():
            if node.tag == inkex.addNS('path','svg'):
                mat = simpletransform.composeParents(node, [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0]])
                p = cubicsuperpath.parsePath(node.get('d'))
                simpletransform.applyTransformToPath(mat, p)
                factor *= scale/self.unittouu('1'+self.options.unit)
                if self.options.type == "length":
                    slengths, stotal = csplength(p)
                    self.group = inkex.etree.SubElement(node.getparent(),inkex.addNS('text','svg'))
                elif self.options.type == "area":
                    stotal = csparea(p)*factor*self.options.scale
                    self.group = inkex.etree.SubElement(node.getparent(),inkex.addNS('text','svg'))
                else:
                    xc, yc = cspcofm(p)
                    self.group = inkex.etree.SubElement(node.getparent(),inkex.addNS('path','svg'))
                    self.group.set('id', 'MassCenter_' + node.get('id'))
                    self.addCross(self.group, xc, yc, scale)
                    continue
                # Format the length as string
                lenstr = locale.format("%(len)25."+str(prec)+"f",{'len':round(stotal*factor*self.options.scale,prec)}).strip()
                if self.options.format == 'textonpath':
                    if self.options.type == "length":
                        self.addTextOnPath(self.group, 0, 0, lenstr+' '+self.options.unit, id, 'start', '50%', self.options.offset)
                    else:
                        self.addTextOnPath(self.group, 0, 0, lenstr+' '+self.options.unit+'^2', id, 'start', '0%', self.options.offset)
                else:
                    if self.options.type == "length":
                        self.addTextWithTspan(self.group, p[0][0][1][0], p[0][0][1][1], lenstr+' '+self.options.unit, id, 'start', -int(self.options.angle), self.options.offset + self.options.fontsize/2)
                    else:
                        self.addTextWithTspan(self.group, p[0][0][1][0], p[0][0][1][1], lenstr+' '+self.options.unit+'^2', id, 'start', -int(self.options.angle), -self.options.offset + self.options.fontsize/2)

    def addCross(self, node, x, y, scale):
        l = 3*scale         # 3 pixels in document units
        node.set('d', 'm %s,%s %s,0 %s,0 m %s,%s 0,%s 0,%s' % (str(x-l), str(y), str(l), str(l), str(-l), str(-l), str(l), str(l)))
        node.set('style', 'stroke:#000000;fill:none;stroke-width:%s' % str(0.5*scale))

    def addTextOnPath(self, node, x, y, text, id, anchor, startOffset, dy = 0):
                new = inkex.etree.SubElement(node,inkex.addNS('textPath','svg'))
                s = {'text-align': 'center', 'vertical-align': 'bottom',
                    'text-anchor': anchor, 'font-size': str(self.options.fontsize),
                    'fill-opacity': '1.0', 'stroke': 'none',
                    'font-weight': 'normal', 'font-style': 'normal', 'fill': '#000000'}
                new.set('style', simplestyle.formatStyle(s))
                new.set(inkex.addNS('href','xlink'), '#'+id)
                new.set('startOffset', startOffset)
                new.set('dy', str(dy)) # dubious merit
                #new.append(tp)
                if text[-2:] == "^2":
                    appendSuperScript(new, "2")
                    new.text = str(text)[:-2]
                else:
                    new.text = str(text)
                #node.set('transform','rotate(180,'+str(-x)+','+str(-y)+')')
                node.set('x', str(x))
                node.set('y', str(y))

    def addTextWithTspan(self, node, x, y, text, id, anchor, angle, dy = 0):
                new = inkex.etree.SubElement(node,inkex.addNS('tspan','svg'), {inkex.addNS('role','sodipodi'): 'line'})
                s = {'text-align': 'center', 'vertical-align': 'bottom',
                    'text-anchor': anchor, 'font-size': str(self.options.fontsize),
                    'fill-opacity': '1.0', 'stroke': 'none',
                    'font-weight': 'normal', 'font-style': 'normal', 'fill': '#000000'}
                new.set('style', simplestyle.formatStyle(s))
                new.set('dy', str(dy))
                if text[-2:] == "^2":
                    appendSuperScript(new, "2")
                    new.text = str(text)[:-2]
                else:
                    new.text = str(text)
                node.set('x', str(x))
                node.set('y', str(y))
                node.set('transform', 'rotate(%s, %s, %s)' % (angle, x, y))

if __name__ == '__main__':
    e = Length()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
