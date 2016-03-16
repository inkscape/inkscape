#!/usr/bin/env python 
'''
Copyright (C) 2009 Karlisson Bezerra, contato@nerdson.com

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

import inkex

class Split(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-s", "--splittype", 
                        action="store", type="string", 
                        dest="split_type", default="word", 
                        help="type of split")
        self.OptionParser.add_option("-p", "--preserve", 
                        action="store", type="inkbool", 
                        dest="preserve", default="True", 
                        help="Preserve original")
        self.OptionParser.add_option("--tab",
                        action="store", type="string",
                        dest="tab",
                        help="The selected UI-tab when OK was pressed")

    def split_lines(self, node):
        """Returns a list of lines"""

        lines = []
        count = 1
        
        for n in node:
            if not (n.tag == inkex.addNS("flowPara", "svg") or n.tag == inkex.addNS("tspan", "svg")):
                if n.tag == inkex.addNS("textPath", "svg"):
                    inkex.debug("This type of text element isn't supported. First remove text from path.")
                    break
                else:
                    continue
           
            text = inkex.etree.Element(inkex.addNS("text", "svg"), node.attrib)
            
            #handling flowed text nodes
            if node.tag == inkex.addNS("flowRoot", "svg"):
                try:
                    from simplestyle import parseStyle
                    fontsize = parseStyle(node.get("style"))["font-size"]
                except:
                    fontsize = "12px"
                fs = self.unittouu(fontsize)
                
                #selects the flowRegion's child (svg:rect) to get @X and @Y
                id = node.get("id")
                flowref = self.xpathSingle('/svg:svg//*[@id="%s"]/svg:flowRegion[1]' % id)[0]
                
                if flowref.tag == inkex.addNS("rect", "svg"):
                    text.set("x", flowref.get("x"))
                    text.set("y", str(float(flowref.get("y")) + fs * count))
                    count += 1
                else:
                    inkex.debug("This type of text element isn't supported. First unflow text.")
                    break
                
                #now let's convert flowPara into tspan
                tspan = inkex.etree.Element(inkex.addNS("tspan", "svg"))
                tspan.set(inkex.addNS("role","sodipodi"), "line")
                tspan.text = n.text
                text.append(tspan)
            
            else:
                from copy import copy
                x = n.get("x") or node.get("x")
                y = n.get("y") or node.get("y")

                text.set("x", x)
                text.set("y", y)
                text.append(copy(n))
            
            lines.append(text)

        return lines


    def split_words(self, node):
        """Returns a list of words"""
        
        words = []

        #Function to recursively extract text
        def plain_str(elem):
            words = []
            if elem.text:
                words.append(elem.text)
            for n in elem:
                words.extend(plain_str(n))
                if n.tail:
                    words.append(n.tail)
            return words
        
        #if text has more than one line, iterates through elements
        lines = self.split_lines(node)
        if not lines:
            return words

        for line in lines:
            #gets the position of text node
            x = float(line.get("x"))
            y = line.get("y")
            
            #gets the font size. if element doesn't have a style attribute, it assumes font-size = 12px
            try:
                from simplestyle import parseStyle
                fontsize = parseStyle(line.get("style"))["font-size"]
            except:
                fontsize = "12px"
            fs = self.unittouu(fontsize)

            #extract and returns a list of words
            words_list = "".join(plain_str(line)).split()
            prev_len = 0
            
            #creates new text nodes for each string in words_list
            for word in words_list:
                tspan = inkex.etree.Element(inkex.addNS("tspan", "svg"))
                tspan.text = word
                
                text = inkex.etree.Element(inkex.addNS("text", "svg"), line.attrib)
                tspan.set(inkex.addNS("role","sodipodi"), "line")
                
                #positioning new text elements
                x = x + prev_len * fs
                prev_len = len(word)
                text.set("x", str(x))
                text.set("y", str(y))
                
                text.append(tspan)
                words.append(text)
        
        return words


    def split_letters(self, node):
        """Returns a list of letters"""

        letters = []
        
        words = self.split_words(node)
        if not words:
            return letters

        for word in words:
            
            x = float(word.get("x"))
            y = word.get("y")
           
            #gets the font size. If element doesn't have a style attribute, it assumes font-size = 12px
            try:
                import simplestyle
                fontsize = simplestyle.parseStyle(word.get("style"))["font-size"]
            except:
                fontsize = "12px"
            fs = self.unittouu(fontsize)

            #for each letter in element string
            for letter in word[0].text:
                tspan = inkex.etree.Element(inkex.addNS("tspan", "svg"))
                tspan.text = letter

                text = inkex.etree.Element(inkex.addNS("text", "svg"), node.attrib)
                text.set("x", str(x))
                text.set("y", str(y))
                x += fs
                
                text.append(tspan)
                letters.append(text)
        return letters


    def effect(self):
        """Applies the effect"""

        split_type = self.options.split_type
        preserve = self.options.preserve
       
        #checks if the selected elements are text nodes
        for id, node in self.selected.iteritems():
            if not (node.tag == inkex.addNS("text", "svg") or node.tag == inkex.addNS("flowRoot", "svg")):
                inkex.debug("Please select only text elements.")
                break
            else:
                if split_type == "line":
                    nodes = self.split_lines(node)
                elif split_type == "word":
                    nodes = self.split_words(node)
                elif split_type == "letter":
                    nodes = self.split_letters(node)
                
                for n in nodes:
                    node.getparent().append(n)
                        
                #preserve original element
                if not preserve and nodes:
                    parent = node.getparent()
                    parent.remove(node)

b = Split()
b.affect()
