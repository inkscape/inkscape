#!/usr/bin/env python
# coding=utf-8
'''
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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
'''

# standard library
import re
import string
import sys
# local libraries
import gettext
import hpgl_decoder
import hpgl_encoder
import inkex
inkex.localize()


class Plot(inkex.Effect):

    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option('--tab',               action='store', type='string',  dest='tab')
        self.OptionParser.add_option('--serialPort',        action='store', type='string',  dest='serialPort',        default='COM1',  help='Serial port')
        self.OptionParser.add_option('--serialBaudRate',    action='store', type='string',  dest='serialBaudRate',    default='9600',  help='Serial Baud rate')
        self.OptionParser.add_option('--serialByteSize',    action='store', type='string',  dest='serialByteSize',    default='eight', help='Serial byte size')
        self.OptionParser.add_option('--serialStopBits',    action='store', type='string',  dest='serialStopBits',    default='one',   help='Serial stop bits')
        self.OptionParser.add_option('--serialParity',      action='store', type='string',  dest='serialParity',      default='none',  help='Serial parity')
        self.OptionParser.add_option('--serialFlowControl', action='store', type='string',  dest='serialFlowControl', default='0',     help='Flow control')
        self.OptionParser.add_option('--commandLanguage',   action='store', type='string',  dest='commandLanguage',   default='hpgl',  help='Command Language')
        self.OptionParser.add_option('--resolutionX',       action='store', type='float',   dest='resolutionX',       default=1016.0,  help='Resolution X (dpi)')
        self.OptionParser.add_option('--resolutionY',       action='store', type='float',   dest='resolutionY',       default=1016.0,  help='Resolution Y (dpi)')
        self.OptionParser.add_option('--pen',               action='store', type='int',     dest='pen',               default=1,       help='Pen number')
        self.OptionParser.add_option('--force',             action='store', type='int',     dest='force',             default=24,      help='Pen force (g)')
        self.OptionParser.add_option('--speed',             action='store', type='int',     dest='speed',             default=20,      help='Pen speed (cm/s)')
        self.OptionParser.add_option('--orientation',       action='store', type='string',  dest='orientation',       default='90',    help='Rotation (Clockwise)')
        self.OptionParser.add_option('--mirrorX',           action='store', type='inkbool', dest='mirrorX',           default='FALSE', help='Mirror X axis')
        self.OptionParser.add_option('--mirrorY',           action='store', type='inkbool', dest='mirrorY',           default='FALSE', help='Mirror Y axis')
        self.OptionParser.add_option('--center',            action='store', type='inkbool', dest='center',            default='FALSE', help='Center zero point')
        self.OptionParser.add_option('--overcut',           action='store', type='float',   dest='overcut',           default=1.0,     help='Overcut (mm)')
        self.OptionParser.add_option('--toolOffset',        action='store', type='float',   dest='toolOffset',        default=0.25,    help='Tool (Knife) offset correction (mm)')
        self.OptionParser.add_option('--precut',            action='store', type='inkbool', dest='precut',            default='TRUE',  help='Use precut')
        self.OptionParser.add_option('--flat',              action='store', type='float',   dest='flat',              default=1.2,     help='Curve flatness')
        self.OptionParser.add_option('--autoAlign',         action='store', type='inkbool', dest='autoAlign',         default='TRUE',  help='Auto align')
        self.OptionParser.add_option('--debug',             action='store', type='inkbool', dest='debug',             default='FALSE', help='Show debug information')

    def effect(self):
        # get hpgl data
        myHpglEncoder = hpgl_encoder.hpglEncoder(self)
        try:
            self.hpgl, debugObject = myHpglEncoder.getHpgl()
        except Exception as inst:
            if inst.args[0] == 'NO_PATHS':
                # issue error if no paths found
                inkex.errormsg(_("No paths where found. Please convert all objects you want to plot into paths."))
                return 1
            else:
                type, value, traceback = sys.exc_info()
                raise ValueError, ('', type, value), traceback
        # TODO: Get preview to work. This requires some work on the C++ side to be able to determine if it is
        # a preview or a final run. (Remember to set <effect needs-live-preview='false'> to true)
        '''
        if MAGIC:
            # reparse data for preview
            self.options.showMovements = True
            self.options.docWidth = self.uutounit(self.unittouu(self.document.getroot().get('width')), "px")
            self.options.docHeight = self.uutounit(self.unittouu(self.document.getroot().get('height')), "px")
            myHpglDecoder = hpgl_decoder.hpglDecoder(self.hpgl, self.options)
            doc, warnings = myHpglDecoder.getSvg()
            # deliver document to inkscape
            self.document = doc
        else:
        '''
        # convert to other formats
        if self.options.commandLanguage == 'HPGL':
            self.convertToHpgl()
        if self.options.commandLanguage == 'DMPL':
            self.convertToDmpl()
        if self.options.commandLanguage == 'KNK':
            self.convertToKNK()
        # output
        if self.options.debug:
            self.showDebugInfo(debugObject)
        else:
            self.sendHpglToSerial()

    def convertToHpgl(self):
        # convert raw HPGL to HPGL
        hpglInit = 'IN'
        if self.options.force > 0:
            hpglInit += ';FS%d' % self.options.force
        if self.options.speed > 0:
            hpglInit += ';VS%d' % self.options.speed
        self.hpgl = hpglInit + self.hpgl + ';SP0;PU0,0;IN; '

    def convertToDmpl(self):
        # convert HPGL to DMPL
        # ;: = Initialise plotter
        # H = Home position
        # A = Absolute pen positioning
        # Ln = Line type
        # Pn = Pen select
        # Vn = velocity
        # ECn = Coordinate addressing, 1: 0.001 inch, 5: 0.005 inch, M: 0.1 mm
        # D = Pen down
        # U = Pen up
        # Z = Reset plotter
        # n,n, = Coordinate pair
        self.hpgl = self.hpgl.replace(';', ',')
        self.hpgl = self.hpgl.replace('SP', 'P')
        self.hpgl = self.hpgl.replace('PU', 'U')
        self.hpgl = self.hpgl.replace('PD', 'D')
        dmplInit = ';:HAL0'
        if self.options.speed > 0:
            dmplInit += 'V%d' % self.options.speed
        dmplInit += 'EC1'
        self.hpgl = dmplInit + self.hpgl[1:] + ',P0,U0,0,Z '

    def convertToKNK(self):
        # convert HPGL to KNK Plotter Language
        hpglInit = 'ZG'
        if self.options.force > 0:
            hpglInit += ';FS%d' % self.options.force
        if self.options.speed > 0:
            hpglInit += ';VS%d' % self.options.speed
        self.hpgl = hpglInit + self.hpgl + ';SP0;PU0,0;@ '

    def sendHpglToSerial(self):
        # gracefully exit script when pySerial is missing
        try:
            import serial
        except ImportError, e:
            inkex.errormsg(_("pySerial is not installed."
                + "\n\n1. Download pySerial here (not the \".exe\"!): http://pypi.python.org/pypi/pyserial"
                + "\n2. Extract the \"serial\" subfolder from the zip to the following folder: C:\\[Program files]\\inkscape\\python\\Lib\\"
                + "\n3. Restart Inkscape."))
            return
        # init serial framework
        mySerial = serial.Serial()
        # set serial port
        mySerial.port = self.options.serialPort
        # set baudrate
        mySerial.baudrate = self.options.serialBaudRate
        # set bytesize
        if self.options.serialByteSize == 'five':
            mySerial.bytesize = serial.FIVEBITS
        if self.options.serialByteSize == 'six':
            mySerial.bytesize = serial.SIXBITS
        if self.options.serialByteSize == 'seven':
            mySerial.bytesize = serial.SEVENBITS
        if self.options.serialByteSize == 'eight':
            mySerial.bytesize = serial.EIGHTBITS
        # set stopbits
        if self.options.serialStopBits == 'one':
            mySerial.stopbits = serial.STOPBITS_ONE
        if self.options.serialStopBits == 'onePointFive':
            mySerial.stopbits = serial.STOPBITS_ONE_POINT_FIVE
        if self.options.serialStopBits == 'two':
            mySerial.stopbits = serial.STOPBITS_TWO
        # set parity
        if self.options.serialParity == 'none':
            mySerial.parity = serial.PARITY_NONE
        if self.options.serialParity == 'even':
            mySerial.parity = serial.PARITY_EVEN
        if self.options.serialParity == 'odd':
            mySerial.parity = serial.PARITY_ODD
        if self.options.serialParity == 'mark':
            mySerial.parity = serial.PARITY_MARK
        if self.options.serialParity == 'space':
            mySerial.parity = serial.PARITY_SPACE
        # set short timeout to avoid locked up interface
        mySerial.timeout = 0.1
        # set flow control
        if self.options.serialFlowControl == 'xonxoff':
            mySerial.xonxoff = True
        if self.options.serialFlowControl == 'rtscts' or self.options.serialFlowControl == 'dsrdtrrtscts':
            mySerial.rtscts = True
        if self.options.serialFlowControl == 'dsrdtrrtscts':
            mySerial.dsrdtr = True
        # try to establish connection
        try:
            mySerial.open()
        except Exception as inst:
            if 'ould not open port' in inst.args[0]:
                inkex.errormsg(_("Could not open port. Please check that your plotter is running, connected and the settings are correct."))
                return
            else:
                type, value, traceback = sys.exc_info()
                raise ValueError, ('', type, value), traceback
        # send data to plotter
        mySerial.write(self.hpgl)
        mySerial.read(2)
        mySerial.close()

    def showDebugInfo(self, debugObject):
        # show debug information
        inkex.errormsg("---------------------------------\nDebug information\n---------------------------------\n\nSettings:\n")
        inkex.errormsg('  Serial Port: ' + self.options.serialPort)
        inkex.errormsg('  Serial baud rate: ' + self.options.serialBaudRate)
        inkex.errormsg('  Serial byte size: ' + self.options.serialByteSize + ' Bits')
        inkex.errormsg('  Serial stop bits: ' + self.options.serialStopBits + ' Bits')
        inkex.errormsg('  Serial parity: ' + self.options.serialParity)
        inkex.errormsg('  Serial Flow control: ' + self.options.serialFlowControl)
        inkex.errormsg('  Command language: ' + self.options.commandLanguage)
        inkex.errormsg('  Resolution X (dpi): ' + str(self.options.resolutionX))
        inkex.errormsg('  Resolution Y (dpi): ' + str(self.options.resolutionY))
        inkex.errormsg('  Pen number: ' + str(self.options.pen))
        inkex.errormsg('  Pen force (g): ' + str(self.options.force))
        inkex.errormsg('  Pen speed (cm/s): ' + str(self.options.speed))
        inkex.errormsg('  Rotation (Clockwise): ' + self.options.orientation)
        inkex.errormsg('  Mirror X axis: ' + str(self.options.mirrorX))
        inkex.errormsg('  Mirror Y axis: ' + str(self.options.mirrorY))
        inkex.errormsg('  Center zero point: ' + str(self.options.center))
        inkex.errormsg('  Overcut (mm): ' + str(self.options.overcut))
        inkex.errormsg('  Tool offset (mm): ' + str(self.options.toolOffset))
        inkex.errormsg('  Use precut: ' + str(self.options.precut))
        inkex.errormsg('  Curve flatness: ' + str(self.options.flat))
        inkex.errormsg('  Auto align: ' + str(self.options.autoAlign))
        inkex.errormsg('  Show debug information: ' + str(self.options.debug))
        inkex.errormsg("\nDocument properties:\n")
        version = self.document.getroot().xpath('//@inkscape:version', namespaces=inkex.NSS)
        if version:
            inkex.errormsg('  Inkscape version: ' + version[0])
        fileName = self.document.getroot().xpath('//@sodipodi:docname', namespaces=inkex.NSS)
        if fileName:
            inkex.errormsg('  Filename: ' + fileName[0])
        inkex.errormsg('  Document unit: ' + self.getDocumentUnit())
        inkex.errormsg('  Width: ' + str(debugObject.debugValues['docWidth']) + ' ' + self.getDocumentUnit())
        inkex.errormsg('  Height: ' + str(debugObject.debugValues['docHeight']) + ' ' + self.getDocumentUnit())
        if debugObject.debugValues['viewBoxWidth'] == "-":
            inkex.errormsg('  Viewbox Width: -')
            inkex.errormsg('  Viewbox Height: -')
        else:
            inkex.errormsg('  Viewbox Width: ' + str(self.unittouu(self.addDocumentUnit(debugObject.debugValues['viewBoxWidth']))) + ' ' + self.getDocumentUnit())
            inkex.errormsg('  Viewbox Height: ' + str(self.unittouu(self.addDocumentUnit(debugObject.debugValues['viewBoxHeight']))) + ' ' + self.getDocumentUnit())
        inkex.errormsg("\n" + self.options.commandLanguage + " properties:\n")
        inkex.errormsg('  Drawing width: ' + str(self.unittouu(self.addDocumentUnit(str(debugObject.debugValues['drawingWidthUU'])))) + ' ' + self.getDocumentUnit())
        inkex.errormsg('  Drawing height: ' + str(self.unittouu(self.addDocumentUnit(str(debugObject.debugValues['drawingHeightUU'])))) + ' ' + self.getDocumentUnit())
        inkex.errormsg('  Drawing width: ' + str(debugObject.debugValues['drawingWidth']) + ' plotter steps')
        inkex.errormsg('  Drawing height: ' + str(debugObject.debugValues['drawingHeight']) + ' plotter steps')
        inkex.errormsg('  Offset X: ' + str(debugObject.offsetX) + ' plotter steps')
        inkex.errormsg('  Offset Y: ' + str(debugObject.offsetX) + ' plotter steps')
        inkex.errormsg('  Overcut: ' + str(debugObject.overcut) + ' plotter steps')
        inkex.errormsg('  Tool offset: ' + str(debugObject.toolOffset) + ' plotter steps')
        inkex.errormsg('  Flatness: ' + str(debugObject.flat) + ' plotter steps')
        inkex.errormsg('  Tool offset flatness: ' + str(debugObject.toolOffsetFlat) + ' plotter steps')
        inkex.errormsg("\n" + self.options.commandLanguage + " data:\n")
        inkex.errormsg(self.hpgl)

if __name__ == '__main__':
    # start extension
    e = Plot()
    e.affect()

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99