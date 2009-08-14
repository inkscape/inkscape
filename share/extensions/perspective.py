#!/usr/bin/env python
"""
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

Perspective approach & math by Dmitry Platonov, shadowjack@mail.ru, 2006
"""
import sys, inkex, os, re, simplepath, cubicsuperpath 
import gettext
_ = gettext.gettext
from ffgeom import *
try:
    from numpy import *
    from numpy.linalg import *
except:
    inkex.errormsg(_("Failed to import the numpy or numpy.linalg modules. These modules are required by this extension. Please install them and try again.  On a Debian-like system this can be done with the command, sudo apt-get install python-numpy."))
    exit()

try:
    from subprocess import Popen, PIPE
    bsubprocess = True
except:
    bsubprocess = False

uuconv = {'in':90.0, 'pt':1.25, 'px':1, 'mm':3.5433070866, 'cm':35.433070866, 'pc':15.0}
def unittouu(string):
    unit = re.compile('(%s)$' % '|'.join(uuconv.keys()))
    param = re.compile(r'(([-+]?[0-9]+(\.[0-9]*)?|[-+]?\.[0-9]+)([eE][-+]?[0-9]+)?)')

    p = param.match(string)
    u = unit.search(string)    
    if p:
        retval = float(p.string[p.start():p.end()])
    else:
        retval = 0.0
    if u:
        try:
            return retval * uuconv[u.string[u.start():u.end()]]
        except KeyError:
            pass
    return retval

class Project(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
    def effect(self):
        if len(self.options.ids) < 2:
            inkex.errormsg(_("This extension requires two selected paths."))
            exit()            
            
        #obj is selected second
        obj = self.selected[self.options.ids[0]]
        envelope = self.selected[self.options.ids[1]]
        if obj.get(inkex.addNS('type','sodipodi')):
            inkex.errormsg(_("The first selected object is of type '%s'.\nTry using the procedure Path->Object to Path." % obj.get(inkex.addNS('type','sodipodi'))))
            exit()
        if obj.tag == inkex.addNS('path','svg') or obj.tag == inkex.addNS('g','svg'):
            if envelope.tag == inkex.addNS('path','svg'):
                path = cubicsuperpath.parsePath(envelope.get('d'))
                if len(path) < 1 or len(path[0]) < 4:
                    inkex.errormsg(_("This extension requires that the second selected path be four nodes long."))
                    exit()
                dp = zeros((4,2), dtype=float64)
                for i in range(4):
                    dp[i][0] = path[0][i][1][0]
                    dp[i][1] = path[0][i][1][1]

                #query inkscape about the bounding box of obj
                q = {'x':0,'y':0,'width':0,'height':0}
                file = self.args[-1]
                id = self.options.ids[0]
                for query in q.keys():
                    if bsubprocess:
                        p = Popen('inkscape --query-%s --query-id=%s "%s"' % (query,id,file), shell=True, stdout=PIPE, stderr=PIPE)
                        rc = p.wait()
                        q[query] = float(p.stdout.read())
                        err = p.stderr.read()
                    else:
                        f,err = os.popen3('inkscape --query-%s --query-id=%s "%s"' % (query,id,file))[1:]
                        q[query] = float(f.read())
                        f.close()
                        err.close()
                sp = array([[q['x'], q['y']+q['height']],[q['x'], q['y']],[q['x']+q['width'], q['y']],[q['x']+q['width'], q['y']+q['height']]], dtype=float64)
            else:
                if envelope.tag == inkex.addNS('g','svg'):
                    inkex.errormsg(_("The second selected object is a group, not a path.\nTry using the procedure Object->Ungroup."))
                else:
                    inkex.errormsg(_("The second selected object is not a path.\nTry using the procedure Path->Object to Path."))
                exit()
        else:
            inkex.errormsg(_("The first selected object is not a path.\nTry using the procedure Path->Object to Path."))
            exit()

        solmatrix = zeros((8,8), dtype=float64)
        free_term = zeros((8), dtype=float64)
        for i in (0,1,2,3):
            solmatrix[i][0] = sp[i][0]
            solmatrix[i][1] = sp[i][1]
            solmatrix[i][2] = 1
            solmatrix[i][6] = -dp[i][0]*sp[i][0]
            solmatrix[i][7] = -dp[i][0]*sp[i][1]
            solmatrix[i+4][3] = sp[i][0]
            solmatrix[i+4][4] = sp[i][1]
            solmatrix[i+4][5] = 1
            solmatrix[i+4][6] = -dp[i][1]*sp[i][0]
            solmatrix[i+4][7] = -dp[i][1]*sp[i][1]
            free_term[i] = dp[i][0]
            free_term[i+4] = dp[i][1]

        res = solve(solmatrix, free_term)
        projmatrix = array([[res[0],res[1],res[2]],[res[3],res[4],res[5]],[res[6],res[7],1.0]],dtype=float64)
        if obj.tag == inkex.addNS("path",'svg'):
            self.process_path(obj,projmatrix)
        if obj.tag == inkex.addNS("g",'svg'):
            self.process_group(obj,projmatrix)


    def process_group(self,group,m):
        for node in group:
            if node.tag == inkex.addNS('path','svg'):
                self.process_path(node,m)
            if node.tag == inkex.addNS('g','svg'):
                self.process_group(node,m)    


    def process_path(self,path,m):
        d = path.get('d')
        p = cubicsuperpath.parsePath(d)
        for subs in p:
            for csp in subs:
                csp[0] = self.project_point(csp[0],m)
                csp[1] = self.project_point(csp[1],m)
                csp[2] = self.project_point(csp[2],m)
        path.set('d',cubicsuperpath.formatPath(p))



    def project_point(self,p,m):
        x = p[0]
        y = p[1]
        return [(x*m[0][0] + y*m[0][1] + m[0][2])/(x*m[2][0]+y*m[2][1]+m[2][2]),(x*m[1][0] + y*m[1][1] + m[1][2])/(x*m[2][0]+y*m[2][1]+m[2][2])]

if __name__ == '__main__':
    e = Project()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 encoding=utf-8 textwidth=99
