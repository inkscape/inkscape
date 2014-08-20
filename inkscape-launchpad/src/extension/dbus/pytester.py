#####################################################################
# Python test script for Inkscape DBus API.
#
# Contains many examples of functions and various stress tests etc.
# Multiple tests can be run at once but the output will be a bit chaotic.
# List of test functions can be found at the bottom of the script. 
#####################################################################

import dbus
import random

#####################################################################
# Various test functions, called at bottom of script
#####################################################################

def randomrect (document):
  document.rectangle( random.randint(0,1000), 
                      random.randint(0,1000),
                      random.randint(1,100),
                      random.randint(1,100))

def lottarects ( document ):
  document.pause_updates()
  listy = []
  for x in range(1,2000):
    if x == 1000:
      print "HALFWAY"
    if x == 1:
      print "BEGUN"
    document.rectangle( 0, 0, 100, 100)
    #randomrect(document)
  print "DONE"
  for x in listy:
    print x
    selection_set(x)
  document.resume_updates()
  
def lottaverbs (doc):
  doc.pause_updates()
  doc.document_set_css ("fill:#ff0000;fill-opacity:.5;stroke:#0000ff;stroke-width:5;stroke-miterlimit:4;stroke-opacity:1;stroke-dasharray:none")
  doc.rectangle( 0, 0, 100, 100)
  doc.select_all()
  doc.selection_copy()
  for x in range(1,2000):
    if x == 1000:
      print "HALFWAY"
    if x == 1:
      print "BEGUN"
    doc.selection_paste()
    #doc.rectangle( 0, 0, 100, 100)
  doc.resume_updates()
  
def testDrawing (doc):
  doc.document_set_css ("fill:#000000;fill-opacity:.5;stroke:#000000;stroke-width:5;stroke-miterlimit:4;stroke-opacity:1;stroke-dasharray:none")
  doc.ellipse( 0, 0, 100, 100)
  doc.select_all()
  doc.selection_copy()
  for x in range(1,2000):
    if x == 1000:
      print "HALFWAY"
    if x == 1:
      print "BEGUN"
    doc.selection_paste()
    newrect = doc.selection_get()[0]
    doc.set_color(newrect, 255 - x%255, 0, 200, True)
    doc.set_color(newrect, 0, 255 - x%75, x%75, False)
    doc.mark_as_unmodified()


def testcopypaste (document ):
  #document.pause_updates()
  print document.rectangle (400, 500, 100, 100)
  print document.rectangle (200, 200, 100, 100)
  document.select_all()
  document.selection_copy()
  document.selection_paste()
  #document.resume_updates()

def testShapes (doc):
  doc.rectangle (0, 0, 100, 100)
  doc.ellipse (100, 100, 100, 100)
  doc.star (250, 250, 50, 25, 5, False, .9, 1.4)
  doc.polygon (350, 350, 50, 0, 5)
  doc.line (400,400,500,500)
  doc.spiral (550,550,50,3)

def testMovement (doc):
  rect1 = doc.rectangle (0, 0, 100, 100)
  rect2 = doc.rectangle (0, 100, 100, 100)
  rect3 = doc.rectangle (0, 200, 100, 100)
  doc.select_all()
  doc.move(rect2, 100,100)
  
def testImport (doc):
  # CHANGE TO LOCAL SVG FILE!
  img1 = doc.image(0,0, "/home/soren/chavatar.jpg")
  doc.selection_add(img1)
  doc.selection_scale(500)
  doc.transform(img1, "rotate(30)")

def testSelections (doc):
  rect1 = doc.rectangle (0, 0, 100, 100)
  rect2 = doc.rectangle (0, 100, 100, 100)
  rect3 = doc.rectangle (0, 200, 100, 100)
  rect4 = doc.rectangle (0, 300, 100, 100)

  doc.selection_add (rect1)
  center = doc.selection_get_center()
  for d in center:
    print d
  doc.selection_to_path()
  doc.get_path(rect1)
  doc.selection_move(100.0, 100.0)
  doc.selection_set(rect2)
  doc.selection_move_to(0.0,0.0)
  doc.selection_set(rect3)
  doc.move(rect4, 500.0, 500.0)
  doc.select_all()
  doc.selection_to_path()
  result = doc.selection_get()
  print len(result)
  for d in result:
    print d

def testLevels (doc):
  rect1 = doc.rectangle (0, 0, 100, 100)
  rect2 = doc.rectangle (20, 20, 100, 100)
  rect3 = doc.rectangle (40, 40, 100, 100)
  rect4 = doc.rectangle (60, 60, 100, 100)

  doc.selection_set(rect1)
  doc.selection_change_level("raise")

  doc.selection_set(rect4)
  doc.selection_change_level("to_bottom")

def testCombinations (doc):
  rect1 = doc.rectangle (0, 0, 100, 100)
  rect2 = doc.rectangle (20, 20, 100, 100)
  rect3 = doc.rectangle (40, 40, 100, 100)
  rect4 = doc.rectangle (60, 60, 100, 100)
  rect5 = doc.rectangle (80, 80, 100, 100)
  rect6 = doc.rectangle (100, 100, 100, 100)
  rect7 = doc.rectangle (120, 120, 100, 100)
  rect8 = doc.rectangle (140, 140, 100, 100)
  rect9 = doc.rectangle (160, 160, 100, 100)
  rect10 = doc.rectangle (180, 180, 100, 100)

  doc.selection_set_list([rect1, rect2])
  print doc.selection_combine("union")
  doc.selection_set_list([rect3, rect4])
  print doc.selection_combine("intersection")
  doc.selection_set_list([rect5, rect6])
  print doc.selection_combine("difference")
  doc.selection_set_list([rect7, rect8])
  print doc.selection_combine("exclusion")
  doc.selection_set_list([rect9, rect10])
  for d in doc.selection_divide():
    print d

def testTransforms (doc):
  rect1 = doc.rectangle (0, 0, 100, 100)
  rect2 = doc.rectangle (20, 20, 100, 100)
  doc.set_attribute(rect1, "transform", "matrix(0.08881734,0.94288151,-0.99604793,0.68505564,245.36153,118.60315)")
  doc.selection_set(rect1)

  doc.selection_move_to(200, 200)

def testLayer (doc):
  rect1 = doc.rectangle (0, 0, 100, 100)
  print doc.new_layer()
  rect2 = doc.rectangle (20, 20, 100, 100)

def testGetSelection (doc):
  rect1 = doc.rectangle (0, 0, 100, 100)
  rect2 = doc.rectangle (20, 20, 100, 100)
  rect3 = doc.rectangle (40, 40, 100, 100)
  doc.select_all()
  result = doc.selection_get()
  print result
  print len(result)
  for d in result:
    print d


def testDocStyle (doc):
  rect1 = doc.rectangle (0, 0, 100, 100)
  doc.document_set_css ("fill:#ff0000;fill-opacity:.5;stroke:#0000ff;stroke-width:5;stroke-miterlimit:4;stroke-opacity:1;stroke-dasharray:none")
  rect2 = doc.rectangle (20, 20, 100, 100)
  doc.document_set_css ("fill:#ffff00;fill-opacity:1;stroke:#009002;stroke-width:5;stroke-miterlimit:4;stroke-opacity:1;stroke-dasharray:none")
  rect3 = doc.rectangle (40, 40, 100, 100)
  doc.document_set_css ("fill:#00ff00;fill-opacity:1")
  rect4 = doc.rectangle (60, 60, 100, 100)

def testStyle (doc):
  doc.document_set_css ("fill:#ffff00;fill-opacity:1;stroke:#009002;stroke-width:5;stroke-miterlimit:4;stroke-opacity:1;stroke-dasharray:none")
  rect1 = doc.rectangle (0, 0, 100, 100)
  rect2 = doc.rectangle (20, 20, 100, 100)
  rect3 = doc.rectangle (40, 40, 100, 100)
  rect4 = doc.rectangle (60, 60, 100, 100)

  doc.modify_css (rect3, "fill-opacity", ".5")
  doc.merge_css (rect4, "fill:#0000ff;fill-opacity:.25;")
  print doc.get_css (rect4)

def testLayers (doc):
  rect1 = doc.rectangle (0, 0, 100, 100)
  layer1 = doc.layer_new()
  layer2 = doc.layer_new()
  rect2 = doc.rectangle (20, 20, 100, 100)
  rect3 = doc.rectangle (40, 40, 100, 100)
  doc.selection_add(rect3)
  doc.selection_move_to_layer(layer1)

def testLoadSave (doc):
  doc.load("/home/soren/testfile.svg")
  rect2 = doc.rectangle (0, 0, 200, 200)
  doc.save_as("/home/soren/testsave.svg")
  rect1 = doc.rectangle (20, 20, 200, 200)
  doc.save()
  rect3 = doc.rectangle (40, 40, 200, 200)
  doc.save_as("/home/soren/testsave2.svg")

def testArray (doc):
  rect1 = doc.rectangle (0, 0, 100, 100)
  rect2 = doc.rectangle (20, 20, 100, 100)
  rect3 = doc.rectangle (40, 40, 100, 100)
  doc.selection_set_list([rect1, rect2, rect3])

def testPath (doc):
  cr1 = doc.ellipse(0,0,50,50)
  print doc.get_path(cr1)
  doc.object_to_path(cr1)
  print doc.get_path(cr1)
  #doc.get_node_coordinates(cr1)

# Needs work.
def testText(doc):
  print doc.text(200, 200, "svg:text")
  

#####################################################################
# Setup bus connection, create documents.
#####################################################################

# Connect to bus
bus = dbus.SessionBus()

# Get interface for default document 
inkdoc1 = bus.get_object('org.inkscape', '/org/inkscape/desktop_0')
doc1 = dbus.Interface(inkdoc1, dbus_interface="org.inkscape.document")

# Create new window and get the interface for that. (optional)
inkapp = bus.get_object('org.inkscape',
                       '/org/inkscape/application')
desk2 = inkapp.desktop_new(dbus_interface='org.inkscape.application')
inkdoc2 = bus.get_object('org.inkscape', desk2)
doc2 = dbus.Interface(inkdoc2, dbus_interface="org.inkscape.document")


#####################################################################
# Call desired test functions
#####################################################################

#lottaverbs (doc1)
#lottarects (doc1)
#testDrawing (doc1)

#doc1.pause_updates()

testShapes(doc1)
#testMovement(doc1)
#testImport(doc1) # EDIT FUNCTION TO OPEN EXISTING FILE!
#testcopypaste (doc1)
#testTransforms (doc1)
#testDocStyle(doc1)
#testLayers(doc1)
#testLoadSave(doc1)
#testArray(doc1)
#testSelections(doc1)
#testCombinations(doc1)
#testText(doc1)
#testPath(doc1)

#doc1.resume_updates


# Prevents asking if you want to save when closing document.
doc1.mark_as_unmodified()

