#!/usr/bin/env python
# -*- coding: utf-8 -*-

#  Scour
#
#  Copyright 2010 Jeff Schiller
#
#  This file is part of Scour, http://www.codedread.com/scour/
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
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
# - only remove unreferenced elements if they are not children of a referenced element
# - TODO: fix the removal of comment elements (between <?xml?> and <svg>)
# - add an option to remove ids if they match the Inkscape-style of IDs
# - investigate point-reducing algorithms
# - parse transform attribute
# - if a <g> has only one element in it, collapse the <g> (ensure transform, etc are carried down)
# - option to remove metadata

# necessary to get true division
from __future__ import division

import os
import sys
import xml.dom.minidom
import re
import math
import base64
import urllib
from svg_regex import svg_parser
import gzip
import optparse
from yocto_css import parseCssString

# Python 2.3- did not have Decimal
try:
	from decimal import *
except ImportError:
	from fixedpoint import *
	Decimal = FixedPoint	

# Import Psyco if available
try:
	import psyco
	psyco.full()
except ImportError:
	pass

APP = 'scour'
VER = '0.23'
COPYRIGHT = 'Copyright Jeff Schiller, 2010'

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
				'opacity',
				'overflow',
				'stop-color',
				'stop-opacity',
				'stroke',
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
	
def isSameSign(a,b): return (a <= 0 and b <= 0) or (a >= 0 and b >= 0)
	
coord = re.compile("\\-?\\d+\\.?\\d*")
scinumber = re.compile("[\\-\\+]?(\\d*\\.?)?\\d+[eE][\\-\\+]?\\d+")
number = re.compile("[\\-\\+]?(\\d*\\.?)?\\d+")
sciExponent = re.compile("[eE]([\\-\\+]?\\d+)")
unit = re.compile("(em|ex|px|pt|pc|cm|mm|in|\\%){1,1}$")

class Unit(object):
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
	
#	@staticmethod
	def get(str):
		# GZ: shadowing builtins like 'str' is generally bad form
		# GZ: encoding stuff like this in a dict makes for nicer code
		if str == None or str == '': return Unit.NONE
		elif str == '%': return Unit.PCT
		elif str == 'px': return Unit.PX
		elif str == 'pt': return Unit.PT
		elif str == 'pc': return Unit.PC
		elif str == 'em': return Unit.EM
		elif str == 'ex': return Unit.EX
		elif str == 'cm': return Unit.CM
		elif str == 'mm': return Unit.MM
		elif str == 'in': return Unit.IN
		return Unit.INVALID

#	@staticmethod
	def str(u):
		if u == Unit.NONE: return ''
		elif u == Unit.PCT: return '%'
		elif u == Unit.PX: return 'px'
		elif u == Unit.PT: return 'pt'
		elif u == Unit.PC: return 'pc'
		elif u == Unit.EM: return 'em'
		elif u == Unit.EX: return 'ex'
		elif u == Unit.CM: return 'cm'
		elif u == Unit.MM: return 'mm'
		elif u == Unit.IN: return 'in'
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

# returns the length of a property
# TODO: eventually use the above class once it is complete
def getSVGLength(value):
	try:
		v = float(value)
	except ValueError:
		coordMatch = coord.match(value)
		if coordMatch != None:
			unitMatch = unit.search(value, coordMatch.start(0))
		v = value
	return v

def findElementById(node, id):
	if node == None or node.nodeType != 1: return None
	if node.getAttribute('id') == id: return node
	for child in node.childNodes :
		e = findElementById(child,id)
		if e != None: return e
	return None

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
	that reference it.

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
		# node.firstChild will be either a CDATA or a Text node
		if node.firstChild != None:
			cssRules = parseCssString(node.firstChild.nodeValue)
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
numPointsRemovedFromPolygon = 0

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
	removeTags = ['linearGradient', 'radialGradient', 'pattern']

	identifiedElements = findElementsWithId(doc.documentElement)
	referencedIDs = findReferencedElements(doc.documentElement)

	for id in identifiedElements:
		if not id in referencedIDs:
			goner = findElementById(doc.documentElement, id)
			if goner != None and goner.parentNode != None and goner.nodeName in removeTags:
				goner.parentNode.removeChild(goner)
				num += 1
				numElemsRemoved += 1

	# TODO: should also go through defs and vacuum it
	num = 0
	defs = doc.documentElement.getElementsByTagName('defs')
	for aDef in defs:
		elemsToRemove = removeUnusedDefs(doc, aDef)
		for elem in elemsToRemove:
			elem.parentNode.removeChild(elem)
			numElemsRemoved += 1
			num += 1
	return num

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
		for attrNum in range(attrList.length):
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

def removeNestedGroups(node):
	""" 
	This walks further and further down the tree, removing groups
	which do not have any attributes or a title/desc child and 
	promoting their children up one level
	"""
	global numElemsRemoved
	num = 0
	
	groupsToRemove = []
	for child in node.childNodes:
		if child.nodeName == 'g' and child.namespaceURI == NS['SVG'] and len(child.attributes) == 0:
			# only collapse group if it does not have a title or desc as a direct descendant
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

def moveCommonAttributesToParentGroup(elem):
	""" 
	This recursively calls this function on all children of the passed in element
	and then iterates over all child elements and removes common inheritable attributes 
	from the children and places them in the parent group.  But only if the parent contains
	nothing but element children and whitespace.
	"""
	num = 0
	
	childElements = []
	# recurse first into the children (depth-first)
	for child in elem.childNodes:
		if child.nodeType == 1: 
			childElements.append(child)
			num += moveCommonAttributesToParentGroup(child)
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
	for num in range(attrList.length):
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
	for childNum in range(len(childElements)):
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
	for num in range(attrList.length):
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
	for childNum in range(len(childElements)):
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
				if stops.has_key(offset) :
					oldStop = stops[offset]
					if oldStop[0] == color and oldStop[1] == opacity:
						stopsToRemove.append(stop)
				stops[offset] = [color, opacity]
				
			for stop in stopsToRemove:
				stop.parentNode.removeChild(stop)
				num += 1
				numElemsRemoved += 1
	
	# linear gradients
	return num

def collapseSinglyReferencedGradients(doc):
	global numElemsRemoved
	num = 0
	
	# make sure to reset the ref'ed ids for when we are running this in testscour
	for rid,nodeCount in findReferencedElements(doc.documentElement).iteritems():
		count = nodeCount[0]
		nodes = nodeCount[1]
		if count == 1:
			elem = findElementById(doc.documentElement,rid)
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
				for i in range(stops.length):
					if stopsNotEqual: break
					stop = stops.item(i)
					ostop = ostops.item(i)
					for attr in ['offset', 'stop-color', 'stop-opacity']:
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
			
			# now that all referencing elements have been re-mapped to the master
			# it is safe to remove this gradient from the document
			dupGrad.parentNode.removeChild(dupGrad)
			numElemsRemoved += 1
			num += 1
	return num

def repairStyle(node, options):
	num = 0
	if node.nodeType == 1 and len(node.getAttribute('style')) > 0 :	
		# get all style properties and stuff them into a dictionary
		styleMap = { }
		rawStyles = node.getAttribute('style').split(';')
		for style in rawStyles:
			propval = style.split(':')
			if len(propval) == 2 :
				styleMap[propval[0].strip()] = propval[1].strip()

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
			# opacity='1.0' is useless, remove it
			if opacity == 1.0 :
				del styleMap['opacity']
				num += 1
				
			# if opacity='0' then all fill and stroke properties are useless, remove them
			elif opacity == 0.0 :
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
					
		#  stop-opacity: 1
		if styleMap.has_key('stop-opacity') :
			if float(styleMap['stop-opacity']) == 1.0 :
				del styleMap['stop-opacity']
				num += 1
		
		#  fill-opacity: 1 or 0
		if styleMap.has_key('fill-opacity') :
			fillOpacity = float(styleMap['fill-opacity'])
			#  TODO: This is actually a problem if the parent element does not have fill-opacity=1
			if fillOpacity == 1.0 :
				del styleMap['fill-opacity']
				num += 1
			elif fillOpacity == 0.0 :
				for uselessFillStyle in [ 'fill', 'fill-rule' ] :
					if styleMap.has_key(uselessFillStyle):
						del styleMap[uselessFillStyle]
						num += 1
		
		#  stroke-opacity: 1 or 0
		if styleMap.has_key('stroke-opacity') :
			strokeOpacity = float(styleMap['stroke-opacity']) 
			#  TODO: This is actually a problem if the parent element does not have stroke-opacity=1
			if strokeOpacity == 1.0 :
				del styleMap['stroke-opacity']
				num += 1
			elif strokeOpacity == 0.0 :
				for uselessStrokeStyle in [ 'stroke', 'stroke-width', 'stroke-linejoin', 'stroke-linecap', 
							'stroke-dasharray', 'stroke-dashoffset' ] :
					if styleMap.has_key(uselessStrokeStyle): 
						del styleMap[uselessStrokeStyle]
						num += 1

		# stroke-width: 0
		if styleMap.has_key('stroke-width') :
			strokeWidth = getSVGLength(styleMap['stroke-width']) 
			if strokeWidth == 0.0 :
				for uselessStrokeStyle in [ 'stroke', 'stroke-linejoin', 'stroke-linecap', 
							'stroke-dasharray', 'stroke-dashoffset', 'stroke-opacity' ] :
					if styleMap.has_key(uselessStrokeStyle): 
						del styleMap[uselessStrokeStyle]
						num += 1
		
		# remove font properties for non-text elements
		# I've actually observed this in real SVG content
		if node.nodeName in ['rect', 'circle', 'ellipse', 'line', 'polyline', 'polygon', 'path']:
			for fontstyle in [ 'font-family', 'font-size', 'font-stretch', 'font-size-adjust', 
								'font-style', 'font-variant', 'font-weight', 
								'letter-spacing', 'line-height', 'kerning',
								'text-anchor', 'text-decoration', 'text-rendering',
								'unicode-bidi', 'word-spacing', 'writing-mode'] :
				if styleMap.has_key(fontstyle) :
					del styleMap[fontstyle]
					num += 1

		# remove inkscape-specific styles
		# TODO: need to get a full list of these
		for inkscapeStyle in ['-inkscape-font-specification']:
			if styleMap.has_key(inkscapeStyle):
				del styleMap[inkscapeStyle]
				num += 1

		# visibility: visible
		if styleMap.has_key('visibility') :
			if styleMap['visibility'] == 'visible':
				del styleMap['visibility']
				num += 1
		
		# display: inline
		if styleMap.has_key('display') :
			if styleMap['display'] == 'inline':
				del styleMap['display']
				num += 1
				
		# overflow: visible or overflow specified on element other than svg, marker, pattern
		if styleMap.has_key('overflow') :
			if styleMap['overflow'] == 'visible' or node.nodeName in ['svg','marker','pattern']:
				del styleMap['overflow']
				num += 1
				
		# marker: none
		if styleMap.has_key('marker') :
			if styleMap['marker'] == 'none':
				del styleMap['marker']
				num += 1
		
		# now if any of the properties match known SVG attributes we prefer attributes 
		# over style so emit them and remove them from the style map
		if options.style_to_xml:
			for propName in styleMap.keys() :
				if propName in svgAttributes :
					node.setAttribute(propName, styleMap[propName])
					del styleMap[propName]

		# sew our remaining style properties back together into a style attribute
		fixedStyle = ''
		for prop in styleMap.keys() :
			fixedStyle += prop + ':' + styleMap[prop] + ';'
			
		if fixedStyle != '' :
			node.setAttribute('style', fixedStyle)
		else:
			node.removeAttribute('style')
	
	# recurse for our child elements
	for child in node.childNodes :
		num += repairStyle(child,options)
			
	return num

def removeDefaultAttributeValues(node, options):
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

	# recurse for our child elements
	for child in node.childNodes :
		num += removeDefaultAttributeValues(child,options)
	
	return num

rgb = re.compile("\\s*rgb\\(\\s*(\\d+)\\s*\\,\\s*(\\d+)\\s*\\,\\s*(\\d+)\\s*\\)\\s*")
rgbp = re.compile("\\s*rgb\\(\\s*(\\d*\\.?\\d+)\\%\\s*\\,\\s*(\\d*\\.?\\d+)\\%\\s*\\,\\s*(\\d*\\.?\\d+)\\%\\s*\\)\\s*")
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
		s  = 'rgb(%d,%d,%d)' % (r,g,b)
	
	rgbMatch = rgb.match(s)
	if rgbMatch != None :
		r = hex( int( rgbMatch.group(1) ) )[2:].upper()
		g = hex( int( rgbMatch.group(2) ) )[2:].upper()
		b = hex( int( rgbMatch.group(3) ) )[2:].upper()
		if len(r) == 1: r='0'+r
		if len(g) == 1: g='0'+g
		if len(b) == 1: b='0'+b
		s = '#'+r+g+b
	
	if s[0] == '#' and len(s)==7 and s[1]==s[2] and s[3]==s[4] and s[5]==s[6]:
		s = s.upper()
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
	for attr in attrsToConvert:
		oldColorValue = element.getAttribute(attr)
		if oldColorValue != '':
			newColorValue = convertColor(oldColorValue)
			oldBytes = len(oldColorValue)
			newBytes = len(newColorValue)
			if oldBytes > newBytes:
				element.setAttribute(attr, newColorValue)
				numBytes += (oldBytes - len(element.getAttribute(attr)))
	
	# now recurse for our child elements
	for child in element.childNodes :
		numBytes += convertColors(child)

	return numBytes

# TODO: go over what this method does and see if there is a way to optimize it
# TODO: go over the performance of this method and see if I can save memory/speed by
#       reusing data structures, etc
def cleanPath(element) :
	"""
		Cleans the path string (d attribute) of the element 
	"""
	global numBytesSavedInPathData
	global numPathSegmentsReduced
	global numCurvesStraightened
	
	# this gets the parser object from svg_regex.py
	oldPathStr = element.getAttribute('d')
	pathObj = svg_parser.parse(oldPathStr)
	
	# however, this parser object has some ugliness in it (lists of tuples of tuples of 
	# numbers and booleans).  we just need a list of (cmd,[numbers]):
	path = []
	for (cmd,dataset) in pathObj:
		if cmd in ['M','m','L','l','T','t']:
			# one or more tuples, each containing two numbers
			nums = []
			for t in dataset:
				# convert to a Decimal
				nums.append(Decimal(str(t[0])) * Decimal(1))
				nums.append(Decimal(str(t[1])) * Decimal(1))
					
			# only create this segment if it is not empty
			if nums:
				path.append( (cmd, nums) )
			
		elif cmd in ['V','v','H','h']:
			# one or more numbers
			nums = []
			for n in dataset:
				nums.append(Decimal(str(n)))
			if nums:
				path.append( (cmd, nums) )
			
		elif cmd in ['C','c']:
			# one or more tuples, each containing three tuples of two numbers each
			nums = []
			for t in dataset:
				for pair in t:
					nums.append(Decimal(str(pair[0])) * Decimal(1))
					nums.append(Decimal(str(pair[1])) * Decimal(1))
			path.append( (cmd, nums) )
			
		elif cmd in ['S','s','Q','q']:
			# one or more tuples, each containing two tuples of two numbers each
			nums = []
			for t in dataset:
				for pair in t:
					nums.append(Decimal(str(pair[0])) * Decimal(1))
					nums.append(Decimal(str(pair[1])) * Decimal(1))
			path.append( (cmd, nums) )
			
		elif cmd in ['A','a']:
			# one or more tuples, each containing a tuple of two numbers, a number, a boolean,
			# another boolean, and a tuple of two numbers
			nums = []
			for t in dataset:
				nums.append( Decimal(str(t[0][0])) * Decimal(1) )
				nums.append( Decimal(str(t[0][1])) * Decimal(1) )
				nums.append( Decimal(str(t[1])) * Decimal(1))
				
				if t[2]: nums.append( Decimal(1) )
				else: nums.append( Decimal(0) )

				if t[3]: nums.append( Decimal(1) )
				else: nums.append( Decimal(0) )
				
				nums.append( Decimal(str(t[4][0])) * Decimal(1) )
				nums.append( Decimal(str(t[4][1])) * Decimal(1) )
			path.append( (cmd, nums) )
		
		elif cmd in ['Z','z']:
			path.append( (cmd, []) )

	# calculate the starting x,y coord for the second path command
	if len(path[0][1]) == 2:
		(x,y) = path[0][1]
	else:
		# we have a move and then 1 or more coords for lines
		N = len(path[0][1])
		if path[0] == 'M':
			# take the last pair of coordinates for the starting point
			x = path[0][1][N-2]
			y = path[0][1][N-1]
		else: # relative move, accumulate coordinates for the starting point
			(x,y) = path[0][1][0],path[0][1][1]
			n = 2
			while n < N:
				x += path[0][1][n]
				y += path[0][1][n+1]
				n += 2
	
	# now we have the starting point at x,y so let's save it 
	(startx,starty) = (x,y)
	
	# convert absolute coordinates into relative ones (start with the second subcommand
	# and leave the first M as absolute)
	newPath = [path[0]]
	for (cmd,data) in path[1:]:
		i = 0
		newCmd = cmd
		newData = data
		# adjust abs to rel
		# only the A command has some values that we don't want to adjust (radii, rotation, flags)
		if cmd == 'A':
			newCmd = 'a'
			newData = []
			while i < len(data):
				newData.append(data[i])
				newData.append(data[i+1])
				newData.append(data[i+2])
				newData.append(data[i+3])
				newData.append(data[i+4])
				newData.append(data[i+5]-x)
				newData.append(data[i+6]-y)
				x = data[i+5]
				y = data[i+6]
				i += 7
		elif cmd == 'a':
			while i < len(data):
				x += data[i+5]
				y += data[i+6]
				i += 7			
		elif cmd == 'H':
			newCmd = 'h'
			newData = []
			while i < len(data):
				newData.append(data[i]-x)
				x = data[i]
				i += 1
		elif cmd == 'h':
			while i < len(data):
				x += data[i]
				i += 1
		elif cmd == 'V':
			newCmd = 'v'
			newData = []
			while i < len(data):
				newData.append(data[i] - y)
				y = data[i]
				i += 1
		elif cmd == 'v':
			while i < len(data):
				y += data[i]
				i += 1
		elif cmd in ['M']:
			newCmd = cmd.lower()
			newData = []
			startx = data[0]
			starty = data[1]
			while i < len(data):
				newData.append( data[i] - x )
				newData.append( data[i+1] - y )
				x = data[i]
				y = data[i+1]
				i += 2
		elif cmd in ['L','T']:
			newCmd = cmd.lower()
			newData = []
			while i < len(data):
				newData.append( data[i] - x )
				newData.append( data[i+1] - y )
				x = data[i]
				y = data[i+1]
				i += 2
		elif cmd in ['m']:
			startx += data[0]
			starty += data[1]
			while i < len(data):
				x += data[i]
				y += data[i+1]
				i += 2
		elif cmd in ['l','t']:
			while i < len(data):
				x += data[i]
				y += data[i+1]
				i += 2
		elif cmd in ['S','Q']:
			newCmd = cmd.lower()
			newData = []
			while i < len(data):
				newData.append( data[i] - x )
				newData.append( data[i+1] - y )
				newData.append( data[i+2] - x )
				newData.append( data[i+3] - y )
				x = data[i+2]
				y = data[i+3]
				i += 4
		elif cmd in ['s','q']:
			while i < len(data):
				x += data[i+2]
				y += data[i+3]
				i += 4
		elif cmd == 'C':
			newCmd = 'c'
			newData = []
			while i < len(data):
				newData.append( data[i] - x )
				newData.append( data[i+1] - y )
				newData.append( data[i+2] - x )
				newData.append( data[i+3] - y )
				newData.append( data[i+4] - x )
				newData.append( data[i+5] - y )
				x = data[i+4]
				y = data[i+5]
				i += 6
		elif cmd == 'c':
			while i < len(data):
				x += data[i+4]
				y += data[i+5]
				i += 6
		elif cmd in ['z','Z']:
			x = startx
			y = starty
			newCmd = 'z'
		newPath.append( (newCmd, newData) )
	path = newPath
	
	# remove empty segments
	newPath = [path[0]]
	for (cmd,data) in path[1:]:
		if cmd in ['m','l','t']:
			newData = []
			i = 0
			while i < len(data):
				if data[i] != 0 or data[i+1] != 0:
					newData.append(data[i])
					newData.append(data[i+1])
				else:
					numPathSegmentsReduced += 1
				i += 2
			if newData:
				newPath.append( (cmd,newData) )
		elif cmd == 'c':
			newData = []
			i = 0
			while i < len(data):
				if data[i+4] != 0 or data[i+5] != 0:
					newData.append(data[i])
					newData.append(data[i+1])
					newData.append(data[i+2])
					newData.append(data[i+3])
					newData.append(data[i+4])
					newData.append(data[i+5])
				else:
					numPathSegmentsReduced += 1
				i += 6
			if newData:
				newPath.append( (cmd,newData) )
		elif cmd == 'a':
			newData = []
			i = 0
			while i < len(data):
				if data[i+5] != 0 or data[i+6] != 0:
					newData.append(data[i])
					newData.append(data[i+1])
					newData.append(data[i+2])
					newData.append(data[i+3])
					newData.append(data[i+4])
					newData.append(data[i+5])
					newData.append(data[i+6])
				else:
					numPathSegmentsReduced += 1
				i += 7
			if newData:
				newPath.append( (cmd,newData) )
		elif cmd == 'q':
			newData = []
			i = 0
			while i < len(data):
				if data[i+2] != 0 or data[i+3] != 0:
					newData.append(data[i])
					newData.append(data[i+1])
					newData.append(data[i+2])
					newData.append(data[i+3])
				else:
					numPathSegmentsReduced += 1
				i += 4
			if newData:
				newPath.append( (cmd,newData) )
		elif cmd in ['h','v']:
			newData = []
			i = 0
			while i < len(data):
				if data[i] != 0:
					newData.append(data[i])
				else:
					numPathSegmentsReduced += 1
				i += 1
			if newData:
				newPath.append( (cmd,newData) )
		else:
			newPath.append( (cmd,data) )
	path = newPath
	
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
					if p1y == m*p1x and p2y == m*p2y:
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
					newData.append(data[i])
					newData.append(data[i+1])
					newData.append(data[i+2])
					newData.append(data[i+3])
					newData.append(data[i+4])
					newData.append(data[i+5])
					
				i += 6
		if newData or cmd == 'z' or cmd == 'Z':
			newPath.append( (cmd,newData) )
	path = newPath

	# collapse all consecutive commands of the same type into one command
	prevCmd = ''
	prevData = []
	newPath = [path[0]]
	for (cmd,data) in path[1:]:
		# flush the previous command if it is not the same type as the current command
		if prevCmd != '':
			if cmd != prevCmd:
				newPath.append( (prevCmd, prevData) )
				prevCmd = ''
				prevData = []
		
		# if the previous and current commands are the same type, collapse
		if cmd == prevCmd: 
			for coord in data:
				prevData.append(coord)
		
		# save last command and data
		else:
			prevCmd = cmd
			prevData = data
	# flush last command and data
	if prevCmd != '':
		newPath.append( (prevCmd, prevData) )
	path = newPath

	# convert to shorthand path segments where possible
	newPath = [path[0]]
	for (cmd,data) in path[1:]:
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
					lineTuples.append(data[i])
					lineTuples.append(data[i+1])
				i += 2
			if lineTuples:
				newPath.append( ('l', lineTuples) )
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
	newPath = [path[0]]
	for (cmd,data) in path[1:]:
		if cmd in ['h','v'] and len(data) > 1:
			newData = []
			prevCoord = data[0]
			for coord in data[1:]:
				if isSameSign(prevCoord, coord):
					prevCoord += coord
					numPathSegmentsReduced += 1
				else:
					newData.append(prevCoord)
					prevCoord = coord
			newData.append(prevCoord)
			newPath.append( (cmd, newData) )
		else:
			newPath.append( (cmd, data) )
	path = newPath
	
	# it is possible that we have consecutive h, v, c, t commands now
	# so again collapse all consecutive commands of the same type into one command
	prevCmd = ''
	prevData = []
	newPath = [path[0]]
	for (cmd,data) in path[1:]:
		# flush the previous command if it is not the same type as the current command
		if prevCmd != '':
			if cmd != prevCmd:
				newPath.append( (prevCmd, prevData) )
				prevCmd = ''
				prevData = []
		
		# if the previous and current commands are the same type, collapse
		if cmd == prevCmd: 
			for coord in data:
				prevData.append(coord)
		
		# save last command and data
		else:
			prevCmd = cmd
			prevData = data
	# flush last command and data
	if prevCmd != '':
		newPath.append( (prevCmd, prevData) )
	path = newPath
	
	newPathStr = serializePath(path)
	numBytesSavedInPathData += ( len(oldPathStr) - len(newPathStr) )
	element.setAttribute('d', newPathStr)

def parseListOfPoints(s):
	"""
		Parse string into a list of points.
	
		Returns a list of containing an even number of coordinate strings
	"""
	# (wsp)? comma-or-wsp-separated coordinate pairs (wsp)?
	# coordinate-pair = coordinate comma-or-wsp coordinate
	# coordinate = sign? integer
	nums = re.split("\\s*\\,?\\s*", s.strip())
	i = 0
	points = []
	while i < len(nums):
		x = SVGLength(nums[i])
		# if we had an odd number of points, return empty
		if i == len(nums)-1: return []
		else: y = SVGLength(nums[i+1])
		
		# if the coordinates were not unitless, return empty
		if x.units != Unit.NONE or y.units != Unit.NONE: return []
		points.append( str(x.value) )
		points.append( str(y.value) )
		i += 2
	
	return points
	
def cleanPolygon(elem):
	"""
		Remove unnecessary closing point of polygon points attribute
	"""
	global numPointsRemovedFromPolygon
	
	pts = parseListOfPoints(elem.getAttribute('points'))
	N = len(pts)/2
	if N >= 2:		
		(startx,starty) = (pts[0],pts[0])
		(endx,endy) = (pts[len(pts)-2],pts[len(pts)-1])
		if startx == endx and starty == endy:
			pts = pts[:-2]
			numPointsRemovedFromPolygon += 1		
	elem.setAttribute('points', scourCoordinates(pts))

def cleanPolyline(elem):
	"""
		Scour the polyline points attribute
	"""
	pts = parseListOfPoints(elem.getAttribute('points'))		
	elem.setAttribute('points', scourCoordinates(pts))
	
def serializePath(pathObj):
	"""
		Reserializes the path data with some cleanups.
	"""
	pathStr = ""
	for (cmd,data) in pathObj:
		pathStr += cmd
		# elliptical arc commands must have comma/wsp separating the coordinates
		# this fixes an issue outlined in Fix https://bugs.launchpad.net/scour/+bug/412754
		pathStr += scourCoordinates(data, (cmd == 'a'))
	return pathStr

def scourCoordinates(data, forceCommaWsp = False):
	"""
		Serializes coordinate data with some cleanups:
			- removes all trailing zeros after the decimal
			- integerize coordinates if possible
			- removes extraneous whitespace
			- adds commas between values in a subcommand if required (or if forceCommaWsp is True)
	"""
	coordsStr = ""
	if data != None:
		c = 0
		for coord in data:
			# add the scoured coordinate to the path string
			coordsStr += scourLength(coord)
			
			# only need the comma if the next number is non-negative or if forceCommaWsp is True
			if c < len(data)-1 and (forceCommaWsp or Decimal(data[c+1]) >= 0):
				coordsStr += ','
			c += 1
	return coordsStr

def scourLength(str):
	length = SVGLength(str)
	coord = length.value
	
	# reduce to the proper number of digits
	coord = Decimal(unicode(coord)) * Decimal(1)
	
	# integerize if we can
	if int(coord) == coord: coord = Decimal(unicode(int(coord)))

	# Decimal.trim() is available in Python 2.6+ to trim trailing zeros
	try:
		coord = coord.trim()
	except AttributeError:
		# trim it ourselves
		s = unicode(coord)
		dec = s.find('.')
		if dec != -1:
			while s[-1] == '0':
				s = s[:-1]
		coord = Decimal(s)

		# Decimal.normalize() will uses scientific notation - if that
		# string is smaller, then use it
		normd = coord.normalize()
		if len(unicode(normd)) < len(unicode(coord)):
			coord = normd
	
	return unicode(coord)+Unit.str(length.units)

def embedRasters(element, options) :
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

			# check if href resolves to an existing file
			if os.path.isfile(href) == False :
				if href[:7] != 'http://' and os.path.isfile(href) == False :
						# if this is not an absolute path, set path relative
						# to script file based on input arg 
						infilename = '.'
						if options.infilename: infilename = options.infilename
						href = os.path.join(os.path.dirname(infilename), href)				
			
			rasterdata = ''
			# test if file exists locally
			if os.path.isfile(href) == True :
				# open raster file as raw binary
				raster = open( href, "rb")
				rasterdata = raster.read()

			elif href[:7] == 'http://':
				# raster = open( href, "rb")
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

def properlySizeDoc(docElement):
	# get doc width and height
	w = SVGLength(docElement.getAttribute('width'))
	h = SVGLength(docElement.getAttribute('height'))

	# if width/height are not unitless or px then it is not ok to rewrite them into a viewBox	
	if ((w.units != Unit.NONE and w.units != Unit.PX) or
		(w.units != Unit.NONE and w.units != Unit.PX)):
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
		for i in range(attrList.length):
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
	newstr = str
	
	# encode & as &amp; ( must do this first so that &lt; does not become &amp;lt; )
	if str.find('&') != -1:
		newstr = str.replace('&', '&amp;')
			
	# encode < as &lt;
	if str.find("<") != -1:
		newstr = str.replace('<', '&lt;')
		
	# encode > as &gt; (TODO: is this necessary?)
	if str.find('>') != -1:
		newstr = str.replace('>', '&gt;')
	
	return newstr

# hand-rolled serialization function that has the following benefits:
# - pretty printing
# - somewhat judicious use of whitespace
# - ensure id attributes are first
def serializeXML(element, options, ind = 0, preserveWhitespace = False):
	indent = ind
	I=''
	if options.indent_type == 'tab': I='\t'
	elif options.indent_type == 'space': I=' '
	
	outString = (I * ind) + '<' + element.nodeName

	# always serialize the id or xml:id attributes first
	if element.getAttribute('id') != '':
		id = element.getAttribute('id')
		quot = '"'
		if id.find('"') != -1:
			quot = "'"
		outString += ' ' + 'id=' + quot + id + quot
	if element.getAttribute('xml:id') != '':
		id = element.getAttribute('xml:id')
		quot = '"'
		if id.find('"') != -1:
			quot = "'"
		outString += ' ' + 'xml:id=' + quot + id + quot
	
	# now serialize the other attributes
	attrList = element.attributes
	for num in range(attrList.length) :
		attr = attrList.item(num)
		if attr.nodeName == 'id' or attr.nodeName == 'xml:id': continue
		# if the attribute value contains a double-quote, use single-quotes
		quot = '"'
		if attr.nodeValue.find('"') != -1:
			quot = "'"

		attrValue = makeWellFormed( attr.nodeValue )
		
		outString += ' '
		# preserve xmlns: if it is a namespace prefix declaration
		if attr.prefix != None:
			outString += attr.prefix + ':'
		elif attr.namespaceURI != None:
			if attr.namespaceURI == 'http://www.w3.org/2000/xmlns/' and attr.nodeName.find('xmlns') == -1:
				outString += 'xmlns:'
			elif attr.namespaceURI == 'http://www.w3.org/1999/xlink':
				outString += 'xlink:'
		outString += attr.localName + '=' + quot + attrValue + quot

		if attr.nodeName == 'xml:space':
			if attrValue == 'preserve':
				preserveWhitespace = True
			elif attrValue == 'default':
				preserveWhitespace = False
	
	# if no children, self-close
	children = element.childNodes
	if children.length > 0:
		outString += '>'
	
		onNewLine = False
		for child in element.childNodes:
			# element node
			if child.nodeType == 1:
				if preserveWhitespace:
					outString += serializeXML(child, options, 0, preserveWhitespace)
				else:
					outString += os.linesep + serializeXML(child, options, indent + 1, preserveWhitespace)
					onNewLine = True
			# text node
			elif child.nodeType == 3:
				# trim it only in the case of not being a child of an element
				# where whitespace might be important
				if preserveWhitespace:
					outString += makeWellFormed(child.nodeValue)
				else:
					outString += makeWellFormed(child.nodeValue.strip())
			# CDATA node
			elif child.nodeType == 4:
				outString += '<![CDATA[' + child.nodeValue + ']]>'
			# Comment node
			elif child.nodeType == 8:
				outString += '<!--' + child.nodeValue + '-->'
			# TODO: entities, processing instructions, what else?
			else: # ignore the rest
				pass
				
		if onNewLine: outString += (I * ind)
		outString += '</' + element.nodeName + '>'
		if indent > 0: outString += os.linesep
	else:
		outString += '/>'
		if indent > 0: outString += os.linesep
		
	return outString
	
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
		for num in range(attrList.length) :
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
	for i in range(attrList.length):
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

	# repair style (remove unnecessary style properties and change them into XML attributes)
	numStylePropsFixed = repairStyle(doc.documentElement, options)

	# convert colors to #RRGGBB format
	if options.simple_colors:
		numBytesSavedInColors = convertColors(doc.documentElement)
	
	# remove empty defs, metadata, g
	# NOTE: these elements will be removed even if they have (invalid) text nodes
	elemsToRemove = []
	for tag in ['defs', 'metadata', 'g'] :
		for elem in doc.documentElement.getElementsByTagName(tag) :
			removeElem = not elem.hasChildNodes()
			if removeElem == False :
				for child in elem.childNodes :
					if child.nodeType in [1, 3, 4, 8] :
						break
				else:
					removeElem = True
			if removeElem :
				elem.parentNode.removeChild(elem)
				numElemsRemoved += 1

	# remove unreferenced gradients/patterns outside of defs
	while removeUnreferencedElements(doc) > 0:
		pass

	if options.strip_ids:
		bContinueLooping = True
		while bContinueLooping:
			identifiedElements = findElementsWithId(doc.documentElement)
			referencedIDs = findReferencedElements(doc.documentElement)
			bContinueLooping = (removeUnreferencedIDs(referencedIDs, identifiedElements) > 0)
	
	if options.group_collapse:
		while removeNestedGroups(doc.documentElement) > 0:
			pass

	while removeDuplicateGradientStops(doc) > 0:
		pass
	
	# remove gradients that are only referenced by one other gradient
	while collapseSinglyReferencedGradients(doc) > 0:
		pass
		
	# remove duplicate gradients
	while removeDuplicateGradients(doc) > 0:
		pass
	
	# move common attributes to parent group
	numAttrsRemoved += moveCommonAttributesToParentGroup(doc.documentElement)
	
	# remove unused attributes from parent
	numAttrsRemoved += removeUnusedAttributesOnParent(doc.documentElement)

	# clean path data
	for elem in doc.documentElement.getElementsByTagName('path') :
		if elem.getAttribute('d') == '':
			elem.parentNode.removeChild(elem)
		else:
			cleanPath(elem)

	# remove unnecessary closing point of polygons and scour points
	for polygon in doc.documentElement.getElementsByTagName('polygon') :
		cleanPolygon(polygon)

	# scour points of polyline
	for polyline in doc.documentElement.getElementsByTagName('polyline') :
		cleanPolygon(polyline)
	
	# scour lengths (including coordinates)
	for type in ['svg', 'image', 'rect', 'circle', 'ellipse', 'line', 'linearGradient', 'radialGradient', 'stop']:
		for elem in doc.getElementsByTagName(type):
			for attr in ['x', 'y', 'width', 'height', 'cx', 'cy', 'r', 'rx', 'ry', 
						'x1', 'y1', 'x2', 'y2', 'fx', 'fy', 'offset', 'opacity',
						'fill-opacity', 'stroke-opacity', 'stroke-width', 'stroke-miterlimit']:
				if elem.getAttribute(attr) != '':
					elem.setAttribute(attr, scourLength(elem.getAttribute(attr)))

	# remove default values of attributes
	numAttrsRemoved += removeDefaultAttributeValues(doc.documentElement, options)		
	
	# convert rasters references to base64-encoded strings 
	if options.embed_rasters:
		for elem in doc.documentElement.getElementsByTagName('image') :
			embedRasters(elem, options)		

	# properly size the SVG document (ideally width/height should be 100% with a viewBox)
	if options.enable_viewboxing:
		properlySizeDoc(doc.documentElement)

	# output the document as a pretty string with a single space for indent
	# NOTE: removed pretty printing because of this problem:
	# http://ronrothman.com/public/leftbraned/xml-dom-minidom-toprettyxml-and-silly-whitespace/
	# rolled our own serialize function here to save on space, put id first, customize indentation, etc
#	out_string = doc.documentElement.toprettyxml(' ')
	out_string = serializeXML(doc.documentElement, options) + os.linesep
	
	# now strip out empty lines
	lines = []
	# Get rid of empty lines
	for line in out_string.splitlines(True):
		if line.strip():
			lines.append(line)

	# return the string stripped of empty lines
	if options.strip_xml_prolog == False:
		xmlprolog = '<?xml version="1.0" encoding="UTF-8" standalone="no"?>' + os.linesep
	else:
		xmlprolog = ""
		
	return xmlprolog + "".join(lines)

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
_options_parser.add_option("--enable-id-stripping",
	action="store_true", dest="strip_ids", default=False,
	help="remove all un-referenced ID attributes")
_options_parser.add_option("--disable-embed-rasters",
	action="store_false", dest="embed_rasters", default=True,
	help="won't embed rasters as base64-encoded data")
_options_parser.add_option("--keep-editor-data",
	action="store_true", dest="keep_editor_data", default=False,
	help="won't remove Inkscape, Sodipodi or Adobe Illustrator elements and attributes")
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
_options_parser.add_option("--indent",
	action="store", type="string", dest="indent_type", default="space",
	help="indentation of the output: none, space, tab (default: %default)")

def maybe_gziped_file(filename, mode="r"):
	if os.path.splitext(filename)[1].lower() in (".svgz", ".gz"):
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
		outfile = maybe_gziped_file(options.outfilename, "w")
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
		' Number of points removed from polygons: ' + str(numPointsRemovedFromPolygon)

if __name__ == '__main__':
	if sys.platform == "win32":
		from time import clock as get_tick
	else:
		# GZ: is this different from time.time() in any way?
		def get_tick():
			return os.times()[0]

	start = get_tick()
	
	options, (input, output) = parse_args()
	
	print >>sys.stderr, "%s %s\n%s" % (APP, VER, COPYRIGHT)

	# do the work
	in_string = input.read()
	out_string = scourString(in_string, options).encode("UTF-8")
	output.write(out_string)

	# Close input and output files
	input.close()
	output.close()

	end = get_tick()

	# GZ: unless silenced by -q or something?
	# GZ: not using globals would be good too
	print >>sys.stderr, ' File:', input.name, \
		os.linesep + ' Time taken:', str(end-start) + 's' + os.linesep, \
		getReport()
	
	oldsize = len(in_string)
	newsize = len(out_string)
	sizediff = (newsize / oldsize) * 100
	print >>sys.stderr, ' Original file size:', oldsize, 'bytes;', \
		'new file size:', newsize, 'bytes (' + str(sizediff)[:5] + '%)'


