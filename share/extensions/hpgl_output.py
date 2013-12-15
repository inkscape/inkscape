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
import sys
# local libraries
import hpgl_encoder
import inkex
inkex.localize()


# TODO: Unittests
class MyEffect(inkex.Effect):

    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option('--tab',              action='store', type='string',  dest='tab')
        self.OptionParser.add_option('--resolutionX',      action='store', type='float',   dest='resolutionX',      default=1016.0,  help='Resolution X (dpi)')
        self.OptionParser.add_option('--resolutionY',      action='store', type='float',   dest='resolutionY',      default=1016.0,  help='Resolution Y (dpi)')
        self.OptionParser.add_option('--pen',              action='store', type='int',     dest='pen',              default=1,       help='Pen number')
        self.OptionParser.add_option('--orientation',      action='store', type='string',  dest='orientation',      default='90',    help='Rotation (Clockwise)')
        self.OptionParser.add_option('--mirrorX',          action='store', type='inkbool', dest='mirrorX',          default='FALSE', help='Mirror X-axis')
        self.OptionParser.add_option('--mirrorY',          action='store', type='inkbool', dest='mirrorY',          default='FALSE', help='Mirror Y-axis')
        self.OptionParser.add_option('--center',           action='store', type='inkbool', dest='center',           default='FALSE', help='Center zero point')
        self.OptionParser.add_option('--flat',             action='store', type='float',   dest='flat',             default=1.2,     help='Curve flatness')
        self.OptionParser.add_option('--useOvercut',       action='store', type='inkbool', dest='useOvercut',       default='TRUE',  help='Use overcut')
        self.OptionParser.add_option('--overcut',          action='store', type='float',   dest='overcut',          default=1.0,     help='Overcut (mm)')
        self.OptionParser.add_option('--useToolOffset',    action='store', type='inkbool', dest='useToolOffset',    default='TRUE',  help='Correct tool offset')
        self.OptionParser.add_option('--toolOffset',       action='store', type='float',   dest='toolOffset',       default=0.25,    help='Tool offset (mm)')
        self.OptionParser.add_option('--precut',           action='store', type='inkbool', dest='precut',           default='TRUE',  help='Use precut')
        self.OptionParser.add_option('--offsetX',          action='store', type='float',   dest='offsetX',          default=0.0,     help='X offset (mm)')
        self.OptionParser.add_option('--offsetY',          action='store', type='float',   dest='offsetY',          default=0.0,     help='Y offset (mm)')

    def effect(self):
        self.options.debug = False
        # get hpgl data
        myHpglEncoder = hpgl_encoder.hpglEncoder(self)
        try:
            self.hpgl, debugObject = myHpglEncoder.getHpgl()
        except Exception as inst:
            if inst.args[0] == 'NO_PATHS':
                # issue error if no paths found
                inkex.errormsg(_("No paths where found. Please convert all objects you want to save into paths."))
                self.hpgl = ''
                return
            else:
                type, value, traceback = sys.exc_info()
                raise ValueError, ("", type, value), traceback

    def output(self):
        # print to file
        if self.hpgl != '':
            print self.hpgl

if __name__ == '__main__':
    # start extension
    e = MyEffect()
    e.affect()

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99