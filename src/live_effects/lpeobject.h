#ifndef INKSCAPE_LIVEPATHEFFECT_OBJECT_H
#define INKSCAPE_LIVEPATHEFFECT_OBJECT_H

/*
 * Inkscape::LivePathEffect
 *
* Copyright (C) Johan Engelen 2007-2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
 
#include "sp-object.h"
#include "effect.h"

#define TYPE_LIVEPATHEFFECT  (livepatheffect_get_type())
#define LIVEPATHEFFECT(o)    (G_TYPE_CHECK_INSTANCE_CAST((o), TYPE_LIVEPATHEFFECT, LivePathEffectObject))
#define IS_LIVEPATHEFFECT(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), TYPE_LIVEPATHEFFECT))

class LivePathEffectObject : public SPObject {
public:
    Inkscape::LivePathEffect::EffectType effecttype;
    Inkscape::LivePathEffect::Effect *lpe;

    bool effecttype_set;

    LivePathEffectObject * fork_private_if_necessary(unsigned int nr_of_allowed_users = 1);
};

/// The LivePathEffect vtable.
struct LivePathEffectObjectClass {
    SPObjectClass parent_class;
};

GType livepatheffect_get_type();

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
