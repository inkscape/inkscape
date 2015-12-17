#ifndef SEEN_KNOT_HOLDER_ENTITY_H
#define SEEN_KNOT_HOLDER_ENTITY_H
/*
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *
 * Copyright (C) 1999-2001 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2001 Mitsuru Oka
 * Copyright (C) 2004 Monash University
 * Copyright (C) 2008 Maximilian Albert
 *
 * Released under GNU GPL
 */

#include <2geom/forward.h>

#include "display/sp-canvas-item.h"
#include "knot.h"
#include "snapper.h"

class SPItem;
class SPKnot;
class SPDesktop;
class KnotHolder;

namespace Inkscape {
namespace LivePathEffect {
    class Effect;
} // namespace LivePathEffect
} // namespace Inkscape

typedef void (* SPKnotHolderSetFunc) (SPItem *item, Geom::Point const &p, Geom::Point const &origin, unsigned int state);
typedef Geom::Point (* SPKnotHolderGetFunc) (SPItem *item);

/**
 * KnotHolderEntity definition.
 */
class KnotHolderEntity {
public:
    KnotHolderEntity():
        knot(NULL),
        item(NULL),
        desktop(NULL),
        parent_holder(NULL),
        my_counter(0),
        handler_id(0),
        _click_handler_id(0),
        _ungrab_handler_id(0)
        {}
    virtual ~KnotHolderEntity();

    virtual void create(SPDesktop *desktop, SPItem *item, KnotHolder *parent,
                        Inkscape::ControlType type = Inkscape::CTRL_TYPE_UNKNOWN,
                        char const*tip = "",
                        SPKnotShapeType shape = SP_KNOT_SHAPE_DIAMOND,
                        SPKnotModeType mode = SP_KNOT_MODE_XOR,
                        guint32 color = 0xffffff00);
 
    /* the get/set/click handlers are virtual functions; each handler class for a knot
       should be derived from KnotHolderEntity and override these functions */
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, unsigned int state) = 0;
    virtual Geom::Point knot_get() const = 0;
    virtual void knot_click(unsigned int /*state*/) {}

    void update_knot();

//private:
    Geom::Point snap_knot_position(Geom::Point const &p, unsigned int state);
    Geom::Point snap_knot_position_constrained(Geom::Point const &p, Inkscape::Snapper::SnapConstraint const &constraint, unsigned int state);

    SPKnot *knot;
    SPItem *item;
    SPDesktop *desktop;

    KnotHolder *parent_holder;

    int my_counter;
    static int counter;

    /** Connection to \a knot's "moved" signal. */
    unsigned int   handler_id;
    /** Connection to \a knot's "clicked" signal. */
    unsigned int   _click_handler_id;
    /** Connection to \a knot's "ungrabbed" signal. */
    unsigned int   _ungrab_handler_id;

private:
    sigc::connection _moved_connection;
    sigc::connection _click_connection;
    sigc::connection _ungrabbed_connection;
};

// derived KnotHolderEntity class for LPEs
class LPEKnotHolderEntity : public KnotHolderEntity {
public:
    LPEKnotHolderEntity(Inkscape::LivePathEffect::Effect *effect) : _effect(effect) {};
protected:
    Inkscape::LivePathEffect::Effect *_effect;
};

/* pattern manipulation */

class PatternKnotHolderEntityXY : public KnotHolderEntity {
public:
    PatternKnotHolderEntityXY(bool fill) : KnotHolderEntity(), _fill(fill) {}
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, unsigned int state);
private:
    // true if the entity tracks fill, false for stroke 
    bool _fill;
};

class PatternKnotHolderEntityAngle : public KnotHolderEntity {
public:
    PatternKnotHolderEntityAngle(bool fill) : KnotHolderEntity(), _fill(fill) {}
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, unsigned int state);
private:
    bool _fill;
};

class PatternKnotHolderEntityScale : public KnotHolderEntity {
public:
    PatternKnotHolderEntityScale(bool fill) : KnotHolderEntity(), _fill(fill) {}
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, unsigned int state);
private:
    bool _fill;
};

#endif /* !SEEN_KNOT_HOLDER_ENTITY_H */

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
