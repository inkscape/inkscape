#ifndef __SP_GUIDE_ATTACHMENT_H__
#define __SP_GUIDE_ATTACHMENT_H__

#include <forward.h>

class SPGuideAttachment {
public:
    SPItem *item;
    int snappoint_ix;

public:
    SPGuideAttachment() :
        item(static_cast<SPItem *>(0))
    { }

    SPGuideAttachment(SPItem *i, int s) :
        item(i),
        snappoint_ix(s)
    { }

    bool operator==(SPGuideAttachment const &o) const {
        return ( ( item == o.item )
                 && ( snappoint_ix == o.snappoint_ix ) );
    }

    bool operator!=(SPGuideAttachment const &o) const {
        return !(*this == o);
    }
};


#endif /* !__SP_GUIDE_ATTACHMENT_H__ */

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
