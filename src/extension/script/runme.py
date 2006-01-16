
import inkscape_py

inkscape = inkscape_py.getInkscape()
desktop  = inkscape.getDesktop()
document = desktop.getDocument()
document.hello()

