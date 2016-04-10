#!/usr/bin/env python
"""
    ffgeom.py
    Copyright (C) 2005 Aaron Cyril Spike, aaron@ekips.org

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
"""
import math
try:
    NaN = float('NaN')
except ValueError:
    PosInf = 1e300000
    NaN = PosInf/PosInf

class Point:
    precision = 5
    def __init__(self, x, y):
        self.__coordinates = {'x' : float(x), 'y' : float(y)}
    def __getitem__(self, key):
        return self.__coordinates[key]
    def __setitem__(self, key, value):
        self.__coordinates[key] = float(value)
    def __repr__(self):
        return '(%s, %s)' % (round(self['x'],self.precision),round(self['y'],self.precision))
    def copy(self):
        return Point(self['x'],self['y'])
    def translate(self, x, y):
        self['x'] += x
        self['y'] += y
    def move(self, x, y):
        self['x'] = float(x)
        self['y'] = float(y)

class Segment:
    def __init__(self, e0, e1):
        self.__endpoints = [e0, e1]
    def __getitem__(self, key):
        return self.__endpoints[key]
    def __setitem__(self, key, value):
        self.__endpoints[key] = value
    def __repr__(self):
        return repr(self.__endpoints)
    def copy(self):
        return Segment(self[0],self[1])
    def translate(self, x, y):
        self[0].translate(x,y)
        self[1].translate(x,y)
    def move(self,e0,e1):
        self[0] = e0
        self[1] = e1
    def delta_x(self):
        return self[1]['x'] - self[0]['x']
    def delta_y(self):
        return self[1]['y'] - self[0]['y']
    #alias functions
    run = delta_x
    rise = delta_y
    def slope(self):
        if self.delta_x() != 0:
            return self.delta_x() / self.delta_y()
        return NaN
    def intercept(self):
        if self.delta_x() != 0:
            return self[1]['y'] - (self[0]['x'] * self.slope())
        return NaN
    def distanceToPoint(self, p):
        s2 = Segment(self[0],p)
        c1 = dot(s2,self)
        if c1 <= 0:
            return Segment(p,self[0]).length()
        c2 = dot(self,self)
        if c2 <= c1:
            return Segment(p,self[1]).length()
        return self.perpDistanceToPoint(p)
    def perpDistanceToPoint(self, p):
        len = self.length()
        if len == 0: return NaN
        return math.fabs(((self[1]['x'] - self[0]['x']) * (self[0]['y'] - p['y'])) - \
            ((self[0]['x'] - p['x']) * (self[1]['y'] - self[0]['y']))) / len
    def angle(self):
        return math.pi * (math.atan2(self.delta_y(), self.delta_x())) / 180
    def length(self):
        return math.sqrt((self.delta_x() ** 2) + (self.delta_y() ** 2))
    def pointAtLength(self, len):
        if self.length() == 0: return Point(NaN, NaN)
        ratio = len / self.length()
        x = self[0]['x'] + (ratio * self.delta_x())
        y = self[0]['y'] + (ratio * self.delta_y())
        return Point(x, y)
    def pointAtRatio(self, ratio):
        if self.length() == 0: return Point(NaN, NaN)
        x = self[0]['x'] + (ratio * self.delta_x())
        y = self[0]['y'] + (ratio * self.delta_y())
        return Point(x, y)
    def createParallel(self, p):
        return Segment(Point(p['x'] + self.delta_x(), p['y'] + self.delta_y()), p)
    def intersect(self, s):
        return intersectSegments(self, s)

def intersectSegments(s1, s2):
    x1 = s1[0]['x']
    x2 = s1[1]['x']
    x3 = s2[0]['x']
    x4 = s2[1]['x']
    
    y1 = s1[0]['y']
    y2 = s1[1]['y']
    y3 = s2[0]['y']
    y4 = s2[1]['y']
    
    denom = ((y4 - y3) * (x2 - x1)) - ((x4 - x3) * (y2 - y1))
    num1 = ((x4 - x3) * (y1 - y3)) - ((y4 - y3) * (x1 - x3))
    num2 = ((x2 - x1) * (y1 - y3)) - ((y2 - y1) * (x1 - x3))

    num = num1

    if denom != 0: 
        x = x1 + ((num / denom) * (x2 - x1))
        y = y1 + ((num / denom) * (y2 - y1))
        return Point(x, y)
    return Point(NaN, NaN)

def dot(s1, s2):
    return s1.delta_x() * s2.delta_x() + s1.delta_y() * s2.delta_y()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
