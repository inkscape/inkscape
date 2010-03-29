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

from webslicer_effect import *
import inkex
import gettext
import os.path
import commands

_ = gettext.gettext

class WebSlicer_Export(WebSlicer_Effect):

    def __init__(self):
        WebSlicer_Effect.__init__(self)
        self.OptionParser.add_option("--dir",
                                     action="store", type="string",
                                     dest="dir",
                                     help="")
        self.OptionParser.add_option("--create-dir",
                                     action="store", type="inkbool",
                                     default=False,
                                     dest="create_dir",
                                     help="")
        self.OptionParser.add_option("--with-code",
                                     action="store", type="inkbool",
                                     default=False,
                                     dest="with_code",
                                     help="")

    def effect(self):
        # The user must supply a directory to export:
        if is_empty( self.options.dir ):
            inkex.errormsg(_('You must to give a directory to export the slices.'))
            return {'error':'You must to give a directory to export the slices.'}
        # No directory separator at the path end:
        if self.options.dir[-1] == '/' or self.options.dir[-1] == '\\':
            self.options.dir = self.options.dir[0:-1]
        # Test if the directory exists:
        if not os.path.exists( self.options.dir ):
            if self.options.create_dir:
                # Try to create it:
                try:
                    os.makedirs( self.options.dir )
                except Exception as e:
                    inkex.errormsg( _('Can\'t create "%s".') % self.options.dir )
                    inkex.errormsg( _('Error: %s') % e )
                    return {'error':'Can\'t create the directory to export.'}
            else:
                inkex.errormsg(_('The directory "%s" does not exists.') % self.options.dir)
                return
        # Create HTML and CSS files, if the user wants:
        if self.options.with_code:
            try:
                self.html = open(os.path.join(self.options.dir,'layout.html'), 'w')
                self.css  = open(os.path.join(self.options.dir,'style.css'), 'w')
            except Exception as e:
                inkex.errormsg( _('Can\'t create code files.') )
                inkex.errormsg( _('Error: %s') % e )
                return {'error':'Can\'t create code files.'}
        # Start what we really want!
        self.export_chids_of( self.get_slicer_layer() )
        # Close the HTML and CSS files:
        if self.options.with_code:
            self.html.close()
            self.css.close()


    def export_chids_of(self, parent):
        nmspc = '{http://www.w3.org/2000/svg}'
        for el in parent.getchildren():
            if el.tag == nmspc+'g':
                if self.options.with_code:
                    self.register_group_code( el )
                else:
                    self.export_chids_of( el )
            if el.tag in [ nmspc+'rect', nmspc+'path', nmspc+'circle' ]:
                if self.options.with_code:
                    self.register_unity_code( el )
                self.export_img( el )


    def register_group_code(self, group):
        #inkex.errormsg( 'group CSS and HTML' )
        self.html.write( '<div id="G">\n' )
        self.export_chids_of( group )
        self.html.write( '</div><!-- end id="G" -->\n' )


    def register_unity_code(self, el):
        #inkex.errormsg( 'unity CSS and HTML' )
        self.html.write( el.tag + '\n' )


    def export_img(self, el):
        (status, output) = commands.getstatusoutput("inkscape -e ...")
        #inkex.errormsg( status )
        #inkex.errormsg( output )


if __name__ == '__main__':
    e = WebSlicer_Export()
    e.affect()
