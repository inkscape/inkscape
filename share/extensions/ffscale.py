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
import math

def ETScale(tones, octave=2.0):
    octave, tones = float(octave), float(tones)
    scale = {'steps':[[1.0,1.0]], 'title':'', 'errors':0, 'errorstring':''}
    if not tones:
        scale['errors'] += 1
        scale['errorstring'] = 'Error: Number of tones must be non zero!'
    else:    
        ratio = octave**(1/tones)
        scale['title'] = '%s root of %s Equal Temperament' % (tones, octave)
        scale['steps'].append([ratio,1])
    return scale

def ScalaScale(scala):
    #initial step 0 or 1/1 is implicit
    scale = {'steps':[[1.0,1.0]], 'title':'', 'errors':0, 'errorstring':''}

    #split scale discarding commments
    lines = [l.strip() for l in scala.strip().splitlines() if not l.strip().startswith('!')]

    #first line may be blank and contains the title
    scale['title'] =  lines.pop(0)

    #second line indicates the number of note lines that should follow
    expected = int(lines.pop(0))

    #discard blank lines and anything following whitespace    
    lines = [l.split()[0] for l in lines if l != '']
    
    if len(lines) != expected:
        scale['errors'] += 1
        scale['errorstring'] = 'Error: expected %s more tones but found %s!' % (expected,len(lines))
    else:
        for l in lines:
            #interpret anyline containing a dot as cents
            if l.find('.') >= 0:
                num = 2**(float(l)/1200)
                denom = 1
            #everything else is a ratio
            elif l.find('/') >=0:
                l = l.split('/')
                num = float(int(l[0]))
                denom = float(int(l[1]))
            else:
                num = float(int(l))
                denom = 1.0
            scale['steps'].append([num,denom])
            
            if (num < 0) ^ (denom <= 0):
                scale['errors'] += 1
                scale['errorstring'] += 'Error at "'+l+'": Negative and undefined ratios are not allowed!\n'
    return scale
