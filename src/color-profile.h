#ifndef SEEN_COLOR_PROFILE_H
#define SEEN_COLOR_PROFILE_H

/** \file
 * SPColorProfile: SVG <color-profile> implementation
 */

#include <glib/gtypes.h>
#include <sp-object.h>
#if ENABLE_LCMS
#include <lcms.h>
#endif // ENABLE_LCMS

namespace Inkscape {

/** Color Profile. */
struct ColorProfile : public SPObject {
    gchar* href;
    gchar* local;
    gchar* name;
    guint rendering_intent;
#if ENABLE_LCMS
    cmsHPROFILE profHandle;
#endif // ENABLE_LCMS
};

/// The SPColorProfile vtable.
struct ColorProfileClass {
    SPObjectClass parent_class;
};

} // namespace Inkscape

#endif // !SEEN_COLOR_PROFILE_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
