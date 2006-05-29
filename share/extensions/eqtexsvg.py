#!/usr/bin/env python
# -*- coding: cp1252 -*-
"""
EQTEXSVG.py
functions for converting LATEX equation string into SVG path
This extension need, to work properly :
    - a TEX/LATEX distribution (MiKTEX ...)
    - pstoedit software: <http://www.pstoedit.net/pstoedit>

Copyright (C) 2006 Julien Vitard, julienvitard@gmail.com

- I will try to code XML parsing, not the hard way ;-)

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

import inkex, os, tempfile

def create_equation_tex(filename, equation):
    tex = open(filename, 'w')
    tex.write("""%% processed with EqTeXSVG.py
\documentclass{article}
    
\\thispagestyle{empty}
\\begin{document}
""")
    tex.write("$$\n")
    tex.write(equation)
    tex.write("\n$$\n")
    tex.write("\end{document}\n")
    tex.close()

def svg_open(self,filename):
    # parsing of SVG file with the equation 
    # real parsing XML to use!!!! it will be easier !!!
    svg = open(filename, 'r')
    svg_lines = svg.readlines()
    
    # trip top/bottom lines from svg file
    svg_lines.pop(0)
    svg_lines.pop(1)
    svg_lines.pop(len(svg_lines)-1)

    group = self.document.createElement('svg:g')
    self.current_layer.appendChild(group)

    # deleting "<g... >" "</g>" "<path d=" and "/>" from svg_lines
    nodegroup=''
    s_nodegroup_path=''
        
    for i in range(1,len(svg_lines)):
        if svg_lines[i].find("<g") != -1:
            nodegroup=svg_lines[i].split("<g")
            nodegroup=nodegroup[1].split(" >")
            nodegroup=nodegroup[0]+'\n'
        elif svg_lines[i].find("<path d=") != -1:
            s_nodegroup_path=svg_lines[i].split("<path d=")
            s_nodegroup_path=s_nodegroup_path[1]                
        elif svg_lines[i].find("/>") != -1:
            s_nodegroup_path=s_nodegroup_path+'"\n'
        elif svg_lines[i].find("</g>") != -1:
            nodegroup_svg = self.document.createElement('svg:g')
            nodegroup_svg.setAttribute('style',nodegroup)
            nodegroup_path = self.document.createElement('svg:path')
            nodegroup_path.setAttribute('d',s_nodegroup_path)
            group.appendChild(nodegroup_svg)                
            nodegroup_svg.appendChild(nodegroup_path)
        else:
            s_nodegroup_path=s_nodegroup_path+svg_lines[i]

class EQTEXSVG(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-f", "--formule",
                        action="store", type="string", 
                        dest="formule", default=10.0,
                        help="Formule LaTeX")
    def effect(self):
        
        base_file = os.path.join(tempfile.gettempdir(), "inkscape-latex.tmp")
        latex_file = base_file + ".tex"
        create_equation_tex(latex_file, self.options.formule)

        out_file = os.path.join(tempfile.gettempdir(), "inkscape-latex.tmp.output")
        os.system('latex -output-directory=' + tempfile.gettempdir() + ' ' + latex_file + '> ' + out_file)

        ps_file = base_file + ".ps"
        dvi_file = base_file + ".dvi"
        svg_file = base_file + ".svg"
        os.system('dvips -q -f -E -D 600 -y 5000 -o ' + ps_file + ' ' + dvi_file)
        os.system('pstoedit -f svg -dt -ssp ' + ps_file + ' ' + svg_file + '>> ' + out_file)

        # ouvrir le svg et remplacer #7F7F7F par #000000
        svg_open(self, svg_file)

        # clean up
        aux_file = base_file + ".aux"
        log_file = base_file + ".log"
        os.remove(latex_file)
        os.remove(aux_file)
        os.remove(log_file)
        os.remove(dvi_file)
        os.remove(ps_file)
        os.remove(svg_file)
        os.remove(out_file)
        
e = EQTEXSVG()
e.affect()
