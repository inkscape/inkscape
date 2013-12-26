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


# TODO: Unittests
class MyEffect(inkex.Effect):

    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option('--tab',             action='store', type='string',  dest='tab')
        self.OptionParser.add_option('--resolutionX',     action='store', type='float',   dest='resolutionX',     default=1016.0,  help='Resolution X (dpi)')
        self.OptionParser.add_option('--resolutionY',     action='store', type='float',   dest='resolutionY',     default=1016.0,  help='Resolution Y (dpi)')
        self.OptionParser.add_option('--pen',             action='store', type='int',     dest='pen',             default=1,       help='Pen number')
        self.OptionParser.add_option('--force',           action='store', type='int',     dest='force',           default=24,      help='Pen force (g)')
        self.OptionParser.add_option('--speed',           action='store', type='int',     dest='speed',           default=20,      help='Pen speed (cm/s)')
        self.OptionParser.add_option('--orientation',     action='store', type='string',  dest='orientation',     default='90',    help='Rotation (Clockwise)')
        self.OptionParser.add_option('--mirrorX',         action='store', type='inkbool', dest='mirrorX',         default='FALSE', help='Mirror X axis')
        self.OptionParser.add_option('--mirrorY',         action='store', type='inkbool', dest='mirrorY',         default='FALSE', help='Mirror Y axis')
        self.OptionParser.add_option('--center',          action='store', type='inkbool', dest='center',          default='FALSE', help='Center zero point')
        self.OptionParser.add_option('--flat',            action='store', type='float',   dest='flat',            default=1.2,     help='Curve flatness')
        self.OptionParser.add_option('--useOvercut',      action='store', type='inkbool', dest='useOvercut',      default='TRUE',  help='Use overcut')
        self.OptionParser.add_option('--overcut',         action='store', type='float',   dest='overcut',         default=1.0,     help='Overcut (mm)')
        self.OptionParser.add_option('--useToolOffset',   action='store', type='inkbool', dest='useToolOffset',   default='TRUE',  help='Correct tool offset')
        self.OptionParser.add_option('--toolOffset',      action='store', type='float',   dest='toolOffset',      default=0.25,    help='Tool offset (mm)')
        self.OptionParser.add_option('--precut',          action='store', type='inkbool', dest='precut',          default='TRUE',  help='Use precut')
        self.OptionParser.add_option('--offsetX',         action='store', type='float',   dest='offsetX',         default=0.0,     help='X offset (mm)')
        self.OptionParser.add_option('--offsetY',         action='store', type='float',   dest='offsetY',         default=0.0,     help='Y offset (mm)')
        self.OptionParser.add_option('--serialPort',      action='store', type='string',  dest='serialPort',      default='COM1',  help='Serial port')
        self.OptionParser.add_option('--serialBaudRate',  action='store', type='string',  dest='serialBaudRate',  default='9600',  help='Serial Baud rate')
        self.OptionParser.add_option('--flowControl',     action='store', type='string',  dest='flowControl',     default='0',     help='Flow control')
        self.OptionParser.add_option('--commandLanguage', action='store', type='string',  dest='commandLanguage', default='hpgl',  help='Command Language')
        self.OptionParser.add_option('--debug',           action='store', type='inkbool', dest='debug',           default='FALSE', help='Show debug information')

    def effect(self):
        # gracefully exit script when pySerial is missing
        try:
            import serial
        except ImportError, e:
            inkex.errormsg(_("pySerial is not installed."
                + "\n\n1. Download pySerial here (not the \".exe\"!): http://pypi.python.org/pypi/pyserial"
                + "\n2. Extract the \"serial\" subfolder from the zip to the following folder: C:\\[Program files]\\inkscape\\python\\Lib\\"
                + "\n3. Restart Inkscape."))
            return
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
        # This outcommented code has a user unit issue (getSvg produces px, docWidth could be mm or something else)
        '''
        # reparse data for preview
        self.options.showMovements = True
        self.options.docWidth = float(self.unittouu(self.document.getroot().get('width')))
        self.options.docHeight = float(self.unittouu(self.document.getroot().get('height')))
        myHpglDecoder = hpgl_decoder.hpglDecoder(self.hpgl, self.options)
        doc, warnings = myHpglDecoder.getSvg()
        # deliver document to inkscape
        self.document = doc
        '''
        # convert to other formats
        if self.options.commandLanguage == 'DMPL':
            self.convertToDmpl()
        if self.options.commandLanguage == 'ZING':
            self.convertToZing()
        # output
        if self.options.debug:
            self.showDebugInfo(debugObject)
        else:
            self.sendHpglToSerial()

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
        self.hpgl = self.hpgl.replace('PU', 'U')
        self.hpgl = self.hpgl.replace('PD', 'D')
        velocity = ''
        if self.options.speed > 0:
            velocity = 'V' + str(self.options.speed)
        self.hpgl = re.sub(r'IN,SP[0-9]+(,FS[0-9]+)?(,VS[0-9]+)?,', r';:HAL0P' + str(self.options.pen) + velocity + 'EC1', self.hpgl)
        self.hpgl += 'Z'

    def convertToZing(self):
        # convert HPGL to Zing
        self.hpgl = self.hpgl.replace('IN;', 'ZG;')
        self.hpgl += '@'

    def sendHpglToSerial(self):
        # send data to plotter
        mySerial = serial.Serial()
        mySerial.port = self.options.serialPort
        mySerial.baudrate = self.options.serialBaudRate
        mySerial.timeout = 0.1
        if self.options.flowControl == 'xonxoff':
            mySerial.xonxoff = True
        if self.options.flowControl == 'rtscts' or self.options.flowControl == 'dsrdtrrtscts':
            mySerial.rtscts = True
        if self.options.flowControl == 'dsrdtrrtscts':
            mySerial.dsrdtr = True
        try:
            mySerial.open()
        except Exception as inst:
            if 'ould not open port' in inst.args[0]:
                inkex.errormsg(_("Could not open port. Please check that your plotter is running, connected and the settings are correct."))
                return
            else:
                type, value, traceback = sys.exc_info()
                raise ValueError, ('', type, value), traceback
        mySerial.write(self.hpgl)
        mySerial.read(2)
        mySerial.close()

    def showDebugInfo(self, debugObject):
        # show debug information
        inkex.errormsg("---------------------------------\nDebug information\n---------------------------------\n\nSettings:\n")
        inkex.errormsg('  Serial Port: ' + str(self.options.serialPort))
        inkex.errormsg('  Serial baud rate: ' + str(self.options.serialBaudRate))
        inkex.errormsg('  Flow control: ' + str(self.options.flowControl))
        inkex.errormsg('  Command language: ' + str(self.options.commandLanguage))
        inkex.errormsg('  Resolution X (dpi): ' + str(self.options.resolutionX))
        inkex.errormsg('  Resolution Y (dpi): ' + str(self.options.resolutionY))
        inkex.errormsg('  Pen number: ' + str(self.options.pen))
        inkex.errormsg('  Pen force (g): ' + str(self.options.force))
        inkex.errormsg('  Pen speed (cm/s): ' + str(self.options.speed))
        inkex.errormsg('  Rotation (Clockwise): ' + str(self.options.orientation))
        inkex.errormsg('  Mirror X axis: ' + str(self.options.mirrorX))
        inkex.errormsg('  Mirror Y axis: ' + str(self.options.mirrorY))
        inkex.errormsg('  Center zero point: ' + str(self.options.center))
        inkex.errormsg('  Use overcut: ' + str(self.options.useOvercut))
        inkex.errormsg('  Overcut (mm): ' + str(self.options.overcut))
        inkex.errormsg('  Use tool offset correction: ' + str(self.options.useToolOffset))
        inkex.errormsg('  Tool offset (mm): ' + str(self.options.toolOffset))
        inkex.errormsg('  Use precut: ' + str(self.options.precut))
        inkex.errormsg('  Curve flatness: ' + str(self.options.flat))
        inkex.errormsg('  X offset (mm): ' + str(self.options.offsetX))
        inkex.errormsg('  Y offset (mm): ' + str(self.options.offsetY))
        inkex.errormsg('  Show debug information: ' + str(self.options.debug))
        inkex.errormsg("\nDocument properties:\n")
        version = self.document.getroot().xpath('//@inkscape:version', namespaces=inkex.NSS)
        if version:
            inkex.errormsg('  Inkscape version: ' + version[0])
        fileName = self.document.getroot().xpath('//@sodipodi:docname', namespaces=inkex.NSS)
        if fileName:
            inkex.errormsg('  Filename: ' + fileName[0])
        inkex.errormsg('  Document unit: ' + debugObject.documentUnit)
        inkex.errormsg('  Width: ' + str(debugObject.debugValues[0]) + ' ' + debugObject.documentUnit)
        inkex.errormsg('  Height: ' + str(debugObject.debugValues[1]) + ' ' + debugObject.documentUnit)
        if debugObject.debugValues[2] == 0:
            inkex.errormsg('  Viewbox Width: -')
            inkex.errormsg('  Viewbox Height: -')
        else:
            inkex.errormsg('  Viewbox Width: ' + str(debugObject.debugValues[2]) + ' ' + debugObject.documentUnit)
            inkex.errormsg('  Viewbox Height: ' + str(debugObject.debugValues[3]) + ' ' + debugObject.documentUnit)
        inkex.errormsg("\n" + self.options.commandLanguage + " properties:\n")
        inkex.errormsg('  Drawing width: ' + str(debugObject.debugValues[6]) + ' ' + debugObject.documentUnit)
        inkex.errormsg('  Drawing height: ' + str(debugObject.debugValues[7]) + ' ' + debugObject.documentUnit)
        inkex.errormsg('  Drawing width: ' + str(debugObject.debugValues[4]) + ' plotter steps')
        inkex.errormsg('  Drawing height: ' + str(debugObject.debugValues[5]) + ' plotter steps')
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
    e = MyEffect()
    e.affect()

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99