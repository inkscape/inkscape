/*
 * RenderMode enumeration.
 *
 * Trivially public domain.
 */

#ifndef SEEN_INKSCAPE_DISPLAY_RENDERMODE_H
#define SEEN_INKSCAPE_DISPLAY_RENDERMODE_H

namespace Inkscape {

enum RenderMode {
    RENDERMODE_NORMAL,
    RENDERMODE_NO_FILTERS,
    RENDERMODE_OUTLINE
};

enum ColorMode {
    COLORMODE_NORMAL,
    COLORMODE_GRAYSCALE,
    COLORMODE_PRINT_COLORS_PREVIEW
};

}

#endif
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
