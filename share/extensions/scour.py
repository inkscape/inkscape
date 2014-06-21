#!/usr/bin/env python
# -*- coding: utf-8 -*-

#  Scour
#
#  Copyright 2010 Jeff Schiller
#  Copyright 2010 Louis Simard
#
#  This file is part of Scour, http://www.codedread.com/scour/
#
#   This library is free software; you can redistribute it and/or modify
#   it either under the terms of the Apache License, Version 2.0, or, at
#   your option, under the terms and conditions of the GNU General
#   Public License, Version 2 or newer as published by the Free Software
#   Foundation.  You may obtain a copy of these Licenses at:
#
#       http://www.apache.org/licenses/LICENSE-2.0
#       https://www.gnu.org/licenses/old-licenses/gpl-2.0.html
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

# Notes:

# rubys' path-crunching ideas here: http://intertwingly.net/code/svgtidy/spec.rb
# (and implemented here: http://intertwingly.net/code/svgtidy/svgtidy.rb )

# Yet more ideas here: http://wiki.inkscape.org/wiki/index.php/Save_Cleaned_SVG
#
# * Process Transformations
#  * Collapse all group based transformations

# Even more ideas here: http://esw.w3.org/topic/SvgTidy
#  * analysis of path elements to see if rect can be used instead? (must also need to look
#    at rounded corners)

# Next Up:
# - why are marker-start, -end not removed from the style attribute?
# - why are only overflow style properties considered and not attributes?
# - only remove unreferenced elements if they are not children of a referenced element
# - add an option to remove ids if they match the Inkscape-style of IDs
# - investigate point-reducing algorithms
# - parse transform attribute
# - if a <g> has only one element in it, collapse the <g> (ensure transform, etc are carried down)

# necessary to get true division
from __future__ import division

import os
import sys
import xml.dom.minidom
import re
import math
from svg_regex import svg_parser
from svg_transform import svg_transform_parser
import optparse
from yocto_css import parseCssString

# Python 2.3- did not have Decimal
try:
	from decimal import *
except ImportError:
	print >>sys.stderr, "Scour requires Python 2.4."

# Import Psyco if available
try:
	import psyco
	psyco.full()
except ImportError:
	pass

APP = 'scour'
VER = '0.26+r220'
COPYRIGHT = 'Copyright Jeff Schiller, Louis Simard, 2012'

NS = { 	'SVG': 		'http://www.w3.org/2000/svg', 
		'XLINK': 	'http://www.w3.org/1999/xlink', 
		'SODIPODI': 'http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd',
		'INKSCAPE': 'http://www.inkscape.org/namespaces/inkscape',
		'ADOBE_ILLUSTRATOR': 'http://ns.adobe.com/AdobeIllustrator/10.0/',
		'ADOBE_GRAPHS': 'http://ns.adobe.com/Graphs/1.0/',
		'ADOBE_SVG_VIEWER': 'http://ns.adobe.com/AdobeSVGViewerExtensions/3.0/',
		'ADOBE_VARIABLES': 'http://ns.adobe.com/Variables/1.0/',
		'ADOBE_SFW': 'http://ns.adobe.com/SaveForWeb/1.0/',
		'ADOBE_EXTENSIBILITY': 'http://ns.adobe.com/Extensibility/1.0/',
		'ADOBE_FLOWS': 'http://ns.adobe.com/Flows/1.0/',
		'ADOBE_IMAGE_REPLACEMENT': 'http://ns.adobe.com/ImageReplacement/1.0/',     
		'ADOBE_CUSTOM': 'http://ns.adobe.com/GenericCustomNamespace/1.0/',
		'ADOBE_XPATH': 'http://ns.adobe.com/XPath/1.0/'
		}

unwanted_ns = [ NS['SODIPODI'], NS['INKSCAPE'], NS['ADOBE_ILLUSTRATOR'],
				NS['ADOBE_GRAPHS'], NS['ADOBE_SVG_VIEWER'], NS['ADOBE_VARIABLES'],
				NS['ADOBE_SFW'], NS['ADOBE_EXTENSIBILITY'], NS['ADOBE_FLOWS'],
				NS['ADOBE_IMAGE_REPLACEMENT'], NS['ADOBE_CUSTOM'], NS['ADOBE_XPATH'] ] 

svgAttributes = [
				'clip-rule',
				'display',
				'fill',
				'fill-opacity',
				'fill-rule',
				'filter',
				'font-family',
				'font-size',
				'font-stretch',
				'font-style',
				'font-variant',
				'font-weight',
				'line-height',
				'marker',
				'marker-end',
				'marker-mid',
				'marker-start',
				'opacity',
				'overflow',
				'stop-color',
				'stop-opacity',
				'stroke',
				'stroke-dasharray',
				'stroke-dashoffset',
				'stroke-linecap',
				'stroke-linejoin',
				'stroke-miterlimit',
				'stroke-opacity',
				'stroke-width',
				'visibility'
				]

colors = {
	'aliceblue': 'rgb(240, 248, 255)',
	'antiquewhite': 'rgb(250, 235, 215)',
	'aqua': 'rgb( 0, 255, 255)',
	'aquamarine': 'rgb(127, 255, 212)',
	'azure': 'rgb(240, 255, 255)',
	'beige': 'rgb(245, 245, 220)',
	'bisque': 'rgb(255, 228, 196)',
	'black': 'rgb( 0, 0, 0)',
	'blanchedalmond': 'rgb(255, 235, 205)',
	'blue': 'rgb( 0, 0, 255)',
	'blueviolet': 'rgb(138, 43, 226)',
	'brown': 'rgb(165, 42, 42)',
	'burlywood': 'rgb(222, 184, 135)',
	'cadetblue': 'rgb( 95, 158, 160)',
	'chartreuse': 'rgb(127, 255, 0)',
	'chocolate': 'rgb(210, 105, 30)',
	'coral': 'rgb(255, 127, 80)',
	'cornflowerblue': 'rgb(100, 149, 237)',
	'cornsilk': 'rgb(255, 248, 220)',
	'crimson': 'rgb(220, 20, 60)',
	'cyan': 'rgb( 0, 255, 255)',
	'darkblue': 'rgb( 0, 0, 139)',
	'darkcyan': 'rgb( 0, 139, 139)',
	'darkgoldenrod': 'rgb(184, 134, 11)',
	'darkgray': 'rgb(169, 169, 169)',
	'darkgreen': 'rgb( 0, 100, 0)',
	'darkgrey': 'rgb(169, 169, 169)',
	'darkkhaki': 'rgb(189, 183, 107)',
	'darkmagenta': 'rgb(139, 0, 139)',
	'darkolivegreen': 'rgb( 85, 107, 47)',
	'darkorange': 'rgb(255, 140, 0)',
	'darkorchid': 'rgb(153, 50, 204)',
	'darkred': 'rgb(139, 0, 0)',
	'darksalmon': 'rgb(233, 150, 122)',
	'darkseagreen': 'rgb(143, 188, 143)',
	'darkslateblue': 'rgb( 72, 61, 139)',
	'darkslategray': 'rgb( 47, 79, 79)',
	'darkslategrey': 'rgb( 47, 79, 79)',
	'darkturquoise': 'rgb( 0, 206, 209)',
	'darkviolet': 'rgb(148, 0, 211)',
	'deeppink': 'rgb(255, 20, 147)',
	'deepskyblue': 'rgb( 0, 191, 255)',
	'dimgray': 'rgb(105, 105, 105)',
	'dimgrey': 'rgb(105, 105, 105)',
	'dodgerblue': 'rgb( 30, 144, 255)',
	'firebrick': 'rgb(178, 34, 34)',
	'floralwhite': 'rgb(255, 250, 240)',
	'forestgreen': 'rgb( 34, 139, 34)',
	'fuchsia': 'rgb(255, 0, 255)',
	'gainsboro': 'rgb(220, 220, 220)',
	'ghostwhite': 'rgb(248, 248, 255)',
	'gold': 'rgb(255, 215, 0)',
	'goldenrod': 'rgb(218, 165, 32)',
	'gray': 'rgb(128, 128, 128)',
	'grey': 'rgb(128, 128, 128)',
	'green': 'rgb( 0, 128, 0)',
	'greenyellow': 'rgb(173, 255, 47)',
	'honeydew': 'rgb(240, 255, 240)',
	'hotpink': 'rgb(255, 105, 180)',
	'indianred': 'rgb(205, 92, 92)',
	'indigo': 'rgb( 75, 0, 130)',
	'ivory': 'rgb(255, 255, 240)',
	'khaki': 'rgb(240, 230, 140)',
	'lavender': 'rgb(230, 230, 250)',
	'lavenderblush': 'rgb(255, 240, 245)',
	'lawngreen': 'rgb(124, 252, 0)',
	'lemonchiffon': 'rgb(255, 250, 205)',
	'lightblue': 'rgb(173, 216, 230)',
	'lightcoral': 'rgb(240, 128, 128)',
	'lightcyan': 'rgb(224, 255, 255)',
	'lightgoldenrodyellow': 'rgb(250, 250, 210)',
	'lightgray': 'rgb(211, 211, 211)',
	'lightgreen': 'rgb(144, 238, 144)',
	'lightgrey': 'rgb(211, 211, 211)',
	'lightpink': 'rgb(255, 182, 193)',
	'lightsalmon': 'rgb(255, 160, 122)',
	'lightseagreen': 'rgb( 32, 178, 170)',
	'lightskyblue': 'rgb(135, 206, 250)',
	'lightslategray': 'rgb(119, 136, 153)',
	'lightslategrey': 'rgb(119, 136, 153)',
	'lightsteelblue': 'rgb(176, 196, 222)',
	'lightyellow': 'rgb(255, 255, 224)',
	'lime': 'rgb( 0, 255, 0)',
	'limegreen': 'rgb( 50, 205, 50)',
	'linen': 'rgb(250, 240, 230)',
	'magenta': 'rgb(255, 0, 255)',
	'maroon': 'rgb(128, 0, 0)',
	'mediumaquamarine': 'rgb(102, 205, 170)',
	'mediumblue': 'rgb( 0, 0, 205)',
	'mediumorchid': 'rgb(186, 85, 211)',
	'mediumpurple': 'rgb(147, 112, 219)',
	'mediumseagreen': 'rgb( 60, 179, 113)',
	'mediumslateblue': 'rgb(123, 104, 238)',
	'mediumspringgreen': 'rgb( 0, 250, 154)',
	'mediumturquoise': 'rgb( 72, 209, 204)',
	'mediumvioletred': 'rgb(199, 21, 133)',
	'midnightblue': 'rgb( 25, 25, 112)',
	'mintcream': 'rgb(245, 255, 250)',
	'mistyrose': 'rgb(255, 228, 225)',
	'moccasin': 'rgb(255, 228, 181)',
	'navajowhite': 'rgb(255, 222, 173)',
	'navy': 'rgb( 0, 0, 128)',
	'oldlace': 'rgb(253, 245, 230)',
	'olive': 'rgb(128, 128, 0)',
	'olivedrab': 'rgb(107, 142, 35)',
	'orange': 'rgb(255, 165, 0)',
	'orangered': 'rgb(255, 69, 0)',
	'orchid': 'rgb(218, 112, 214)',
	'palegoldenrod': 'rgb(238, 232, 170)',
	'palegreen': 'rgb(152, 251, 152)',
	'paleturquoise': 'rgb(175, 238, 238)',
	'palevioletred': 'rgb(219, 112, 147)',
	'papayawhip': 'rgb(255, 239, 213)',
	'peachpuff': 'rgb(255, 218, 185)',
	'peru': 'rgb(205, 133, 63)',
	'pink': 'rgb(255, 192, 203)',
	'plum': 'rgb(221, 160, 221)',
	'powderblue': 'rgb(176, 224, 230)',
	'purple': 'rgb(128, 0, 128)',
	'rebeccapurple': 'rgb(102, 51, 153)',
	'red': 'rgb(255, 0, 0)',
	'rosybrown': 'rgb(188, 143, 143)',
	'royalblue': 'rgb( 65, 105, 225)',
	'saddlebrown': 'rgb(139, 69, 19)',
	'salmon': 'rgb(250, 128, 114)',
	'sandybrown': 'rgb(244, 164, 96)',
	'seagreen': 'rgb( 46, 139, 87)',
	'seashell': 'rgb(255, 245, 238)',
	'sienna': 'rgb(160, 82, 45)',
	'silver': 'rgb(192, 192, 192)',
	'skyblue': 'rgb(135, 206, 235)',
	'slateblue': 'rgb(106, 90, 205)',
	'slategray': 'rgb(112, 128, 144)',
	'slategrey': 'rgb(112, 128, 144)',
	'snow': 'rgb(255, 250, 250)',
	'springgreen': 'rgb( 0, 255, 127)',
	'steelblue': 'rgb( 70, 130, 180)',
	'tan': 'rgb(210, 180, 140)',
	'teal': 'rgb( 0, 128, 128)',
	'thistle': 'rgb(216, 191, 216)',
	'tomato': 'rgb(255, 99, 71)',
	'turquoise': 'rgb( 64, 224, 208)',
	'violet': 'rgb(238, 130, 238)',
	'wheat': 'rgb(245, 222, 179)',
	'white': 'rgb(255, 255, 255)',
	'whitesmoke': 'rgb(245, 245, 245)',
	'yellow': 'rgb(255, 255, 0)',
	'yellowgreen': 'rgb(154, 205, 50)',
	}

default_attributes = { # excluded all attributes with 'auto' as default
	# SVG 1.1 presentation attributes
	'baseline-shift': 'baseline',
	'clip-path': 'none',
	'clip-rule': 'nonzero',
	'color': '#000',
	'color-interpolation-filters': 'linearRGB',
	'color-interpolation': 'sRGB',
	'direction': 'ltr',
	'display': 'inline',
	'enable-background': 'accumulate',
	'fill': '#000',
	'fill-opacity': '1',
	'fill-rule': 'nonzero',
	'filter': 'none',
	'flood-color': '#000',
	'flood-opacity': '1',
	'font-size-adjust': 'none',
	'font-size': 'medium',
	'font-stretch': 'normal',
	'font-style': 'normal',
	'font-variant': 'normal',
	'font-weight': 'normal',
	'glyph-orientation-horizontal': '0deg',
	'letter-spacing': 'normal',
	'lighting-color': '#fff',
	'marker': 'none',
	'marker-start': 'none',
	'marker-mid': 'none',
	'marker-end': 'none',
	'mask': 'none',
	'opacity': '1',
	'pointer-events': 'visiblePainted',
	'stop-color': '#000',
	'stop-opacity': '1',
	'stroke': 'none',
	'stroke-dasharray': 'none',
	'stroke-dashoffset': '0',
	'stroke-linecap': 'butt',
	'stroke-linejoin': 'miter',
	'stroke-miterlimit': '4',
	'stroke-opacity': '1',
	'stroke-width': '1',
	'text-anchor': 'start',
	'text-decoration': 'none',
	'unicode-bidi': 'normal',
	'visibility': 'visible',
	'word-spacing': 'normal',
	'writing-mode': 'lr-tb',
	# SVG 1.2 tiny properties
	'audio-level': '1',
	'solid-color': '#000',
	'solid-opacity': '1',
	'text-align': 'start',
	'vector-effect': 'none',
	'viewport-fill': 'none',
	'viewport-fill-opacity': '1',
	}

def isSameSign(a,b): return (a <= 0 and b <= 0) or (a >= 0 and b >= 0)

scinumber = re.compile(r"[-+]?(\d*\.?)?\d+[eE][-+]?\d+")
number = re.compile(r"[-+]?(\d*\.?)?\d+")
sciExponent = re.compile(r"[eE]([-+]?\d+)")
unit = re.compile("(em|ex|px|pt|pc|cm|mm|in|%){1,1}$")

class Unit(object):
	# Integer constants for units.
	INVALID = -1
	NONE = 0
	PCT = 1
	PX = 2
	PT = 3
	PC = 4
	EM = 5
	EX = 6
	CM = 7
	MM = 8
	IN = 9
	
	# String to Unit. Basically, converts unit strings to their integer constants.
	s2u = {
		'': NONE,
		'%': PCT,
		'px': PX,
		'pt': PT,
		'pc': PC,
		'em': EM,
		'ex': EX,
		'cm': CM,
		'mm': MM,
		'in': IN,
	}
	
	# Unit to String. Basically, converts unit integer constants to their corresponding strings.
	u2s = {
		NONE: '',
		PCT: '%',
		PX: 'px',
		PT: 'pt',
		PC: 'pc',
		EM: 'em',
		EX: 'ex',
		CM: 'cm',
		MM: 'mm',
		IN: 'in',
	}
	
#	@staticmethod
	def get(unitstr):
		if unitstr is None: return Unit.NONE
		try:
			return Unit.s2u[unitstr]
		except KeyError:
			return Unit.INVALID

#	@staticmethod
	def str(unitint):
		try:
			return Unit.u2s[unitint]
		except KeyError:
			return 'INVALID'
		
	get = staticmethod(get)
	str = staticmethod(str)
	
class SVGLength(object):
	def __init__(self, str):
		try: # simple unitless and no scientific notation
			self.value = float(str)
			if int(self.value) == self.value: 
				self.value = int(self.value)
			self.units = Unit.NONE
		except ValueError:
			# we know that the length string has an exponent, a unit, both or is invalid

			# parse out number, exponent and unit
			self.value = 0
			unitBegin = 0
			scinum = scinumber.match(str)
			if scinum != None:
				# this will always match, no need to check it
				numMatch = number.match(str)
				expMatch = sciExponent.search(str, numMatch.start(0))
				self.value = (float(numMatch.group(0)) *
					10 ** float(expMatch.group(1)))
				unitBegin = expMatch.end(1)
			else:
				# unit or invalid
				numMatch = number.match(str)
				if numMatch != None:
					self.value = float(numMatch.group(0))
					unitBegin = numMatch.end(0)
					
			if int(self.value) == self.value:
				self.value = int(self.value)

			if unitBegin != 0 :
				unitMatch = unit.search(str, unitBegin)
				if unitMatch != None :
					self.units = Unit.get(unitMatch.group(0))
				
			# invalid
			else:
				# TODO: this needs to set the default for the given attribute (how?)
				self.value = 0 
				self.units = Unit.INVALID

def findElementsWithId(node, elems=None):
	"""
	Returns all elements with id attributes
	"""
	if elems is None:
		elems = {}
	id = node.getAttribute('id')
	if id != '' :
		elems[id] = node
	if node.hasChildNodes() :
		for child in node.childNodes:
			# from http://www.w3.org/TR/DOM-Level-2-Core/idl-definitions.html
			# we are only really interested in nodes of type Element (1)
			if child.nodeType == 1 :
				findElementsWithId(child, elems)
	return elems

referencingProps = ['fill', 'stroke', 'filter', 'clip-path', 'mask',  'marker-start', 
					'marker-end', 'marker-mid']

def findReferencedElements(node, ids=None):
	"""
	Returns the number of times an ID is referenced as well as all elements
	that reference it.  node is the node at which to start the search.  The
	return value is a map which has the id as key and each value is an array
	where the first value is a count and the second value is a list of nodes
	that referenced it.

	Currently looks at fill, stroke, clip-path, mask, marker, and
	xlink:href attributes.
	"""
	global referencingProps
	if ids is None:
		ids = {}
	# TODO: input argument ids is clunky here (see below how it is called)
	# GZ: alternative to passing dict, use **kwargs

	# if this node is a style element, parse its text into CSS
	if node.nodeName == 'style' and node.namespaceURI == NS['SVG']:
		# one stretch of text, please! (we could use node.normalize(), but
		# this actually modifies the node, and we don't want to keep
		# whitespace around if there's any)
		stylesheet = "".join([child.nodeValue for child in node.childNodes])
		if stylesheet != '':
			cssRules = parseCssString(stylesheet)
			for rule in cssRules:
				for propname in rule['properties']:
					propval = rule['properties'][propname]
					findReferencingProperty(node, propname, propval, ids)
		return ids
	
	# else if xlink:href is set, then grab the id
	href = node.getAttributeNS(NS['XLINK'],'href')
	if href != '' and len(href) > 1 and href[0] == '#':
		# we remove the hash mark from the beginning of the id
		id = href[1:]
		if id in ids:
			ids[id][0] += 1
			ids[id][1].append(node)
		else:
			ids[id] = [1,[node]]

	# now get all style properties and the fill, stroke, filter attributes
	styles = node.getAttribute('style').split(';')
	for attr in referencingProps:
		styles.append(':'.join([attr, node.getAttribute(attr)]))
			
	for style in styles:
		propval = style.split(':')
		if len(propval) == 2 :
			prop = propval[0].strip()
			val = propval[1].strip()
			findReferencingProperty(node, prop, val, ids)

	if node.hasChildNodes() :
		for child in node.childNodes:
			if child.nodeType == 1 :
				findReferencedElements(child, ids)
	return ids

def findReferencingProperty(node, prop, val, ids):
	global referencingProps
	if prop in referencingProps and val != '' :
		if len(val) >= 7 and val[0:5] == 'url(#' :
			id = val[5:val.find(')')]
			if ids.has_key(id) :
				ids[id][0] += 1
				ids[id][1].append(node)
			else:
				ids[id] = [1,[node]]
		# if the url has a quote in it, we need to compensate
		elif len(val) >= 8 :
			id = None
			# double-quote
			if val[0:6] == 'url("#' :
				id = val[6:val.find('")')]
			# single-quote
			elif val[0:6] == "url('#" :
				id = val[6:val.find("')")]
			if id != None:
				if ids.has_key(id) :
					ids[id][0] += 1
					ids[id][1].append(node)
				else:
					ids[id] = [1,[node]]

numIDsRemoved = 0
numElemsRemoved = 0
numAttrsRemoved = 0
numRastersEmbedded = 0
numPathSegmentsReduced = 0
numCurvesStraightened = 0
numBytesSavedInPathData = 0
numBytesSavedInColors = 0
numBytesSavedInIDs = 0
numBytesSavedInLengths = 0
numBytesSavedInTransforms = 0
numPointsRemovedFromPolygon = 0
numCommentBytes = 0

def flattenDefs(doc):
	"""
	Puts all defined elements into a newly created defs in the document.  This function
	handles recursive defs elements.
	"""
	defs = doc.documentElement.getElementsByTagName('defs')

	if defs.length > 1:
		topDef = doc.createElementNS(NS['SVG'], 'defs')

		for defElem in defs:
			# Remove all children of this defs and put it into the topDef.
			while defElem.hasChildNodes():
				topDef.appendChild(defElem.firstChild)
			defElem.parentNode.removeChild(defElem)

		if topDef.hasChildNodes():
			doc.documentElement.insertBefore(topDef, doc.documentElement.firstChild)

def removeUnusedDefs(doc, defElem, elemsToRemove=None):
	if elemsToRemove is None:
		elemsToRemove = []

	identifiedElements = findElementsWithId(doc.documentElement)
	referencedIDs = findReferencedElements(doc.documentElement)

	keepTags = ['font', 'style', 'metadata', 'script', 'title', 'desc']
	for elem in defElem.childNodes:
		# only look at it if an element and not referenced anywhere else
		if elem.nodeType == 1 and (elem.getAttribute('id') == '' or \
				(not elem.getAttribute('id') in referencedIDs)):

			# we only inspect the children of a group in a defs if the group
			# is not referenced anywhere else
			if elem.nodeName == 'g' and elem.namespaceURI == NS['SVG']:
				elemsToRemove = removeUnusedDefs(doc, elem, elemsToRemove)
			# we only remove if it is not one of our tags we always keep (see above)
			elif not elem.nodeName in keepTags:
				elemsToRemove.append(elem)
	return elemsToRemove

def removeUnreferencedElements(doc):
	"""
	Removes all unreferenced elements except for <svg>, <font>, <metadata>, <title>, and <desc>.	
	Also vacuums the defs of any non-referenced renderable elements.
	
	Returns the number of unreferenced elements removed from the document.
	"""
	global numElemsRemoved
	num = 0
	
	# Remove certain unreferenced elements outside of defs
	removeTags = ['linearGradient', 'radialGradient', 'pattern']
	identifiedElements = findElementsWithId(doc.documentElement)
	referencedIDs = findReferencedElements(doc.documentElement)

	for id in identifiedElements:
		if not id in referencedIDs:
			goner = identifiedElements[id]
			if goner != None and goner.parentNode != None and goner.nodeName in removeTags:
				goner.parentNode.removeChild(goner)
				num += 1
				numElemsRemoved += 1

	# Remove most unreferenced elements inside defs
	defs = doc.documentElement.getElementsByTagName('defs')
	for aDef in defs:
		elemsToRemove = removeUnusedDefs(doc, aDef)
		for elem in elemsToRemove:
			elem.parentNode.removeChild(elem)
			numElemsRemoved += 1
			num += 1
	return num

def shortenIDs(doc, unprotectedElements=None):
	"""
	Shortens ID names used in the document. ID names referenced the most often are assigned the
	shortest ID names.
	If the list unprotectedElements is provided, only IDs from this list will be shortened.
	
	Returns the number of bytes saved by shortening ID names in the document.
	"""
	num = 0

	identifiedElements = findElementsWithId(doc.documentElement)
	if unprotectedElements is None:
		unprotectedElements = identifiedElements
	referencedIDs = findReferencedElements(doc.documentElement)

	# Make idList (list of idnames) sorted by reference count
	# descending, so the highest reference count is first.
	# First check that there's actually a defining element for the current ID name.
	# (Cyn: I've seen documents with #id references but no element with that ID!)
	idList = [(referencedIDs[rid][0], rid)  for rid in referencedIDs 
				if rid in unprotectedElements]
	idList.sort(reverse=True)
	idList = [rid for count, rid in idList]
	
	curIdNum = 1
	
	for rid in idList:
		curId = intToID(curIdNum)
		# First make sure that *this* element isn't already using
		# the ID name we want to give it.
		if curId != rid:
			# Then, skip ahead if the new ID is already in identifiedElement.
			while curId in identifiedElements:
				curIdNum += 1
				curId = intToID(curIdNum)
			# Then go rename it.
			num += renameID(doc, rid, curId, identifiedElements, referencedIDs)
		curIdNum += 1
	
	return num

def intToID(idnum):
	"""
	Returns the ID name for the given ID number, spreadsheet-style, i.e. from a to z,
	then from aa to az, ba to bz, etc., until zz.
	"""
	rid = ''
	
	while idnum > 0:
		idnum -= 1
		rid = chr((idnum % 26) + ord('a')) + rid
		idnum = int(idnum / 26)
	
	return rid

def renameID(doc, idFrom, idTo, identifiedElements, referencedIDs):
	"""
	Changes the ID name from idFrom to idTo, on the declaring element
	as well as all references in the document doc.
	
	Updates identifiedElements and referencedIDs.
	Does not handle the case where idTo is already the ID name
	of another element in doc.
	
	Returns the number of bytes saved by this replacement.
	"""
	
	num = 0
	
	definingNode = identifiedElements[idFrom]
	definingNode.setAttribute("id", idTo)
	del identifiedElements[idFrom]
	identifiedElements[idTo] = definingNode
	
	referringNodes = referencedIDs[idFrom]
	
	# Look for the idFrom ID name in each of the referencing elements,
	# exactly like findReferencedElements would. 
	# Cyn: Duplicated processing!

	for node in referringNodes[1]:
		# if this node is a style element, parse its text into CSS
		if node.nodeName == 'style' and node.namespaceURI == NS['SVG']:
			# node.firstChild will be either a CDATA or a Text node now
			if node.firstChild != None:
				# concatenate the value of all children, in case
				# there's a CDATASection node surrounded by whitespace
				# nodes
				# (node.normalize() will NOT work here, it only acts on Text nodes)
				oldValue = "".join([child.nodeValue for child in node.childNodes])
				# not going to reparse the whole thing
				newValue = oldValue.replace('url(#' + idFrom + ')', 'url(#' + idTo + ')')
				newValue = newValue.replace("url(#'" + idFrom + "')", 'url(#' + idTo + ')')
				newValue = newValue.replace('url(#"' + idFrom + '")', 'url(#' + idTo + ')')
				# and now replace all the children with this new stylesheet.
				# again, this is in case the stylesheet was a CDATASection
				node.childNodes[:] = [node.ownerDocument.createTextNode(newValue)]
				num += len(oldValue) - len(newValue)
	
		# if xlink:href is set to #idFrom, then change the id
		href = node.getAttributeNS(NS['XLINK'],'href')
		if href == '#' + idFrom:
			node.setAttributeNS(NS['XLINK'],'href', '#' + idTo)
			num += len(idFrom) - len(idTo)

		# if the style has url(#idFrom), then change the id
		styles = node.getAttribute('style')
		if styles != '':
			newValue = styles.replace('url(#' + idFrom + ')', 'url(#' + idTo + ')')
			newValue = newValue.replace("url('#" + idFrom + "')", 'url(#' + idTo + ')')
			newValue = newValue.replace('url("#' + idFrom + '")', 'url(#' + idTo + ')')
			node.setAttribute('style', newValue)
			num += len(styles) - len(newValue)
			
		# now try the fill, stroke, filter attributes
		for attr in referencingProps:
			oldValue = node.getAttribute(attr)
			if oldValue != '':
				newValue = oldValue.replace('url(#' + idFrom + ')', 'url(#' + idTo + ')')
				newValue = newValue.replace("url('#" + idFrom + "')", 'url(#' + idTo + ')')
				newValue = newValue.replace('url("#' + idFrom + '")', 'url(#' + idTo + ')')
				node.setAttribute(attr, newValue)
				num += len(oldValue) - len(newValue)
				
	del referencedIDs[idFrom]
	referencedIDs[idTo] = referringNodes
	
	return num

def unprotected_ids(doc, options):
	u"""Returns a list of unprotected IDs within the document doc."""
	identifiedElements = findElementsWithId(doc.documentElement)
	if not (options.protect_ids_noninkscape or
			options.protect_ids_list or
			options.protect_ids_prefix):
		return identifiedElements
	if options.protect_ids_list:
		protect_ids_list = options.protect_ids_list.split(",")
	if options.protect_ids_prefix:
		protect_ids_prefixes = options.protect_ids_prefix.split(",")
	for id in identifiedElements.keys():
		protected = False
		if options.protect_ids_noninkscape and not id[-1].isdigit():
			protected = True
		if options.protect_ids_list and id in protect_ids_list:
			protected = True
		if options.protect_ids_prefix:
			for prefix in protect_ids_prefixes:
				if id.startswith(prefix):
					protected = True
		if protected:
			del identifiedElements[id]
	return identifiedElements

def removeUnreferencedIDs(referencedIDs, identifiedElements):
	"""
	Removes the unreferenced ID attributes.
	
	Returns the number of ID attributes removed
	"""
	global numIDsRemoved
	keepTags = ['font']
	num = 0;
	for id in identifiedElements.keys():
		node = identifiedElements[id]
		if referencedIDs.has_key(id) == False and not node.nodeName in keepTags:
			node.removeAttribute('id')
			numIDsRemoved += 1
			num += 1
	return num
	
def removeNamespacedAttributes(node, namespaces):
	global numAttrsRemoved
	num = 0
	if node.nodeType == 1 :
		# remove all namespace'd attributes from this element
		attrList = node.attributes
		attrsToRemove = []
		for attrNum in xrange(attrList.length):
			attr = attrList.item(attrNum)
			if attr != None and attr.namespaceURI in namespaces:
				attrsToRemove.append(attr.nodeName)
		for attrName in attrsToRemove :
			num += 1
			numAttrsRemoved += 1
			node.removeAttribute(attrName)
		
		# now recurse for children
		for child in node.childNodes:
			num += removeNamespacedAttributes(child, namespaces)
	return num
	
def removeNamespacedElements(node, namespaces):
	global numElemsRemoved
	num = 0
	if node.nodeType == 1 :
		# remove all namespace'd child nodes from this element
		childList = node.childNodes
		childrenToRemove = []
		for child in childList:
			if child != None and child.namespaceURI in namespaces:
				childrenToRemove.append(child)
		for child in childrenToRemove :
			num += 1
			numElemsRemoved += 1
			node.removeChild(child)
		
		# now recurse for children
		for child in node.childNodes:
			num += removeNamespacedElements(child, namespaces)
	return num
	
def removeMetadataElements(doc):
	global numElemsRemoved
	num = 0
	# clone the list, as the tag list is live from the DOM
	elementsToRemove = [element for element in doc.documentElement.getElementsByTagName('metadata')]
	
	for element in elementsToRemove:
		element.parentNode.removeChild(element)
		num += 1
		numElemsRemoved += 1
	
	return num

def removeNestedGroups(node):
	""" 
	This walks further and further down the tree, removing groups
	which do not have any attributes or a title/desc child and 
	promoting their children up one level
	"""
	global numElemsRemoved
	num = 0
	
	groupsToRemove = []
	# Only consider <g> elements for promotion if this element isn't a <switch>.
	# (partial fix for bug 594930, required by the SVG spec however)
	if not (node.nodeType == 1 and node.nodeName == 'switch'):
		for child in node.childNodes:
			if child.nodeName == 'g' and child.namespaceURI == NS['SVG'] and len(child.attributes) == 0:
				# only collapse group if it does not have a title or desc as a direct descendant,
				for grandchild in child.childNodes:
					if grandchild.nodeType == 1 and grandchild.namespaceURI == NS['SVG'] and \
							grandchild.nodeName in ['title','desc']:
						break
				else:
					groupsToRemove.append(child)

	for g in groupsToRemove:
		while g.childNodes.length > 0:
			g.parentNode.insertBefore(g.firstChild, g)
		g.parentNode.removeChild(g)
		numElemsRemoved += 1
		num += 1

	# now recurse for children
	for child in node.childNodes:
		if child.nodeType == 1:
			num += removeNestedGroups(child)		
	return num

def moveCommonAttributesToParentGroup(elem, referencedElements):
	""" 
	This recursively calls this function on all children of the passed in element
	and then iterates over all child elements and removes common inheritable attributes 
	from the children and places them in the parent group.  But only if the parent contains
	nothing but element children and whitespace.  The attributes are only removed from the
	children if the children are not referenced by other elements in the document.
	"""
	num = 0
	
	childElements = []
	# recurse first into the children (depth-first)
	for child in elem.childNodes:
		if child.nodeType == 1:
			# only add and recurse if the child is not referenced elsewhere
			if not child.getAttribute('id') in referencedElements:
				childElements.append(child)
				num += moveCommonAttributesToParentGroup(child, referencedElements)
		# else if the parent has non-whitespace text children, do not
		# try to move common attributes
		elif child.nodeType == 3 and child.nodeValue.strip():
			return num

	# only process the children if there are more than one element
	if len(childElements) <= 1: return num
	
	commonAttrs = {}
	# add all inheritable properties of the first child element
	# FIXME: Note there is a chance that the first child is a set/animate in which case
	# its fill attribute is not what we want to look at, we should look for the first
	# non-animate/set element
	attrList = childElements[0].attributes
	for num in xrange(attrList.length):
		attr = attrList.item(num)
		# this is most of the inheritable properties from http://www.w3.org/TR/SVG11/propidx.html
		# and http://www.w3.org/TR/SVGTiny12/attributeTable.html
		if attr.nodeName in ['clip-rule',
					'display-align', 
					'fill', 'fill-opacity', 'fill-rule', 
					'font', 'font-family', 'font-size', 'font-size-adjust', 'font-stretch',
					'font-style', 'font-variant', 'font-weight',
					'letter-spacing',
					'pointer-events', 'shape-rendering',
					'stroke', 'stroke-dasharray', 'stroke-dashoffset', 'stroke-linecap', 'stroke-linejoin',
					'stroke-miterlimit', 'stroke-opacity', 'stroke-width',
					'text-anchor', 'text-decoration', 'text-rendering', 'visibility', 
					'word-spacing', 'writing-mode']:
			# we just add all the attributes from the first child
			commonAttrs[attr.nodeName] = attr.nodeValue
	
	# for each subsequent child element
	for childNum in xrange(len(childElements)):
		# skip first child
		if childNum == 0: 
			continue
			
		child = childElements[childNum]
		# if we are on an animateXXX/set element, ignore it (due to the 'fill' attribute)
		if child.localName in ['set', 'animate', 'animateColor', 'animateTransform', 'animateMotion']:
			continue
			
		distinctAttrs = []
		# loop through all current 'common' attributes
		for name in commonAttrs.keys():
			# if this child doesn't match that attribute, schedule it for removal
			if child.getAttribute(name) != commonAttrs[name]:
				distinctAttrs.append(name)
		# remove those attributes which are not common
		for name in distinctAttrs:
			del commonAttrs[name]
	
	# commonAttrs now has all the inheritable attributes which are common among all child elements
	for name in commonAttrs.keys():
		for child in childElements:
			child.removeAttribute(name)
		elem.setAttribute(name, commonAttrs[name])

	# update our statistic (we remove N*M attributes and add back in M attributes)
	num += (len(childElements)-1) * len(commonAttrs)
	return num

def createGroupsForCommonAttributes(elem):
	"""
	Creates <g> elements to contain runs of 3 or more
	consecutive child elements having at least one common attribute.
	
	Common attributes are not promoted to the <g> by this function.
	This is handled by moveCommonAttributesToParentGroup.
	
	If all children have a common attribute, an extra <g> is not created.
	
	This function acts recursively on the given element.
	"""
	num = 0
	global numElemsRemoved
	
	# TODO perhaps all of the Presentation attributes in http://www.w3.org/TR/SVG/struct.html#GElement
	# could be added here
	# Cyn: These attributes are the same as in moveAttributesToParentGroup, and must always be
	for curAttr in ['clip-rule',
				'display-align', 
				'fill', 'fill-opacity', 'fill-rule', 
				'font', 'font-family', 'font-size', 'font-size-adjust', 'font-stretch',
				'font-style', 'font-variant', 'font-weight',
				'letter-spacing',
				'pointer-events', 'shape-rendering',
				'stroke', 'stroke-dasharray', 'stroke-dashoffset', 'stroke-linecap', 'stroke-linejoin',
				'stroke-miterlimit', 'stroke-opacity', 'stroke-width',
				'text-anchor', 'text-decoration', 'text-rendering', 'visibility', 
				'word-spacing', 'writing-mode']:
		# Iterate through the children in reverse order, so item(i) for
		# items we have yet to visit still returns the correct nodes.
		curChild = elem.childNodes.length - 1
		while curChild >= 0:
			childNode = elem.childNodes.item(curChild)
			
			if childNode.nodeType == 1 and childNode.getAttribute(curAttr) != '':
				# We're in a possible run! Track the value and run length.
				value = childNode.getAttribute(curAttr)
				runStart, runEnd = curChild, curChild
				# Run elements includes only element tags, no whitespace/comments/etc.
				# Later, we calculate a run length which includes these.
				runElements = 1
				
				# Backtrack to get all the nodes having the same
				# attribute value, preserving any nodes in-between.
				while runStart > 0:
					nextNode = elem.childNodes.item(runStart - 1)
					if nextNode.nodeType == 1:
						if nextNode.getAttribute(curAttr) != value: break
						else:
							runElements += 1
							runStart -= 1
					else: runStart -= 1
				
				if runElements >= 3:
					# Include whitespace/comment/etc. nodes in the run.
					while runEnd < elem.childNodes.length - 1:
						if elem.childNodes.item(runEnd + 1).nodeType == 1: break
						else: runEnd += 1
					
					runLength = runEnd - runStart + 1
					if runLength == elem.childNodes.length: # Every child has this
						# If the current parent is a <g> already,
						if elem.nodeName == 'g' and elem.namespaceURI == NS['SVG']:
							# do not act altogether on this attribute; all the
							# children have it in common.
							# Let moveCommonAttributesToParentGroup do it.
							curChild = -1
							continue
						# otherwise, it might be an <svg> element, and
						# even if all children have the same attribute value,
						# it's going to be worth making the <g> since
						# <svg> doesn't support attributes like 'stroke'.
						# Fall through.
					
					# Create a <g> element from scratch.
					# We need the Document for this.
					document = elem.ownerDocument
					group = document.createElementNS(NS['SVG'], 'g')
					# Move the run of elements to the group.
					# a) ADD the nodes to the new group.
					group.childNodes[:] = elem.childNodes[runStart:runEnd + 1]
					for child in group.childNodes:
						child.parentNode = group
					# b) REMOVE the nodes from the element.
					elem.childNodes[runStart:runEnd + 1] = []
					# Include the group in elem's children.
					elem.childNodes.insert(runStart, group)
					group.parentNode = elem
					num += 1
					curChild = runStart - 1
					numElemsRemoved -= 1
				else:
					curChild -= 1
			else:
				curChild -= 1
	
	# each child gets the same treatment, recursively
	for childNode in elem.childNodes:
		if childNode.nodeType == 1:
			num += createGroupsForCommonAttributes(childNode)

	return num

def removeUnusedAttributesOnParent(elem):
	"""
	This recursively calls this function on all children of the element passed in,
	then removes any unused attributes on this elem if none of the children inherit it
	"""
	num = 0

	childElements = []
	# recurse first into the children (depth-first)
	for child in elem.childNodes:
		if child.nodeType == 1: 
			childElements.append(child)
			num += removeUnusedAttributesOnParent(child)
	
	# only process the children if there are more than one element
	if len(childElements) <= 1: return num

	# get all attribute values on this parent
	attrList = elem.attributes
	unusedAttrs = {}
	for num in xrange(attrList.length):
		attr = attrList.item(num)
		if attr.nodeName in ['clip-rule',
					'display-align', 
					'fill', 'fill-opacity', 'fill-rule', 
					'font', 'font-family', 'font-size', 'font-size-adjust', 'font-stretch',
					'font-style', 'font-variant', 'font-weight',
					'letter-spacing',
					'pointer-events', 'shape-rendering',
					'stroke', 'stroke-dasharray', 'stroke-dashoffset', 'stroke-linecap', 'stroke-linejoin',
					'stroke-miterlimit', 'stroke-opacity', 'stroke-width',
					'text-anchor', 'text-decoration', 'text-rendering', 'visibility', 
					'word-spacing', 'writing-mode']:
			unusedAttrs[attr.nodeName] = attr.nodeValue
	
	# for each child, if at least one child inherits the parent's attribute, then remove
	for childNum in xrange(len(childElements)):
		child = childElements[childNum]
		inheritedAttrs = []
		for name in unusedAttrs.keys():
			val = child.getAttribute(name)
			if val == '' or val == None or val == 'inherit':
				inheritedAttrs.append(name)
		for a in inheritedAttrs:
			del unusedAttrs[a]
	
	# unusedAttrs now has all the parent attributes that are unused
	for name in unusedAttrs.keys():
		elem.removeAttribute(name)
		num += 1
	
	return num
	
def removeDuplicateGradientStops(doc):
	global numElemsRemoved
	num = 0
	
	for gradType in ['linearGradient', 'radialGradient']:
		for grad in doc.getElementsByTagName(gradType):
			stops = {}
			stopsToRemove = []
			for stop in grad.getElementsByTagName('stop'):
				# convert percentages into a floating point number
				offsetU = SVGLength(stop.getAttribute('offset'))
				if offsetU.units == Unit.PCT:
					offset = offsetU.value / 100.0
				elif offsetU.units == Unit.NONE:
					offset = offsetU.value
				else:
					offset = 0
				# set the stop offset value to the integer or floating point equivalent
				if int(offset) == offset: stop.setAttribute('offset', str(int(offset)))
				else: stop.setAttribute('offset', str(offset))
					
				color = stop.getAttribute('stop-color')
				opacity = stop.getAttribute('stop-opacity')
				style = stop.getAttribute('style')
				if stops.has_key(offset) :
					oldStop = stops[offset]
					if oldStop[0] == color and oldStop[1] == opacity and oldStop[2] == style:
						stopsToRemove.append(stop)
				stops[offset] = [color, opacity, style]
				
			for stop in stopsToRemove:
				stop.parentNode.removeChild(stop)
				num += 1
				numElemsRemoved += 1
	
	# linear gradients
	return num

def collapseSinglyReferencedGradients(doc):
	global numElemsRemoved
	num = 0
	
	identifiedElements = findElementsWithId(doc.documentElement)
	
	# make sure to reset the ref'ed ids for when we are running this in testscour
	for rid,nodeCount in findReferencedElements(doc.documentElement).iteritems():
		count = nodeCount[0]
		nodes = nodeCount[1]
		# Make sure that there's actually a defining element for the current ID name.
		# (Cyn: I've seen documents with #id references but no element with that ID!)
		if count == 1 and rid in identifiedElements:
			elem = identifiedElements[rid]
			if elem != None and elem.nodeType == 1 and elem.nodeName in ['linearGradient', 'radialGradient'] \
					and elem.namespaceURI == NS['SVG']:
				# found a gradient that is referenced by only 1 other element
				refElem = nodes[0]
				if refElem.nodeType == 1 and refElem.nodeName in ['linearGradient', 'radialGradient'] \
						and refElem.namespaceURI == NS['SVG']:
					# elem is a gradient referenced by only one other gradient (refElem)
					
					# add the stops to the referencing gradient (this removes them from elem)
					if len(refElem.getElementsByTagName('stop')) == 0:
						stopsToAdd = elem.getElementsByTagName('stop')
						for stop in stopsToAdd:
							refElem.appendChild(stop)
							
					# adopt the gradientUnits, spreadMethod,  gradientTransform attributes if
					# they are unspecified on refElem
					for attr in ['gradientUnits','spreadMethod','gradientTransform']:
						if refElem.getAttribute(attr) == '' and not elem.getAttribute(attr) == '':
							refElem.setAttributeNS(None, attr, elem.getAttribute(attr))
							
					# if both are radialGradients, adopt elem's fx,fy,cx,cy,r attributes if
					# they are unspecified on refElem
					if elem.nodeName == 'radialGradient' and refElem.nodeName == 'radialGradient':
						for attr in ['fx','fy','cx','cy','r']:
							if refElem.getAttribute(attr) == '' and not elem.getAttribute(attr) == '':
								refElem.setAttributeNS(None, attr, elem.getAttribute(attr))
					
					# if both are linearGradients, adopt elem's x1,y1,x2,y2 attributes if 
					# they are unspecified on refElem
					if elem.nodeName == 'linearGradient' and refElem.nodeName == 'linearGradient':
						for attr in ['x1','y1','x2','y2']:
							if refElem.getAttribute(attr) == '' and not elem.getAttribute(attr) == '':
								refElem.setAttributeNS(None, attr, elem.getAttribute(attr))
								
					# now remove the xlink:href from refElem
					refElem.removeAttributeNS(NS['XLINK'], 'href')
					
					# now delete elem
					elem.parentNode.removeChild(elem)
					numElemsRemoved += 1
					num += 1		
	return num

def removeDuplicateGradients(doc):
	global numElemsRemoved
	num = 0
	
	gradientsToRemove = {}
	duplicateToMaster = {}

	for gradType in ['linearGradient', 'radialGradient']:
		grads = doc.getElementsByTagName(gradType)
		for grad in grads:
			# TODO: should slice grads from 'grad' here to optimize
			for ograd in grads:
				# do not compare gradient to itself
				if grad == ograd: continue

				# compare grad to ograd (all properties, then all stops)
				# if attributes do not match, go to next gradient
				someGradAttrsDoNotMatch = False
				for attr in ['gradientUnits','spreadMethod','gradientTransform','x1','y1','x2','y2','cx','cy','fx','fy','r']:
					if grad.getAttribute(attr) != ograd.getAttribute(attr):
						someGradAttrsDoNotMatch = True
						break;
				
				if someGradAttrsDoNotMatch: continue

				# compare xlink:href values too
				if grad.getAttributeNS(NS['XLINK'], 'href') != ograd.getAttributeNS(NS['XLINK'], 'href'):
					continue

				# all gradient properties match, now time to compare stops
				stops = grad.getElementsByTagName('stop')
				ostops = ograd.getElementsByTagName('stop')

				if stops.length != ostops.length: continue

				# now compare stops
				stopsNotEqual = False
				for i in xrange(stops.length):
					if stopsNotEqual: break
					stop = stops.item(i)
					ostop = ostops.item(i)
					for attr in ['offset', 'stop-color', 'stop-opacity', 'style']:
						if stop.getAttribute(attr) != ostop.getAttribute(attr):
							stopsNotEqual = True
							break
				if stopsNotEqual: continue

				# ograd is a duplicate of grad, we schedule it to be removed UNLESS
				# ograd is ALREADY considered a 'master' element
				if not gradientsToRemove.has_key(ograd):
					if not duplicateToMaster.has_key(ograd):
						if not gradientsToRemove.has_key(grad):
							gradientsToRemove[grad] = []
						gradientsToRemove[grad].append( ograd )
						duplicateToMaster[ograd] = grad
	
	# get a collection of all elements that are referenced and their referencing elements
	referencedIDs = findReferencedElements(doc.documentElement)
	for masterGrad in gradientsToRemove.keys():
		master_id = masterGrad.getAttribute('id')
#		print 'master='+master_id
		for dupGrad in gradientsToRemove[masterGrad]:
			# if the duplicate gradient no longer has a parent that means it was
			# already re-mapped to another master gradient
			if not dupGrad.parentNode: continue
			dup_id = dupGrad.getAttribute('id')
#			print 'dup='+dup_id
#			print referencedIDs[dup_id]
			# for each element that referenced the gradient we are going to remove
			for elem in referencedIDs[dup_id][1]:
				# find out which attribute referenced the duplicate gradient
				for attr in ['fill', 'stroke']:
					v = elem.getAttribute(attr)
					if v == 'url(#'+dup_id+')' or v == 'url("#'+dup_id+'")' or v == "url('#"+dup_id+"')":
						elem.setAttribute(attr, 'url(#'+master_id+')')
				if elem.getAttributeNS(NS['XLINK'], 'href') == '#'+dup_id:
					elem.setAttributeNS(NS['XLINK'], 'href', '#'+master_id)
				styles = _getStyle(elem)
				for style in styles:
					v = styles[style]
					if v == 'url(#'+dup_id+')' or v == 'url("#'+dup_id+'")' or v == "url('#"+dup_id+"')":
						styles[style] = 'url(#'+master_id+')'
				_setStyle(elem, styles)
			
			# now that all referencing elements have been re-mapped to the master
			# it is safe to remove this gradient from the document
			dupGrad.parentNode.removeChild(dupGrad)
			numElemsRemoved += 1
			num += 1
	return num

def _getStyle(node):
	u"""Returns the style attribute of a node as a dictionary."""
	if node.nodeType == 1 and len(node.getAttribute('style')) > 0 :	
		styleMap = { }
		rawStyles = node.getAttribute('style').split(';')
		for style in rawStyles:
			propval = style.split(':')
			if len(propval) == 2 :
				styleMap[propval[0].strip()] = propval[1].strip()
		return styleMap
	else:
		return {}

def _setStyle(node, styleMap):
	u"""Sets the style attribute of a node to the dictionary ``styleMap``."""
	fixedStyle = ';'.join([prop + ':' + styleMap[prop] for prop in styleMap.keys()])
	if fixedStyle != '' :
		node.setAttribute('style', fixedStyle)
	elif node.getAttribute('style'):
		node.removeAttribute('style')
	return node

def repairStyle(node, options):
	num = 0
	styleMap = _getStyle(node)
	if styleMap:

		# I've seen this enough to know that I need to correct it:
		# fill: url(#linearGradient4918) rgb(0, 0, 0);
		for prop in ['fill', 'stroke'] :
			if styleMap.has_key(prop) :
				chunk = styleMap[prop].split(') ')
				if len(chunk) == 2 and (chunk[0][:5] == 'url(#' or chunk[0][:6] == 'url("#' or chunk[0][:6] == "url('#") and chunk[1] == 'rgb(0, 0, 0)' :
					styleMap[prop] = chunk[0] + ')'
					num += 1

		# Here is where we can weed out unnecessary styles like:
		#  opacity:1
		if styleMap.has_key('opacity') :
			opacity = float(styleMap['opacity'])
			# if opacity='0' then all fill and stroke properties are useless, remove them
			if opacity == 0.0 :
				for uselessStyle in ['fill', 'fill-opacity', 'fill-rule', 'stroke', 'stroke-linejoin',
					'stroke-opacity', 'stroke-miterlimit', 'stroke-linecap', 'stroke-dasharray',
					'stroke-dashoffset', 'stroke-opacity'] :
					if styleMap.has_key(uselessStyle):
						del styleMap[uselessStyle]
						num += 1

		#  if stroke:none, then remove all stroke-related properties (stroke-width, etc)
		#  TODO: should also detect if the computed value of this element is stroke="none"
		if styleMap.has_key('stroke') and styleMap['stroke'] == 'none' :
			for strokestyle in [ 'stroke-width', 'stroke-linejoin', 'stroke-miterlimit', 
					'stroke-linecap', 'stroke-dasharray', 'stroke-dashoffset', 'stroke-opacity'] :
				if styleMap.has_key(strokestyle) :
					del styleMap[strokestyle]
					num += 1
			# TODO: This is actually a problem if a parent element has a specified stroke
			# we need to properly calculate computed values
			del styleMap['stroke']

		#  if fill:none, then remove all fill-related properties (fill-rule, etc)
		if styleMap.has_key('fill') and styleMap['fill'] == 'none' :
			for fillstyle in [ 'fill-rule', 'fill-opacity' ] :
				if styleMap.has_key(fillstyle) :
					del styleMap[fillstyle]
					num += 1
					
		#  fill-opacity: 0
		if styleMap.has_key('fill-opacity') :
			fillOpacity = float(styleMap['fill-opacity'])
			if fillOpacity == 0.0 :
				for uselessFillStyle in [ 'fill', 'fill-rule' ] :
					if styleMap.has_key(uselessFillStyle):
						del styleMap[uselessFillStyle]
						num += 1
		
		#  stroke-opacity: 0
		if styleMap.has_key('stroke-opacity') :
			strokeOpacity = float(styleMap['stroke-opacity']) 
			if strokeOpacity == 0.0 :
				for uselessStrokeStyle in [ 'stroke', 'stroke-width', 'stroke-linejoin', 'stroke-linecap', 
							'stroke-dasharray', 'stroke-dashoffset' ] :
					if styleMap.has_key(uselessStrokeStyle): 
						del styleMap[uselessStrokeStyle]
						num += 1

		# stroke-width: 0
		if styleMap.has_key('stroke-width') :
			strokeWidth = SVGLength(styleMap['stroke-width']) 
			if strokeWidth.value == 0.0 :
				for uselessStrokeStyle in [ 'stroke', 'stroke-linejoin', 'stroke-linecap', 
							'stroke-dasharray', 'stroke-dashoffset', 'stroke-opacity' ] :
					if styleMap.has_key(uselessStrokeStyle): 
						del styleMap[uselessStrokeStyle]
						num += 1
		
		# remove font properties for non-text elements
		# I've actually observed this in real SVG content
		if not mayContainTextNodes(node):
			for fontstyle in [ 'font-family', 'font-size', 'font-stretch', 'font-size-adjust', 
								'font-style', 'font-variant', 'font-weight', 
								'letter-spacing', 'line-height', 'kerning',
								'text-align', 'text-anchor', 'text-decoration',
								'text-rendering', 'unicode-bidi',
								'word-spacing', 'writing-mode'] :
				if styleMap.has_key(fontstyle) :
					del styleMap[fontstyle]
					num += 1

		# remove inkscape-specific styles
		# TODO: need to get a full list of these
		for inkscapeStyle in ['-inkscape-font-specification']:
			if styleMap.has_key(inkscapeStyle):
				del styleMap[inkscapeStyle]
				num += 1

		if styleMap.has_key('overflow') :
			# overflow specified on element other than svg, marker, pattern
			if not node.nodeName in ['svg','marker','pattern']:
				del styleMap['overflow']
				num += 1
			# it is a marker, pattern or svg
			# as long as this node is not the document <svg>, then only
			# remove overflow='hidden'.  See 
			# http://www.w3.org/TR/2010/WD-SVG11-20100622/masking.html#OverflowProperty
			elif node != node.ownerDocument.documentElement:
				if styleMap['overflow'] == 'hidden':
					del styleMap['overflow']
					num += 1
			# else if outer svg has a overflow="visible", we can remove it
			elif styleMap['overflow'] == 'visible':
					del styleMap['overflow']
					num += 1
		
		# now if any of the properties match known SVG attributes we prefer attributes 
		# over style so emit them and remove them from the style map
		if options.style_to_xml:
			for propName in styleMap.keys() :
				if propName in svgAttributes :
					node.setAttribute(propName, styleMap[propName])
					del styleMap[propName]
		
		_setStyle(node, styleMap)
		
	# recurse for our child elements
	for child in node.childNodes :
		num += repairStyle(child,options)
			
	return num

def mayContainTextNodes(node):
	"""
	Returns True if the passed-in node is probably a text element, or at least
	one of its descendants is probably a text element.

	If False is returned, it is guaranteed that the passed-in node has no
	business having text-based attributes.

	If True is returned, the passed-in node should not have its text-based
	attributes removed.
	"""
	# Cached result of a prior call?
	try:
		return node.mayContainTextNodes
	except AttributeError:
		pass

	result = True # Default value
	# Comment, text and CDATA nodes don't have attributes and aren't containers
	if node.nodeType != 1:
		result = False
	# Non-SVG elements? Unknown elements!
	elif node.namespaceURI != NS['SVG']:
		result = True
	# Blacklisted elements. Those are guaranteed not to be text elements.
	elif node.nodeName in ['rect', 'circle', 'ellipse', 'line', 'polygon',
	                       'polyline', 'path', 'image', 'stop']:
		result = False
	# Group elements. If we're missing any here, the default of True is used.
	elif node.nodeName in ['g', 'clipPath', 'marker', 'mask', 'pattern',
	                       'linearGradient', 'radialGradient', 'symbol']:
		result = False
		for child in node.childNodes:
			if mayContainTextNodes(child):
				result = True
	# Everything else should be considered a future SVG-version text element
	# at best, or an unknown element at worst. result will stay True.

	# Cache this result before returning it.
	node.mayContainTextNodes = result
	return result

def taint(taintedSet, taintedAttribute):
	u"""Adds an attribute to a set of attributes.
	
	Related attributes are also included."""
	taintedSet.add(taintedAttribute)
	if taintedAttribute == 'marker':
		taintedSet |= set(['marker-start', 'marker-mid', 'marker-end'])
	if taintedAttribute in ['marker-start', 'marker-mid', 'marker-end']:
		taintedSet.add('marker')
	return taintedSet

def removeDefaultAttributeValues(node, options, tainted=set()):
	u"""'tainted' keeps a set of attributes defined in parent nodes.
	
	For such attributes, we don't delete attributes with default values."""
	num = 0
	if node.nodeType != 1: return 0
	
	# gradientUnits: objectBoundingBox
	if node.getAttribute('gradientUnits') == 'objectBoundingBox':
		node.removeAttribute('gradientUnits')
		num += 1
		
	# spreadMethod: pad
	if node.getAttribute('spreadMethod') == 'pad':
		node.removeAttribute('spreadMethod')
		num += 1
		
	# x1: 0%
	if node.getAttribute('x1') != '':
		x1 = SVGLength(node.getAttribute('x1'))
		if x1.value == 0:
			node.removeAttribute('x1')
			num += 1

	# y1: 0%
	if node.getAttribute('y1') != '':
		y1 = SVGLength(node.getAttribute('y1'))
		if y1.value == 0:
			node.removeAttribute('y1')
			num += 1

	# x2: 100%
	if node.getAttribute('x2') != '':
		x2 = SVGLength(node.getAttribute('x2'))
		if (x2.value == 100 and x2.units == Unit.PCT) or (x2.value == 1 and x2.units == Unit.NONE):
			node.removeAttribute('x2')
			num += 1

	# y2: 0%
	if node.getAttribute('y2') != '':
		y2 = SVGLength(node.getAttribute('y2'))
		if y2.value == 0:
			node.removeAttribute('y2')
			num += 1

	# fx: equal to rx
	if node.getAttribute('fx') != '':
		if node.getAttribute('fx') == node.getAttribute('cx'):
			node.removeAttribute('fx')
			num += 1

	# fy: equal to ry
	if node.getAttribute('fy') != '':
		if node.getAttribute('fy') == node.getAttribute('cy'):
			node.removeAttribute('fy')
			num += 1

	# cx: 50%
	if node.getAttribute('cx') != '':
		cx = SVGLength(node.getAttribute('cx'))
		if (cx.value == 50 and cx.units == Unit.PCT) or (cx.value == 0.5 and cx.units == Unit.NONE):
			node.removeAttribute('cx')
			num += 1

	# cy: 50%
	if node.getAttribute('cy') != '':
		cy = SVGLength(node.getAttribute('cy'))
		if (cy.value == 50 and cy.units == Unit.PCT) or (cy.value == 0.5 and cy.units == Unit.NONE):
			node.removeAttribute('cy')
			num += 1

	# r: 50%
	if node.getAttribute('r') != '':
		r = SVGLength(node.getAttribute('r'))
		if (r.value == 50 and r.units == Unit.PCT) or (r.value == 0.5 and r.units == Unit.NONE):
			node.removeAttribute('r')
			num += 1
	
	# Summarily get rid of some more attributes
	attributes = [node.attributes.item(i).nodeName 
				  for i in range(node.attributes.length)]
	for attribute in attributes:
		if attribute not in tainted:
			if attribute in default_attributes.keys():
				if node.getAttribute(attribute) == default_attributes[attribute]:
					node.removeAttribute(attribute)
					num += 1
				else:
					tainted = taint(tainted, attribute)
	# These attributes might also occur as styles
	styles = _getStyle(node)
	for attribute in styles.keys():
		if attribute not in tainted:
			if attribute in default_attributes.keys():
				if styles[attribute] == default_attributes[attribute]:
					del styles[attribute]
					num += 1
				else:
					tainted = taint(tainted, attribute)
	_setStyle(node, styles)

	# recurse for our child elements
	for child in node.childNodes :
		num += removeDefaultAttributeValues(child, options, tainted.copy())
	
	return num

rgb = re.compile(r"\s*rgb\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)\s*")
rgbp = re.compile(r"\s*rgb\(\s*(\d*\.?\d+)%\s*,\s*(\d*\.?\d+)%\s*,\s*(\d*\.?\d+)%\s*\)\s*")
def convertColor(value):
	"""
		Converts the input color string and returns a #RRGGBB (or #RGB if possible) string
	"""
	s = value
	
	if s in colors.keys():
		s = colors[s]
	
	rgbpMatch = rgbp.match(s)
	if rgbpMatch != None :
		r = int(float(rgbpMatch.group(1)) * 255.0 / 100.0)
		g = int(float(rgbpMatch.group(2)) * 255.0 / 100.0)
		b = int(float(rgbpMatch.group(3)) * 255.0 / 100.0)
		s  = '#%02x%02x%02x' % (r, g, b)
	else:
		rgbMatch = rgb.match(s)
		if rgbMatch != None :
			r = int( rgbMatch.group(1) )
			g = int( rgbMatch.group(2) )
			b = int( rgbMatch.group(3) )
			s = '#%02x%02x%02x' % (r, g, b)
	
	if s[0] == '#':
		s = s.lower()
		if len(s)==7 and s[1]==s[2] and s[3]==s[4] and s[5]==s[6]:
			s = '#'+s[1]+s[3]+s[5]

	return s
	
def convertColors(element) :
	"""
		Recursively converts all color properties into #RRGGBB format if shorter
	"""
	numBytes = 0
	
	if element.nodeType != 1: return 0

	# set up list of color attributes for each element type
	attrsToConvert = []
	if element.nodeName in ['rect', 'circle', 'ellipse', 'polygon', \
							'line', 'polyline', 'path', 'g', 'a']:
		attrsToConvert = ['fill', 'stroke']
	elif element.nodeName in ['stop']:
		attrsToConvert = ['stop-color']
	elif element.nodeName in ['solidColor']:
		attrsToConvert = ['solid-color']

	# now convert all the color formats
	styles = _getStyle(element)
	for attr in attrsToConvert:
		oldColorValue = element.getAttribute(attr)
		if oldColorValue != '':
			newColorValue = convertColor(oldColorValue)
			oldBytes = len(oldColorValue)
			newBytes = len(newColorValue)
			if oldBytes > newBytes:
				element.setAttribute(attr, newColorValue)
				numBytes += (oldBytes - len(element.getAttribute(attr)))
		# colors might also hide in styles
		if attr in styles.keys():
			oldColorValue = styles[attr]
			newColorValue = convertColor(oldColorValue)
			oldBytes = len(oldColorValue)
			newBytes = len(newColorValue)
			if oldBytes > newBytes:
				styles[attr] = newColorValue
				numBytes += (oldBytes - len(element.getAttribute(attr)))
	_setStyle(element, styles)
	
	# now recurse for our child elements
	for child in element.childNodes :
		numBytes += convertColors(child)

	return numBytes

# TODO: go over what this method does and see if there is a way to optimize it
# TODO: go over the performance of this method and see if I can save memory/speed by
#       reusing data structures, etc
def cleanPath(element, options) :
	"""
		Cleans the path string (d attribute) of the element 
	"""
	global numBytesSavedInPathData
	global numPathSegmentsReduced
	global numCurvesStraightened
	
	# this gets the parser object from svg_regex.py
	oldPathStr = element.getAttribute('d')
	path = svg_parser.parse(oldPathStr)

	# This determines whether the stroke has round linecaps.  If it does,
	# we do not want to collapse empty segments, as they are actually rendered.
	withRoundLineCaps = element.getAttribute('stroke-linecap') == 'round'

	# The first command must be a moveto, and whether it's relative (m)
	# or absolute (M), the first set of coordinates *is* absolute. So
	# the first iteration of the loop below will get x,y and startx,starty.
	
	# convert absolute coordinates into relative ones.
	# Reuse the data structure 'path', since we're not adding or removing subcommands.
	# Also reuse the coordinate lists since we're not adding or removing any.
	for pathIndex in xrange(0, len(path)):
		cmd, data = path[pathIndex] # Changes to cmd don't get through to the data structure
		i = 0
		# adjust abs to rel
		# only the A command has some values that we don't want to adjust (radii, rotation, flags)
		if cmd == 'A':
			for i in xrange(i, len(data), 7):
				data[i+5] -= x
				data[i+6] -= y
				x += data[i+5]
				y += data[i+6]
			path[pathIndex] = ('a', data)
		elif cmd == 'a':
			x += sum(data[5::7])
			y += sum(data[6::7])
		elif cmd == 'H':
			for i in xrange(i, len(data)):
				data[i] -= x
				x += data[i]
			path[pathIndex] = ('h', data)
		elif cmd == 'h':
			x += sum(data)
		elif cmd == 'V':
			for i in xrange(i, len(data)):
				data[i] -= y
				y += data[i]
			path[pathIndex] = ('v', data)
		elif cmd == 'v':
			y += sum(data)
		elif cmd == 'M':
			startx, starty = data[0], data[1]
			# If this is a path starter, don't convert its first
			# coordinate to relative; that would just make it (0, 0)
			if pathIndex != 0:
				data[0] -= x
				data[1] -= y
			
			x, y = startx, starty
			i = 2
			for i in xrange(i, len(data), 2):
				data[i] -= x
				data[i+1] -= y
				x += data[i]
				y += data[i+1]
			path[pathIndex] = ('m', data)
		elif cmd in ['L','T']:
			for i in xrange(i, len(data), 2):
				data[i] -= x
				data[i+1] -= y
				x += data[i]
				y += data[i+1]
			path[pathIndex] = (cmd.lower(), data)
		elif cmd in ['m']:
			if pathIndex == 0:
				# START OF PATH - this is an absolute moveto
				# followed by relative linetos
				startx, starty = data[0], data[1]
				x, y = startx, starty
				i = 2
			else:
				startx = x + data[0]
				starty = y + data[1]
			for i in xrange(i, len(data), 2):
				x += data[i]
				y += data[i+1]
		elif cmd in ['l','t']:
			x += sum(data[0::2])
			y += sum(data[1::2])
		elif cmd in ['S','Q']:
			for i in xrange(i, len(data), 4):
				data[i] -= x
				data[i+1] -= y
				data[i+2] -= x
				data[i+3] -= y
				x += data[i+2]
				y += data[i+3]
			path[pathIndex] = (cmd.lower(), data)
		elif cmd in ['s','q']:
			x += sum(data[2::4])
			y += sum(data[3::4])
		elif cmd == 'C':
			for i in xrange(i, len(data), 6):
				data[i] -= x
				data[i+1] -= y
				data[i+2] -= x
				data[i+3] -= y
				data[i+4] -= x
				data[i+5] -= y
				x += data[i+4]
				y += data[i+5]
			path[pathIndex] = ('c', data)
		elif cmd == 'c':
			x += sum(data[4::6])
			y += sum(data[5::6])
		elif cmd in ['z','Z']:
			x, y = startx, starty
			path[pathIndex] = ('z', data)
	
	# remove empty segments
	# Reuse the data structure 'path' and the coordinate lists, even if we're
	# deleting items, because these deletions are relatively cheap.
	if not withRoundLineCaps:
		for pathIndex in xrange(0, len(path)):
			cmd, data = path[pathIndex]
			i = 0
			if cmd in ['m','l','t']:
				if cmd == 'm':
					# remove m0,0 segments
					if pathIndex > 0 and data[0] == data[i+1] == 0:
						# 'm0,0 x,y' can be replaces with 'lx,y',
						# except the first m which is a required absolute moveto
						path[pathIndex] = ('l', data[2:])
						numPathSegmentsReduced += 1
					else: # else skip move coordinate
						i = 2
				while i < len(data):
					if data[i] == data[i+1] == 0:
						del data[i:i+2]
						numPathSegmentsReduced += 1
					else:
						i += 2
			elif cmd == 'c':
				while i < len(data):
					if data[i] == data[i+1] == data[i+2] == data[i+3] == data[i+4] == data[i+5] == 0:
						del data[i:i+6]
						numPathSegmentsReduced += 1
					else:
						i += 6
			elif cmd == 'a':
				while i < len(data):
					if data[i+5] == data[i+6] == 0:
						del data[i:i+7]
						numPathSegmentsReduced += 1
					else:
						i += 7
			elif cmd == 'q':
				while i < len(data):
					if data[i] == data[i+1] == data[i+2] == data[i+3] == 0:
						del data[i:i+4]
						numPathSegmentsReduced += 1
					else:
						i += 4
			elif cmd in ['h','v']:
				oldLen = len(data)
				path[pathIndex] = (cmd, [coord for coord in data if coord != 0])
				numPathSegmentsReduced += len(path[pathIndex][1]) - oldLen
	
	# fixup: Delete subcommands having no coordinates.
	path = [elem for elem in path if len(elem[1]) > 0 or elem[0] == 'z']
	
	# convert straight curves into lines
	newPath = [path[0]]
	for (cmd,data) in path[1:]:
		i = 0
		newData = data
		if cmd == 'c':
			newData = []
			while i < len(data):
				# since all commands are now relative, we can think of previous point as (0,0)
				# and new point (dx,dy) is (data[i+4],data[i+5])
				# eqn of line will be y = (dy/dx)*x or if dx=0 then eqn of line is x=0
				(p1x,p1y) = (data[i],data[i+1])
				(p2x,p2y) = (data[i+2],data[i+3])
				dx = data[i+4]
				dy = data[i+5]
				
				foundStraightCurve = False
				
				if dx == 0:
					if p1x == 0 and p2x == 0:
						foundStraightCurve = True
				else:
					m = dy/dx
					if p1y == m*p1x and p2y == m*p2x:
						foundStraightCurve = True

				if foundStraightCurve:
					# flush any existing curve coords first
					if newData:
						newPath.append( (cmd,newData) )
						newData = []
					# now create a straight line segment
					newPath.append( ('l', [dx,dy]) )
					numCurvesStraightened += 1
				else:
					newData.extend(data[i:i+6])
					
				i += 6
		if newData or cmd == 'z' or cmd == 'Z':
			newPath.append( (cmd,newData) )
	path = newPath

	# collapse all consecutive commands of the same type into one command
	prevCmd = ''
	prevData = []
	newPath = []
	for (cmd,data) in path:
		# flush the previous command if it is not the same type as the current command
		if prevCmd != '':
			if cmd != prevCmd or cmd == 'm':
				newPath.append( (prevCmd, prevData) )
				prevCmd = ''
				prevData = []
		
		# if the previous and current commands are the same type,
		# or the previous command is moveto and the current is lineto, collapse,
		# but only if they are not move commands (since move can contain implicit lineto commands)
		if (cmd == prevCmd or (cmd == 'l' and prevCmd == 'm')) and cmd != 'm':
			prevData.extend(data)
		
		# save last command and data
		else:
			prevCmd = cmd
			prevData = data
	# flush last command and data
	if prevCmd != '':
		newPath.append( (prevCmd, prevData) )
	path = newPath

	# convert to shorthand path segments where possible
	newPath = []
	for (cmd,data) in path:
		# convert line segments into h,v where possible
		if cmd == 'l':
			i = 0
			lineTuples = []
			while i < len(data):
				if data[i] == 0:
					# vertical
					if lineTuples:
						# flush the existing line command
						newPath.append( ('l', lineTuples) )
						lineTuples = []
					# append the v and then the remaining line coords						
					newPath.append( ('v', [data[i+1]]) )
					numPathSegmentsReduced += 1
				elif data[i+1] == 0:
					if lineTuples:
						# flush the line command, then append the h and then the remaining line coords
						newPath.append( ('l', lineTuples) )
						lineTuples = []
					newPath.append( ('h', [data[i]]) )
					numPathSegmentsReduced += 1
				else:
					lineTuples.extend(data[i:i+2])
				i += 2
			if lineTuples:
				newPath.append( ('l', lineTuples) )
		# also handle implied relative linetos
		elif cmd == 'm':
			i = 2
			lineTuples = [data[0], data[1]]
			while i < len(data):
				if data[i] == 0:
					# vertical
					if lineTuples:
						# flush the existing m/l command
						newPath.append( (cmd, lineTuples) )
						lineTuples = []
						cmd = 'l' # dealing with linetos now
					# append the v and then the remaining line coords						
					newPath.append( ('v', [data[i+1]]) )
					numPathSegmentsReduced += 1
				elif data[i+1] == 0:
					if lineTuples:
						# flush the m/l command, then append the h and then the remaining line coords
						newPath.append( (cmd, lineTuples) )
						lineTuples = []
						cmd = 'l' # dealing with linetos now
					newPath.append( ('h', [data[i]]) )
					numPathSegmentsReduced += 1
				else:
					lineTuples.extend(data[i:i+2])
				i += 2
			if lineTuples:
				newPath.append( (cmd, lineTuples) )
		# convert Bézier curve segments into s where possible	
		elif cmd == 'c':
			bez_ctl_pt = (0,0)
			i = 0
			curveTuples = []
			while i < len(data):
				# rotate by 180deg means negate both coordinates
				# if the previous control point is equal then we can substitute a
				# shorthand bezier command
				if bez_ctl_pt[0] == data[i] and bez_ctl_pt[1] == data[i+1]:
					if curveTuples:
						newPath.append( ('c', curveTuples) )
						curveTuples = []
					# append the s command
					newPath.append( ('s', [data[i+2], data[i+3], data[i+4], data[i+5]]) )
					numPathSegmentsReduced += 1
				else:
					j = 0
					while j <= 5:
						curveTuples.append(data[i+j])
						j += 1
				
				# set up control point for next curve segment
				bez_ctl_pt = (data[i+4]-data[i+2], data[i+5]-data[i+3])
				i += 6
				
			if curveTuples:
				newPath.append( ('c', curveTuples) )
		# convert quadratic curve segments into t where possible	
		elif cmd == 'q':
			quad_ctl_pt = (0,0)
			i = 0
			curveTuples = []
			while i < len(data):
				if quad_ctl_pt[0] == data[i] and quad_ctl_pt[1] == data[i+1]:
					if curveTuples:
						newPath.append( ('q', curveTuples) )
						curveTuples = []
					# append the t command
					newPath.append( ('t', [data[i+2], data[i+3]]) )
					numPathSegmentsReduced += 1
				else:
					j = 0;
					while j <= 3:
						curveTuples.append(data[i+j])
						j += 1
				
				quad_ctl_pt = (data[i+2]-data[i], data[i+3]-data[i+1])
				i += 4
				
			if curveTuples:
				newPath.append( ('q', curveTuples) )
		else:
			newPath.append( (cmd, data) )
	path = newPath
		
	# for each h or v, collapse unnecessary coordinates that run in the same direction
	# i.e. "h-100-100" becomes "h-200" but "h300-100" does not change
	# Reuse the data structure 'path', since we're not adding or removing subcommands.
	# Also reuse the coordinate lists, even if we're deleting items, because these
	# deletions are relatively cheap.
	for pathIndex in xrange(1, len(path)):
		cmd, data = path[pathIndex]
		if cmd in ['h','v'] and len(data) > 1:
			coordIndex = 1
			while coordIndex < len(data):
				if isSameSign(data[coordIndex - 1], data[coordIndex]):
					data[coordIndex - 1] += data[coordIndex]
					del data[coordIndex]
					numPathSegmentsReduced += 1
				else:
					coordIndex += 1
	
	# it is possible that we have consecutive h, v, c, t commands now
	# so again collapse all consecutive commands of the same type into one command
	prevCmd = ''
	prevData = []
	newPath = [path[0]]
	for (cmd,data) in path[1:]:
		# flush the previous command if it is not the same type as the current command
		if prevCmd != '':
			if cmd != prevCmd or cmd == 'm':
				newPath.append( (prevCmd, prevData) )
				prevCmd = ''
				prevData = []
		
		# if the previous and current commands are the same type, collapse
		if cmd == prevCmd and cmd != 'm':
				prevData.extend(data)
		
		# save last command and data
		else:
			prevCmd = cmd
			prevData = data
	# flush last command and data
	if prevCmd != '':
		newPath.append( (prevCmd, prevData) )
	path = newPath
	
	newPathStr = serializePath(path, options)
	numBytesSavedInPathData += ( len(oldPathStr) - len(newPathStr) )
	element.setAttribute('d', newPathStr)

def parseListOfPoints(s):
	"""
		Parse string into a list of points.
	
		Returns a list of containing an even number of coordinate strings
	"""
	i = 0
	
	# (wsp)? comma-or-wsp-separated coordinate pairs (wsp)?
	# coordinate-pair = coordinate comma-or-wsp coordinate
	# coordinate = sign? integer
	# comma-wsp: (wsp+ comma? wsp*) | (comma wsp*)
	ws_nums = re.split(r"\s*,?\s*", s.strip())
	nums = []
	
	# also, if 100-100 is found, split it into two also
	#  <polygon points="100,-100,100-100,100-100-100,-100-100" />
	for i in xrange(len(ws_nums)):
		negcoords = ws_nums[i].split("-")
		
		# this string didn't have any negative coordinates
		if len(negcoords) == 1:
			nums.append(negcoords[0])
		# we got negative coords
		else:
			for j in xrange(len(negcoords)):
				if j == 0:
					# first number could be positive
					if negcoords[0] != '':
						nums.append(negcoords[0])
				# but it could also be negative
				elif len(nums) == 0:
					nums.append('-' + negcoords[j])
				# otherwise all other strings will be negative
				else:
					# unless we accidentally split a number that was in scientific notation
					# and had a negative exponent (500.00e-1)
					prev = nums[len(nums)-1]
					if prev[len(prev)-1] in ['e', 'E']:
						nums[len(nums)-1] = prev + '-' + negcoords[j]
					else:
						nums.append( '-'+negcoords[j] )

	# if we have an odd number of points, return empty
	if len(nums) % 2 != 0: return []
		
	# now resolve into Decimal values
	i = 0
	while i < len(nums):
		try:
			nums[i] = getcontext().create_decimal(nums[i])
			nums[i + 1] = getcontext().create_decimal(nums[i + 1])
		except decimal.InvalidOperation: # one of the lengths had a unit or is an invalid number
			return []
		
		i += 2

	return nums
	
def cleanPolygon(elem, options):
	"""
		Remove unnecessary closing point of polygon points attribute
	"""
	global numPointsRemovedFromPolygon
	
	pts = parseListOfPoints(elem.getAttribute('points'))
	N = len(pts)/2
	if N >= 2:		
		(startx,starty) = pts[:2]
		(endx,endy) = pts[-2:]
		if startx == endx and starty == endy:
			del pts[-2:]
			numPointsRemovedFromPolygon += 1
	elem.setAttribute('points', scourCoordinates(pts, options, True))

def cleanPolyline(elem, options):
	"""
		Scour the polyline points attribute
	"""
	pts = parseListOfPoints(elem.getAttribute('points'))		
	elem.setAttribute('points', scourCoordinates(pts, options, True))

def serializePath(pathObj, options):
	"""
		Reserializes the path data with some cleanups.
	"""
	# elliptical arc commands must have comma/wsp separating the coordinates
	# this fixes an issue outlined in Fix https://bugs.launchpad.net/scour/+bug/412754
	return ''.join([cmd + scourCoordinates(data, options, (cmd == 'a'))  for cmd, data  in pathObj])

def serializeTransform(transformObj):
	"""
		Reserializes the transform data with some cleanups.
	"""
	return ' '.join(
		[command + '(' + ' '.join(
			[scourUnitlessLength(number) for number in numbers]
		) + ')'
		for command, numbers in transformObj]
	)

def scourCoordinates(data, options, forceCommaWsp = False):
	"""
		Serializes coordinate data with some cleanups:
			- removes all trailing zeros after the decimal
			- integerize coordinates if possible
			- removes extraneous whitespace
			- adds spaces between values in a subcommand if required (or if forceCommaWsp is True)
	"""
	if data != None:
		newData = []
		c = 0
		previousCoord = ''
		for coord in data:
			scouredCoord = scourUnitlessLength(coord, needsRendererWorkaround=options.renderer_workaround)
			# only need the comma if the current number starts with a digit
			# (numbers can start with - without needing a comma before)
			# or if forceCommaWsp is True
			# or if this number starts with a dot and the previous number
			#   had *no* dot or exponent (so we can go like -5.5.5 for -5.5,0.5
			#   and 4e4.5 for 40000,0.5)
			if c > 0 and (forceCommaWsp
				or scouredCoord[0].isdigit()
				or (scouredCoord[0] == '.' and not ('.' in previousCoord or 'e' in previousCoord))
				):
				newData.append( ' ' )
				
			# add the scoured coordinate to the path string
			newData.append( scouredCoord )
			previousCoord = scouredCoord
			c += 1
	
		# What we need to do to work around GNOME bugs 548494, 563933 and
		# 620565, which are being fixed and unfixed in Ubuntu, is
		# to make sure that a dot doesn't immediately follow a command
		# (so 'h50' and 'h0.5' are allowed, but not 'h.5').
		# Then, we need to add a space character after any coordinates
		# having an 'e' (scientific notation), so as to have the exponent
		# separate from the next number.
		if options.renderer_workaround:
			if len(newData) > 0:
				for i in xrange(1, len(newData)):
					if newData[i][0] == '-' and 'e' in newData[i - 1]:
						newData[i - 1] += ' '
				return ''.join(newData)
		else:
			return ''.join(newData)
	
	return ''

def scourLength(length):
	"""
	Scours a length. Accepts units.
	"""
	length = SVGLength(length)
	
	return scourUnitlessLength(length.value) + Unit.str(length.units)

def scourUnitlessLength(length, needsRendererWorkaround=False): # length is of a numeric type
	"""
	Scours the numeric part of a length only. Does not accept units.
	
	This is faster than scourLength on elements guaranteed not to
	contain units.
	"""
	# reduce to the proper number of digits
	if not isinstance(length, Decimal):
		length = getcontext().create_decimal(str(length))
	# if the value is an integer, it may still have .0[...] attached to it for some reason
	# remove those
	if int(length) == length:
		length = getcontext().create_decimal(int(length))
	
	# gather the non-scientific notation version of the coordinate.
	# this may actually be in scientific notation if the value is
	# sufficiently large or small, so this is a misnomer.
	nonsci = unicode(length).lower().replace("e+", "e")
	if not needsRendererWorkaround:
		if len(nonsci) > 2 and nonsci[:2] == '0.':
			nonsci = nonsci[1:] # remove the 0, leave the dot
		elif len(nonsci) > 3 and nonsci[:3] == '-0.':
			nonsci = '-' + nonsci[2:] # remove the 0, leave the minus and dot
	
	if len(nonsci) > 3: # avoid calling normalize unless strictly necessary
		# and then the scientific notation version, with E+NUMBER replaced with
		# just eNUMBER, since SVG accepts this.
		sci = unicode(length.normalize()).lower().replace("e+", "e")
	
		if len(sci) < len(nonsci): return sci
		else: return nonsci
	else: return nonsci

def reducePrecision(element) :
	"""
	Because opacities, letter spacings, stroke widths and all that don't need
	to be preserved in SVG files with 9 digits of precision.
	
	Takes all of these attributes, in the given element node and its children,
	and reduces their precision to the current Decimal context's precision.
	Also checks for the attributes actually being lengths, not 'inherit', 'none'
	or anything that isn't an SVGLength.
	
	Returns the number of bytes saved after performing these reductions.
	"""
	num = 0
	
	styles = _getStyle(element)
	for lengthAttr in ['opacity', 'flood-opacity', 'fill-opacity', 
						'stroke-opacity', 'stop-opacity', 'stroke-miterlimit',
						'stroke-dashoffset', 'letter-spacing', 'word-spacing',
						'kerning', 'font-size-adjust', 'font-size',
						'stroke-width']:
		val = element.getAttribute(lengthAttr)
		if val != '':
			valLen = SVGLength(val)
			if valLen.units != Unit.INVALID: # not an absolute/relative size or inherit, can be % though
				newVal = scourLength(val)
				if len(newVal) < len(val):
					num += len(val) - len(newVal)
					element.setAttribute(lengthAttr, newVal)
		# repeat for attributes hidden in styles
		if lengthAttr in styles.keys():
			val = styles[lengthAttr]
			valLen = SVGLength(val)
			if valLen.units != Unit.INVALID:
				newVal = scourLength(val)
				if len(newVal) < len(val):
					num += len(val) - len(newVal)
					styles[lengthAttr] = newVal
	_setStyle(element, styles)
	
	for child in element.childNodes:
		if child.nodeType == 1:
			num += reducePrecision(child)
	
	return num

def optimizeAngle(angle):
	"""
	Because any rotation can be expressed within 360 degrees
	of any given number, and since negative angles sometimes
	are one character longer than corresponding positive angle,
	we shorten the number to one in the range to [-90, 270[.
	"""
	# First, we put the new angle in the range ]-360, 360[.
	# The modulo operator yields results with the sign of the
	# divisor, so for negative dividends, we preserve the sign
	# of the angle.
	if angle < 0:     angle %= -360
	else:             angle %=  360
	# 720 degrees is unneccessary, as 360 covers all angles.
	# As "-x" is shorter than "35x" and "-xxx" one character
	# longer than positive angles <= 260, we constrain angle
	# range to [-90, 270[ (or, equally valid: ]-100, 260]).
	if  angle >= 270: angle -=  360
	elif angle < -90: angle +=  360
	return angle


def optimizeTransform(transform):
	"""
	Optimises a series of transformations parsed from a single
	transform="" attribute.
	
	The transformation list is modified in-place.
	"""
	# FIXME: reordering these would optimize even more cases:
	#	 first: Fold consecutive runs of the same transformation
	#	 extra:	Attempt to cast between types to create sameness:
	#		"matrix(0 1 -1 0 0 0) rotate(180) scale(-1)" all
	#		are rotations (90, 180, 180) -- thus "rotate(90)"
	#	second: Simplify transforms where numbers are optional.
	#	 third: Attempt to simplify any single remaining matrix()
	#
	# if there's only one transformation and it's a matrix,
	# try to make it a shorter non-matrix transformation
	# NOTE: as matrix(a b c d e f) in SVG means the matrix:
	# |¯  a  c  e  ¯|   make constants   |¯  A1  A2  A3  ¯|
	# |   b  d  f   |  translating them  |   B1  B2  B3   |
	# |_  0  0  1  _|  to more readable  |_  0    0   1  _|
	if len(transform) == 1 and transform[0][0] == 'matrix':
		matrix = A1, B1, A2, B2, A3, B3 = transform[0][1]
		# |¯  1  0  0  ¯|
		# |   0  1  0   |  Identity matrix (no transformation)
		# |_  0  0  1  _|
		if matrix == [1, 0, 0, 1, 0, 0]:
			del transform[0]
		# |¯  1  0  X  ¯|
		# |   0  1  Y   |  Translation by (X, Y).
		# |_  0  0  1  _|
		elif (A1 ==  1 and A2 ==  0
		 and  B1 ==  0 and B2 ==  1):
			transform[0] = ('translate', [A3, B3])
		# |¯  X  0  0  ¯|
		# |   0  Y  0   |  Scaling by (X, Y).
		# |_  0  0  1  _|
		elif (             A2 ==  0 and A3 ==  0
		 and  B1 ==  0 and              B3 ==  0):
			transform[0] = ('scale', [A1, B2])
		# |¯  cos(A) -sin(A)    0    ¯|  Rotation by angle A,
		# |   sin(A)  cos(A)    0     |  clockwise, about the origin.
		# |_    0       0       1    _|  A is in degrees, [-180...180].
		elif (A1 == B2 and -1 <= A1 <= 1 and A3 == 0
		 and -B1 == A2 and -1 <= B1 <= 1 and B3 == 0
		 # as cos² A + sin² A == 1 and as decimal trig is approximate:
		 # FIXME: the "epsilon" term here should really be some function
		 #        of the precision of the (sin|cos)_A terms, not 1e-15:
		 and  abs((B1 ** 2) + (A1 ** 2) - 1) < Decimal("1e-15")):
			sin_A, cos_A = B1, A1
			# while asin(A) and acos(A) both only have an 180° range
			# the sign of sin(A) and cos(A) varies across quadrants,
			# letting us hone in on the angle the matrix represents:
			# -- => < -90 | -+ => -90..0 | ++ => 0..90 | +- => >= 90
			#
			# http://en.wikipedia.org/wiki/File:Sine_cosine_plot.svg
			# shows asin has the correct angle the middle quadrants:
			A = Decimal(str(math.degrees(math.asin(float(sin_A)))))
			if cos_A < 0: # otherwise needs adjusting from the edges
				if sin_A < 0:
					A = -180 - A
				else:
					A =  180 - A
			transform[0] = ('rotate', [A])
	
	# Simplify transformations where numbers are optional.
	for type, args in transform:
		if type == 'translate':
			# Only the X coordinate is required for translations.
			# If the Y coordinate is unspecified, it's 0.
			if len(args) == 2 and args[1] == 0:
				del args[1]
		elif type == 'rotate':
			args[0] = optimizeAngle(args[0]) # angle
			# Only the angle is required for rotations.
			# If the coordinates are unspecified, it's the origin (0, 0).
			if len(args) == 3 and args[1] == args[2] == 0:
				del args[1:]
		elif type == 'scale':
			# Only the X scaling factor is required.
			# If the Y factor is unspecified, it's the same as X.
			if len(args) == 2 and args[0] == args[1]:
				del args[1]

	# Attempt to coalesce runs of the same transformation.
	# Translations followed immediately by other translations,
	# rotations followed immediately by other rotations,
	# scaling followed immediately by other scaling,
	# are safe to add.
	# Identity skewX/skewY are safe to remove, but how do they accrete?
	# |¯    1     0    0    ¯|
	# |   tan(A)  1    0     |  skews X coordinates by angle A
	# |_    0     0    1    _|
	#
	# |¯    1  tan(A)  0    ¯|
	# |     0     1    0     |  skews Y coordinates by angle A
	# |_    0     0    1    _|
	#
	# FIXME: A matrix followed immediately by another matrix
	#	 would be safe to multiply together, too.
	i = 1
	while i < len(transform):
		currType, currArgs = transform[i]
		prevType, prevArgs = transform[i - 1]
		if currType == prevType == 'translate':
			prevArgs[0] += currArgs[0] # x
			# for y, only add if the second translation has an explicit y
			if len(currArgs) == 2:
				if len(prevArgs) == 2:
					prevArgs[1] += currArgs[1] # y
				elif len(prevArgs) == 1:
					prevArgs.append(currArgs[1]) # y
			del transform[i]
			if prevArgs[0] == prevArgs[1] == 0:
				# Identity translation!
				i -= 1
				del transform[i]
		elif (currType == prevType == 'rotate'
		      and len(prevArgs) == len(currArgs) == 1):
			# Only coalesce if both rotations are from the origin.
			prevArgs[0] = optimizeAngle(prevArgs[0] + currArgs[0])
			del transform[i]
		elif currType == prevType == 'scale':
			prevArgs[0] *= currArgs[0] # x
			# handle an implicit y
			if len(prevArgs) == 2 and len(currArgs) == 2:
				# y1 * y2
				prevArgs[1] *= currArgs[1]
			elif len(prevArgs) == 1 and len(currArgs) == 2:
				# create y2 = uniformscalefactor1 * y2
				prevArgs.append(prevArgs[0] * currArgs[1])
			elif len(prevArgs) == 2 and len(currArgs) == 1:
				# y1 * uniformscalefactor2
				prevArgs[1] *= currArgs[0]
			del transform[i]
			if prevArgs[0] == prevArgs[1] == 1:
				# Identity scale!
				i -= 1
				del transform[i]
		else:
			i += 1

	# Some fixups are needed for single-element transformation lists, since
	# the loop above was to coalesce elements with their predecessors in the
	# list, and thus it required 2 elements.
	i = 0
	while i < len(transform):
		currType, currArgs = transform[i]
		if ((currType == 'skewX' or currType == 'skewY')
		  and len(currArgs) == 1 and currArgs[0] == 0):
			# Identity skew!
			del transform[i]
		elif ((currType == 'rotate')
		  and len(currArgs) == 1 and currArgs[0] == 0):
			# Identity rotation!
			del transform[i]
		else:
			i += 1

def optimizeTransforms(element, options) :
	"""
	Attempts to optimise transform specifications on the given node and its children.
	
	Returns the number of bytes saved after performing these reductions.
	"""
	num = 0
	
	for transformAttr in ['transform', 'patternTransform', 'gradientTransform']:
		val = element.getAttribute(transformAttr)
		if val != '':
			transform = svg_transform_parser.parse(val)
			
			optimizeTransform(transform)
			
			newVal = serializeTransform(transform)
			
			if len(newVal) < len(val):
				if len(newVal):
					element.setAttribute(transformAttr, newVal)
				else:
					element.removeAttribute(transformAttr)
				num += len(val) - len(newVal)
	
	for child in element.childNodes:
		if child.nodeType == 1:
			num += optimizeTransforms(child, options)
	
	return num

def removeComments(element) :
	"""
		Removes comments from the element and its children.
	"""
	global numCommentBytes

	if isinstance(element, xml.dom.minidom.Document):
		# must process the document object separately, because its
		# documentElement's nodes have None as their parentNode
		# iterate in reverse order to prevent mess-ups with renumbering
		for index in xrange(len(element.childNodes) - 1, -1, -1):
			subelement = element.childNodes[index]
			if isinstance(subelement, xml.dom.minidom.Comment):
				numCommentBytes += len(subelement.data)
				element.removeChild(subelement)
			else:
				removeComments(subelement)
	elif isinstance(element, xml.dom.minidom.Comment):
		numCommentBytes += len(element.data)
		element.parentNode.removeChild(element)
	else:
		# iterate in reverse order to prevent mess-ups with renumbering
		for index in xrange(len(element.childNodes) - 1, -1, -1):
			subelement = element.childNodes[index]
			removeComments(subelement)

def embedRasters(element, options) :
	import base64
	import urllib
	"""
		Converts raster references to inline images.
		NOTE: there are size limits to base64-encoding handling in browsers 
	"""
	global numRastersEmbedded

	href = element.getAttributeNS(NS['XLINK'],'href')
	
	# if xlink:href is set, then grab the id
	if href != '' and len(href) > 1:
		# find if href value has filename ext		
		ext = os.path.splitext(os.path.basename(href))[1].lower()[1:]
				
		# look for 'png', 'jpg', and 'gif' extensions 
		if ext == 'png' or ext == 'jpg' or ext == 'gif':

			# file:// URLs denote files on the local system too
			if href[:7] == 'file://':
				href = href[7:]
			# does the file exist?
			if os.path.isfile(href):
				# if this is not an absolute path, set path relative
				# to script file based on input arg 
				infilename = '.'
				if options.infilename: infilename = options.infilename
				href = os.path.join(os.path.dirname(infilename), href)

			rasterdata = ''
			# test if file exists locally
			if os.path.isfile(href):
				# open raster file as raw binary
				raster = open( href, "rb")
				rasterdata = raster.read()
			elif href[:7] == 'http://':
				webFile = urllib.urlopen( href )
				rasterdata = webFile.read()
				webFile.close()
			
			# ... should we remove all images which don't resolve?	
			if rasterdata != '' :
				# base64-encode raster
				b64eRaster = base64.b64encode( rasterdata )

				# set href attribute to base64-encoded equivalent
				if b64eRaster != '':
					# PNG and GIF both have MIME Type 'image/[ext]', but 
					# JPEG has MIME Type 'image/jpeg'
					if ext == 'jpg':
						ext = 'jpeg'

					element.setAttributeNS(NS['XLINK'], 'href', 'data:image/' + ext + ';base64,' + b64eRaster)
					numRastersEmbedded += 1
					del b64eRaster				

def properlySizeDoc(docElement, options):
	# get doc width and height
	w = SVGLength(docElement.getAttribute('width'))
	h = SVGLength(docElement.getAttribute('height'))

	# if width/height are not unitless or px then it is not ok to rewrite them into a viewBox.
	# well, it may be OK for Web browsers and vector editors, but not for librsvg.
	if options.renderer_workaround:
		if ((w.units != Unit.NONE and w.units != Unit.PX) or
			(h.units != Unit.NONE and h.units != Unit.PX)):
		    return

	# else we have a statically sized image and we should try to remedy that	

	# parse viewBox attribute
	vbSep = re.split("\\s*\\,?\\s*", docElement.getAttribute('viewBox'), 3)
	# if we have a valid viewBox we need to check it
	vbWidth,vbHeight = 0,0
	if len(vbSep) == 4:
		try:
			# if x or y are specified and non-zero then it is not ok to overwrite it
			vbX = float(vbSep[0])
			vbY = float(vbSep[1])
			if vbX != 0 or vbY != 0:
				return
				
			# if width or height are not equal to doc width/height then it is not ok to overwrite it
			vbWidth = float(vbSep[2])
			vbHeight = float(vbSep[3])
			if vbWidth != w.value or vbHeight != h.value:
				return
		# if the viewBox did not parse properly it is invalid and ok to overwrite it
		except ValueError:
			pass
	
	# at this point it's safe to set the viewBox and remove width/height
	docElement.setAttribute('viewBox', '0 0 %s %s' % (w.value, h.value))
	docElement.removeAttribute('width')
	docElement.removeAttribute('height')

def remapNamespacePrefix(node, oldprefix, newprefix):
	if node == None or node.nodeType != 1: return
	
	if node.prefix == oldprefix:
		localName = node.localName
		namespace = node.namespaceURI
		doc = node.ownerDocument
		parent = node.parentNode
	
		# create a replacement node
		newNode = None
		if newprefix != '':
			newNode = doc.createElementNS(namespace, newprefix+":"+localName)
		else:
			newNode = doc.createElement(localName);
			
		# add all the attributes
		attrList = node.attributes
		for i in xrange(attrList.length):
			attr = attrList.item(i)
			newNode.setAttributeNS( attr.namespaceURI, attr.localName, attr.nodeValue)
	
		# clone and add all the child nodes
		for child in node.childNodes:
			newNode.appendChild(child.cloneNode(True))
			
		# replace old node with new node
		parent.replaceChild( newNode, node )
		# set the node to the new node in the remapped namespace prefix
		node = newNode
	
	# now do all child nodes
	for child in node.childNodes :
		remapNamespacePrefix(child, oldprefix, newprefix)	

def makeWellFormed(str):
	xml_ents = { '<':'&lt;', '>':'&gt;', '&':'&amp;', "'":'&apos;', '"':'&quot;'}
	
#	starr = []
#	for c in str:
#		if c in xml_ents:
#			starr.append(xml_ents[c])
#		else:
#			starr.append(c)
			
	# this list comprehension is short-form for the above for-loop:
	return ''.join([xml_ents[c] if c in xml_ents else c for c in str])

# hand-rolled serialization function that has the following benefits:
# - pretty printing
# - somewhat judicious use of whitespace
# - ensure id attributes are first
def serializeXML(element, options, ind = 0, preserveWhitespace = False):
	outParts = []

	indent = ind
	I=''
	if options.indent_type == 'tab': I='\t'
	elif options.indent_type == 'space': I=' '
	
	outParts.extend([(I * ind), '<', element.nodeName])

	# always serialize the id or xml:id attributes first
	if element.getAttribute('id') != '':
		id = element.getAttribute('id')
		quot = '"'
		if id.find('"') != -1:
			quot = "'"
		outParts.extend([' id=', quot, id, quot])
	if element.getAttribute('xml:id') != '':
		id = element.getAttribute('xml:id')
		quot = '"'
		if id.find('"') != -1:
			quot = "'"
		outParts.extend([' xml:id=', quot, id, quot])
	
	# now serialize the other attributes
	attrList = element.attributes
	for num in xrange(attrList.length) :
		attr = attrList.item(num)
		if attr.nodeName == 'id' or attr.nodeName == 'xml:id': continue
		# if the attribute value contains a double-quote, use single-quotes
		quot = '"'
		if attr.nodeValue.find('"') != -1:
			quot = "'"

		attrValue = makeWellFormed( attr.nodeValue )
		
		outParts.append(' ')
		# preserve xmlns: if it is a namespace prefix declaration
		if attr.prefix != None:
			outParts.extend([attr.prefix, ':'])
		elif attr.namespaceURI != None:
			if attr.namespaceURI == 'http://www.w3.org/2000/xmlns/' and attr.nodeName.find('xmlns') == -1:
				outParts.append('xmlns:')
			elif attr.namespaceURI == 'http://www.w3.org/1999/xlink':
				outParts.append('xlink:')
		outParts.extend([attr.localName, '=', quot, attrValue, quot])

		if attr.nodeName == 'xml:space':
			if attrValue == 'preserve':
				preserveWhitespace = True
			elif attrValue == 'default':
				preserveWhitespace = False
	
	# if no children, self-close
	children = element.childNodes
	if children.length > 0:
		outParts.append('>')
	
		onNewLine = False
		for child in element.childNodes:
			# element node
			if child.nodeType == 1:
				if preserveWhitespace:
					outParts.append(serializeXML(child, options, 0, preserveWhitespace))
				else:
					outParts.extend(['\n', serializeXML(child, options, indent + 1, preserveWhitespace)])
					onNewLine = True
			# text node
			elif child.nodeType == 3:
				# trim it only in the case of not being a child of an element
				# where whitespace might be important
				if preserveWhitespace:
					outParts.append(makeWellFormed(child.nodeValue))
				else:
					outParts.append(makeWellFormed(child.nodeValue.strip()))
			# CDATA node
			elif child.nodeType == 4:
				outParts.extend(['<![CDATA[', child.nodeValue, ']]>'])
			# Comment node
			elif child.nodeType == 8:
				outParts.extend(['<!--', child.nodeValue, '-->'])
			# TODO: entities, processing instructions, what else?
			else: # ignore the rest
				pass
				
		if onNewLine: outParts.append(I * ind)
		outParts.extend(['</', element.nodeName, '>'])
		if indent > 0: outParts.append('\n')
	else:
		outParts.append('/>')
		if indent > 0: outParts.append('\n')
		
	return "".join(outParts)
	
# this is the main method
# input is a string representation of the input XML
# returns a string representation of the output XML
def scourString(in_string, options=None):
	if options is None:
		options = _options_parser.get_default_values()
	getcontext().prec = options.digits
	global numAttrsRemoved
	global numStylePropsFixed
	global numElemsRemoved
	global numBytesSavedInColors
	global numCommentsRemoved
	global numBytesSavedInIDs
	global numBytesSavedInLengths
	global numBytesSavedInTransforms
	doc = xml.dom.minidom.parseString(in_string)

	# for whatever reason this does not always remove all inkscape/sodipodi attributes/elements
	# on the first pass, so we do it multiple times
	# does it have to do with removal of children affecting the childlist?
	if options.keep_editor_data == False:
		while removeNamespacedElements( doc.documentElement, unwanted_ns ) > 0 :
			pass	
		while removeNamespacedAttributes( doc.documentElement, unwanted_ns ) > 0 :
			pass
		
		# remove the xmlns: declarations now
		xmlnsDeclsToRemove = []
		attrList = doc.documentElement.attributes
		for num in xrange(attrList.length) :
			if attrList.item(num).nodeValue in unwanted_ns :
				xmlnsDeclsToRemove.append(attrList.item(num).nodeName)
		
		for attr in xmlnsDeclsToRemove :
			doc.documentElement.removeAttribute(attr)
			numAttrsRemoved += 1

	# ensure namespace for SVG is declared
	# TODO: what if the default namespace is something else (i.e. some valid namespace)?
	if doc.documentElement.getAttribute('xmlns') != 'http://www.w3.org/2000/svg':
		doc.documentElement.setAttribute('xmlns', 'http://www.w3.org/2000/svg')
		# TODO: throw error or warning?
	
	# check for redundant SVG namespace declaration
	attrList = doc.documentElement.attributes
	xmlnsDeclsToRemove = []
	redundantPrefixes = []
	for i in xrange(attrList.length):
		attr = attrList.item(i)
		name = attr.nodeName
		val = attr.nodeValue
		if name[0:6] == 'xmlns:' and val == 'http://www.w3.org/2000/svg':
			redundantPrefixes.append(name[6:])
			xmlnsDeclsToRemove.append(name)
			
	for attrName in xmlnsDeclsToRemove:
		doc.documentElement.removeAttribute(attrName)
	
	for prefix in redundantPrefixes:
		remapNamespacePrefix(doc.documentElement, prefix, '')

	if options.strip_comments:
		numCommentsRemoved = removeComments(doc)

	# repair style (remove unnecessary style properties and change them into XML attributes)
	numStylePropsFixed = repairStyle(doc.documentElement, options)

	# convert colors to #RRGGBB format
	if options.simple_colors:
		numBytesSavedInColors = convertColors(doc.documentElement)
	
	# remove <metadata> if the user wants to
	if options.remove_metadata:
		removeMetadataElements(doc)
	
	# flattend defs elements into just one defs element
	flattenDefs(doc)
	
	# remove unreferenced gradients/patterns outside of defs
	# and most unreferenced elements inside of defs
	while removeUnreferencedElements(doc) > 0:
		pass

	# remove empty defs, metadata, g
	# NOTE: these elements will be removed if they just have whitespace-only text nodes
	for tag in ['defs', 'metadata', 'g'] :
		for elem in doc.documentElement.getElementsByTagName(tag) :
			removeElem = not elem.hasChildNodes()
			if removeElem == False :
				for child in elem.childNodes :
					if child.nodeType in [1, 4, 8]:
						break
					elif child.nodeType == 3 and not child.nodeValue.isspace():
						break
				else:
					removeElem = True
			if removeElem :
				elem.parentNode.removeChild(elem)
				numElemsRemoved += 1

	if options.strip_ids:
		bContinueLooping = True
		while bContinueLooping:
			identifiedElements = unprotected_ids(doc, options)
			referencedIDs = findReferencedElements(doc.documentElement)
			bContinueLooping = (removeUnreferencedIDs(referencedIDs, identifiedElements) > 0)

	while removeDuplicateGradientStops(doc) > 0:
		pass
	
	# remove gradients that are only referenced by one other gradient
	while collapseSinglyReferencedGradients(doc) > 0:
		pass
		
	# remove duplicate gradients
	while removeDuplicateGradients(doc) > 0:
		pass
	
	# create <g> elements if there are runs of elements with the same attributes.
	# this MUST be before moveCommonAttributesToParentGroup.
	if options.group_create:
		createGroupsForCommonAttributes(doc.documentElement)
	
	# move common attributes to parent group
	# NOTE: the if the <svg> element's immediate children
	# all have the same value for an attribute, it must not
	# get moved to the <svg> element. The <svg> element
	# doesn't accept fill=, stroke= etc.!
	referencedIds = findReferencedElements(doc.documentElement)
	for child in doc.documentElement.childNodes:
		numAttrsRemoved += moveCommonAttributesToParentGroup(child, referencedIds)
	
	# remove unused attributes from parent
	numAttrsRemoved += removeUnusedAttributesOnParent(doc.documentElement)
	
	# Collapse groups LAST, because we've created groups. If done before
	# moveAttributesToParentGroup, empty <g>'s may remain.
	if options.group_collapse:
		while removeNestedGroups(doc.documentElement) > 0:
			pass

	# remove unnecessary closing point of polygons and scour points
	for polygon in doc.documentElement.getElementsByTagName('polygon') :
		cleanPolygon(polygon, options)

	# scour points of polyline
	for polyline in doc.documentElement.getElementsByTagName('polyline') :
		cleanPolyline(polyline, options)

	# clean path data
	for elem in doc.documentElement.getElementsByTagName('path') :
		if elem.getAttribute('d') == '':
			elem.parentNode.removeChild(elem)
		else:
			cleanPath(elem, options)
	
	# shorten ID names as much as possible
	if options.shorten_ids:
		numBytesSavedInIDs += shortenIDs(doc, unprotected_ids(doc, options))

	# scour lengths (including coordinates)
	for type in ['svg', 'image', 'rect', 'circle', 'ellipse', 'line', 'linearGradient', 'radialGradient', 'stop', 'filter']:
		for elem in doc.getElementsByTagName(type):
			for attr in ['x', 'y', 'width', 'height', 'cx', 'cy', 'r', 'rx', 'ry', 
						'x1', 'y1', 'x2', 'y2', 'fx', 'fy', 'offset']:
				if elem.getAttribute(attr) != '':
					elem.setAttribute(attr, scourLength(elem.getAttribute(attr)))	
	
	# more length scouring in this function
	numBytesSavedInLengths = reducePrecision(doc.documentElement)
	
	# remove default values of attributes
	numAttrsRemoved += removeDefaultAttributeValues(doc.documentElement, options)	
	
	# reduce the length of transformation attributes
	numBytesSavedInTransforms = optimizeTransforms(doc.documentElement, options)
	
	# convert rasters references to base64-encoded strings 
	if options.embed_rasters:
		for elem in doc.documentElement.getElementsByTagName('image') :
			embedRasters(elem, options)		

	# properly size the SVG document (ideally width/height should be 100% with a viewBox)
	if options.enable_viewboxing:
		properlySizeDoc(doc.documentElement, options)

	# output the document as a pretty string with a single space for indent
	# NOTE: removed pretty printing because of this problem:
	# http://ronrothman.com/public/leftbraned/xml-dom-minidom-toprettyxml-and-silly-whitespace/
	# rolled our own serialize function here to save on space, put id first, customize indentation, etc
#	out_string = doc.documentElement.toprettyxml(' ')
	out_string = serializeXML(doc.documentElement, options) + '\n'
	
	# now strip out empty lines
	lines = []
	# Get rid of empty lines
	for line in out_string.splitlines(True):
		if line.strip():
			lines.append(line)

	# return the string with its XML prolog and surrounding comments
	if options.strip_xml_prolog == False:
		total_output = '<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n'
	else:
		total_output = ""
	
	for child in doc.childNodes:
		if child.nodeType == 1:
			total_output += "".join(lines)
		else: # doctypes, entities, comments
			total_output += child.toxml() + '\n'
		
	return total_output

# used mostly by unit tests
# input is a filename
# returns the minidom doc representation of the SVG
def scourXmlFile(filename, options=None):
	in_string = open(filename).read()
	out_string = scourString(in_string, options)
	return xml.dom.minidom.parseString(out_string.encode('utf-8'))

# GZ: Seems most other commandline tools don't do this, is it really wanted?
class HeaderedFormatter(optparse.IndentedHelpFormatter):
	"""
		Show application name, version number, and copyright statement
		above usage information.
	"""
	def format_usage(self, usage):
		return "%s %s\n%s\n%s" % (APP, VER, COPYRIGHT,
			optparse.IndentedHelpFormatter.format_usage(self, usage))

# GZ: would prefer this to be in a function or class scope, but tests etc need
#     access to the defaults anyway
_options_parser = optparse.OptionParser(
	usage="%prog [-i input.svg] [-o output.svg] [OPTIONS]",
	description=("If the input/output files are specified with a svgz"
	" extension, then compressed SVG is assumed. If the input file is not"
	" specified, stdin is used. If the output file is not specified, "
	" stdout is used."),
	formatter=HeaderedFormatter(max_help_position=30),
	version=VER)

_options_parser.add_option("--disable-simplify-colors",
	action="store_false", dest="simple_colors", default=True,
	help="won't convert all colors to #RRGGBB format")
_options_parser.add_option("--disable-style-to-xml",
	action="store_false", dest="style_to_xml", default=True,
	help="won't convert styles into XML attributes")
_options_parser.add_option("--disable-group-collapsing",
	action="store_false", dest="group_collapse", default=True,
	help="won't collapse <g> elements")
_options_parser.add_option("--create-groups",
	action="store_true", dest="group_create", default=False,
	help="create <g> elements for runs of elements with identical attributes")
_options_parser.add_option("--enable-id-stripping",
	action="store_true", dest="strip_ids", default=False,
	help="remove all un-referenced ID attributes")
_options_parser.add_option("--enable-comment-stripping",
	action="store_true", dest="strip_comments", default=False,
	help="remove all <!-- --> comments")
_options_parser.add_option("--shorten-ids",
	action="store_true", dest="shorten_ids", default=False,
	help="shorten all ID attributes to the least number of letters possible")
_options_parser.add_option("--disable-embed-rasters",
	action="store_false", dest="embed_rasters", default=True,
	help="won't embed rasters as base64-encoded data")
_options_parser.add_option("--keep-editor-data",
	action="store_true", dest="keep_editor_data", default=False,
	help="won't remove Inkscape, Sodipodi or Adobe Illustrator elements and attributes")
_options_parser.add_option("--remove-metadata",
	action="store_true", dest="remove_metadata", default=False,
	help="remove <metadata> elements (which may contain license metadata etc.)")
_options_parser.add_option("--renderer-workaround",
	action="store_true", dest="renderer_workaround", default=True,
	help="work around various renderer bugs (currently only librsvg) (default)")
_options_parser.add_option("--no-renderer-workaround",
	action="store_false", dest="renderer_workaround", default=True,
	help="do not work around various renderer bugs (currently only librsvg)")
_options_parser.add_option("--strip-xml-prolog",
	action="store_true", dest="strip_xml_prolog", default=False,
	help="won't output the <?xml ?> prolog")
_options_parser.add_option("--enable-viewboxing",
	action="store_true", dest="enable_viewboxing", default=False,
	help="changes document width/height to 100%/100% and creates viewbox coordinates")

# GZ: this is confusing, most people will be thinking in terms of
#     decimal places, which is not what decimal precision is doing
_options_parser.add_option("-p", "--set-precision",
	action="store", type=int, dest="digits", default=5,
	help="set number of significant digits (default: %default)")
_options_parser.add_option("-i",
	action="store", dest="infilename", help=optparse.SUPPRESS_HELP)
_options_parser.add_option("-o",
	action="store", dest="outfilename", help=optparse.SUPPRESS_HELP)
_options_parser.add_option("-q", "--quiet",
	action="store_true", dest="quiet", default=False,
	help="suppress non-error output")
_options_parser.add_option("--indent",
	action="store", type="string", dest="indent_type", default="space",
	help="indentation of the output: none, space, tab (default: %default)")
_options_parser.add_option("--protect-ids-noninkscape",
	action="store_true", dest="protect_ids_noninkscape", default=False,
	help="Don't change IDs not ending with a digit")
_options_parser.add_option("--protect-ids-list",
	action="store", type="string", dest="protect_ids_list", default=None,
	help="Don't change IDs given in a comma-separated list")
_options_parser.add_option("--protect-ids-prefix",
	action="store", type="string", dest="protect_ids_prefix", default=None,
	help="Don't change IDs starting with the given prefix")

def maybe_gziped_file(filename, mode="r"):
	if os.path.splitext(filename)[1].lower() in (".svgz", ".gz"):
		import gzip
		return gzip.GzipFile(filename, mode)
	return file(filename, mode)

def parse_args(args=None):
	options, rargs = _options_parser.parse_args(args)

	if rargs:
		_options_parser.error("Additional arguments not handled: %r, see --help" % rargs)
	if options.digits < 0:
		_options_parser.error("Can't have negative significant digits, see --help")
	if not options.indent_type in ["tab", "space", "none"]:
		_options_parser.error("Invalid value for --indent, see --help")
	if options.infilename and options.outfilename and options.infilename == options.outfilename:
		_options_parser.error("Input filename is the same as output filename")

	if options.infilename:
		infile = maybe_gziped_file(options.infilename)
		# GZ: could catch a raised IOError here and report
	else:
		# GZ: could sniff for gzip compression here
		infile = sys.stdin
	if options.outfilename:
		outfile = maybe_gziped_file(options.outfilename, "wb")
	else:
		outfile = sys.stdout
		
	return options, [infile, outfile]

def getReport():
	return ' Number of elements removed: ' + str(numElemsRemoved) + os.linesep + \
		' Number of attributes removed: ' + str(numAttrsRemoved) + os.linesep + \
		' Number of unreferenced id attributes removed: ' + str(numIDsRemoved) + os.linesep + \
		' Number of style properties fixed: ' + str(numStylePropsFixed) + os.linesep + \
		' Number of raster images embedded inline: ' + str(numRastersEmbedded) + os.linesep + \
		' Number of path segments reduced/removed: ' + str(numPathSegmentsReduced) + os.linesep + \
		' Number of bytes saved in path data: ' + str(numBytesSavedInPathData) + os.linesep + \
		' Number of bytes saved in colors: ' + str(numBytesSavedInColors) + os.linesep + \
		' Number of points removed from polygons: ' + str(numPointsRemovedFromPolygon) + os.linesep + \
		' Number of bytes saved in comments: ' + str(numCommentBytes) + os.linesep + \
		' Number of bytes saved in id attributes: ' + str(numBytesSavedInIDs) + os.linesep + \
		' Number of bytes saved in lengths: ' + str(numBytesSavedInLengths) + os.linesep + \
		' Number of bytes saved in transformations: ' + str(numBytesSavedInTransforms)

if __name__ == '__main__':
	if sys.platform == "win32":
		from time import clock as get_tick
	else:
		# GZ: is this different from time.time() in any way?
		def get_tick():
			return os.times()[0]

	start = get_tick()
	
	options, (input, output) = parse_args()
	
	if not options.quiet:
		print >>sys.stderr, "%s %s\n%s" % (APP, VER, COPYRIGHT)

	# do the work
	in_string = input.read()
	out_string = scourString(in_string, options).encode("UTF-8")
	output.write(out_string)

	# Close input and output files
	input.close()
	output.close()

	end = get_tick()

	# GZ: not using globals would be good too
	if not options.quiet:
		print >>sys.stderr, ' File:', input.name, \
			os.linesep + ' Time taken:', str(end-start) + 's' + os.linesep, \
			getReport()
	
		oldsize = len(in_string)
		newsize = len(out_string)
		sizediff = (newsize / oldsize) * 100
		print >>sys.stderr, ' Original file size:', oldsize, 'bytes;', \
			'new file size:', newsize, 'bytes (' + str(sizediff)[:5] + '%)'
