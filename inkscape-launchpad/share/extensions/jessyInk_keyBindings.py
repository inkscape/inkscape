#!/usr/bin/env python
# Copyright 2008, 2009 Hannes Hochreiner
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.

# These lines are only needed if you don't put the script directly into
# the installation directory
import sys
# Unix
sys.path.append('/usr/share/inkscape/extensions')
# OS X
sys.path.append('/Applications/Inkscape.app/Contents/Resources/extensions')
# Windows
sys.path.append('C:\Program Files\Inkscape\share\extensions')

# We will use the inkex module with the predefined Effect base class.
import inkex


class	JessyInk_CustomKeyBindings(inkex.Effect):
	modes = ('slide', 'index', 'drawing')
	keyCodes = ('LEFT', 'RIGHT', 'DOWN', 'UP', 'HOME', 'END', 'ENTER', 'SPACE', 'PAGE_UP', 'PAGE_DOWN', 'ESCAPE')
	slideActions = {}
	slideCharCodes = {}
	slideKeyCodes = {}
	drawingActions = {}
	drawingCharCodes = {}
	drawingKeyCodes = {}
	indexActions = {}
	indexCharCodes = {}
	indexKeyCodes = {}


	def __init__(self):
		# Call the base class constructor.
		inkex.Effect.__init__(self)

		self.OptionParser.add_option('--tab', action = 'store', type = 'string', dest = 'what')
		self.OptionParser.add_option('--slide_backWithEffects', action = 'callback', type = 'string', callback = self.slideOptions, default = '')
		self.OptionParser.add_option('--slide_nextWithEffects', action = 'callback', type = 'string', callback = self.slideOptions, default = '')
		self.OptionParser.add_option('--slide_backWithoutEffects', action = 'callback', type = 'string', callback = self.slideOptions, default = '')
		self.OptionParser.add_option('--slide_nextWithoutEffects', action = 'callback', type = 'string', callback = self.slideOptions, default = '')
		self.OptionParser.add_option('--slide_firstSlide', action = 'callback', type = 'string', callback = self.slideOptions, default = '')
		self.OptionParser.add_option('--slide_lastSlide', action = 'callback', type = 'string', callback = self.slideOptions, default = '')
		self.OptionParser.add_option('--slide_switchToIndexMode', action = 'callback', type = 'string', callback = self.slideOptions, default = '')
		self.OptionParser.add_option('--slide_switchToDrawingMode', action = 'callback', type = 'string', callback = self.slideOptions, default = '')
		self.OptionParser.add_option('--slide_setDuration', action = 'callback', type = 'string', callback = self.slideOptions, default = '')
		self.OptionParser.add_option('--slide_addSlide', action = 'callback', type = 'string', callback = self.slideOptions, default = '')
		self.OptionParser.add_option('--slide_toggleProgressBar', action = 'callback', type = 'string', callback = self.slideOptions, default = '')
		self.OptionParser.add_option('--slide_resetTimer', action = 'callback', type = 'string', callback = self.slideOptions, default = '')
		self.OptionParser.add_option('--slide_export', action = 'callback', type = 'string', callback = self.slideOptions, default = '')
		self.OptionParser.add_option('--drawing_switchToSlideMode', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_pathWidthDefault', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_pathWidth1', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_pathWidth2', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_pathWidth3', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_pathWidth4', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_pathWidth5', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_pathWidth6', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_pathWidth7', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_pathWidth8', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_pathWidth9', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_pathColourBlue', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_pathColourCyan', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_pathColourGreen', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_pathColourBlack', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_pathColourMagenta', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_pathColourOrange', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_pathColourRed', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_pathColourWhite', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_pathColourYellow', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--drawing_undo', action = 'callback', type = 'string', callback = self.drawingOptions, default = '')
		self.OptionParser.add_option('--index_selectSlideToLeft', action = 'callback', type = 'string', callback = self.indexOptions, default = '')
		self.OptionParser.add_option('--index_selectSlideToRight', action = 'callback', type = 'string', callback = self.indexOptions, default = '')
		self.OptionParser.add_option('--index_selectSlideAbove', action = 'callback', type = 'string', callback = self.indexOptions, default = '')
		self.OptionParser.add_option('--index_selectSlideBelow', action = 'callback', type = 'string', callback = self.indexOptions, default = '')
		self.OptionParser.add_option('--index_previousPage', action = 'callback', type = 'string', callback = self.indexOptions, default = '')
		self.OptionParser.add_option('--index_nextPage', action = 'callback', type = 'string', callback = self.indexOptions, default = '')
		self.OptionParser.add_option('--index_firstSlide', action = 'callback', type = 'string', callback = self.indexOptions, default = '')
		self.OptionParser.add_option('--index_lastSlide', action = 'callback', type = 'string', callback = self.indexOptions, default = '')
		self.OptionParser.add_option('--index_switchToSlideMode', action = 'callback', type = 'string', callback = self.indexOptions, default = '')
		self.OptionParser.add_option('--index_decreaseNumberOfColumns', action = 'callback', type = 'string', callback = self.indexOptions, default = '')
		self.OptionParser.add_option('--index_increaseNumberOfColumns', action = 'callback', type = 'string', callback = self.indexOptions, default = '')
		self.OptionParser.add_option('--index_setNumberOfColumnsToDefault', action = 'callback', type = 'string', callback = self.indexOptions, default = '')

		inkex.NSS[u"jessyink"] = u"https://launchpad.net/jessyink"
		
		self.slideActions["backWithEffects"] = "dispatchEffects(-1);"
		self.slideActions["nextWithEffects"] = "dispatchEffects(1);"
		self.slideActions["backWithoutEffects"] = "skipEffects(-1);"
		self.slideActions["nextWithoutEffects"] = "skipEffects(1);"
		self.slideActions["firstSlide"] = "slideSetActiveSlide(0);"
		self.slideActions["lastSlide"] = "slideSetActiveSlide(slides.length - 1);"
		self.slideActions["switchToIndexMode"] = "toggleSlideIndex();"
		self.slideActions["switchToDrawingMode"] = "slideSwitchToDrawingMode();"
		self.slideActions["setDuration"] = "slideQueryDuration();"
		self.slideActions["addSlide"] = "slideAddSlide(activeSlide);"
		self.slideActions["toggleProgressBar"] = "slideToggleProgressBarVisibility();"
		self.slideActions["resetTimer"] = "slideResetTimer();"
		self.slideActions["export"] = "slideUpdateExportLayer();"

		self.drawingActions["switchToSlideMode"] = "drawingSwitchToSlideMode();"
		self.drawingActions["pathWidthDefault"] = "drawingResetPathWidth();"
		self.drawingActions["pathWidth1"] = "drawingSetPathWidth(1.0);"
		self.drawingActions["pathWidth3"] = "drawingSetPathWidth(3.0);"
		self.drawingActions["pathWidth5"] = "drawingSetPathWidth(5.0);"
		self.drawingActions["pathWidth7"] = "drawingSetPathWidth(7.0);"
		self.drawingActions["pathWidth9"] = "drawingSetPathWidth(9.0);"
		self.drawingActions["pathColourBlue"] = "drawingSetPathColour(\"blue\");"
		self.drawingActions["pathColourCyan"] = "drawingSetPathColour(\"cyan\");"
		self.drawingActions["pathColourGreen"] = "drawingSetPathColour(\"green\");"
		self.drawingActions["pathColourBlack"] = "drawingSetPathColour(\"black\");"
		self.drawingActions["pathColourMagenta"] = "drawingSetPathColour(\"magenta\");"
		self.drawingActions["pathColourOrange"] = "drawingSetPathColour(\"orange\");"
		self.drawingActions["pathColourRed"] = "drawingSetPathColour(\"red\");"
		self.drawingActions["pathColourWhite"] = "drawingSetPathColour(\"white\");"
		self.drawingActions["pathColourYellow"] = "drawingSetPathColour(\"yellow\");"
		self.drawingActions["undo"] = "drawingUndo();"

		self.indexActions["selectSlideToLeft"] = "indexSetPageSlide(activeSlide - 1);"
		self.indexActions["selectSlideToRight"] = "indexSetPageSlide(activeSlide + 1);"
		self.indexActions["selectSlideAbove"] = "indexSetPageSlide(activeSlide - INDEX_COLUMNS);"
		self.indexActions["selectSlideBelow"] = "indexSetPageSlide(activeSlide + INDEX_COLUMNS);"
		self.indexActions["previousPage"] = "indexSetPageSlide(activeSlide - INDEX_COLUMNS * INDEX_COLUMNS);"
		self.indexActions["nextPage"] = "indexSetPageSlide(activeSlide + INDEX_COLUMNS * INDEX_COLUMNS);"
		self.indexActions["firstSlide"] = "indexSetPageSlide(0);"
		self.indexActions["lastSlide"] = "indexSetPageSlide(slides.length - 1);"
		self.indexActions["switchToSlideMode"] = "toggleSlideIndex();"
		self.indexActions["decreaseNumberOfColumns"] = "indexDecreaseNumberOfColumns();"
		self.indexActions["increaseNumberOfColumns"] = "indexIncreaseNumberOfColumns();"
		self.indexActions["setNumberOfColumnsToDefault"] = "indexResetNumberOfColumns();"

	def slideOptions(self, option, opt_str, value, parser):
		action = self.getAction(opt_str)

		valueArray = value.split(",")

		for val in valueArray:
			val = val.strip()

			if val in self.keyCodes:
				self.slideKeyCodes[val + "_KEY"] = self.slideActions[action]
			elif len(val) == 1:
				self.slideCharCodes[val] = self.slideActions[action]

	def drawingOptions(self, option, opt_str, value, parser):
		action = self.getAction(opt_str)

		valueArray = value.split(",")

		for val in valueArray:
			val = val.strip()

			if val in self.keyCodes:
				self.drawingKeyCodes[val + "_KEY"] = self.drawingActions[action]
			elif len(val) == 1:
				self.drawingCharCodes[val] = self.drawingActions[action]

	def indexOptions(self, option, opt_str, value, parser):
		action = self.getAction(opt_str)

		valueArray = value.split(",")

		for val in valueArray:
			val = val.strip()

			if val in self.keyCodes:
				self.indexKeyCodes[val + "_KEY"] = self.indexActions[action]
			elif len(val) == 1:
				self.indexCharCodes[val] = self.indexActions[action]

	def effect(self):
		# Check version.
		scriptNodes = self.document.xpath("//svg:script[@jessyink:version='1.5.5']", namespaces=inkex.NSS)

		if len(scriptNodes) != 1:
			inkex.errormsg(_("The JessyInk script is not installed in this SVG file or has a different version than the JessyInk extensions. Please select \"install/update...\" from the \"JessyInk\" sub-menu of the \"Extensions\" menu to install or update the JessyInk script.\n\n"))

		# Remove old master slide property
		for node in self.document.xpath("//svg:g[@jessyink:customKeyBindings='customKeyBindings']", namespaces=inkex.NSS):
			node.getparent().remove(node)

		# Set custom key bindings.
		nodeText = "function getCustomKeyBindingsSub()" + "\n"
		nodeText += "{" + "\n"
		nodeText += "	var keyDict = new Object();" + "\n"
		nodeText += "	keyDict[SLIDE_MODE] = new Object();" + "\n"
		nodeText += "	keyDict[INDEX_MODE] = new Object();" + "\n"
		nodeText += "	keyDict[DRAWING_MODE] = new Object();" + "\n"

		for key, value in self.slideKeyCodes.items():
			nodeText += "	keyDict[SLIDE_MODE][" + key + "] = function() { " + value + " };" + "\n"
		
		for key, value in self.drawingKeyCodes.items():
			nodeText += "	keyDict[DRAWING_MODE][" + key + "] = function() { " + value + " };" + "\n"
		
		for key, value in self.indexKeyCodes.items():
			nodeText += "	keyDict[INDEX_MODE][" + key + "] = function() { " + value + " };" + "\n"
		
		nodeText += "	return keyDict;" + "\n"
		nodeText += "}" + "\n\n"

		# Set custom char bindings.
		nodeText += "function getCustomCharBindingsSub()" + "\n"
		nodeText += "{" + "\n"
		nodeText += "	var charDict = new Object();" + "\n"
		nodeText += "	charDict[SLIDE_MODE] = new Object();" + "\n"
		nodeText += "	charDict[INDEX_MODE] = new Object();" + "\n"
		nodeText += "	charDict[DRAWING_MODE] = new Object();" + "\n"

		for key, value in self.slideCharCodes.items():
			nodeText += "	charDict[SLIDE_MODE][\"" + key + "\"] = function() { " + value + " };" + "\n"
		
		for key, value in self.drawingCharCodes.items():
			nodeText += "	charDict[DRAWING_MODE][\"" + key + "\"] = function() { " + value + " };" + "\n"
		
		for key, value in self.indexCharCodes.items():
			nodeText += "	charDict[INDEX_MODE][\"" + key + "\"] = function() { " + value + " };" + "\n"
		
		nodeText += "	return charDict;" + "\n"
		nodeText += "}" + "\n"

		# Create new script node
		scriptElm = inkex.etree.Element(inkex.addNS("script", "svg"))
		scriptElm.text = nodeText
		groupElm = inkex.etree.Element(inkex.addNS("g", "svg"))
		groupElm.set("{" + inkex.NSS["jessyink"] + "}customKeyBindings", "customKeyBindings")
		groupElm.set("onload", "this.getCustomCharBindings = function() { return getCustomCharBindingsSub(); }; this.getCustomKeyBindings = function() { return getCustomKeyBindingsSub(); };")
		groupElm.append(scriptElm)
		self.document.getroot().append(groupElm)

	def getAction(self, varName):
		parts = varName.split('_')
		
		if (len(parts) != 2):
			raise StandardException("Error parsing variable name.")
		
		return parts[1]

# Create effect instance
effect = JessyInk_CustomKeyBindings()
effect.affect()

