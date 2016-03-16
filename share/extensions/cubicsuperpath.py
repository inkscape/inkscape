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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

"""
import simplepath 
from math import *

def matprod(mlist):
    prod=mlist[0]
    for m in mlist[1:]:
        a00=prod[0][0]*m[0][0]+prod[0][1]*m[1][0]
        a01=prod[0][0]*m[0][1]+prod[0][1]*m[1][1]
        a10=prod[1][0]*m[0][0]+prod[1][1]*m[1][0]
        a11=prod[1][0]*m[0][1]+prod[1][1]*m[1][1]
        prod=[[a00,a01],[a10,a11]]
    return prod
def rotmat(teta):
    return [[cos(teta),-sin(teta)],[sin(teta),cos(teta)]]
def applymat(mat, pt):
    x=mat[0][0]*pt[0]+mat[0][1]*pt[1]
    y=mat[1][0]*pt[0]+mat[1][1]*pt[1]
    pt[0]=x
    pt[1]=y
def norm(pt):
    return sqrt(pt[0]*pt[0]+pt[1]*pt[1])

def ArcToPath(p1,params):
    A=p1[:]
    rx,ry,teta,longflag,sweepflag,x2,y2=params[:]
    teta = teta*pi/180.0
    B=[x2,y2]
    if rx==0 or ry==0 or A==B:
        return([[A[:],A[:],A[:]],[B[:],B[:],B[:]]])
    mat=matprod((rotmat(teta),[[1/rx,0],[0,1/ry]],rotmat(-teta)))
    applymat(mat, A)
    applymat(mat, B)
    k=[-(B[1]-A[1]),B[0]-A[0]]
    d=k[0]*k[0]+k[1]*k[1]
    k[0]/=sqrt(d)
    k[1]/=sqrt(d)
    d=sqrt(max(0,1-d/4))
    if longflag==sweepflag:
        d*=-1
    O=[(B[0]+A[0])/2+d*k[0],(B[1]+A[1])/2+d*k[1]]
    OA=[A[0]-O[0],A[1]-O[1]]
    OB=[B[0]-O[0],B[1]-O[1]]
    start=acos(OA[0]/norm(OA))
    if OA[1]<0:
        start*=-1
    end=acos(OB[0]/norm(OB))
    if OB[1]<0:
        end*=-1

    if sweepflag and start>end:
        end +=2*pi
    if (not sweepflag) and start<end:
        end -=2*pi

    NbSectors=int(abs(start-end)*2/pi)+1
    dTeta=(end-start)/NbSectors
    #v=dTeta*2/pi*0.552
    #v=dTeta*2/pi*4*(sqrt(2)-1)/3
    v = 4*tan(dTeta/4)/3
    #if not sweepflag:
    #    v*=-1
    p=[]
    for i in range(0,NbSectors+1,1):
        angle=start+i*dTeta
        v1=[O[0]+cos(angle)-(-v)*sin(angle),O[1]+sin(angle)+(-v)*cos(angle)]
        pt=[O[0]+cos(angle)                ,O[1]+sin(angle)                ]
        v2=[O[0]+cos(angle)-  v *sin(angle),O[1]+sin(angle)+  v *cos(angle)]
        p.append([v1,pt,v2])
    p[ 0][0]=p[ 0][1][:]
    p[-1][2]=p[-1][1][:]

    mat=matprod((rotmat(teta),[[rx,0],[0,ry]],rotmat(-teta)))
    for pts in p:
        applymat(mat, pts[0])
        applymat(mat, pts[1])
        applymat(mat, pts[2])
    return(p)
    
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
            q0=last[:]
            q1=params[0:2]
            q2=params[2:4]
            x0=     q0[0]
            x1=1./3*q0[0]+2./3*q1[0]
            x2=           2./3*q1[0]+1./3*q2[0]
            x3=                           q2[0]
            y0=     q0[1]
            y1=1./3*q0[1]+2./3*q1[1]
            y2=           2./3*q1[1]+1./3*q2[1]
            y3=                           q2[1]
            csp[subpath].append([lastctrl[:],[x0,y0],[x1,y1]])
            last = [x3,y3]
            lastctrl = [x2,y2]
        elif cmd == 'A':
            arcp=ArcToPath(last[:],params[:])
            arcp[ 0][0]=lastctrl[:]
            last=arcp[-1][1]
            lastctrl = arcp[-1][0]
            csp[subpath]+=arcp[:-1]
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


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
