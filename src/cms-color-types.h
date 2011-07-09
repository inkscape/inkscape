#ifndef SEEN_CMS_COLOR_TYPES_H
#define SEEN_CMS_COLOR_TYPES_H

/** \file
 * A simple abstraction to provide opaque compatibility with either lcms or lcms2.
 */

#include <glib/gtypes.h>


typedef void * cmsHPROFILE;
typedef void * cmsHTRANSFORM;

namespace Inkscape {

/**
 * Opaque holder of a 32-bit signature type.
 */
class FourCCSig {
public:
    FourCCSig( FourCCSig const &other ) : value(other.value) {};

protected:
    FourCCSig( guint32 value ) : value(value) {};

    guint32 value;
};

class ColorSpaceSig : public FourCCSig {
public:
    ColorSpaceSig( ColorSpaceSig const &other ) : FourCCSig(other) {};

protected:
    ColorSpaceSig( guint32 value ) : FourCCSig(value) {};
};

class ColorProfileClassSig : public FourCCSig {
public:
     ColorProfileClassSig( ColorProfileClassSig const &other ) : FourCCSig(other) {};

protected:
     ColorProfileClassSig( guint32 value ) : FourCCSig(value) {};
};

} // namespace Inkscape

#endif // SEEN_CMS_COLOR_TYPES_H

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
