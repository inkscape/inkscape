import coloreffect

import inkex

class C(coloreffect.ColorEffect):
  def __init__(self):
    coloreffect.ColorEffect.__init__(self)
    self.OptionParser.add_option("-f", "--from_color", action="store", type="string", dest="from_color", default="000000", help="Replace color")
    self.OptionParser.add_option("-t", "--to_color", action="store", type="string", dest="to_color", default="000000", help="By color")

  def colmod(self,r,g,b):
    this_color = '%02x%02x%02x' % (r, g, b)

    if self.options.from_color[0] == '"':
       self.options.from_color =  self.options.from_color[1:]
    if self.options.from_color[0] == '#':
       self.options.from_color =  self.options.from_color[1:]
    if self.options.from_color[-1] == '"':
       self.options.from_color =  self.options.from_color[:-1]
    if self.options.to_color[0] == '"':
       self.options.to_color =  self.options.to_color[1:]
    if self.options.to_color[0] == '#':
       self.options.to_color =  self.options.to_color[1:]
    if self.options.to_color[-1] == '"':
       self.options.to_color =  self.options.to_color[:-1]
       
    #inkex.debug(this_color+"|"+self.options.from_color)
    if this_color == self.options.from_color:
      return self.options.to_color
    else:
      return this_color

c = C()
c.affect()