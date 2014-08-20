#ifndef SEEN_SP_GRADIENT_REFERENCE_H
#define SEEN_SP_GRADIENT_REFERENCE_H

#include "uri-references.h"

class SPGradient;
class SPObject;

class SPGradientReference : public Inkscape::URIReference {
public:
    SPGradientReference(SPObject *obj) : URIReference(obj) {}

    SPGradient *getObject() const {
        return reinterpret_cast<SPGradient *>(URIReference::getObject());
    }

protected:
    virtual bool _acceptObject(SPObject *obj) const;
};


#endif /* !SEEN_SP_GRADIENT_REFERENCE_H */

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
