# coding=utf-8
'''
Copyright (C) 2008 Aaron Spike, aaron@ekips.org
Copyright (C) 2013 Sebastian Wüst, sebi@timewaster.de

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

# standard libraries
import math
import re
import string
# local libraries
import bezmisc
import cspsubdiv
import cubicsuperpath
import inkex
import simplestyle
import simpletransform


class hpglEncoder:
    PI = math.pi
    TWO_PI = PI * 2

    def __init__(self, effect):
        ''' options:
                "resolutionX":float
                "resolutionY":float
                "pen":int
                "force:int
                "speed:int
                "orientation":string // "0", "90", "-90", "180"
                "mirrorX":bool
                "mirrorY":bool
                "center":bool
                "flat":float
                "overcut":float
                "toolOffset":float
                "precut":bool
                "autoAlign":bool
                "debug":bool
        '''
        self.options = effect.options
        self.doc = effect.document.getroot()
        self.docWidth = effect.unittouu(self.doc.get('width'))
        self.docHeight = effect.unittouu(self.doc.get('height'))
        self.hpgl = ''
        self.divergenceX = 'False'
        self.divergenceY = 'False'
        self.sizeX = 'False'
        self.sizeY = 'False'
        self.dryRun = True
        self.lastPoint = [0, 0, 0]
        self.lastPen = -1
        self.offsetX = 0
        self.offsetY = 0
        self.scaleX = self.options.resolutionX / effect.unittouu("1.0in") # dots per inch to dots per user unit
        self.scaleY = self.options.resolutionY / effect.unittouu("1.0in") # dots per inch to dots per user unit
        scaleXY = (self.scaleX + self.scaleY) / 2
        self.overcut = effect.unittouu(str(self.options.overcut) + "mm") * scaleXY # mm to dots (plotter coordinate system)
        self.toolOffset = effect.unittouu(str(self.options.toolOffset) + "mm") * scaleXY # mm to dots
        self.flat = self.options.flat / (1016 / ((self.options.resolutionX + self.options.resolutionY) / 2)) # scale flatness to resolution
        if self.toolOffset > 0.0:
            self.toolOffsetFlat = self.flat / self.toolOffset * 4.5 # scale flatness to offset
        else:
            self.toolOffsetFlat = 0.0
        self.mirrorX = 1.0
        if self.options.mirrorX:
            self.mirrorX = -1.0
        self.mirrorY = -1.0
        if self.options.mirrorY:
            self.mirrorY = 1.0
        if self.options.debug:
            self.debugValues = {}
            self.debugValues['docWidth'] = self.docWidth
            self.debugValues['docHeight'] = self.docHeight
        # process viewBox attribute to correct page scaling
        self.viewBoxTransformX = 1
        self.viewBoxTransformY = 1
        if self.options.debug:
            self.debugValues['viewBoxWidth'] = "-"
            self.debugValues['viewBoxHeight'] = "-"
        viewBox = self.doc.get('viewBox')
        if viewBox:
            viewBox2 = string.split(viewBox, ',')
            if len(viewBox2) < 4:
                viewBox2 = string.split(viewBox, ' ')
            if self.options.debug:
                self.debugValues['viewBoxWidth'] = viewBox2[2]
                self.debugValues['viewBoxHeight'] = viewBox2[3]
            self.viewBoxTransformX = self.docWidth / effect.unittouu(effect.addDocumentUnit(viewBox2[2]))
            self.viewBoxTransformY = self.docHeight / effect.unittouu(effect.addDocumentUnit(viewBox2[3]))

    def getHpgl(self):
        # dryRun to find edges
        groupmat = [[self.mirrorX * self.scaleX * self.viewBoxTransformX, 0.0, 0.0], [0.0, self.mirrorY * self.scaleY * self.viewBoxTransformY, 0.0]]
        groupmat = simpletransform.composeTransform(groupmat, simpletransform.parseTransform('rotate(' + self.options.orientation + ')'))
        self.vData = [['', 'False', 0, 0], ['', 'False', 0, 0], ['', 'False', 0, 0], ['', 'False', 0, 0]]
        self.processGroups(self.doc, groupmat)
        if self.divergenceX == 'False' or self.divergenceY == 'False' or self.sizeX == 'False' or self.sizeY == 'False':
            raise Exception('NO_PATHS')
        # live run
        self.dryRun = False
        if self.options.debug:
            self.debugValues['drawingWidth'] = self.sizeX - self.divergenceX
            self.debugValues['drawingHeight'] = self.sizeY - self.divergenceY
            self.debugValues['drawingWidthUU'] = self.debugValues['drawingWidth'] / self.scaleX
            self.debugValues['drawingHeightUU'] = self.debugValues['drawingHeight'] / self.scaleY
        # move drawing according to various modifiers
        if self.options.autoAlign:
            if self.options.center:
                self.offsetX -= (self.sizeX - self.divergenceX) / 2
                self.offsetY -= (self.sizeY - self.divergenceY) / 2
        else:
            self.divergenceX = 0.0
            self.divergenceY = 0.0
            if self.options.center:
                if self.options.orientation == '0':
                    self.offsetX -= (self.docWidth * self.scaleX) / 2
                    self.offsetY += (self.docHeight * self.scaleY) / 2
                if self.options.orientation == '90':
                    self.offsetY += (self.docWidth * self.scaleX) / 2
                    self.offsetX += (self.docHeight * self.scaleY) / 2
                if self.options.orientation == '180':
                    self.offsetX += (self.docWidth * self.scaleX) / 2
                    self.offsetY -= (self.docHeight * self.scaleY) / 2
                if self.options.orientation == '270':
                    self.offsetY -= (self.docWidth * self.scaleX) / 2
                    self.offsetX -= (self.docHeight * self.scaleY) / 2
            else:
                if self.options.orientation == '0':
                    self.offsetY += self.docHeight * self.scaleY
                if self.options.orientation == '90':
                    self.offsetY += self.docWidth * self.scaleX
                    self.offsetX += self.docHeight * self.scaleY
                if self.options.orientation == '180':
                    self.offsetX += self.docWidth * self.scaleX
        if not self.options.center and self.toolOffset > 0.0:
            self.offsetX += self.toolOffset
            self.offsetY += self.toolOffset
        # initialize transformation matrix and cache
        groupmat = [[self.mirrorX * self.scaleX * self.viewBoxTransformX, 0.0, -self.divergenceX + self.offsetX],
            [0.0, self.mirrorY * self.scaleY * self.viewBoxTransformY, -self.divergenceY + self.offsetY]]
        groupmat = simpletransform.composeTransform(groupmat, simpletransform.parseTransform('rotate(' + self.options.orientation + ')'))
        self.vData = [['', 'False', 0, 0], ['', 'False', 0, 0], ['', 'False', 0, 0], ['', 'False', 0, 0]]
        # add move to zero point and precut
        if self.toolOffset > 0.0 and self.options.precut:
            if self.options.center:
                # position precut outside of drawing plus one time the tooloffset
                if self.offsetX >= 0.0:
                    precutX = self.offsetX + self.toolOffset
                else:
                    precutX = self.offsetX - self.toolOffset
                if self.offsetY >= 0.0:
                    precutY = self.offsetY + self.toolOffset
                else:
                    precutY = self.offsetY - self.toolOffset
                self.processOffset('PU', precutX, precutY, self.options.pen)
                self.processOffset('PD', precutX, precutY + self.toolOffset * 8, self.options.pen)
            else:
                self.processOffset('PU', 0, 0, self.options.pen)
                self.processOffset('PD', 0, self.toolOffset * 8, self.options.pen)
        # start conversion
        self.processGroups(self.doc, groupmat)
        # shift an empty node in in order to process last node in cache
        if self.toolOffset > 0.0 and not self.dryRun:
            self.processOffset('PU', 0, 0, 0)
        if self.options.debug:
            return self.hpgl, self
        else:
            return self.hpgl, ""

    def processGroups(self, doc, groupmat):
        # flatten layers and groups to avoid recursion
        paths = []
        for node in doc:
            if (node.tag == inkex.addNS('g', 'svg') and self.isGroupVisible(node)) or node.tag == inkex.addNS('path', 'svg'):
                paths.append([node.tag, node, self.mergeTransform(node, groupmat), self.getPenNumber(node)])
        doc = ''
        hasGroups = True
        while hasGroups:
            hasGroups = False
            for i, elm in enumerate(paths):
                if paths[i][0] == inkex.addNS('g', 'svg') and self.isGroupVisible(paths[i][1]):
                    hasGroups = True
                    for path in paths[i][1]:
                        if (path.tag == inkex.addNS('g', 'svg') and self.isGroupVisible(path)) or path.tag == inkex.addNS('path', 'svg'):
                            paths.insert(i + 1, [path.tag, path, self.mergeTransform(path, paths[i][2]), paths[i][3]])
                    paths[i][0] = ''
        for node in paths:
            if node[0] == inkex.addNS('path', 'svg'):
                self.processPath(node[1], node[2], node[3])

    def getPenNumber(self, doc):
        penNum = str(doc.get('{' + inkex.NSS['inkscape'] + '}label')).lower().strip(' \t\n\r')
        if re.search(r'( |\A)pen *\d+( |\Z)', penNum):
            penNum = re.sub(r'(.* |\A)pen *(\d+)( .*|\Z)', r'\2', penNum, 1)
            return int(penNum)
        else:
            return self.options.pen

    def mergeTransform(self, doc, matrix):
        # get and merge two matrixes into one
        trans = doc.get('transform')
        if trans:
            return simpletransform.composeTransform(matrix, simpletransform.parseTransform(trans))
        else:
            return matrix

    def isGroupVisible(self, group):
        style = group.get('style')
        if style:
            style = simplestyle.parseStyle(style)
            if 'display' in style and style['display'] == 'none':
                return False
        return True

    def processPath(self, node, mat, pen):
        # process path
        path = node.get('d')
        if path:
            # parse and transform path
            path = cubicsuperpath.parsePath(path)
            simpletransform.applyTransformToPath(mat, path)
            cspsubdiv.cspsubdiv(path, self.flat)
            # path to HPGL commands
            oldPosX = 0.0
            oldPosY = 0.0
            for singlePath in path:
                cmd = 'PU'
                for singlePathPoint in singlePath:
                    posX, posY = singlePathPoint[1]
                    # check if point is repeating, if so, ignore
                    if int(round(posX)) != int(round(oldPosX)) or int(round(posY)) != int(round(oldPosY)):
                        self.processOffset(cmd, posX, posY, pen)
                        cmd = 'PD'
                        oldPosX = posX
                        oldPosY = posY
                # perform overcut
                if self.overcut > 0.0 and not self.dryRun:
                    # check if last and first points are the same, otherwise the path is not closed and no overcut can be performed
                    if int(round(oldPosX)) == int(round(singlePath[0][1][0])) and int(round(oldPosY)) == int(round(singlePath[0][1][1])):
                        overcutLength = 0
                        for singlePathPoint in singlePath:
                            posX, posY = singlePathPoint[1]
                            # check if point is repeating, if so, ignore
                            if int(round(posX)) != int(round(oldPosX)) or int(round(posY)) != int(round(oldPosY)):
                                overcutLength += self.getLength(oldPosX, oldPosY, posX, posY)
                                if overcutLength >= self.overcut:
                                    newLength = self.changeLength(oldPosX, oldPosY, posX, posY, - (overcutLength - self.overcut))
                                    self.processOffset(cmd, newLength[0], newLength[1], pen)
                                    break
                                else:
                                    self.processOffset(cmd, posX, posY, pen)
                                oldPosX = posX
                                oldPosY = posY

    def getLength(self, x1, y1, x2, y2, absolute=True):
        # calc absoulute or relative length between two points
        length = math.sqrt((x2 - x1) ** 2.0 + (y2 - y1) ** 2.0)
        if absolute:
            length = math.fabs(length)
        return length

    def changeLength(self, x1, y1, x2, y2, offset):
        # change length of line
        if offset < 0:
            offset = max( - self.getLength(x1, y1, x2, y2), offset)
        x = x2 + (x2 - x1) / self.getLength(x1, y1, x2, y2, False) * offset
        y = y2 + (y2 - y1) / self.getLength(x1, y1, x2, y2, False) * offset
        return [x, y]

    def processOffset(self, cmd, posX, posY, pen):
        # calculate offset correction (or dont)
        if self.toolOffset == 0.0 or self.dryRun:
            self.storePoint(cmd, posX, posY, pen)
        else:
            # insert data into cache
            self.vData.pop(0)
            self.vData.insert(3, [cmd, posX, posY, pen])
            # decide if enough data is availabe
            if self.vData[2][1] != 'False':
                if self.vData[1][1] == 'False':
                    self.storePoint(self.vData[2][0], self.vData[2][1], self.vData[2][2], self.vData[2][3])
                else:
                    # perform tool offset correction (It's a *tad* complicated, if you want to understand it draw the data as lines on paper)
                    if self.vData[2][0] == 'PD': # If the 3rd entry in the cache is a pen down command make the line longer by the tool offset
                        pointThree = self.changeLength(self.vData[1][1], self.vData[1][2], self.vData[2][1], self.vData[2][2], self.toolOffset)
                        self.storePoint('PD', pointThree[0], pointThree[1], self.vData[2][3])
                    elif self.vData[0][1] != 'False':
                        # Elif the 1st entry in the cache is filled with data and the 3rd entry is a pen up command shift
                        # the 3rd entry by the current tool offset position according to the 2nd command
                        pointThree = self.changeLength(self.vData[0][1], self.vData[0][2], self.vData[1][1], self.vData[1][2], self.toolOffset)
                        pointThree[0] = self.vData[2][1] - (self.vData[1][1] - pointThree[0])
                        pointThree[1] = self.vData[2][2] - (self.vData[1][2] - pointThree[1])
                        self.storePoint('PU', pointThree[0], pointThree[1], self.vData[2][3])
                    else:
                        # Else just write the 3rd entry
                        pointThree = [self.vData[2][1], self.vData[2][2]]
                        self.storePoint('PU', pointThree[0], pointThree[1], self.vData[2][3])
                    if self.vData[3][0] == 'PD':
                        # If the 4th entry in the cache is a pen down command guide tool to next line with a circle between the prolonged 3rd and 4th entry
                        if self.getLength(self.vData[2][1], self.vData[2][2], self.vData[3][1], self.vData[3][2]) >= self.toolOffset:
                            pointFour = self.changeLength(self.vData[3][1], self.vData[3][2], self.vData[2][1], self.vData[2][2], - self.toolOffset)
                        else:
                            pointFour = self.changeLength(self.vData[2][1], self.vData[2][2], self.vData[3][1], self.vData[3][2],
                                (self.toolOffset - self.getLength(self.vData[2][1], self.vData[2][2], self.vData[3][1], self.vData[3][2])))
                        # get angle start and angle vector
                        angleStart = math.atan2(pointThree[1] - self.vData[2][2], pointThree[0] - self.vData[2][1])
                        angleVector = math.atan2(pointFour[1] - self.vData[2][2], pointFour[0] - self.vData[2][1]) - angleStart
                        # switch direction when arc is bigger than 180°
                        if angleVector > self.PI:
                            angleVector -= self.TWO_PI
                        elif angleVector < - self.PI:
                            angleVector += self.TWO_PI
                        # draw arc
                        if angleVector >= 0:
                            angle = angleStart + self.toolOffsetFlat
                            while angle < angleStart + angleVector:
                                self.storePoint('PD', self.vData[2][1] + math.cos(angle) * self.toolOffset, self.vData[2][2] + math.sin(angle) * self.toolOffset, self.vData[2][3])
                                angle += self.toolOffsetFlat
                        else:
                            angle = angleStart - self.toolOffsetFlat
                            while angle > angleStart + angleVector:
                                self.storePoint('PD', self.vData[2][1] + math.cos(angle) * self.toolOffset, self.vData[2][2] + math.sin(angle) * self.toolOffset, self.vData[2][3])
                                angle -= self.toolOffsetFlat
                        self.storePoint('PD', pointFour[0], pointFour[1], self.vData[3][3])

    def storePoint(self, command, x, y, pen):
        x = int(round(x))
        y = int(round(y))
        # skip when no change in movement
        if self.lastPoint[0] == command and self.lastPoint[1] == x and self.lastPoint[2] == y:
            return
        if self.dryRun:
            # find edges
            if self.divergenceX == 'False' or x < self.divergenceX:
                self.divergenceX = x
            if self.divergenceY == 'False' or y < self.divergenceY:
                self.divergenceY = y
            if self.sizeX == 'False' or x > self.sizeX:
                self.sizeX = x
            if self.sizeY == 'False' or y > self.sizeY:
                self.sizeY = y
        else:
            # store point
            if not self.options.center:
                # only positive values are allowed (usually)
                if x < 0:
                    x = 0
                if y < 0:
                    y = 0
            # select correct pen
            if self.lastPen != pen:
                self.hpgl += ';SP%d' % pen
            # do not repeat command
            if command == 'PD' and self.lastPoint[0] == 'PD' and self.lastPen == pen:
                self.hpgl += ',%d,%d' % (x, y)
            else:
                self.hpgl += ';%s%d,%d' % (command, x, y)
            self.lastPen = pen
        self.lastPoint = [command, x, y]

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
