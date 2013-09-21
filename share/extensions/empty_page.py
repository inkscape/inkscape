#!/usr/bin/env python

import inkex

class C(inkex.Effect):
  def __init__(self):
    inkex.Effect.__init__(self)
    self.OptionParser.add_option("-s", "--size", action="store", type="string", dest="page_size", default="a4", help="Page size")
    self.OptionParser.add_option("-o", "--orientation", action="store", type="string", dest="page_orientation", default="vertical", help="Page orientation")

  def effect(self): 
    root = self.document.getroot()
    root.set("width", "12in")
    root.set("height", "12in")
    if self.options.page_size == "a4" and self.options.page_orientation == "vertical":
        root.set("width", "210mm")
        root.set("height", "297mm")
    if self.options.page_size == "a4" and self.options.page_orientation == "horizontal":
        root.set("height", "210mm")
        root.set("width", "297mm")
        
    if self.options.page_size == "a5" and self.options.page_orientation == "vertical":
        root.set("width", "148mm")
        root.set("height", "210mm")
    if self.options.page_size == "a5" and self.options.page_orientation == "horizontal":
        root.set("width", "210mm")
        root.set("height", "148mm")
        
    if self.options.page_size == "a3" and self.options.page_orientation == "vertical":
        root.set("width", "297mm")
        root.set("height", "420mm")
    if self.options.page_size == "a3" and self.options.page_orientation == "horizontal":
        root.set("width", "420mm")
        root.set("height", "297mm")
        
    if self.options.page_size == "letter" and self.options.page_orientation == "vertical":
        root.set("width", "8.5in")
        root.set("height", "11in")
    if self.options.page_size == "letter" and self.options.page_orientation == "horizontal":
        root.set("width", "11in")
        root.set("height", "8.5in")

c = C()
c.affect()
