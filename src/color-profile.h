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

enum {
    RENDERING_INTENT_UNKNOWN = 0,
    RENDERING_INTENT_AUTO = 1,
    RENDERING_INTENT_PERCEPTUAL = 2,
    RENDERING_INTENT_RELATIVE_COLORIMETRIC = 3,
    RENDERING_INTENT_SATURATION = 4,
    RENDERING_INTENT_ABSOLUTE_COLORIMETRIC = 5
};

/// The SPColorProfile vtable.
struct ColorProfileClass {
    SPObjectClass parent_class;
};

/** Color Profile. */
struct ColorProfile : public SPObject {
    static GType getType();
    static void classInit( ColorProfileClass *klass );

#if ENABLE_LCMS
    static cmsHPROFILE getSRGBProfile();
    static std::list<gchar *> getProfileDirs();

    icColorSpaceSignature getColorSpace() const {return _profileSpace;}
    icProfileClassSignature getProfileClass() const {return _profileClass;}
    cmsHTRANSFORM getTransfToSRGB8();
    cmsHTRANSFORM getTransfFromSRGB8();
#endif // ENABLE_LCMS

    gchar* href;
    gchar* local;
    gchar* name;
    gchar* intentStr;
    guint rendering_intent;
#if ENABLE_LCMS
    cmsHPROFILE profHandle;
#endif // ENABLE_LCMS

private:
    static void init( ColorProfile *cprof );

    static void release( SPObject *object );
    static void build( SPObject *object, SPDocument *document, Inkscape::XML::Node *repr );
    static void set( SPObject *object, unsigned key, gchar const *value );
    static Inkscape::XML::Node *write( SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags );
#if ENABLE_LCMS
    static DWORD _getInputFormat( icColorSpaceSignature space );
    void _clearProfile();

    static cmsHPROFILE _sRGBProf;

    icProfileClassSignature _profileClass;
    icColorSpaceSignature _profileSpace;
    cmsHTRANSFORM _transf;
    cmsHTRANSFORM _revTransf;
#endif // ENABLE_LCMS
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
