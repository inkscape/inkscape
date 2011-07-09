#ifndef SEEN_COLOR_PROFILE_FNS_H
#define SEEN_COLOR_PROFILE_FNS_H

/** \file 
 * Macros and fn declarations related to linear gradients.
 */

#include <glib-object.h>
#include <glib/gtypes.h>
#if ENABLE_LCMS
#include <vector>
#include <glibmm/ustring.h>
#endif // ENABLE_LCMS
#include "cms-color-types.h"

class SPDocument;

namespace Inkscape {

namespace XML {
class Node;
} // namespace XML

class ColorProfile;

#if ENABLE_LCMS

cmsHPROFILE colorprofile_get_handle( SPDocument* document, guint* intent, gchar const* name );
cmsHTRANSFORM colorprofile_get_display_transform();

Glib::ustring colorprofile_get_display_id( int screen, int monitor );
Glib::ustring colorprofile_set_display_per( gpointer buf, guint bufLen, int screen, int monitor );
cmsHTRANSFORM colorprofile_get_display_per( Glib::ustring const& id );

std::vector<Glib::ustring> colorprofile_get_display_names();
std::vector<Glib::ustring> colorprofile_get_softproof_names();

Glib::ustring get_path_for_profile(Glib::ustring const& name);

void colorprofile_cmsDoTransform(cmsHTRANSFORM transform, void *inBuf, void *outBuf, unsigned int size);

bool colorprofile_isPrintColorSpace(ColorProfile const *profile);

#endif //  ENABLE_LCMS

} // namespace Inkscape


#endif // !SEEN_COLOR_PROFILE_FNS_H

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
