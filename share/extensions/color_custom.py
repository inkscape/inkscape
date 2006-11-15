import coloreffect

class C(coloreffect.ColorEffect):
  def __init__(self):
    coloreffect.ColorEffect.__init__(self)
    self.OptionParser.add_option("--r", action="store", type="string", dest="rFunction", default="r",help="red channel function")
    self.OptionParser.add_option("--g", action="store", type="string", dest="gFunction", default="g",help="green channel function")
    self.OptionParser.add_option("--b", action="store", type="string", dest="bFunction", default="b",help="blue channel function")
  def normalize(self, v):
    if v<0:
      return 0.0
    if v>1:
      return 1.0
    return v
  def colmod(self,_r,_g,_b):
    r=float(_r)/255
    g=float(_g)/255
    b=float(_b)/255
    #coloreffect.debug('I: %f %f %f' % (r,g,b))
    r=eval(self.options.rFunction)
    g=eval(self.options.gFunction)
    b=eval(self.options.bFunction)
    #coloreffect.debug('E: %f %f %f' % (r,g,b))
    r=self.normalize(r)
    g=self.normalize(g)
    b=self.normalize(b)
    #coloreffect.debug('N: %f %f %f' % (r,g,b))
    return '%02x%02x%02x' % (int(round(r*255)),int(round(g*255)),int(round(b*255)))

c = C()
c.affect()