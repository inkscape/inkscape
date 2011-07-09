#ifndef SEEN_COLOR_PROFILE_CMS_FNS_H
#define SEEN_COLOR_PROFILE_CMS_FNS_H

#if ENABLE_LCMS
#include <lcms.h>
#endif // ENABLE_LCMS

#include "cms-color-types.h"

namespace Inkscape {

#if ENABLE_LCMS

// Note: these can later be adjusted to adapt for lcms2:

class ColorSpaceSigWrapper : public ColorSpaceSig {
public :
    ColorSpaceSigWrapper( icColorSpaceSignature sig ) : ColorSpaceSig( static_cast<guint32>(sig) ) {}
    ColorSpaceSigWrapper( ColorSpaceSig const &other ) : ColorSpaceSig( other ) {}

    operator icColorSpaceSignature() const { return static_cast<icColorSpaceSignature>(value); }
};

class ColorProfileClassSigWrapper : public ColorProfileClassSig {
public :
    ColorProfileClassSigWrapper( icProfileClassSignature sig ) : ColorProfileClassSig( static_cast<guint32>(sig) ) {}
    ColorProfileClassSigWrapper( ColorProfileClassSig const &other ) : ColorProfileClassSig( other ) {}

    operator icProfileClassSignature() const { return static_cast<icProfileClassSignature>(value); }
};

icColorSpaceSignature asICColorSpaceSig(ColorSpaceSig const & sig);
icProfileClassSignature asICColorProfileClassSig(ColorProfileClassSig const & sig);

#endif //  ENABLE_LCMS

} // namespace Inkscape


#endif // !SEEN_COLOR_PROFILE_CMS_FNS_H

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
