#!/usr/bin/python


# list files in directory
import os
import uuid

directory_ids = {}
file_ids = {}

def indent(level):
	indentstring = ''
	for i in range(level):
		indentstring += '    '
	return indentstring

def directory(root, breadcrumb, level, exclude=[]):
	"""
	list all files and directory recursivly
	create the file_ids dictionary to be used in ComponentGroup references
	"""
	global file_ids
	global directory_ids
	# first list files within directory
	files = [ f for f in os.listdir(root) if os.path.isfile(os.path.join(root,f)) and f not in exclude]
	for file in files:
		file_key = os.path.join(root, file)
		_id = '_%06d' % (len(file_ids.keys()) + 1)
		file_ids[file_key] = 'component' + _id
		wxs.write(indent(level)+ "<Component Id='component" + _id + "' Guid='" + str(uuid.uuid4()) + "' DiskId='1' Win64='$(var.Win64)'>\n")
		if file == 'inkscape.exe':
			# we refenrence inkscape.exe in inkscape.wxs
			_id = '_inkscape_exe'
		wxs.write(indent(level + 1)+ "<File Id='file" + _id + "' Name='" + file + "' DiskId='1' Source='" + file_key + "' KeyPath='yes' />\n")
		wxs.write(indent(level)+ "</Component>\n")
	# then all directories
	
	dirs = [ f for f in os.listdir(root) if os.path.isdir(os.path.join(root,f)) ]
	for dir in dirs:
		directory_key = breadcrumb + '__' + dir
		if not directory_key in directory_ids.keys():
			directory_ids[directory_key] = 'dir_%06d' % (len(directory_ids.keys()) + 1)
		wxs.write(indent(level) + "<Directory Id='" + directory_ids[directory_key] + "' Name='" + dir + "'>\n")
		directory(os.path.join(root, dir), directory_key, level + 1)
		wxs.write(indent(level) + "</Directory>\n")
	
	
def ComponentGroup(name, condition, level):
	"""
	add componentgroup that contain all items from file_ids that match condition
	remove the matched elements from file_ids
	"""
	global file_ids
	keys = [k for k in file_ids.keys() if condition in k]
	wxs.write(indent(level) + "<ComponentGroup Id='" + name + "'>\n")
	for component in keys:
		wxs.write(indent(level + 1) + "<ComponentRef Id='" + file_ids[component] + "' />\n")
	wxs.write(indent(level) + "</ComponentGroup>\n")
	for key in keys:
		del file_ids[key]

with open('files.wxs', 'w') as wxs:
	wxs.write("<!-- do not edit, this file is created by files.py tool any changes will be lost -->\n")
	wxs.write("<Wix xmlns='http://schemas.microsoft.com/wix/2006/wi'>\n")
	wxs.write(indent(1) + "<?include version.wxi?>\n")
	wxs.write(indent(1) + "<Fragment>\n")
	wxs.write(indent(2) + "<!-- Step 1: Define the directory structure -->\n")
	wxs.write(indent(2) + "<Directory Id='TARGETDIR' Name='SourceDir'>\n")
	wxs.write(indent(3) + "<Directory Id='$(var.ProgramFilesFolder)' Name='PFiles'>\n")
	wxs.write(indent(4) + "<Directory Id='INSTALLDIR' Name='Inkscape'>\n")
	print "start parsing ..\..\inkscape"
	directory('..\..\inkscape', 'inkscape', 5, ['inkscape.dbg', 'inkview.dbg', 'gdb.exe'])
	print "found %d files" % len(file_ids.keys())
	wxs.write(indent(4) + "</Directory>\n")
	wxs.write(indent(3) + "</Directory>\n")
	# link to ProgrmMenu
	wxs.write(indent(3) + "<Directory Id='ProgramMenuFolder'>\n")
	wxs.write(indent(4) + "<Directory Id='ApplicationProgramsFolder' Name='$(var.FullProductName)'/>\n")
	wxs.write(indent(3) + "</Directory>\n")
	wxs.write(indent(3) + "<Directory Id='DesktopFolder' Name='Desktop' />\n")
	wxs.write(indent(2) + "</Directory>\n")
	ComponentGroup("Examples", 'inkscape\\share\\examples', 2)
	ComponentGroup("Tutorials", 'inkscape\\share\\tutorials\\', 2)
	ComponentGroup("Translations", '\\locale\\', 2)
	ComponentGroup("AllOther", '', 2)
	wxs.write(indent(1) + "</Fragment>\n")
	wxs.write("</Wix>\n")

	

