#!/usr/bin/env python 
'''
Copyright (C) 2001-2002 Matt Chisholm matt@theory.org
Copyright (C) 2008 Joel Holdsworth joel@airwebreathe.org.uk
    for AP

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
'''
# standard library
import copy
import math
import cmath
import string
import random
import os
import sys
import re
# local library
import inkex
import simplestyle
import render_alphabetsoup_config
import bezmisc
import simplepath
import simpletransform

syntax   = render_alphabetsoup_config.syntax
alphabet = render_alphabetsoup_config.alphabet
units	= render_alphabetsoup_config.units
font	 = render_alphabetsoup_config.font

# Loads a super-path from a given SVG file
def loadPath( svgPath ):
	extensionDir = os.path.normpath(
	                    os.path.join( os.getcwd(), os.path.dirname(__file__) )
	                  )
	# __file__ is better then sys.argv[0] because this file may be a module
	# for another one.
	tree = inkex.etree.parse( extensionDir + "/" + svgPath )
	root = tree.getroot()
	pathElement = root.find('{http://www.w3.org/2000/svg}path')
	if pathElement == None:
		return None, 0, 0
	d = pathElement.get("d")
	width = float(root.get("width"))
	height = float(root.get("height"))
	return simplepath.parsePath(d), width, height # Currently we only support a single path

def combinePaths( pathA, pathB ):
	if pathA == None and pathB == None:
		return None
	elif pathA == None:
		return pathB
	elif pathB == None:
		return pathA
	else:
		return pathA + pathB

def reverseComponent(c):
	nc = []
	last = c.pop()
	nc.append(['M', last[1][-2:]])
	while c:
		this = c.pop()
		cmd = last[0]
		if cmd == 'C':
			nc.append([last[0], last[1][2:4] + last[1][:2] + this[1][-2:]])
		else:
			nc.append([last[0], this[1][-2:]])
		last = this
	return nc

def reversePath(sp):
	rp = []
	component = []
	for p in sp:
		cmd, params = p
		if cmd == 'Z':
			rp.extend(reverseComponent(component))
			rp.append(['Z', []])
			component = []
		else:
			component.append(p)
	return rp

def flipLeftRight( sp, width ):
	for cmd,params in sp:
		defs = simplepath.pathdefs[cmd]
		for i in range(defs[1]):
			if defs[3][i] == 'x':
				params[i] = width - params[i]

def flipTopBottom( sp, height ):
	for cmd,params in sp:
		defs = simplepath.pathdefs[cmd]
		for i in range(defs[1]):
			if defs[3][i] == 'y':
				params[i] = height - params[i]

def solveQuadratic(a, b, c):
	det = b*b - 4.0*a*c
	if det >= 0: # real roots
		sdet = math.sqrt(det)
	else: # complex roots
		sdet = cmath.sqrt(det)
	return (-b + sdet) / (2*a), (-b - sdet) / (2*a)

def cbrt(x): 
	if x >= 0:
		return x**(1.0/3.0)
	else:
		return -((-x)**(1.0/3.0))

def findRealRoots(a,b,c,d):
	if a != 0:
		a, b, c, d = 1, b/float(a), c/float(a), d/float(a)	# Divide through by a
		t = b / 3.0
		p, q = c - 3 * t**2, d - c * t + 2 * t**3
		u, v = solveQuadratic(1, q, -(p/3.0)**3)
		if type(u) == type(0j): 		# Complex Cubic Root
			r = math.sqrt(u.real**2 + u.imag**2)
			w = math.atan2(u.imag, u.real)
			y1 = 2 * cbrt(r) * math.cos(w / 3.0)
		else: 		# Complex Real Root
			y1 = cbrt(u) + cbrt(v)
		
		y2, y3 = solveQuadratic(1, y1, p + y1**2)

		if type(y2) == type(0j):	# Are y2 and y3 complex?
			return [y1 - t]
		return [y1 - t, y2 - t, y3 - t]
	elif b != 0:
		det=c*c - 4.0*b*d
		if det >= 0:
			return [(-c + math.sqrt(det))/(2.0*b),(-c - math.sqrt(det))/(2.0*b)]
	elif c != 0:
		return [-d/c]
	return []

def getPathBoundingBox( sp ):
	
	box = None
	last = None
	lostctrl = None

	for cmd,params in sp:

		segmentBox = None

		if cmd == 'M':
			# A move cannot contribute to the bounding box
			last = params[:]
			lastctrl = params[:]
		elif cmd == 'L':
			if last:
				segmentBox = (min(params[0], last[0]), max(params[0], last[0]), min(params[1], last[1]), max(params[1], last[1]))
			last = params[:]
			lastctrl = params[:]
		elif cmd == 'C':
			if last:		
				segmentBox = (min(params[4], last[0]), max(params[4], last[0]), min(params[5], last[1]), max(params[5], last[1]))
				
				bx0, by0 = last[:]
				bx1, by1, bx2, by2, bx3, by3 = params[:]

				# Compute the x limits
				a = (-bx0 + 3*bx1 - 3*bx2 + bx3)*3
				b = (3*bx0 - 6*bx1  + 3*bx2)*2
				c = (-3*bx0 + 3*bx1)
				ts = findRealRoots(0, a, b, c)
				for t in ts:
					if t >= 0 and t <= 1:		
						x = (-bx0 + 3*bx1 - 3*bx2 + bx3)*(t**3) + \
							(3*bx0 - 6*bx1 + 3*bx2)*(t**2) + \
							(-3*bx0 + 3*bx1)*t + \
							bx0
						segmentBox = (min(segmentBox[0], x), max(segmentBox[1], x), segmentBox[2], segmentBox[3])

				# Compute the y limits
				a = (-by0 + 3*by1 - 3*by2 + by3)*3
				b = (3*by0 - 6*by1  + 3*by2)*2
				c = (-3*by0 + 3*by1)
				ts = findRealRoots(0, a, b, c)
				for t in ts:
					if t >= 0 and t <= 1:		
						y = (-by0 + 3*by1 - 3*by2 + by3)*(t**3) + \
							(3*by0 - 6*by1 + 3*by2)*(t**2) + \
							(-3*by0 + 3*by1)*t + \
							by0
						segmentBox = (segmentBox[0], segmentBox[1], min(segmentBox[2], y), max(segmentBox[3], y))

			last = params[-2:]
			lastctrl = params[2:4]

		elif cmd == 'Q':
			# Provisional
			if last:
				segmentBox = (min(params[0], last[0]), max(params[0], last[0]), min(params[1], last[1]), max(params[1], last[1]))
			last = params[-2:]
			lastctrl = params[2:4]

		elif cmd == 'A':
			# Provisional
			if last:
				segmentBox = (min(params[0], last[0]), max(params[0], last[0]), min(params[1], last[1]), max(params[1], last[1]))
			last = params[-2:]
			lastctrl = params[2:4]

		if segmentBox:
			if box:
				box = (min(segmentBox[0],box[0]), max(segmentBox[1],box[1]), min(segmentBox[2],box[2]), max(segmentBox[3],box[3]))
			else:
				box = segmentBox			
	return box

def mxfm( image, width, height, stack ):								# returns possibly transformed image
	tbimage = image	
	if ( stack[0] == "-" ):							  # top-bottom flip
		flipTopBottom(tbimage, height)
		tbimage = reversePath(tbimage)
		stack.pop( 0 )

	lrimage = tbimage
	if ( stack[0] == "|" ):							  # left-right flip
		flipLeftRight(tbimage, width)
		lrimage = reversePath(lrimage)
		stack.pop( 0 )
	return lrimage

def comparerule( rule, nodes ):						  # compare node list to nodes in rule
	for i in range( 0, len(nodes)):					  # range( a, b ) = (a, a+1, a+2 ... b-2, b-1)
		if (nodes[i] == rule[i][0]):
			pass
		else: return 0
	return 1

def findrule( state, nodes ):							# find the rule which generated this subtree
	ruleset = syntax[state][1]
	nodelen = len(nodes)
	for rule in ruleset:
		rulelen = len(rule)
		if ((rulelen == nodelen) and (comparerule( rule, nodes ))):
			return rule
	return 

def generate( state ):								   # generate a random tree (in stack form)
	stack  = [ state ]
	if ( len(syntax[state]) == 1 ):						# if this is a stop symbol
		return stack
	else:
		stack.append( "[" )
		path = random.randint(0, (len(syntax[state][1])-1)) # choose randomly from next states
		for symbol in syntax[state][1][path]:			# recurse down each non-terminal
			if ( symbol != 0 ):						  # 0 denotes end of list ###
				substack = generate( symbol[0] )		 # get subtree
				for elt in substack:	   
					stack.append( elt )
				if (symbol[3]):stack.append( "-" )	   # top-bottom flip
				if (symbol[4]):stack.append( "|" )	   # left-right flip
			#else:
				#inkex.debug("found end of list in generate( state =", state, ")") # this should be deprecated/never happen
		stack.append("]")
		return stack

def draw( stack ):									   # draw a character based on a tree stack
	state = stack.pop(0)
	#print state,

	image, width, height = loadPath( font+syntax[state][0] )		  # load the image
	if (stack[0] != "["):								# terminal stack element
		if (len(syntax[state]) == 1):					  # this state is a terminal node
			return image, width, height
		else:
			substack = generate( state )				 # generate random substack
			return draw( substack )					  # draw random substack
	else:
		#inkex.debug("[")
		stack.pop(0)
		images = []								  # list of daughter images
		nodes  = []								  # list of daughter names
		while (stack[0] != "]"):					 # for all nodes in stack
			newstate = stack[0]					  # the new state
			newimage, width, height = draw( stack )				 # draw the daughter state
			if (newimage):
				tfimage = mxfm( newimage, width, height, stack )	# maybe transform daughter state
				images.append( [tfimage, width, height] )			 # list of daughter images
				nodes.append( newstate )			 # list of daughter nodes
			else:
				#inkex.debug(("recurse on",newstate,"failed")) # this should never happen
				return None, 0, 0
		rule = findrule( state, nodes )			  # find the rule for this subtree

		for i in range( 0, len(images)):
			currimg, width, height = images[i]

			if currimg:
				#box = getPathBoundingBox(currimg)
				dx = rule[i][1]*units
				dy = rule[i][2]*units
				#newbox = ((box[0]+dx),(box[1]+dy),(box[2]+dx),(box[3]+dy))
				simplepath.translatePath(currimg, dx, dy)
				image = combinePaths( image, currimg )

		stack.pop( 0 )
		return image, width, height

def draw_crop_scale( stack, zoom ):							# draw, crop and scale letter image
	image, width, height = draw(stack)
	bbox = getPathBoundingBox(image)			
	simplepath.translatePath(image, -bbox[0], 0)	
	simplepath.scalePath(image, zoom/units, zoom/units)
	return image, bbox[1] - bbox[0], bbox[3] - bbox[2]

def randomize_input_string(tokens, zoom ):					   # generate a glyph starting from each token in the input string
	imagelist = []

	for i in range(0,len(tokens)):
		char = tokens[i]
		#if ( re.match("[a-zA-Z0-9?]", char)):
		if ( alphabet.has_key(char)):
			if ((i > 0) and (char == tokens[i-1])):		 # if this letter matches previous letter
				imagelist.append(imagelist[len(stack)-1])# make them the same image
			else:										# generate image for letter
				stack = string.split( alphabet[char][random.randint(0,(len(alphabet[char])-1))] , "." )
				#stack = string.split( alphabet[char][random.randint(0,(len(alphabet[char])-2))] , "." ) 
				imagelist.append( draw_crop_scale( stack, zoom ))
		elif( char == " "):							  # add a " " space to the image list
			imagelist.append( " " )
		else:											# this character is not in config.alphabet, skip it
			sys.stderr.write('bad character "%s"\n' % char)
	return imagelist

def generate_random_string( tokens, zoom ):                       # generate a totally random glyph for each glyph in the input string
	imagelist = []
	for char in tokens:
		if ( char == " "):                               # add a " " space to the image list
			imagelist.append( " " )
		else:
			if ( re.match("[a-z]", char )):              # generate lowercase letter
				stack = generate("lc")
			elif ( re.match("[A-Z]", char )):            # generate uppercase letter
				stack = generate("UC")
			else:                                        # this character is not in config.alphabet, skip it
				sys.stderr.write('bad character"%s"\n' % char)
				stack = generate("start")
			imagelist.append( draw_crop_scale( stack, zoom ))

	return imagelist

def optikern( image, width, zoom ):                                   # optical kerning algorithm
	left  = []
	right = []

	resolution = 8
	for i in range( 0, 18 * resolution ):
		y = 1.0/resolution * (i + 0.5) * zoom
		xmin = None
		xmax = None

		for cmd,params in image:

			segmentBox = None

			if cmd == 'M':
				# A move cannot contribute to the bounding box
				last = params[:]
				lastctrl = params[:]
			elif cmd == 'L':
				if (y >= last[1] and y <= params[1]) or (y >= params[1] and y <= last[1]):
					if params[0] == last[0]:
						x = params[0]
					else:
						a = (params[1] - last[1]) / (params[0] - last[0])
						b = last[1] - a * last[0]
						if a != 0:
							x = (y - b) / a
						else: x = None
					
					if x:
						if xmin == None or x < xmin: xmin = x
						if xmax == None or x > xmax: xmax = x

				last = params[:]
				lastctrl = params[:]
			elif cmd == 'C':
				if last:		
					bx0, by0 = last[:]
					bx1, by1, bx2, by2, bx3, by3 = params[:]

					d = by0 - y
					c = -3*by0 + 3*by1
					b = 3*by0 - 6*by1 + 3*by2
					a = -by0 + 3*by1 - 3*by2 + by3
					
					ts = findRealRoots(a, b, c, d)

					for t in ts:
						if t >= 0 and t <= 1:		
							x = (-bx0 + 3*bx1 - 3*bx2 + bx3)*(t**3) + \
								(3*bx0 - 6*bx1 + 3*bx2)*(t**2) + \
								(-3*bx0 + 3*bx1)*t + \
								bx0
							if xmin == None or x < xmin: xmin = x
							if xmax == None or x > xmax: xmax = x

				last = params[-2:]
				lastctrl = params[2:4]

			elif cmd == 'Q':
				# Quadratic beziers are ignored
				last = params[-2:]
				lastctrl = params[2:4]

			elif cmd == 'A':
				# Arcs are ignored
				last = params[-2:]
				lastctrl = params[2:4]


		if xmin != None and xmax != None:
			left.append( xmin )                        # distance from left edge of region to left edge of bbox
			right.append( width - xmax )               # distance from right edge of region to right edge of bbox
		else:
			left.append(  width )
			right.append( width )

	return (left, right)

def layoutstring( imagelist, zoom ):					 # layout string of letter-images using optical kerning
	kernlist  = []
	length = zoom
	for entry in imagelist:
		if (entry == " "):							   # leaving room for " " space characters
			length = length + (zoom * render_alphabetsoup_config.space)
		else:
			image, width, height = entry
			length = length + width + zoom   # add letter length to overall length
			kernlist.append( optikern(image, width, zoom) )		   # append kerning data for this image 

	workspace = None

	position = zoom
	for i in range(0, len(kernlist)):
		while(imagelist[i] == " "):
			position = position + (zoom * render_alphabetsoup_config.space )
			imagelist.pop(i)
		image, width, height = imagelist[i]

		# set the kerning
		if i == 0: kern = 0						  # for first image, kerning is zero
		else:
			kerncompare = []							 # kerning comparison array
			for j in range( 0, len(kernlist[i][0])):
				kerncompare.append( kernlist[i][0][j]+kernlist[i-1][1][j] )
			kern = min( kerncompare )

		position = position - kern					   # move position back by kern amount
		thisimage = copy.deepcopy(image)		
		simplepath.translatePath(thisimage, position, 0)
		workspace = combinePaths(workspace, thisimage)
		position = position + width + zoom	# advance position by letter width

	return workspace

def tokenize(text):
	"""Tokenize the string, looking for LaTeX style, multi-character tokens in the string, like \\yogh."""
	tokens = []
	i = 0
	while i < len(text):
		c = text[i]
		i += 1
		if c == '\\': # found the beginning of an escape
			t = ''
			while i < len(text): # gobble up content of the escape
				c = text[i]
				if c == '\\': # found another escape, stop this one
					break
				i += 1
				if c == ' ': # a space terminates this escape
					break
				t += c # stick this character onto the token
			if t:
				tokens.append(t)
		else:
			tokens.append(c)
	return tokens

class AlphabetSoup(inkex.Effect):
	def __init__(self):
		inkex.Effect.__init__(self)
		self.OptionParser.add_option("-t", "--text",
						action="store", type="string", 
						dest="text", default="Inkscape",
						help="The text for alphabet soup")
		self.OptionParser.add_option("-z", "--zoom",
						action="store", type="float", 
						dest="zoom", default="8.0",
						help="The zoom on the output graphics")
		self.OptionParser.add_option("-r", "--randomize",
						action="store", type="inkbool", 
						dest="randomize", default=False,
						help="Generate random (unreadable) text")

	def effect(self):
		zoom = self.unittouu( str(self.options.zoom) + 'px')

		if self.options.randomize:
			imagelist = generate_random_string(self.options.text, zoom)
		else:
			tokens = tokenize(self.options.text)
			imagelist = randomize_input_string(tokens, zoom)

		image = layoutstring( imagelist, zoom )

		if image:
			s = { 'stroke': 'none', 'fill': '#000000' }

			new = inkex.etree.Element(inkex.addNS('path','svg'))
			new.set('style', simplestyle.formatStyle(s))

			new.set('d', simplepath.formatPath(image))
			self.current_layer.append(new)

			# compensate preserved transforms of parent layer
			if self.current_layer.getparent() is not None:
				mat = simpletransform.composeParents(self.current_layer, [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0]])
				simpletransform.applyTransformToNode(simpletransform.invertTransform(mat), new)


if __name__ == '__main__':
    e = AlphabetSoup()
    e.affect()

