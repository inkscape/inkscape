#!/usr/bin/env python
'''
    Copyright (C) 2004 Aaron Cyril Spike

    This file is part of FretFind 2-D.

    FretFind 2-D is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    FretFind 2-D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FretFind 2-D; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
'''
import sys
from ffgeom import *
threshold=0.0000000001

def FindFrets(strings, meta, scale, tuning, numfrets):
	scale = scale['steps']

	#if the string ends don't fall on the nut and bridge
	#don't look for partial frets.
	numStrings = len(strings)
	doPartials = True
	parallelFrets = True
	
	nut = Segment(strings[0][0],strings[-1][0])
	bridge = Segment(strings[0][1],strings[-1][1])
	midline = Segment(
		Point((nut[1]['x']+nut[0]['x'])/2.0,(nut[1]['y']+nut[0]['y'])/2.0),
		Point((bridge[1]['x']+bridge[0]['x'])/2.0,(bridge[1]['y']+bridge[0]['y'])/2.0))
	for s in strings:
		if nut.distanceToPoint(s[0])>=threshold or bridge.distanceToPoint(s[1])>=threshold:
			doPartials = False
			break

	denom = ((bridge[1]['y']-bridge[0]['y'])*(nut[1]['x']-nut[0]['x']))-((bridge[1]['x']-bridge[0]['x'])*(nut[1]['y']-nut[0]['y']))
	if denom != 0:
		parallelFrets = False

	fretboard = []
	tones = len(scale)-1
	for i in range(len(strings)):
		base = tuning[i]
		frets = []
		if doPartials:
			frets.append(Segment(meta[i][0],meta[i+1][0]))
		else:
			frets.append(Segment(strings[i][0],strings[i][0]))
		last = strings[i][0]

		for j in range(numfrets):
			step=((base+j-1)%(tones))+1
			ratio=1.0-((scale[step][1]*scale[step-1][0])/(scale[step][0]*scale[step-1][1]))
			x = last['x']+(ratio*(strings[i][1]['x']-last['x']))
			y = last['y']+(ratio*(strings[i][1]['y']-last['y']))
			current = Point(x,y)	
			temp = Segment(strings[i][0],current)
			totalRatio = temp.length()/strings[i].length()
			
			if doPartials:
				#partials depending on outer strings (questionable)
				if parallelFrets:
					temp = nut.createParallel(current)
				else:
					temp = Segment(strings[0].pointAtLength(strings[0].length()*totalRatio),
						strings[-1].pointAtLength(strings[-1].length()*totalRatio))
				frets.append(Segment(intersectSegments(temp,meta[i]),intersectSegments(temp,meta[i+1])))
			else:
				frets.append(Segment(current,current))
			last = current
		fretboard.append(frets)
	return fretboard
	
def FindStringsSingleScale(numStrings,scaleLength,nutWidth,bridgeWidth,oNF,oBF,oNL,oBL):
	strings = []
	meta = []
	nutHalf = nutWidth/2
	bridgeHalf = bridgeWidth/2
	nutCandidateCenter = (nutHalf) + oNL
	bridgeCandidateCenter = (bridgeHalf) + oBL
	if bridgeCandidateCenter >= nutCandidateCenter:
		center = bridgeCandidateCenter
	else:
		center = nutCandidateCenter
	nutStringSpacing = nutWidth/(numStrings-1)
	bridgeStringSpacing = bridgeWidth/(numStrings-1)
	
	for i in range(numStrings):
		strings.append(Segment(Point(center+nutHalf-(i*nutStringSpacing),0),
			Point(center+bridgeHalf-(i*bridgeStringSpacing),scaleLength)))

	meta.append(Segment(Point(center+nutHalf+oNF,0),Point(center+bridgeHalf+oBF,scaleLength)))
	for i in range(1,numStrings):
		meta.append(Segment(
			Point((strings[i-1][0]['x']+strings[i][0]['x'])/2.0,
				(strings[i-1][0]['y']+strings[i][0]['y'])/2.0),
			Point((strings[i-1][1]['x']+strings[i][1]['x'])/2.0,
				(strings[i-1][1]['y']+strings[i][1]['y'])/2.0)))
	meta.append(Segment(Point(center-(nutHalf+oNL),0),Point(center-(bridgeHalf+oBL),scaleLength)))

	return strings, meta

def FindStringsMultiScale(numStrings,scaleLengthF,scaleLengthL,nutWidth,bridgeWidth,perp,oNF,oBF,oNL,oBL):
	strings = []
	meta = []
	nutHalf = nutWidth/2
	bridgeHalf = bridgeWidth/2
	nutCandidateCenter = (nutHalf)+oNL
	bridgeCandidateCenter = (bridgeHalf)+oBL
	if bridgeCandidateCenter >= nutCandidateCenter:
		xcenter = bridgeCandidateCenter
	else:
		nutCandidateCenter

	fbnxf = xcenter+nutHalf+oNF
	fbbxf = xcenter+bridgeHalf+oBF
	fbnxl = xcenter-(nutHalf+oNL)
	fbbxl = xcenter-(bridgeHalf+oBL)

	snxf = xcenter+nutHalf
	sbxf = xcenter+bridgeHalf
	snxl = xcenter-nutHalf
	sbxl = xcenter-bridgeHalf

	fdeltax = sbxf-snxf
	ldeltax = sbxl-snxl
	fdeltay = math.sqrt((scaleLengthF*scaleLengthF)-(fdeltax*fdeltax))
	ldeltay = math.sqrt((scaleLengthL*scaleLengthL)-(ldeltax*ldeltax))

	fperp = perp*fdeltay
	lperp = perp*ldeltay

	#temporarily place first and last strings
	first = Segment(Point(snxf,0),Point(sbxf,fdeltay))
	last = Segment(Point(snxl,0),Point(sbxl,ldeltay))
	
	if fdeltay<=ldeltay:
		first.translate(0,(lperp-fperp))
	else:
		last.translate(0,(fperp-lperp))

	nut = Segment(first[0].copy(),last[0].copy())
	bridge = Segment(first[1].copy(),last[1].copy())
	#overhang measurements are now converted from delta x to along line lengths
	oNF = (oNF*nut.length())/nutWidth
	oNL = (oNL*nut.length())/nutWidth
	oBF = (oBF*bridge.length())/bridgeWidth
	oBL = (oBL*bridge.length())/bridgeWidth
	#place fretboard edges
	fbf = Segment(nut.pointAtLength(-oNF),bridge.pointAtLength(-oBF))
	fbl = Segment(nut.pointAtLength(nut.length()+oNL),bridge.pointAtLength(bridge.length()+oBL))
	#normalize values into the first quadrant via translate
	if fbf[0]['y']<0 or fbl[0]['y']<0:
		if fbf[0]['y']<=fbl[0]['y']:
			move = -fbf[0]['y']
		else:
			move = -fbl[0]['y']
		
		first.translate(0,move)
		last.translate(0,move)
		nut.translate(0,move)
		bridge.translate(0,move)
		fbf.translate(0,move)
		fbl.translate(0,move)

	#output values
	nutStringSpacing = nut.length()/(numStrings-1)
	bridgeStringSpacing = bridge.length()/(numStrings-1)
	strings.append(first)
	for i in range(1,numStrings-1):
		n = nut.pointAtLength(i*nutStringSpacing)
		b = bridge.pointAtLength(i*bridgeStringSpacing)
		strings.append(Segment(Point(n['x'],n['y']),Point(b['x'],b['y'])))
	strings.append(last)

	meta.append(fbf)
	for i in range(1,numStrings):
		meta.append(Segment(
			Point((strings[i-1][0]['x']+strings[i][0]['x'])/2.0,
				(strings[i-1][0]['y']+strings[i][0]['y'])/2.0),
			Point((strings[i-1][1]['x']+strings[i][1]['x'])/2.0,
				(strings[i-1][1]['y']+strings[i][1]['y'])/2.0)))
	
	meta.append(fbl)
	
	return strings, meta
