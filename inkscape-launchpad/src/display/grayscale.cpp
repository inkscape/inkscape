/** \file
 * Provide methods to calculate grayscale values (e.g. convert rgba value to grayscale rgba value)
 */

/*
 * Author:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2011 Author
 *
 * Released under GNU GPL
 */

#include "display/grayscale.h"
#include "color.h"

// for activeDesktopIsGrayscale:
#include "display/rendermode.h"
#include "inkscape.h"
#include "desktop.h"

namespace Grayscale {

/* Original values from Johan:
const float red_factor = 0.3;
const float green_factor = 0.59;
const float blue_factor = 0.11;
*/

// Values below are from the SVG specification
const float red_factor = 0.2125;
const float green_factor = 0.7154;
const float blue_factor = 0.0721;

guint32 process(guint32 rgba) {
    return process(SP_RGBA32_R_U(rgba), SP_RGBA32_G_U(rgba), SP_RGBA32_B_U(rgba), SP_RGBA32_A_U(rgba));
}

guint32 process(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {

    /** To reduce banding in gradients, this calculation is tweaked a bit
     *  by outputing blue+1 or red+1 or both. The luminance is calculated
     *  times 4. Then last two bits are used to determine if red and/or blue
     *  can be increased by one. Then these two bits are discarded.
     *  So the output color it still looks gray, but has more than 256 steps.
     *  The assumption is that the eye is most sensitive to green, then red, then blue.
     *  (hope this trick works :-) Johan)
     */

    guint32 luminance = ( red_factor * (r << 3)
                          + green_factor * (g << 3)
                          + blue_factor * (b << 3) );
    unsigned blue_plus_one = (luminance & 0x01) ? 1 : 0;
    unsigned red_plus_one = (luminance & 0x02) ? 1 : 0;
    unsigned green_plus_one = (luminance & 0x04) ? 1 : 0;
    luminance = luminance >> 3;

    if (luminance >= 0xff) {
        return SP_RGBA32_U_COMPOSE(0xff, 0xff, 0xff, a);
    } else {
        return SP_RGBA32_U_COMPOSE(luminance + red_plus_one, luminance + green_plus_one, luminance + blue_plus_one, a);
    }
}

unsigned char luminance(unsigned char r, unsigned char g, unsigned char b) {
    guint32 luminance = ( red_factor * r
                          + green_factor * g
                          + blue_factor * b );
    if (luminance > 0xff) {
        luminance = 0xff;
    }

    return luminance & 0xff;
}

/**
 * Use this method if there is no other way to find out if grayscale view or not.
 *
 *  In some cases, the choice between normal or grayscale is so deep in the code hierarchy,
 *  that it is not possible to determine whether grayscale is desired or not, without using
 *  the global SP_ACTIVE_DESKTOP macro. Then use this method, so we know where the abuse is
 *  happening...
 */
bool activeDesktopIsGrayscale() {
    if (SP_ACTIVE_DESKTOP) {
        return (SP_ACTIVE_DESKTOP->getColorMode() == Inkscape::COLORMODE_GRAYSCALE);
    } else {
        return false;
    }
}


};

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
