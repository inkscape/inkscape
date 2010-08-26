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

  def _hexstr(self,r,g,b):
    return '%02x%02x%02x' % (int(round(r*255)),int(round(g*255)),int(round(b*255)))
  
  def colmod(self,_r,_g,_b):
    r=float(_r)/255
    g=float(_g)/255
    b=float(_b)/255
    
    # add stuff to be accessible from within the custom color function here.
    safeenv = {'__builtins__':{},'r':r,'g':g,'b':b}

    try:
      r2=self.normalize(eval(self.options.rFunction,safeenv))
      g2=self.normalize(eval(self.options.gFunction,safeenv))
      b2=self.normalize(eval(self.options.bFunction,safeenv))
    except:
      return self._hexstr(1.0,0.0,0.0)
    return self._hexstr(r2,g2,b2)

c = C()
c.affect()
