#ifndef __NR_ARENA_GROUP_H__
#define __NR_ARENA_GROUP_H__

/*
 * RGBA display list system for inkscape
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Lauris Kaplinski and Ximian, Inc.
 *
 * Released under GNU GPL
 *
 */

#define NR_TYPE_ARENA_GROUP (nr_arena_group_get_type ())
#define NR_ARENA_GROUP(o) (NR_CHECK_INSTANCE_CAST ((o), NR_TYPE_ARENA_GROUP, NRArenaGroup))
#define NR_IS_ARENA_GROUP(o) (NR_CHECK_INSTANCE_TYPE ((o), NR_TYPE_ARENA_GROUP))

#include "nr-arena-item.h"
#include "style.h"

NRType nr_arena_group_get_type (void);

struct NRArenaGroup : public NRArenaItem{
    unsigned int transparent : 1;
    NRArenaItem *children;
    NRArenaItem *last;
    NR::Matrix child_transform;
    SPStyle *style;

    static NRArenaGroup *create(NRArena *arena) {
        NRArenaGroup *obj = reinterpret_cast<NRArenaGroup *>(nr_object_new(NR_TYPE_ARENA_GROUP));
        obj->init(arena);
        return obj;
    }
};

struct NRArenaGroupClass {
    NRArenaItemClass parent_class;
};

void nr_arena_group_set_transparent(NRArenaGroup *group, unsigned int transparent);

void nr_arena_group_set_child_transform(NRArenaGroup *group, NR::Matrix const &t);
void nr_arena_group_set_child_transform(NRArenaGroup *group, NR::Matrix const *t);
void nr_arena_group_set_style(NRArenaGroup *group, SPStyle *style);

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
