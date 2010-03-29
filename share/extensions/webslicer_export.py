#!/usr/bin/env python
'''
Copyright (C) 2010 Aurelio A. Heckert, aurium (a) gmail dot com

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

import webslicer_effect
import inkex
import gettext
import os.path
import commands

_ = gettext.gettext

class WebSlicer_Export(webslicer_effect.WebSlicer_Effect):

    def __init__(self):
        webslicer_effect.WebSlicer_Effect.__init__(self)
        self.OptionParser.add_option("--with-code",
                                     action="store", type="string",
                                     dest="with_code",
                                     help="")
        self.OptionParser.add_option("--dir",
                                     action="store", type="string",
                                     dest="dir",
                                     help="")

    def effect(self):
        if is_empty( self.options.dir ):
            inkex.errormsg(_('You must to give a directory to export the slices.'))
            return
        if not os.path.exists( self.options.dir ):
            inkex.errormsg(_('The directory "%s" does not exists.') % self.options.dir)
            return
        (status, output) = commands.getstatusoutput("inkscape -e ...")
        inkex.errormsg( status )
        inkex.errormsg( output )


if __name__ == '__main__':
    e = WebSlicer_Export()
    e.affect()
