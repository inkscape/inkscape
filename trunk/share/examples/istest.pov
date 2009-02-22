/*#########################################################################
##
## File:  itest.pov
## 
## Authors:
##   Bob Jamison
##
## Copyright (C) 2004-2007 The Inkscape Organization
##
## Released under GNU GPL, read the file 'COPYING' for more information
##
###########################################################################   
##
## Notes:
##
## 070312: Tested on PovRay 3.7.0 beta
##
###########################################################################
##
##  This simple file is provided to demonstrate POV output from Inkscape.
##  PovRay output is intended for people who have had moderate experience
##  with authoring POV files.  This is NOT for beginners.
##
##  To use:
##  1)  Install PovRay, version 3.5 or above, and put the povray executable
##      in your PATH.  PovRay is found at http://www.povray.org .  For
##      PovRay-specific questions, please look there.  They are the experts.
##
##  2)  Copy this file to a working area.
##
##  3)  Make or load a document in Inkscape with some shapes in it.
##
##  4)  Save as a .pov file using the SaveAs dialog.  For this example, save
##      it in the same directory as this file, with the name 'isshapes.pov'
##
##  5)  Execute povray with this file.  An example command would be:
##
##  povray +X +V +A +W320 +H320 +Iistest.pov +FN
##
##  6)  Adjust the values to suit your needs and desires.  Have fun.
##
##
##  Remember that this is not the intended use of Inkscape's POVRay
##  output, but is merely a demo.  The main purpose of POVRay output
##  is to supply curves and other data to your own POVRay projects.
##
#########################################################################*/

/*#########################################################################
# Some standard PovRay scene additives
#########################################################################*/

#include "colors.inc"
#include "textures.inc"
#include "shapes.inc"
#include "metals.inc"
#include "skies.inc"

/*#########################################################################
# Note the Z-order differentation of the prisms in the object is
# normally just sufficient to display all of the objects, and
# avoid 'black spots' where surfaces are coincident.  This can
# be adjusted by changing the value below.
#########################################################################*/

/*
#declare AllShapes_Z_Increment = 0.008;
*/


/*#########################################################################
# Note that the finish of the "all shapes" object exported
# at the end of the shapes file can be modified, by defining
# if before the #include.  Uncomment the following declaration
# to adjust it.  Have fun.
#########################################################################*/

/*
#declare AllShapes_Finish = finish {
           phong 0.7
           reflection 0.5
           specular 0.8
       }
*/

/*#########################################################################
# Our Inkscape-exported shapes file
#########################################################################*/

#include "isshapes.pov"

/*#########################################################################
# Move the camera back in the -Z direction, about 1.5 times the width
# or height of the image.  This will provide about a 60-degree view.
# 'AllShapes' is an item in isshapes.pov, and refers to the union of
# all of the shapes exported.
#########################################################################*/

camera {
    location <0, 0, -(AllShapes_WIDTH * 1.5)>
    look_at  <0, 0, 0>
    right x*image_width/image_height
}

/*#########################################################################
# Put one or two lights in front of the objects, and at an angle to
# the viewer.
#########################################################################*/

light_source { <-200,   1, -8000> color White}
light_source { < 200, 100,  -600> color White}


/*#########################################################################
# Make a pretty background, for contrast
#########################################################################*/

sky_sphere {
    pigment {
      gradient y
      color_map {
        [ 0.5  color CornflowerBlue ]
        [ 1.0  color MidnightBlue   ]
      }
      scale 1000
      translate -1
    }
  }


/*#########################################################################
# Now let us use our shapes.  We can include them individually, or include
# them as a group.  Notice that we have two AllShapes.  One plain, and
# an AllShapesZ.  The 'Z' version is different in that the shapes are
# shifted slightly higher in their order of creation, so that coincident
# shapes can be discerned.
#########################################################################*/


object {
    /*
    //## Individually
    union{
    object { droplet01       }
    object { droplet02       }
    object { droplet03       }
    object { mountainDroplet }
    }
    */

    //## As a group
    object { AllShapes_Z }


    translate<-AllShapes_CENTER_X, 0, -AllShapes_CENTER_Y>
    scale  <  1,  60,   1>
    rotate <-90,   0,   0>   //x first
    rotate <  0,   0,   0>   //z second
    rotate < 20,   0,   0>   //whatever else
    rotate <  0, -27,   0>   //whatever else

}//object



/*#########################################################################
# End of File
#########################################################################*/






