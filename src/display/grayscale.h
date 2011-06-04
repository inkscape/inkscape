#ifndef SEEN_DISPLAY_GRAYSCALE_H
#define SEEN_DISPLAY_GRAYSCALE_H

/** \file
 * Provide methods to calculate grayscale values (e.g. convert rgba value to grayscale rgba value)
 *
 * Author:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2011 Author
 *
 * Released under GNU GPL
 */

#include <gdk/gdk.h>

namespace Grayscale {
    guint32 process(guint32 rgba);
    guint32 process(guchar r, guchar g, guchar b, guchar a);
    guchar  luminance(guchar r, guchar g, guchar b);

    const float red_factor = 0.3;
    const float green_factor = 0.59;
    const float blue_factor = 0.11;

    bool activeDesktopIsGrayscale();
};

#endif /* !SEEN_DISPLAY_GRAYSCALE_H */

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
