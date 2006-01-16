#!/usr/bin/env python
"""
cubicsuperpath.py

Copyright (C) 2005 Aaron Spike, aaron@ekips.org

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

"""
import simplepath 

def CubicSuperPath(simplepath):
	csp = []
	subpath = -1
	subpathstart = []
	last = []
	lastctrl = []
	for s in simplepath:
		cmd, params = s		
		if cmd == 'M':
			if last:
				csp[subpath].append([lastctrl[:],last[:],last[:]])
			subpath += 1
			csp.append([])
			subpathstart =  params[:]
			last = params[:]
			lastctrl = params[:]
		elif cmd == 'L':
			csp[subpath].append([lastctrl[:],last[:],last[:]])
			last = params[:]
			lastctrl = params[:]
		elif cmd == 'C':
			csp[subpath].append([lastctrl[:],last[:],params[:2]])
			last = params[-2:]
			lastctrl = params[2:4]
		elif cmd == 'Q':
			#TODO: convert to cubic
			csp[subpath].append([lastctrl[:],last[:],last[:]])
			last = params[-2:]
			lastctrl = params[-2:]
		elif cmd == 'A':
			#TODO: convert to cubics
			csp[subpath].append([lastctrl[:],last[:],last[:]])
			last = params[-2:]
			lastctrl = params[-2:]
		elif cmd == 'Z':
			csp[subpath].append([lastctrl[:],last[:],last[:]])
			last = subpathstart[:]
			lastctrl = subpathstart[:]
	#append final superpoint
	csp[subpath].append([lastctrl[:],last[:],last[:]])
	return csp	

def unCubicSuperPath(csp):
	a = []
	for subpath in csp:
		if subpath:
			a.append(['M',subpath[0][1][:]])
			for i in range(1,len(subpath)):
				a.append(['C',subpath[i-1][2][:] + subpath[i][0][:] + subpath[i][1][:]])
	return a

def parsePath(d):
	return CubicSuperPath(simplepath.parsePath(d))

def formatPath(p):
	return simplepath.formatPath(unCubicSuperPath(p))




