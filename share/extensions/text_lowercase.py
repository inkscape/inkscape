#!/usr/bin/env python
import chardataeffect, inkex, string

class C(chardataeffect.CharDataEffect):
  def process_chardata(self,text, line=False, par=False):
    return text.lower()

c = C()
c.affect()
