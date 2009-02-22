#ifndef __SP_GUIDE_CONSTRAINT_H__
#define __SP_GUIDE_CONSTRAINT_H__

#include <forward.h>

class SPGuideConstraint {
public:
    SPGuide *g;
    int snappoint_ix;

public:
    explicit SPGuideConstraint() :
        g(static_cast<SPGuide *>(0))
    { }

    explicit SPGuideConstraint(SPGuide *g, int snappoint_ix) :
        g(g),
        snappoint_ix(snappoint_ix)
    { }

    bool operator==(SPGuideConstraint const &o) const {
        return ( ( g == o.g )
                 && ( snappoint_ix == o.snappoint_ix ) );
    }

    bool operator!=(SPGuideConstraint const &o) const {
        return !( *this == o );
    }
};


#endif /* !__SP_GUIDE_CONSTRAINT_H__ */


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
