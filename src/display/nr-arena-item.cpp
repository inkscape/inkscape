#define __NR_ARENA_ITEM_C__

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

#define noNR_ARENA_ITEM_VERBOSE
#define noNR_ARENA_ITEM_DEBUG_CASCADE

#include <cstring>
#include <string>
#include <cairomm/cairomm.h>

#include "display/cairo-utils.h"
#include "display/cairo-templates.h"
#include "nr-arena.h"
#include "nr-arena-item.h"
#include "gc-core.h"
#include "helper/geom.h"

#include "nr-filter.h"
#include "nr-arena-group.h"
#include "preferences.h"

namespace GC = Inkscape::GC;

static void nr_arena_item_class_init (NRArenaItemClass *klass);
static void nr_arena_item_init (NRArenaItem *item);
static void nr_arena_item_private_finalize (NRObject *object);

static NRObjectClass *parent_class;

NRType 
nr_arena_item_get_type (void)
{
    static NRType type = 0;
    if (!type) {
        type = nr_object_register_type (NR_TYPE_OBJECT,
                                        "NRArenaItem",
                                        sizeof (NRArenaItemClass),
                                        sizeof (NRArenaItem),
                                        (void (*)(NRObjectClass *))
                                        nr_arena_item_class_init,
                                        (void (*)(NRObject *))
                                        nr_arena_item_init);
    }
    return type;
}

static void
nr_arena_item_class_init (NRArenaItemClass *klass)
{
    NRObjectClass *object_class;

    object_class = (NRObjectClass *) klass;

    parent_class = ((NRObjectClass *) klass)->parent;

    object_class->finalize = nr_arena_item_private_finalize;
    object_class->cpp_ctor = NRObject::invoke_ctor < NRArenaItem >;
}

static void
nr_arena_item_init (NRArenaItem *item)
{
    item->arena = NULL;
    item->parent = NULL;
    item->next = item->prev = NULL;

    item->key = 0;

    item->state = 0;
    item->sensitive = TRUE;
    item->visible = TRUE;

    memset (&item->bbox, 0, sizeof (item->bbox));
    memset (&item->drawbox, 0, sizeof (item->drawbox));
    item->transform = NULL;
    item->ctm.setIdentity();
    item->opacity = 255;
    item->render_opacity = FALSE;

    item->transform = NULL;
    item->clip = NULL;
    item->mask = NULL;
    item->px = NULL;
    item->data = NULL;
    item->filter = NULL;
    item->background_pb = NULL;
    item->background_new = false;
}

static void
nr_arena_item_private_finalize (NRObject *object)
{
    NRArenaItem *item = static_cast < NRArenaItem * >(object);

    item->px = NULL;
    item->transform = NULL;

    if (item->clip)
        nr_arena_item_detach(item, item->clip);
    if (item->mask)
        nr_arena_item_detach(item, item->mask);

    ((NRObjectClass *) (parent_class))->finalize (object);
}

NRArenaItem *
nr_arena_item_children (NRArenaItem *item)
{
    nr_return_val_if_fail (item != NULL, NULL);
    nr_return_val_if_fail (NR_IS_ARENA_ITEM (item), NULL);

    if (NR_ARENA_ITEM_VIRTUAL (item, children))
        return NR_ARENA_ITEM_VIRTUAL (item, children) (item);

    return NULL;
}

NRArenaItem *
nr_arena_item_last_child (NRArenaItem *item)
{
    nr_return_val_if_fail (item != NULL, NULL);
    nr_return_val_if_fail (NR_IS_ARENA_ITEM (item), NULL);

    if (NR_ARENA_ITEM_VIRTUAL (item, last_child)) {
        return NR_ARENA_ITEM_VIRTUAL (item, last_child) (item);
    } else {
        NRArenaItem *ref = nr_arena_item_children (item);
        if (ref)
            while (ref->next)
                ref = ref->next;
        return ref;
    }
}

void
nr_arena_item_add_child (NRArenaItem *item, NRArenaItem *child,
                         NRArenaItem *ref)
{
    nr_return_if_fail (item != NULL);
    nr_return_if_fail (NR_IS_ARENA_ITEM (item));
    nr_return_if_fail (child != NULL);
    nr_return_if_fail (NR_IS_ARENA_ITEM (child));
    nr_return_if_fail (child->parent == NULL);
    nr_return_if_fail (child->prev == NULL);
    nr_return_if_fail (child->next == NULL);
    nr_return_if_fail (child->arena == item->arena);
    nr_return_if_fail (child != ref);
    nr_return_if_fail (!ref || NR_IS_ARENA_ITEM (ref));
    nr_return_if_fail (!ref || (ref->parent == item));

    if (NR_ARENA_ITEM_VIRTUAL (item, add_child))
        NR_ARENA_ITEM_VIRTUAL (item, add_child) (item, child, ref);
}

void
nr_arena_item_remove_child (NRArenaItem *item, NRArenaItem *child)
{
    nr_return_if_fail (item != NULL);
    nr_return_if_fail (NR_IS_ARENA_ITEM (item));
    nr_return_if_fail (child != NULL);
    nr_return_if_fail (NR_IS_ARENA_ITEM (child));
    nr_return_if_fail (child->parent == item);

    if (NR_ARENA_ITEM_VIRTUAL (item, remove_child))
        NR_ARENA_ITEM_VIRTUAL (item, remove_child) (item, child);
}

void
nr_arena_item_set_child_position (NRArenaItem *item, NRArenaItem *child,
                                  NRArenaItem *ref)
{
    nr_return_if_fail (item != NULL);
    nr_return_if_fail (NR_IS_ARENA_ITEM (item));
    nr_return_if_fail (child != NULL);
    nr_return_if_fail (NR_IS_ARENA_ITEM (child));
    nr_return_if_fail (child->parent == item);
    nr_return_if_fail (!ref || NR_IS_ARENA_ITEM (ref));
    nr_return_if_fail (!ref || (ref->parent == item));

    if (NR_ARENA_ITEM_VIRTUAL (item, set_child_position))
        NR_ARENA_ITEM_VIRTUAL (item, set_child_position) (item, child, ref);
}

NRArenaItem *
nr_arena_item_ref (NRArenaItem *item)
{
    nr_object_ref ((NRObject *) item);

    return item;
}

NRArenaItem *
nr_arena_item_unref (NRArenaItem *item)
{
    nr_object_unref ((NRObject *) item);

    return NULL;
}

unsigned int
nr_arena_item_invoke_update (NRArenaItem *item, NRRectL *area, NRGC *gc,
                             unsigned int state, unsigned int reset)
{
    NRGC childgc (gc);
    bool filter = (item->arena->rendermode == Inkscape::RENDERMODE_NORMAL);

    nr_return_val_if_fail (item != NULL, NR_ARENA_ITEM_STATE_INVALID);
    nr_return_val_if_fail (NR_IS_ARENA_ITEM (item),
                           NR_ARENA_ITEM_STATE_INVALID);
    nr_return_val_if_fail (!(state & NR_ARENA_ITEM_STATE_INVALID),
                           NR_ARENA_ITEM_STATE_INVALID);

#ifdef NR_ARENA_ITEM_DEBUG_CASCADE
    printf ("Update %s:%p %x %x %x\n",
            nr_type_name_from_instance ((GTypeInstance *) item), item, state,
            item->state, reset);
#endif

    /* return if in error */
    if (item->state & NR_ARENA_ITEM_STATE_INVALID)
        return item->state;
    /* Set reset flags according to propagation status */
    if (item->propagate) {
        reset |= ~item->state;
        item->propagate = FALSE;
    }
    /* Reset our state */
    item->state &= ~reset;
    /* Return if NOP */
    if (!(~item->state & state))
        return item->state;
    /* Test whether to return immediately */
    if (area && (item->state & NR_ARENA_ITEM_STATE_BBOX)) {
        if (!nr_rect_l_test_intersect_ptr(area, &item->drawbox))
            return item->state;
    }

    /* Reset image cache, if not to be kept */
    if (!(item->state & NR_ARENA_ITEM_STATE_IMAGE) && (item->px)) {
        item->px = NULL;
    }

    /* Set up local gc */
    childgc = *gc;
    if (item->transform) {
        childgc.transform = (*item->transform) * childgc.transform;
    }
    /* Remember the transformation matrix */
    item->ctm = childgc.transform;

    /* Invoke the real method */
    // that will update bbox
    item->state = NR_ARENA_ITEM_VIRTUAL (item, update) (item, area, &childgc, state, reset);
    if (item->state & NR_ARENA_ITEM_STATE_INVALID)
        return item->state;

    /* Enlarge the drawbox to contain filter effects */
    if (item->filter && filter && item->item_bbox) {
        item->drawbox.x0 = item->item_bbox->min()[Geom::X];
        item->drawbox.y0 = item->item_bbox->min()[Geom::Y];
        item->drawbox.x1 = item->item_bbox->max()[Geom::X];
        item->drawbox.y1 = item->item_bbox->max()[Geom::Y];
        item->filter->compute_drawbox (item, item->drawbox);
    } else {
        memcpy(&item->drawbox, &item->bbox, sizeof(item->bbox));
    }
    // fixme: to fix the display glitches, in outline mode bbox must be a combination of 
    // full item bbox and its clip and mask (after we have the API to get these)

    /* Clipping */
    if (item->clip) {
        // FIXME: since here we only need bbox, consider passing 
        // ((state & !(NR_ARENA_ITEM_STATE_RENDER)) | NR_ARENA_ITEM_STATE_BBOX)
        // instead of state, so it does not have to create rendering structures in nr_arena_shape_update
        unsigned int newstate = nr_arena_item_invoke_update (item->clip, area, &childgc, state, reset); 
        if (newstate & NR_ARENA_ITEM_STATE_INVALID) {
            item->state |= NR_ARENA_ITEM_STATE_INVALID;
            return item->state;
        }
        // for clipping, we need geometric bbox
        nr_rect_l_intersect (&item->drawbox, &item->drawbox, &item->clip->bbox);
    }
    /* Masking */
    if (item->mask) {
        unsigned int newstate = nr_arena_item_invoke_update (item->mask, area, &childgc, state, reset);
        if (newstate & NR_ARENA_ITEM_STATE_INVALID) {
            item->state |= NR_ARENA_ITEM_STATE_INVALID;
            return item->state;
        }
        // for masking, we need full drawbox of mask
        nr_rect_l_intersect (&item->drawbox, &item->drawbox, &item->mask->drawbox);
    }

    // now that we know drawbox, dirty the corresponding rect on canvas:
    if (!NR_IS_ARENA_GROUP(item) || (item->filter && filter)) {
        // unless filtered, groups do not need to render by themselves, only their members
        nr_arena_item_request_render (item);
    }

    return item->state;
}

struct MaskLuminanceToAlpha {
    guint32 operator()(guint32 in) {
        EXTRACT_ARGB32(in, a, r, g, b)
        // the operation of unpremul -> luminance-to-alpha -> multiply by alpha
        // is equivalent to luminance-to-alpha on premultiplied color values
        // original computation in double: r*0.2125 + g*0.7154 + b*0.0721
        guint32 ao = r*109 + g*366 + b*37; // coeffs add up to 512
        return ((ao + 256) << 15) & 0xff000000; // equivalent to ((ao + 256) / 512) << 24
    }
};

unsigned int
nr_arena_item_invoke_render (cairo_t *ct, NRArenaItem *item, NRRectL const *area,
                             NRPixBlock *pb, unsigned int flags)
{
   bool outline = (item->arena->rendermode == Inkscape::RENDERMODE_OUTLINE);
    bool filter = (item->arena->rendermode != Inkscape::RENDERMODE_OUTLINE &&
                   item->arena->rendermode != Inkscape::RENDERMODE_NO_FILTERS);

    nr_return_val_if_fail (item != NULL, NR_ARENA_ITEM_STATE_INVALID);
    nr_return_val_if_fail (NR_IS_ARENA_ITEM (item),
                           NR_ARENA_ITEM_STATE_INVALID);
    nr_return_val_if_fail (item->state & NR_ARENA_ITEM_STATE_BBOX,
                           item->state);
    if (!ct) return item->state;

#ifdef NR_ARENA_ITEM_VERBOSE
    g_message ("Invoke render %p on %p: %d %d - %d %d, %d %d - %d %d", item, pb,
            area->x0, area->y0,
            area->x1, area->y1,
            item->drawbox.x0, item->drawbox.y0,
            item->drawbox.x1, item->drawbox.y1);
#endif

    /* If we are invisible, just return successfully */
    if (!item->visible)
        return item->state | NR_ARENA_ITEM_STATE_RENDER;

    // carea is the bounding box for intermediate rendering.
    // NOTE: carea might be larger than area, because of filter effects.
    NRRectL carea;
    nr_rect_l_intersect (&carea, area, &item->drawbox);
    if (nr_rect_l_test_empty(carea))
        return item->state | NR_ARENA_ITEM_STATE_RENDER;
    if (item->filter && filter) {
        item->filter->area_enlarge (carea, item);
        nr_rect_l_intersect (&carea, &carea, &item->drawbox);
    }

    if (outline) {
        // No caching in outline mode for now; investigate if it really gives any advantage with cairo.
        // Also no attempts to clip anything; just render everything: item, clip, mask   
        // First, render the object itself 
        unsigned int state = NR_ARENA_ITEM_VIRTUAL (item, render) (ct, item, &carea, pb, flags);
        if (state & NR_ARENA_ITEM_STATE_INVALID) {
            /* Clean up and return error */
            item->state |= NR_ARENA_ITEM_STATE_INVALID;
            return item->state;
        }

        // render clip and mask, if any
        guint32 saved_rgba = item->arena->outlinecolor; // save current outline color
        // render clippath as an object, using a different color
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        if (item->clip) {
            item->arena->outlinecolor = prefs->getInt("/options/wireframecolors/clips", 0x00ff00ff); // green clips
            NR_ARENA_ITEM_VIRTUAL (item->clip, render) (ct, item->clip, &carea, pb, flags);
        } 
        // render mask as an object, using a different color
        if (item->mask) {
            item->arena->outlinecolor = prefs->getInt("/options/wireframecolors/masks", 0x0000ffff); // blue masks
            NR_ARENA_ITEM_VIRTUAL (item->mask, render) (ct, item->mask, &carea, pb, flags);
        }
        item->arena->outlinecolor = saved_rgba; // restore outline color

        return item->state | NR_ARENA_ITEM_STATE_RENDER;
    }

    using namespace Inkscape;

    // clipping and masks
    unsigned int state;

    cairo_t *this_ct = ct;
    NRRectL *this_area = const_cast<NRRectL*>(area);

    bool needs_intermediate_rendering = false;
    bool &nir = needs_intermediate_rendering;
    bool needs_opacity = (item->opacity != 255 && !item->render_opacity);

    // this item needs an intermediate rendering if:
    nir |= (item->mask != NULL); // 1. it has a mask
    nir |= (item->filter != NULL && filter); // 2. it has a filter
    nir |= needs_opacity; // 3. it is non-opaque

    double opacity = static_cast<double>(item->opacity) / 255.0;

    if (needs_intermediate_rendering) {
        cairo_surface_t *intermediate = cairo_surface_create_similar(
            cairo_get_target(ct), CAIRO_CONTENT_COLOR_ALPHA,
            carea.x1 - carea.x0, carea.y1 - carea.y0);
        this_ct = cairo_create(intermediate);
        cairo_translate(this_ct, -carea.x0, -carea.y0);
        this_area = &carea;
        cairo_surface_destroy(intermediate); // the surface will be held in memory by this_ct
    } else {
        cairo_reference(this_ct);
    }

    // The pipeline needs to be different for filters.
    // First we render the item into an intermediate surface. Then the filter rotates
    // the surface to user coordinates (if necessary) and runs the rendering.
    // Once that's done we retrieve the result, rotating it back to screen coords.
    // Clipping and masking happens after the filter result is ready.
    if (item->filter && filter) {
    }

    Cairo::Context cct(this_ct, true);
    Cairo::Context base_ct(ct);
    Cairo::RefPtr<Cairo::Pattern> mask;
    CairoSave clipsave(ct); // RAII for save / restore
    CairoGroup maskgroup(this_ct); // RAII for push_group / pop_group
    CairoGroup drawgroup(this_ct);
    CairoGroup maskopacitygroup(this_ct);

    // always clip the base context, not the one on the intermediate surface
    // this is because filters must be done before clipping
    if (item->clip) {
        clipsave.save();
        state = nr_arena_item_invoke_clip(ct, item->clip, const_cast<NRRectL*>(area));
        if (state & NR_ARENA_ITEM_STATE_INVALID) {
            item->state |= NR_ARENA_ITEM_STATE_INVALID;
            return item->state;
        }
        base_ct.clip();
    }

    // render mask on the intermediate context and store it
    if (item->mask) {
        maskgroup.push_with_content(CAIRO_CONTENT_COLOR_ALPHA);
        // handle opacity of a masked object by composing it with the mask
        if (needs_opacity) {
            maskopacitygroup.push();
        }
        state = NR_ARENA_ITEM_VIRTUAL (item->mask, render) (this_ct, item->mask, this_area, pb, flags);
        if (state & NR_ARENA_ITEM_STATE_INVALID) {
            item->state |= NR_ARENA_ITEM_STATE_INVALID;
            return item->state;
        }
        if (needs_opacity) {
            maskopacitygroup.pop_to_source();
            cct.paint_with_alpha(opacity);
        }
        mask = maskgroup.popmm();
        // convert luminance to alpha
        cairo_pattern_t *p = mask->cobj();
        cairo_surface_t *s;
        cairo_pattern_get_surface(p, &s);
        ink_cairo_surface_filter(s, s, MaskLuminanceToAlpha());
    }

    // render the object (possibly to the intermediate surface)
    state = NR_ARENA_ITEM_VIRTUAL (item, render) (this_ct, item, this_area, pb, flags);
    if (state & NR_ARENA_ITEM_STATE_INVALID) {
        /* Clean up and return error */
        item->state |= NR_ARENA_ITEM_STATE_INVALID;
        return item->state;
    }

    // apply filter
    if (item->filter && filter) {
        item->filter->render(item, ct, area, this_ct, &carea);
    }

    if (needs_intermediate_rendering) {
        cairo_surface_t *intermediate = cairo_get_target(this_ct);
        cairo_set_source_surface(ct, intermediate, carea.x0, carea.y0);
        if (mask) {
            cairo_mask(ct, mask->cobj());
            // opacity of masked objects is handled by premultiplying the mask
        } else {
            // opacity of non-masked objects must be rendered explicitly
            if (needs_opacity) {
                cairo_paint_with_alpha(ct, opacity);
            } else {
                cairo_paint(ct);
            }
        }
        cairo_set_source_rgba(ct,0,0,0,0);
    }

    return item->state | NR_ARENA_ITEM_STATE_RENDER;
}

unsigned int
nr_arena_item_invoke_clip (cairo_t *ct, NRArenaItem *item, NRRectL *area)
{
    nr_return_val_if_fail (item != NULL, NR_ARENA_ITEM_STATE_INVALID);
    nr_return_val_if_fail (NR_IS_ARENA_ITEM (item),
                           NR_ARENA_ITEM_STATE_INVALID);
    /* we originally short-circuited if the object state included
     * NR_ARENA_ITEM_STATE_CLIP (and showed a warning on the console);
     * anyone know why we stopped doing so?
     */
    /*nr_return_val_if_fail ((pb->area.x1 - pb->area.x0) >=
                           (area->x1 - area->x0),
                           NR_ARENA_ITEM_STATE_INVALID);
    nr_return_val_if_fail ((pb->area.y1 - pb->area.y0) >=
                           (area->y1 - area->y0),
                           NR_ARENA_ITEM_STATE_INVALID);*/

#ifdef NR_ARENA_ITEM_VERBOSE
    printf ("Invoke clip by %p: %d %d - %d %d, item bbox %d %d - %d %d\n",
            item, area->x0, area->y0, area->x1, area->y1, (&item->bbox)->x0,
            (&item->bbox)->y0, (&item->bbox)->x1, (&item->bbox)->y1);
#endif

    if (item->visible && nr_rect_l_test_intersect_ptr(area, &item->bbox)) {
        /* Need render that item */
        if (((NRArenaItemClass *) NR_OBJECT_GET_CLASS (item))->clip) {
            return ((NRArenaItemClass *) NR_OBJECT_GET_CLASS (item))->
                clip (ct, item, area);
        }
    }

    return item->state;
}

NRArenaItem *
nr_arena_item_invoke_pick (NRArenaItem *item, Geom::Point p, double delta,
                           unsigned int sticky)
{
    nr_return_val_if_fail (item != NULL, NULL);
    nr_return_val_if_fail (NR_IS_ARENA_ITEM (item), NULL);

    // Sometimes there's no BBOX in item->state, reason unknown (bug 992817); I made this not an assert to remove the warning
    if (!(item->state & NR_ARENA_ITEM_STATE_BBOX)
        || !(item->state & NR_ARENA_ITEM_STATE_PICK))
        return NULL;

    if (!sticky && !(item->visible && item->sensitive))
        return NULL;

    // TODO: rewrite using Geom::Rect
    const double x = p[Geom::X];
    const double y = p[Geom::Y];

    if (((x + delta) >= item->bbox.x0) &&
        ((x - delta) < item->bbox.x1) &&
        ((y + delta) >= item->bbox.y0) && ((y - delta) < item->bbox.y1))
    {
        if (((NRArenaItemClass *) NR_OBJECT_GET_CLASS (item))->pick)
            return ((NRArenaItemClass *) NR_OBJECT_GET_CLASS (item))->
                pick (item, p, delta, sticky);
    }

    return NULL;
}

void
nr_arena_item_request_update (NRArenaItem *item, unsigned int reset,
                              unsigned int propagate)
{
    nr_return_if_fail (item != NULL);
    nr_return_if_fail (NR_IS_ARENA_ITEM (item));
    nr_return_if_fail (!(reset & NR_ARENA_ITEM_STATE_INVALID));

    if (propagate && !item->propagate)
        item->propagate = TRUE;

    if (item->state & reset) {
        item->state &= ~reset;
        if (item->parent) {
            nr_arena_item_request_update (item->parent, reset, FALSE);
        } else {
            nr_arena_request_update (item->arena, item);
        }
    }
}

void
nr_arena_item_request_render (NRArenaItem *item)
{
    nr_return_if_fail (item != NULL);
    nr_return_if_fail (NR_IS_ARENA_ITEM (item));

    nr_arena_request_render_rect (item->arena, &item->drawbox);
}

/* Public */

NRArenaItem *
nr_arena_item_unparent (NRArenaItem *item)
{
    nr_return_val_if_fail (item != NULL, NULL);
    nr_return_val_if_fail (NR_IS_ARENA_ITEM (item), NULL);

    nr_arena_item_request_render (item);

    if (item->parent) {
        nr_arena_item_remove_child (item->parent, item);
    }

    return NULL;
}

void
nr_arena_item_append_child (NRArenaItem *parent, NRArenaItem *child)
{
    nr_return_if_fail (parent != NULL);
    nr_return_if_fail (NR_IS_ARENA_ITEM (parent));
    nr_return_if_fail (child != NULL);
    nr_return_if_fail (NR_IS_ARENA_ITEM (child));
    nr_return_if_fail (parent->arena == child->arena);
    nr_return_if_fail (child->parent == NULL);
    nr_return_if_fail (child->prev == NULL);
    nr_return_if_fail (child->next == NULL);

    nr_arena_item_add_child (parent, child, nr_arena_item_last_child (parent));
}

void
nr_arena_item_set_transform (NRArenaItem *item, Geom::Affine const &transform)
{
    Geom::Affine const t (transform);
    nr_arena_item_set_transform (item, &t);
}

void
nr_arena_item_set_transform (NRArenaItem *item, Geom::Affine const *transform)
{
    nr_return_if_fail (item != NULL);
    nr_return_if_fail (NR_IS_ARENA_ITEM (item));

    if (!transform && !item->transform)
        return;

    const Geom::Affine *md = (item->transform) ? item->transform : &GEOM_MATRIX_IDENTITY;
    const Geom::Affine *ms = (transform) ? transform : &GEOM_MATRIX_IDENTITY;

    if (!Geom::matrix_equalp(*md, *ms, NR_EPSILON)) {
        nr_arena_item_request_render (item);
        if (!transform || transform->isIdentity()) {
            /* Set to identity affine */
            item->transform = NULL;
        } else {
            if (!item->transform)
                item->transform = new (GC::ATOMIC) Geom::Affine ();
            *item->transform = *transform;
        }
        nr_arena_item_request_update (item, NR_ARENA_ITEM_STATE_ALL, TRUE);
    }
}

void
nr_arena_item_set_opacity (NRArenaItem *item, double opacity)
{
    nr_return_if_fail (item != NULL);
    nr_return_if_fail (NR_IS_ARENA_ITEM (item));

    nr_arena_item_request_render (item);

    item->opacity = (unsigned int) (opacity * 255.9999);
}

void
nr_arena_item_set_sensitive (NRArenaItem *item, unsigned int sensitive)
{
    nr_return_if_fail (item != NULL);
    nr_return_if_fail (NR_IS_ARENA_ITEM (item));

    /* fixme: mess with pick/repick... */

    item->sensitive = sensitive;
}

void
nr_arena_item_set_visible (NRArenaItem *item, unsigned int visible)
{
    nr_return_if_fail (item != NULL);
    nr_return_if_fail (NR_IS_ARENA_ITEM (item));

    item->visible = visible;

    nr_arena_item_request_render (item);
}

void
nr_arena_item_set_clip (NRArenaItem *item, NRArenaItem *clip)
{
    nr_return_if_fail (item != NULL);
    nr_return_if_fail (NR_IS_ARENA_ITEM (item));
    nr_return_if_fail (!clip || NR_IS_ARENA_ITEM (clip));

    if (clip != item->clip) {
        nr_arena_item_request_render (item);
        if (item->clip)
            item->clip = nr_arena_item_detach (item, item->clip);
        if (clip)
            item->clip = nr_arena_item_attach (item, clip, NULL, NULL);
        nr_arena_item_request_update (item, NR_ARENA_ITEM_STATE_ALL, TRUE);
    }
}

void
nr_arena_item_set_mask (NRArenaItem *item, NRArenaItem *mask)
{
    nr_return_if_fail (item != NULL);
    nr_return_if_fail (NR_IS_ARENA_ITEM (item));
    nr_return_if_fail (!mask || NR_IS_ARENA_ITEM (mask));

    if (mask != item->mask) {
        nr_arena_item_request_render (item);
        if (item->mask)
            item->mask = nr_arena_item_detach (item, item->mask);
        if (mask)
            item->mask = nr_arena_item_attach (item, mask, NULL, NULL);
        nr_arena_item_request_update (item, NR_ARENA_ITEM_STATE_ALL, TRUE);
    }
}

void
nr_arena_item_set_order (NRArenaItem *item, int order)
{
    nr_return_if_fail (item != NULL);
    nr_return_if_fail (NR_IS_ARENA_ITEM (item));

    if (!item->parent)
        return;

    NRArenaItem *children = nr_arena_item_children (item->parent);

    NRArenaItem *ref = NULL;
    int pos = 0;
    for (NRArenaItem *child = children; child != NULL; child = child->next) {
        if (pos >= order)
            break;
        if (child != item) {
            ref = child;
            pos += 1;
        }
    }

    nr_arena_item_set_child_position (item->parent, item, ref);
}

void
nr_arena_item_set_item_bbox (NRArenaItem *item, Geom::OptRect &bbox)
{
    nr_return_if_fail(item != NULL);
    nr_return_if_fail(NR_IS_ARENA_ITEM(item));

    item->item_bbox = bbox;
}

/** Returns a background image for use with filter effects. */
NRPixBlock *nr_arena_item_get_background(NRArenaItem const * /*item*/)
{
    return NULL;
}

/* Helpers */

NRArenaItem *
nr_arena_item_attach (NRArenaItem *parent, NRArenaItem *child,
                      NRArenaItem *prev, NRArenaItem *next)
{
    nr_return_val_if_fail (parent != NULL, NULL);
    nr_return_val_if_fail (NR_IS_ARENA_ITEM (parent), NULL);
    nr_return_val_if_fail (child != NULL, NULL);
    nr_return_val_if_fail (NR_IS_ARENA_ITEM (child), NULL);
    nr_return_val_if_fail (child->parent == NULL, NULL);
    nr_return_val_if_fail (child->prev == NULL, NULL);
    nr_return_val_if_fail (child->next == NULL, NULL);
    nr_return_val_if_fail (!prev || NR_IS_ARENA_ITEM (prev), NULL);
    nr_return_val_if_fail (!prev || (prev->parent == parent), NULL);
    nr_return_val_if_fail (!prev || (prev->next == next), NULL);
    nr_return_val_if_fail (!next || NR_IS_ARENA_ITEM (next), NULL);
    nr_return_val_if_fail (!next || (next->parent == parent), NULL);
    nr_return_val_if_fail (!next || (next->prev == prev), NULL);

    child->parent = parent;
    child->prev = prev;
    child->next = next;

    if (prev)
        prev->next = child;
    if (next)
        next->prev = child;

    return child;
}

NRArenaItem *
nr_arena_item_detach (NRArenaItem *parent, NRArenaItem *child)
{
    nr_return_val_if_fail (parent != NULL, NULL);
    nr_return_val_if_fail (NR_IS_ARENA_ITEM (parent), NULL);
    nr_return_val_if_fail (child != NULL, NULL);
    nr_return_val_if_fail (NR_IS_ARENA_ITEM (child), NULL);
    nr_return_val_if_fail (child->parent == parent, NULL);

    NRArenaItem *prev = child->prev;
    NRArenaItem *next = child->next;

    child->parent = NULL;
    child->prev = NULL;
    child->next = NULL;

    if (prev)
        prev->next = next;
    if (next)
        next->prev = prev;

    return next;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
