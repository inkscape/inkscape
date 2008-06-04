#ifndef __SP_OBJECT_EDIT_H__
#define __SP_OBJECT_EDIT_H__

/*
 * Node editing extension to objects
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Mitsuru Oka
 *
 * Licensed under GNU GPL
 */

#include "knotholder.h"

KnotHolder *sp_item_knot_holder (SPItem *item, SPDesktop *desktop);

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

#endif

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
