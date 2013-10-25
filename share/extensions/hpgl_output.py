#!/usr/bin/env python 
# coding=utf-8
'''
Copyright (C) 2008 Aaron Spike, aaron@ekips.org
Overcut, Tool Offset, Rotation, Serial Com., Many Bugfixes and Improvements: Sebastian Wüst, sebi@timewaster.de, http://www.timewasters-place.com/

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
import inkex, simpletransform, cubicsuperpath, simplestyle, sys, math, bezmisc, string, cspsubdiv

class MyEffect(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-f", "--flatness",
                        action="store", type="float", 
                        dest="flat", default=1.2,
                        help="Minimum flatness of the subdivided curves")
        self.OptionParser.add_option("-m", "--mirror",
                        action="store", type="inkbool", 
                        dest="mirror", default="FALSE",
                        help="Mirror Y-Axis")
        self.OptionParser.add_option("-v", "--center",
                        action="store", type="inkbool", 
                        dest="center", default="FALSE",
                        help="Center Zero Point")
        self.OptionParser.add_option("-x", "--xOffset",
                        action="store", type="float", 
                        dest="xOffset", default=0.0,
                        help="X offset (mm)")
        self.OptionParser.add_option("-y", "--yOffset",
                        action="store", type="float", 
                        dest="yOffset", default=0.0,
                        help="Y offset (mm)")
        self.OptionParser.add_option("-r", "--resolution",
                        action="store", type="int", 
                        dest="resolution", default=1016,
                        help="Resolution (dpi)")
        self.OptionParser.add_option("-n", "--pen",
                        action="store", type="int",
                        dest="pen", default=1,
                        help="Pen number")
        self.OptionParser.add_option("-p", "--plotInvisibleLayers",
                        action="store", type="inkbool", 
                        dest="plotInvisibleLayers", default="FALSE",
                        help="Plot invisible layers")
        self.OptionParser.add_option("-u", "--useOvercut",
                        action="store", type="inkbool", 
                        dest="useOvercut", default="TRUE",
                        help="Use Overcut")
        self.OptionParser.add_option("-o", "--overcut",
                        action="store", type="float", 
                        dest="overcut", default=1.0,
                        help="Overcut (mm)")
        self.OptionParser.add_option("-c", "--correctToolOffset",
                        action="store", type="inkbool", 
                        dest="correctToolOffset", default="TRUE",
                        help="Correct tool offset")
        self.OptionParser.add_option("-t", "--toolOffset",
                        action="store", type="float", 
                        dest="toolOffset", default=0.25,
                        help="Tool offset (mm)")
        self.OptionParser.add_option("-w", "--toolOffsetReturn",
                        action="store", type="float", 
                        dest="toolOffsetReturn", default=2.5,
                        help="Return Factor")
        self.OptionParser.add_option("-a", "--angle",
                        action="store", type="string", 
                        dest="rotation", default="90",
                        help="Orientation")
        self.OptionParser.add_option("-s", "--sendToPlotter",
                        action="store", type="inkbool", 
                        dest="sendToPlotter", default="FALSE",
                        help="Send to Plotter also")
        self.OptionParser.add_option("-z", "--port",
                        action="store", type="string", 
                        dest="port", default="COM1",
                        help="Serial Port")
        self.OptionParser.add_option("-b", "--baudRate",
                        action="store", type="string", 
                        dest="baudRate", default="9600",
                        help="Baud Rate")
 
    def effect(self):
        # initiate vars
        self.xDivergence = 9999999999999.0
        self.yDivergence = 9999999999999.0
        self.xSize = -9999999999999.0
        self.ySize = -9999999999999.0
        scale = float(self.options.resolution) / 90 # dots/inch to dots/pixels
        x0 = self.options.xOffset * 3.5433070866 * scale # mm to dots
        y0 = self.options.yOffset * 3.5433070866 * scale # mm to dots
        self.options.overcut = self.options.overcut * 3.5433070866 * scale # mm to dots
        self.options.toolOffset = self.options.toolOffset * 3.5433070866 * scale # mm to dots
        self.options.flat = float(self.options.resolution) * self.options.flat / 1000
        doc = self.document.getroot()
        mirror = 1.0
        if self.options.mirror:
            mirror = -1.0
        # get viewBox parameter to correct scaling
        viewBox = doc.get('viewBox')
        viewBoxTransformX = 1
        viewBoxTransformY = 1
        if viewBox:
            viewBox = string.split(viewBox, ' ')
            if viewBox[2] and viewBox[3]:
                viewBoxTransformX = float(self.unittouu(doc.get('width'))) / float(viewBox[2])
                viewBoxTransformY = float(self.unittouu(doc.get('height'))) / float(viewBox[3])
        # dryRun to find edges
        self.dryRun = True
        self.groupmat = [[[scale*viewBoxTransformX, 0.0, 0.0], [0.0, mirror*scale*viewBoxTransformY, 0.0]]]
        self.groupmat[0] = simpletransform.composeTransform(self.groupmat[0], simpletransform.parseTransform('rotate(' + self.options.rotation + ')'))
        self.vData = [['', -1.0, -1.0], ['', -1.0, -1.0], ['', -1.0, -1.0], ['', -1.0, -1.0]]
        self.process_group(doc)
        if self.options.center:
            self.xDivergence += (self.xSize - self.xDivergence) / 2
            self.yDivergence += (self.ySize - self.yDivergence) / 2
        # life run
        self.dryRun = False
        self.groupmat = [[[scale*viewBoxTransformX, 0.0, -self.xDivergence + x0], [0.0, mirror*scale*viewBoxTransformY, -self.yDivergence + y0]]]
        self.groupmat[0] = simpletransform.composeTransform(self.groupmat[0], simpletransform.parseTransform('rotate(' + self.options.rotation + ')'))
        self.vData = [['', -1.0, -1.0], ['', -1.0, -1.0], ['', -1.0, -1.0], ['', -1.0, -1.0]]
        self.hpgl = 'IN;SP%d;' % self.options.pen
        if self.options.sendToPlotter:
            try:
                import serial
            except ImportError, e:
                inkex.errormsg('pySerial is not installed.\n\n1. Download pySerial here: http://pypi.python.org/pypi/pyserial\n2. Extract the "serial" folder from the zip to the following folder: "C:\Program Files (x86)\inkscape\python\Lib"\n3. Restart Inkscape.')
                self.options.sendToPlotter = False
            if self.options.sendToPlotter:          
                self.S = serial.Serial(port=self.options.port, baudrate=self.options.baudRate, timeout=0.01, writeTimeout=None)
                self.S.write('IN;SP%d;' % self.options.pen)
        self.process_group(doc)
        self.calcOffset('PU', 0, 0)
        self.hpgl += 'PU0,0;'
        if self.options.sendToPlotter:
            self.S.write('PU0,0;')
            self.S.read(2)
            self.S.close()

    def process_group(self, group):
        # process groups
        style = group.get('style')
        if style:
            style = simplestyle.parseStyle(style)
            if style.has_key('display'):
                if style['display'] == 'none':
                    if not self.options.plotInvisibleLayers:
                        return
        trans = group.get('transform')
        if trans:
            self.groupmat.append(simpletransform.composeTransform(self.groupmat[-1], simpletransform.parseTransform(trans)))
        for node in group:
            if node.tag == inkex.addNS('path','svg'):
                self.process_path(node, self.groupmat[-1])
            if node.tag == inkex.addNS('g','svg'):
                self.process_group(node)
        if trans:
            self.groupmat.pop()

    def process_path(self, node, mat):
        # process oath 
        d = node.get('d')
        if d:
            # transform path
            p = cubicsuperpath.parsePath(d)
            trans = node.get('transform')
            if trans:
                mat = simpletransform.composeTransform(mat, simpletransform.parseTransform(trans))
            simpletransform.applyTransformToPath(mat, p)
            cspsubdiv.cspsubdiv(p, self.options.flat)
            # break path into HPGL commands
            xPosOld = -1
            yPosOld = -1
            for sp in p:
                cmd = 'PU'
                for csp in sp:
                    xPos = csp[1][0]
                    yPos = csp[1][1]
                    if int(xPos) != int(xPosOld) or int(yPos) != int(yPosOld): 
                        self.calcOffset(cmd, xPos, yPos)
                        cmd = 'PD'
                        xPosOld = xPos
                        yPosOld = yPos
                # perform overcut
                if self.options.useOvercut:
                    if int(xPos) == int(sp[0][1][0]) and int(yPos) == int(sp[0][1][1]):
                        for csp in sp:
                            xPos2 = csp[1][0]
                            yPos2 = csp[1][1]
                            if int(xPos) != int(xPos2) or int(yPos) != int(yPos2):
                                self.calcOffset(cmd, xPos2, yPos2)
                                if self.options.overcut - self.getLength(xPosOld, yPosOld, xPos2, yPos2) <= 0:
                                    break                                      
                                xPos = xPos2
                                yPos = yPos2

    def getLength(self, x1, y1, x2, y2, abs = True):
        # calc absoulute or relative length of two points
        if abs: return math.fabs(math.sqrt((x2 - x1) ** 2.0 + (y2 - y1) ** 2.0))
        else: return math.sqrt((x2 - x1) ** 2.0 + (y2 - y1) ** 2.0)

    def getAlpha(self, x1, y1, x2, y2, x3, y3):
        temp1 = (x1-x2)**2 + (y1-y2)**2 + (x3-x2)**2 + (y3-y2)**2 - (x1-x3)**2 - (y1-y3)**2
        temp2 = 2 * math.sqrt((x1-x2)**2 + (y1-y2)**2) * math.sqrt((x3-x2)**2 + (y3-y2)**2)
        temp3 = temp1 / temp2
        if temp3 < -1:
            temp3 = -1
        return math.acos(temp3)

    def calcOffset(self, cmd, xPos, yPos):
        # calculate offset correction (or dont))
        if not self.options.correctToolOffset:
            self.storeData(cmd, xPos, yPos)
        else:
            self.vData.pop(0)
            self.vData.insert(3, [cmd, xPos, yPos])
            if self.vData[2][1] != -1.0:
                if self.vData[1][1] != -1.0:
                    if self.vData[2][0] == 'PD' and self.vData[3][0] == 'PD':
                        if self.getAlpha(self.vData[1][1], self.vData[1][2], self.vData[2][1], self.vData[2][2], self.vData[3][1], self.vData[3][2]) > 2.748893:
                            self.storeData(self.vData[2][0], self.vData[2][1], self.vData[2][2])
                            return
                    if self.vData[2][0] == 'PD':
                        self.storeData('PD', 
                            self.changeLengthX(self.vData[1][1], self.vData[1][2], self.vData[2][1], self.vData[2][2], self.options.toolOffset), 
                            self.changeLengthY(self.vData[1][1], self.vData[1][2], self.vData[2][1], self.vData[2][2], self.options.toolOffset))
                    elif self.vData[0][1] != -1.0:
                        self.storeData('PU', 
                            self.vData[2][1] - (self.vData[1][1] - self.changeLengthX(self.vData[0][1], self.vData[0][2], self.vData[1][1], self.vData[1][2], self.options.toolOffset)), 
                            self.vData[2][2] - (self.vData[1][2] - self.changeLengthY(self.vData[0][1], self.vData[0][2], self.vData[1][1], self.vData[1][2], self.options.toolOffset)))
                    else:
                        self.storeData('PU', 
                            self.vData[2][1], 
                            self.vData[2][2])
                    if self.vData[3][0] == 'PD':
                        self.storeData('PD', 
                            self.changeLengthX(self.vData[3][1], self.vData[3][2], self.vData[2][1], self.vData[2][2], -(self.options.toolOffset * self.options.toolOffsetReturn)), 
                            self.changeLengthY(self.vData[3][1], self.vData[3][2], self.vData[2][1], self.vData[2][2], -(self.options.toolOffset * self.options.toolOffsetReturn)))
                else:
                    self.storeData(self.vData[2][0], self.vData[2][1], self.vData[2][2])

    def storeData(self, command, x, y):
        if self.dryRun:
            if x < self.xDivergence: self.xDivergence = x 
            if y < self.yDivergence: self.yDivergence = y
            if x > self.xSize: self.xSize = x
            if y > self.ySize: self.ySize = y
        else:
            if not self.options.center:
                if x < 0: x = 0 # only positive values are allowed
                if y < 0: y = 0
            self.hpgl += '%s%d,%d;' % (command, x, y)
            if self.options.sendToPlotter:
                self.S.write('%s%d,%d;' % (command, x, y))

    def changeLengthX(self, x1, y1, x2, y2, offset):
        # change length between two points - x axis
        if offset < 0: offset = max(-self.getLength(x1, y1, x2, y2), offset)
        return x2 + (x2 - x1) / self.getLength(x1, y1, x2, y2, False) * offset;

    def changeLengthY(self, x1, y1, x2, y2, offset):
        # change length between two points - y axis
        if offset < 0: offset = max(-self.getLength(x1, y1, x2, y2), offset)
        return y2 + (y2 - y1) / self.getLength(x1, y1, x2, y2, False) * offset;

    def output(self):
        # print to file
        print self.hpgl

if __name__ == '__main__':
    # Raise recursion limit to avoid exceptions on big documents
    sys.setrecursionlimit(20000);
    # start calculations
    e = MyEffect()
    e.affect()

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
