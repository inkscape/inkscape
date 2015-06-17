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


class HpglOutput(inkex.Effect):

    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option('--tab',           action='store', type='string',  dest='tab')
        self.OptionParser.add_option('--resolutionX',   action='store', type='float',   dest='resolutionX',   default=1016.0,  help='Resolution X (dpi)')
        self.OptionParser.add_option('--resolutionY',   action='store', type='float',   dest='resolutionY',   default=1016.0,  help='Resolution Y (dpi)')
        self.OptionParser.add_option('--pen',           action='store', type='int',     dest='pen',           default=1,       help='Pen number')
        self.OptionParser.add_option('--force',         action='store', type='int',     dest='force',         default=24,      help='Pen force (g)')
        self.OptionParser.add_option('--speed',         action='store', type='int',     dest='speed',         default=20,      help='Pen speed (cm/s)')
        self.OptionParser.add_option('--orientation',   action='store', type='string',  dest='orientation',   default='90',    help='Rotation (Clockwise)')
        self.OptionParser.add_option('--mirrorX',       action='store', type='inkbool', dest='mirrorX',       default='FALSE', help='Mirror X axis')
        self.OptionParser.add_option('--mirrorY',       action='store', type='inkbool', dest='mirrorY',       default='FALSE', help='Mirror Y axis')
        self.OptionParser.add_option('--center',        action='store', type='inkbool', dest='center',        default='FALSE', help='Center zero point')
        self.OptionParser.add_option('--overcut',       action='store', type='float',   dest='overcut',       default=1.0,     help='Overcut (mm)')
        self.OptionParser.add_option('--toolOffset',    action='store', type='float',   dest='toolOffset',    default=0.25,    help='Tool offset (mm)')
        self.OptionParser.add_option('--precut',        action='store', type='inkbool', dest='precut',        default='TRUE',  help='Use precut')
        self.OptionParser.add_option('--flat',          action='store', type='float',   dest='flat',          default=1.2,     help='Curve flatness')
        self.OptionParser.add_option('--autoAlign',     action='store', type='inkbool', dest='autoAlign',     default='TRUE',  help='Auto align')

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
        # convert raw HPGL to HPGL
        hpglInit = 'IN'
        if self.options.force > 0:
            hpglInit += ';FS%d' % self.options.force
        if self.options.speed > 0:
            hpglInit += ';VS%d' % self.options.speed
        self.hpgl = hpglInit + self.hpgl + ';SP0;PU0,0;IN; '

    def output(self):
        # print to file
        if self.hpgl != '':
            print self.hpgl

if __name__ == '__main__':
    # start extension
    e = HpglOutput()
    e.affect()

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99