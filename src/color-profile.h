#ifndef SEEN_COLOR_PROFILE_H
#define SEEN_COLOR_PROFILE_H

#include <vector>
#include <glib.h>
#include <sp-object.h>
#include <glibmm/ustring.h>
#include "cms-color-types.h"

struct SPColor;

namespace Inkscape {

enum {
    RENDERING_INTENT_UNKNOWN = 0,
    RENDERING_INTENT_AUTO = 1,
    RENDERING_INTENT_PERCEPTUAL = 2,
    RENDERING_INTENT_RELATIVE_COLORIMETRIC = 3,
    RENDERING_INTENT_SATURATION = 4,
    RENDERING_INTENT_ABSOLUTE_COLORIMETRIC = 5
};

class ColorProfileImpl;


/**
 * The SPColorProfile vtable.
 */
struct ColorProfileClass {
    SPObjectClass parent_class;
};

/**
 * Color Profile.
 */
struct ColorProfile : public SPObject {
    friend cmsHPROFILE colorprofile_get_handle( SPDocument*, guint*, gchar const* );
    friend class CMSSystem;

    static GType getType();
    static void classInit( ColorProfileClass *klass );

    static std::vector<Glib::ustring> getBaseProfileDirs();
    static std::vector<Glib::ustring> getProfileFiles();
    static std::vector<std::pair<Glib::ustring, Glib::ustring> > getProfileFilesWithNames();
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    //icColorSpaceSignature getColorSpace() const;
    ColorSpaceSig getColorSpace() const;
    //icProfileClassSignature getProfileClass() const;
    ColorProfileClassSig getProfileClass() const;
    cmsHTRANSFORM getTransfToSRGB8();
    cmsHTRANSFORM getTransfFromSRGB8();
    cmsHTRANSFORM getTransfGamutCheck();
    bool GamutCheck(SPColor color);

#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

    gchar* href;
    gchar* local;
    gchar* name;
    gchar* intentStr;
    guint rendering_intent;

private:
    static void init( ColorProfile *cprof );

    static void release( SPObject *object );
    static void build( SPObject *object, SPDocument *document, Inkscape::XML::Node *repr );
    static void set( SPObject *object, unsigned key, gchar const *value );
    static Inkscape::XML::Node *write( SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags );

    ColorProfileImpl *impl;
};

GType colorprofile_get_type();

} // namespace Inkscape

#define COLORPROFILE_TYPE (Inkscape::colorprofile_get_type())
#define COLORPROFILE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), COLORPROFILE_TYPE, Inkscape::ColorProfile))
#define COLORPROFILE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), COLORPROFILE_TYPE, Inkscape::ColorProfileClass))
#define IS_COLORPROFILE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), COLORPROFILE_TYPE))
#define IS_COLORPROFILE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), COLORPROFILE_TYPE))

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
