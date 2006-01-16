/** \file
 * Inkscape::Extension::Internal::GimpGrad implementation
 */

/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2004-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <color-rgba.h>
#include "io/sys.h"
#include "extension/system.h"

#include "gimpgrad.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

/**
    \brief  A function to allocated anything -- just an example here
    \param  module  Unused
    \return Whether the load was sucessful
*/
bool
GimpGrad::load (Inkscape::Extension::Extension *module)
{
    // std::cout << "Hey, I'm loading!\n" << std::endl;
    return TRUE;
}

/**
    \brief  A function to remove what was allocated
    \param  module  Unused
    \return None
*/
void
GimpGrad::unload (Inkscape::Extension::Extension *module)
{
    // std::cout << "Nooo! I'm being unloaded!" << std::endl;
    return;
}

/**
    \brief  A function to turn a color into a gradient stop
    \param  in_color  The color for the stop
    \param  location  Where the stop is placed in the gradient
    \return The text that is the stop.  Full SVG containing the element.

    This function encapsulates all of the translation of the ColorRGBA
    and the location into the gradient.  It is really pretty simple except
    that the ColorRGBA is in floats that are 0 to 1 and the SVG wants
    hex values from 0 to 255 for color.  Otherwise mostly this is just
    turning the values into strings and returning it.
*/
Glib::ustring
GimpGrad::new_stop (ColorRGBA in_color, float location)
{
    char temp_string[25];
    Glib::ustring mystring("<stop style=\"stop-color:#");

    for (int i = 0; i < 3; i++) {
        unsigned char temp;

        temp = (unsigned char)(in_color[i] * 255.0);

        sprintf(temp_string, "%2.2X", temp);
        mystring += temp_string;
    }

    mystring += ";stop-opacity:";
    sprintf(temp_string, "%1.8f", in_color[3]);
    mystring += temp_string;
    mystring += ";\" offset=\"";
    sprintf(temp_string, "%1.8f", location);
    mystring += temp_string;
    mystring += "\"/>\n";
    return mystring;
}

/**
    \brief  Actually open the gradient and turn it into an SPDocument
    \param  module    The input module being used
    \param  filename  The filename of the gradient to be opened
    \return A Document with the gradient in it.

    GIMP gradients are pretty simple (atleast the newer format, this
    function does not handle the old one yet).  They start out with
    the like "GIMP Gradient", then name it, and tell how many entries
    there are.  This function currently ignores the name and the number
    of entries just reading until it fails.

    The other small piece of trickery here is that GIMP gradients define
    a left possition, right possition and middle possition.  SVG gradients
    have no middle possition in them.  In order to handle this case the
    left and right colors are averaged in a linear manner and the middle
    possition is used for that color.

    That is another point, the GIMP gradients support many different types
    of gradients -- linear being the most simple.  This plugin assumes
    that they are all linear.  Most GIMP gradients are done this way,
    but it is possible to encounter more complex ones -- which won't be
    handled correctly.

    The one optimization that this plugin makes that if the right side
    of the previous segment is the same color as the left side of the
    current segment, then the second one is dropped.  This is often
    done in GIMP gradients and they are not necissary in SVG.

    What this function does is build up an SVG document with a single
    linear gradient in it with all the stops of the colors in the GIMP
    gradient that is passed in.  This document is then turned into a
    document using the \c sp_document_from_mem.  That is then returned
    to Inkscape.
*/
SPDocument *
GimpGrad::open (Inkscape::Extension::Input *module, gchar const *filename)
{
    FILE * gradient;
    // std::cout << "Open filename: " << filename << std::endl;

    Inkscape::IO::dump_fopen_call(filename, "I");
    gradient = Inkscape::IO::fopen_utf8name(filename, "r");
    if (gradient == NULL) return NULL;

    char tempstr[1024];
    if (fgets(tempstr, 1024, gradient) == 0) {
        // std::cout << "Seems that the read failed" << std::endl;
        fclose(gradient);
        return NULL;
    }

    if (!strcmp(tempstr, "GIMP Gradient")) {
        // std::cout << "This doesn't appear to be a GIMP gradient" << std::endl;
        fclose(gradient);
        return NULL;
    }

    if (fgets(tempstr, 1024, gradient) == 0) {
        // std::cout << "Seems that the second read failed" << std::endl;
        fclose(gradient);
        return NULL;
    }

    if (fgets(tempstr, 1024, gradient) == 0) {
        // std::cout << "Seems that the third read failed" << std::endl;
        fclose(gradient);
        return NULL;
    }

    ColorRGBA last_color(-1.0, -1.0, -1.0, -1.0);
    float lastlocation = -1.0;
    Glib::ustring outsvg("<svg><defs><linearGradient>\n");
    while (fgets(tempstr, 1024, gradient) != 0) {
        float left, middle, right;
        float temp_color[4];
        int type;
        int color;
        gchar * end;

        left = g_ascii_strtod(tempstr, &end);
        middle = g_ascii_strtod(end, &end);
        right = g_ascii_strtod(end, &end);

        for (int i = 0; i < 4; i++) {
            temp_color[i] = g_ascii_strtod(end, &end);
        }
        ColorRGBA leftcolor(temp_color[0], temp_color[1], temp_color[2], temp_color[3]);

        for (int i = 0; i < 4; i++) {
            temp_color[i] = g_ascii_strtod(end, &end);
        }
        ColorRGBA rightcolor(temp_color[0], temp_color[1], temp_color[2], temp_color[3]);

        sscanf(end, "%d %d", &type, &color);

        if (!(last_color == leftcolor) || left != lastlocation) {
            outsvg += new_stop(leftcolor, left);
        }
        outsvg += new_stop(leftcolor.average(rightcolor), middle);
        outsvg += new_stop(rightcolor, right);

        last_color = rightcolor;
        lastlocation = right;
    }

    outsvg += "</linearGradient></defs></svg>";

    // std::cout << "SVG Output: " << outsvg << std::endl;

    fclose(gradient);

    return sp_document_new_from_mem(outsvg.c_str(), outsvg.length(), TRUE);
}

void
GimpGrad::init (void)
{
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension>\n"
            "<name>GIMP Gradients</name>\n"
            "<id>org.inkscape.input.gimpgrad</id>\n"
            "<dependency type=\"plugin\" location=\"plugins\">gimpgrad</dependency>\n"
            "<input>\n"
                "<extension>.ggr</extension>\n"
                "<mimetype>application/x-gimp-gradient</mimetype>\n"
                "<filetypename>GIMP Gradient (*.ggr)</filetypename>\n"
                "<filetypetooltip>Gradients used in GIMP</filetypetooltip>\n"
            "</input>\n"
        "</inkscape-extension>\n", new GimpGrad());
    return;
}

} } }  /* namespace Internal; Extension; Inkscape */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
