#!/usr/bin/env python
import chardataeffect, inkex, string

class C(chardataeffect.CharDataEffect):
  def process_chardata(self,text, line=False, par=False):
    return text.upper()

c = C()
c.affect()
