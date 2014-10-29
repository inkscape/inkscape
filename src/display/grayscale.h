#ifndef SEEN_DISPLAY_GRAYSCALE_H
#define SEEN_DISPLAY_GRAYSCALE_H

/*
 * Author:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2011 Author
 *
 * Released under GNU GPL
 */

typedef unsigned int guint32;

/**
 * Provide methods to calculate grayscale values (e.g. convert rgba value to grayscale rgba value).
 */
namespace Grayscale {
    guint32 process(guint32 rgba);
    guint32 process(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
    unsigned char luminance(unsigned char r, unsigned char g, unsigned char b);

    bool activeDesktopIsGrayscale();
};

#endif // !SEEN_DISPLAY_GRAYSCALE_H

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
