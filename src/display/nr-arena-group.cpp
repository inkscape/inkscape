#define __NR_ARENA_GROUP_C__

/*
 * RGBA display list system for inkscape
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-arena-group.h"
#include "display/nr-filter.h"
#include "display/nr-filter-gaussian.h"
#include "display/nr-filter-types.h"
#include "style.h"
#include "sp-filter.h"
#include "sp-filter-reference.h"
#include "sp-gaussian-blur.h"
#include "sp-feblend.h"
#include "display/nr-filter-blend.h"
#include "libnr/nr-matrix-fns.h"
#include "libnr/nr-matrix-ops.h"

static void nr_arena_group_class_init (NRArenaGroupClass *klass);
static void nr_arena_group_init (NRArenaGroup *group);

static NRArenaItem *nr_arena_group_children (NRArenaItem *item);
static NRArenaItem *nr_arena_group_last_child (NRArenaItem *item);
static void nr_arena_group_add_child (NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref);
static void nr_arena_group_remove_child (NRArenaItem *item, NRArenaItem *child);
static void nr_arena_group_set_child_position (NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref);

static unsigned int nr_arena_group_update (NRArenaItem *item, NRRectL *area, NRGC *gc, unsigned int state, unsigned int reset);
static unsigned int nr_arena_group_render (cairo_t *ct, NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags);
static unsigned int nr_arena_group_clip (NRArenaItem *item, NRRectL *area, NRPixBlock *pb);
static NRArenaItem *nr_arena_group_pick (NRArenaItem *item, NR::Point p, double delta, unsigned int sticky);

static NRArenaItemClass *parent_class;

NRType
nr_arena_group_get_type (void)
{
    static NRType type = 0;
    if (!type) {
        type = nr_object_register_type (NR_TYPE_ARENA_ITEM,
                                        "NRArenaGroup",
                                        sizeof (NRArenaGroupClass),
                                        sizeof (NRArenaGroup),
                                        (void (*) (NRObjectClass *)) nr_arena_group_class_init,
                                        (void (*) (NRObject *)) nr_arena_group_init);
    }
    return type;
}

static void
nr_arena_group_class_init (NRArenaGroupClass *klass)
{
    NRObjectClass *object_class;
    NRArenaItemClass *item_class;

    object_class = (NRObjectClass *) klass;
    item_class = (NRArenaItemClass *) klass;

    parent_class = (NRArenaItemClass *) ((NRObjectClass *) klass)->parent;

    object_class->cpp_ctor = NRObject::invoke_ctor<NRArenaGroup>;

    item_class->children = nr_arena_group_children;
    item_class->last_child = nr_arena_group_last_child;
    item_class->add_child = nr_arena_group_add_child;
    item_class->set_child_position = nr_arena_group_set_child_position;
    item_class->remove_child = nr_arena_group_remove_child;
    item_class->update = nr_arena_group_update;
    item_class->render = nr_arena_group_render;
    item_class->clip = nr_arena_group_clip;
    item_class->pick = nr_arena_group_pick;
}

static void
nr_arena_group_init (NRArenaGroup *group)
{
    group->transparent = FALSE;
    group->children = NULL;
    group->last = NULL;
    group->style = NULL;
    group->child_transform.set_identity();
}

static NRArenaItem *
nr_arena_group_children (NRArenaItem *item)
{
    NRArenaGroup *group = NR_ARENA_GROUP (item);

    return group->children;
}

static NRArenaItem *
nr_arena_group_last_child (NRArenaItem *item)
{
    NRArenaGroup *group = NR_ARENA_GROUP (item);

    return group->last;
}

static void
nr_arena_group_add_child (NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref)
{
    NRArenaGroup *group = NR_ARENA_GROUP (item);

    if (!ref) {
        group->children = nr_arena_item_attach (item, child, NULL, group->children);
    } else {
        ref->next = nr_arena_item_attach (item, child, ref, ref->next);
    }

    if (ref == group->last) group->last = child;

    nr_arena_item_request_update (item, NR_ARENA_ITEM_STATE_ALL, FALSE);
}

static void
nr_arena_group_remove_child (NRArenaItem *item, NRArenaItem *child)
{
    NRArenaGroup *group = NR_ARENA_GROUP (item);

    if (child == group->last) group->last = child->prev;

    if (child->prev) {
        nr_arena_item_detach (item, child);
    } else {
        group->children = nr_arena_item_detach (item, child);
    }

    nr_arena_item_request_update (item, NR_ARENA_ITEM_STATE_ALL, FALSE);
}

static void
nr_arena_group_set_child_position (NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref)
{
    NRArenaGroup *group = NR_ARENA_GROUP (item);

    if (child == group->last) group->last = child->prev;

    if (child->prev) {
        nr_arena_item_detach (item, child);
    } else {
        group->children = nr_arena_item_detach (item, child);
    }

    if (!ref) {
        group->children = nr_arena_item_attach (item, child, NULL, group->children);
    } else {
        ref->next = nr_arena_item_attach (item, child, ref, ref->next);
    }

    if (ref == group->last) group->last = child;

    nr_arena_item_request_render (child);
}

static unsigned int
nr_arena_group_update (NRArenaItem *item, NRRectL *area, NRGC *gc, unsigned int state, unsigned int reset)
{
    unsigned int newstate;

    NRArenaGroup *group = NR_ARENA_GROUP (item);

    unsigned int beststate = NR_ARENA_ITEM_STATE_ALL;

    for (NRArenaItem *child = group->children; child != NULL; child = child->next) {
        NRGC cgc(gc);
        cgc.transform = group->child_transform * gc->transform;
        newstate = nr_arena_item_invoke_update (child, area, &cgc, state, reset);
        beststate = beststate & newstate;
    }

    if (beststate & NR_ARENA_ITEM_STATE_BBOX) {
        nr_rect_l_set_empty (&item->bbox);
        for (NRArenaItem *child = group->children; child != NULL; child = child->next) {
            if (child->visible)
                nr_rect_l_union (&item->bbox, &item->bbox, &child->bbox);
        }
    }

    return beststate;
}

void nr_arena_group_set_style (NRArenaGroup *group, SPStyle *style)
{
    g_return_if_fail(group != NULL);
    g_return_if_fail(NR_IS_ARENA_GROUP(group));

    if (style) sp_style_ref(style);
    if (group->style) sp_style_unref(group->style);
    group->style = style;

    //if group has a filter
    if (style->filter.set && style->getFilter()) {
        if (!group->filter) {
            int primitives = sp_filter_primitive_count(SP_FILTER(style->getFilter()));
            group->filter = new NR::Filter(primitives);
        }
        sp_filter_build_renderer(SP_FILTER(style->getFilter()), group->filter);
    } else {
        //no filter set for this group
        delete group->filter;
        group->filter = NULL;
    }

    if (style && style->enable_background.set
        && style->enable_background.value == SP_CSS_BACKGROUND_NEW) {
        group->background_new = true;
    }
}

static unsigned int
nr_arena_group_render (cairo_t *ct, NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags)
{
    NRArenaGroup *group = NR_ARENA_GROUP (item);

    unsigned int ret = item->state;

    /* Just compose children into parent buffer */
    for (NRArenaItem *child = group->children; child != NULL; child = child->next) {
        ret = nr_arena_item_invoke_render (ct, child, area, pb, flags);
        if (ret & NR_ARENA_ITEM_STATE_INVALID) break;
    }

    return ret;
}

static unsigned int
nr_arena_group_clip (NRArenaItem *item, NRRectL *area, NRPixBlock *pb)
{
    NRArenaGroup *group = NR_ARENA_GROUP (item);

    unsigned int ret = item->state;

    /* Just compose children into parent buffer */
    for (NRArenaItem *child = group->children; child != NULL; child = child->next) {
        ret = nr_arena_item_invoke_clip (child, area, pb);
        if (ret & NR_ARENA_ITEM_STATE_INVALID) break;
    }

    return ret;
}

static NRArenaItem *
nr_arena_group_pick (NRArenaItem *item, NR::Point p, double delta, unsigned int sticky)
{
    NRArenaGroup *group = NR_ARENA_GROUP (item);

    for (NRArenaItem *child = group->last; child != NULL; child = child->prev) {
        NRArenaItem *picked = nr_arena_item_invoke_pick (child, p, delta, sticky);
        if (picked)
            return (group->transparent) ? picked : item;
    }

    return NULL;
}

void
nr_arena_group_set_transparent (NRArenaGroup *group, unsigned int transparent)
{
    nr_return_if_fail (group != NULL);
    nr_return_if_fail (NR_IS_ARENA_GROUP (group));

    group->transparent = transparent;
}

void nr_arena_group_set_child_transform(NRArenaGroup *group, NR::Matrix const &t)
{
    NR::Matrix nt(t);
    nr_arena_group_set_child_transform(group, &nt);
}

void nr_arena_group_set_child_transform(NRArenaGroup *group, NR::Matrix const *t)
{
    if (!t) t = &NR_MATRIX_IDENTITY;

    if (!NR::matrix_equalp(*t, group->child_transform, NR_EPSILON)) {
        nr_arena_item_request_render (NR_ARENA_ITEM (group));
        group->child_transform = *t;
        nr_arena_item_request_update (NR_ARENA_ITEM (group), NR_ARENA_ITEM_STATE_ALL, TRUE);
    }
}


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
