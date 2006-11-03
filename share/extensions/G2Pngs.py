#!/usr/bin/env python 
'''
Copyright (C) 2006 Aaron Spike, aaron@ekips.org

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

TODO :
specify save format, scale, and if it is a layer or g export

RELEASE NOTE
- accepte l'export des calques
- dossier par défaut = dossier utilisateur
- crée le dossier spécifié s'il n'existe pas

'''
import inkex
import sys, os, tempfile

class MyEffect(inkex.Effect):
        def __init__(self):
                inkex.Effect.__init__(self)
		self.OptionParser.add_option("-d", "--directory",
						action="store", type="string", 
						dest="directory", default=os.path.expanduser("~"),
						help="Existing destination directory")
		self.OptionParser.add_option("-l", "--layers",
						action="store", type="inkbool", 
						dest="layers", default=False,
						help="Save layers with their groups")
		'''self.OptionParser.add_option("-s", "--scale",
						action="store", type="float", 
						dest="scale", default=100,
						help="Scales the group at the specified value")
		self.OptionParser.add_option("-f", "--format",
						action="store", type="string", 
						dest="format", default="png",
						help="Save at the specified format [only PNG implemented yet]")	
		'''
	def output(self):
		pass
	
	def effect(self):
		svg_file = self.args[-1]
		node = inkex.xml.xpath.Evaluate('/svg',self.document)[0]
		'''docname = node.attributes.getNamedItemNS(inkex.NSS[u'sodipodi'],'docname').value[:-4]'''

		#create os temp dir
		'''tmp_dir = tempfile.mkdtemp()'''
		directory = self.options.directory
		"""area = '--export-area-canvas'"""
		pngs = []
		if self.options.layers:
			path = "/svg/*[name()='g' or @style][@id]"
		else:
			path = "/svg/g/*[name()='g' or @style][@id]"
		
		for node in inkex.xml.xpath.Evaluate(path,self.document):
			id = node.attributes.getNamedItem('id').value
			name = "%s.png" % id
			filename = os.path.join(directory, name)
			command = "inkscape -i %s -e %s %s " % (id, filename, svg_file)
			f = os.popen(command,'r')
			f.read()
			f.close()
			pngs.append(filename)

e = MyEffect()
e.affect()