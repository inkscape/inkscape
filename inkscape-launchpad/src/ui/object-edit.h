#ifndef OBJECT_EDIT_H_SEEN
#define OBJECT_EDIT_H_SEEN

/*
 * Node editing extension to objects
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Mitsuru Oka
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Licensed under GNU GPL
 */

#include "knotholder.h"

namespace Inkscape {
namespace UI {

KnotHolder *createKnotHolder(SPItem *item, SPDesktop *desktop);

}
}

class RectKnotHolder : public KnotHolder {
public:
    RectKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler);
    virtual ~RectKnotHolder() {};
};

class Box3DKnotHolder : public KnotHolder {
public:
    Box3DKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler);
    virtual ~Box3DKnotHolder() {};
};

class ArcKnotHolder : public KnotHolder {
public:
    ArcKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler);
    virtual ~ArcKnotHolder() {};
};

class StarKnotHolder : public KnotHolder {
public:
    StarKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler);
    virtual ~StarKnotHolder() {};
};

class SpiralKnotHolder : public KnotHolder {
public:
    SpiralKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler);
    virtual ~SpiralKnotHolder() {};
};

class OffsetKnotHolder : public KnotHolder {
public:
    OffsetKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler);
    virtual ~OffsetKnotHolder() {};
};

class FlowtextKnotHolder : public KnotHolder {
public:
    FlowtextKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler);
    virtual ~FlowtextKnotHolder() {};
};

class MiscKnotHolder : public KnotHolder {
public:
    MiscKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler);
    virtual ~MiscKnotHolder() {};
};

#endif // OBJECT_EDIT_H_SEEN

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
