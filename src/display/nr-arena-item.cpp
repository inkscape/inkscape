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

#include <libnr/nr-blit.h>
#include <libnr/nr-pixops.h>
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

    // get a copy of bbox
    memcpy(&item->drawbox, &item->bbox, sizeof(item->bbox));

    /* Enlarge the drawbox to contain filter effects */
    if (item->filter && filter) {
        item->filter->bbox_enlarge (item->drawbox);
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

/**
 *    Render item to pixblock.
 *
 *    \return Has NR_ARENA_ITEM_STATE_RENDER set on success.
 */

unsigned int
nr_arena_item_invoke_render (cairo_t *ct, NRArenaItem *item, NRRectL const *area,
                             NRPixBlock *pb, unsigned int flags)
{
   bool outline = (item->arena->rendermode == Inkscape::RENDERMODE_OUTLINE);
    bool filter = (item->arena->rendermode != Inkscape::RENDERMODE_OUTLINE &&
                   item->arena->rendermode != Inkscape::RENDERMODE_NO_FILTERS);
    bool print_colors = (item->arena->rendermode == Inkscape::RENDERMODE_PRINT_COLORS_PREVIEW);

    nr_return_val_if_fail (item != NULL, NR_ARENA_ITEM_STATE_INVALID);
    nr_return_val_if_fail (NR_IS_ARENA_ITEM (item),
                           NR_ARENA_ITEM_STATE_INVALID);
    nr_return_val_if_fail (item->state & NR_ARENA_ITEM_STATE_BBOX,
                           item->state);

#ifdef NR_ARENA_ITEM_VERBOSE
    printf ("Invoke render %p: %d %d - %d %d\n", item, area->x0, area->y0,
            area->x1, area->y1);
#endif

    /* If we are invisible, just return successfully */
    if (!item->visible)
        return item->state | NR_ARENA_ITEM_STATE_RENDER;

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

    NRPixBlock cpb;
    if (item->px) {
        /* Has cache pixblock, render this and return */
        nr_pixblock_setup_extern (&cpb, NR_PIXBLOCK_MODE_R8G8B8A8P,
                                  /* fixme: This probably cannot overflow, because we render only if visible */
                                  /* fixme: and pixel cache is there only for small items */
                                  /* fixme: But this still needs extra check (Lauris) */
                                  item->drawbox.x0, item->drawbox.y0,
                                  item->drawbox.x1, item->drawbox.y1,
                                  item->px,
                                  4 * (item->drawbox.x1 - item->drawbox.x0), FALSE,
                                  FALSE);
        nr_blit_pixblock_pixblock (pb, &cpb);
        nr_pixblock_release (&cpb);
        pb->empty = FALSE;
        return item->state | NR_ARENA_ITEM_STATE_RENDER;
    }

    NRPixBlock *dpb = pb;

    /* Setup cache if we can */
    if ((!(flags & NR_ARENA_ITEM_RENDER_NO_CACHE)) &&
        (carea.x0 <= item->drawbox.x0) && (carea.y0 <= item->drawbox.y0) &&
        (carea.x1 >= item->drawbox.x1) && (carea.y1 >= item->drawbox.y1) &&
        (((item->drawbox.x1 - item->drawbox.x0) * (item->drawbox.y1 -
                                             item->drawbox.y0)) <= 4096)) {
        // Item drawbox is fully in renderable area and size is acceptable
        carea.x0 = item->drawbox.x0;
        carea.y0 = item->drawbox.y0;
        carea.x1 = item->drawbox.x1;
        carea.y1 = item->drawbox.y1;
        item->px =
            new (GC::ATOMIC) unsigned char[4 * (carea.x1 - carea.x0) *
                                           (carea.y1 - carea.y0)];
        nr_pixblock_setup_extern (&cpb, NR_PIXBLOCK_MODE_R8G8B8A8P, carea.x0,
                                  carea.y0, carea.x1, carea.y1, item->px,
                                  4 * (carea.x1 - carea.x0), TRUE, TRUE);
        cpb.visible_area = pb->visible_area;
        dpb = &cpb;
        // Set nocache flag for downstream rendering
        flags |= NR_ARENA_ITEM_RENDER_NO_CACHE;
    }

    /* Determine, whether we need temporary buffer */
    if (item->clip || item->mask
        || ((item->opacity != 255) && !item->render_opacity)
        || (item->filter && filter) || item->background_new
        || (item->parent && item->parent->background_pb)) {

        /* Setup and render item buffer */
        NRPixBlock ipb;
        nr_pixblock_setup_fast (&ipb, NR_PIXBLOCK_MODE_R8G8B8A8P,
                                carea.x0, carea.y0, carea.x1, carea.y1,
                                TRUE);

        //  if memory allocation failed, abort render
        if (ipb.size != NR_PIXBLOCK_SIZE_TINY && ipb.data.px == NULL) {
            nr_pixblock_release (&ipb);
            return (item->state);
        }

        /* If background access is used, save the pixblock address.
         * This address is set to NULL at the end of this block */
        if (item->background_new ||
            (item->parent && item->parent->background_pb)) {
            item->background_pb = &ipb;
        }

        ipb.visible_area = pb->visible_area;
        if (item->filter && filter) {
              item->filter->area_enlarge (ipb.visible_area, item);
        }

        unsigned int state = NR_ARENA_ITEM_VIRTUAL (item, render) (ct, item, &carea, &ipb, flags);
        if (state & NR_ARENA_ITEM_STATE_INVALID) {
            /* Clean up and return error */
            nr_pixblock_release (&ipb);
            if (dpb != pb)
                nr_pixblock_release (dpb);
            item->state |= NR_ARENA_ITEM_STATE_INVALID;
            return item->state;
        }
        ipb.empty = FALSE;

        /* Run filtering, if a filter is set for this object */
        if (item->filter && filter) {
            item->filter->render (item, &ipb);
        }

        if (item->clip || item->mask) {
            /* Setup mask pixblock */
            NRPixBlock mpb;
            nr_pixblock_setup_fast (&mpb, NR_PIXBLOCK_MODE_A8, carea.x0,
                                    carea.y0, carea.x1, carea.y1, TRUE);

            if (mpb.data.px != NULL) { // if memory allocation was successful

                mpb.visible_area = pb->visible_area;
                /* Do clip if needed */
                if (item->clip) {
                    state = nr_arena_item_invoke_clip (item->clip, &carea, &mpb);
                    if (state & NR_ARENA_ITEM_STATE_INVALID) {
                        /* Clean up and return error */
                        nr_pixblock_release (&mpb);
                        nr_pixblock_release (&ipb);
                        if (dpb != pb)
                            nr_pixblock_release (dpb);
                        item->state |= NR_ARENA_ITEM_STATE_INVALID;
                        return item->state;
                    }
                    mpb.empty = FALSE;
                }
                /* Do mask if needed */
                if (item->mask) {
                    NRPixBlock tpb;
                    /* Set up yet another temporary pixblock */
                    nr_pixblock_setup_fast (&tpb, NR_PIXBLOCK_MODE_R8G8B8A8N,
                                            carea.x0, carea.y0, carea.x1,
                                            carea.y1, TRUE);

                    if (tpb.data.px != NULL) { // if memory allocation was successful

                        tpb.visible_area = pb->visible_area;
                        unsigned int state = NR_ARENA_ITEM_VIRTUAL (item->mask, render) (ct, item->mask, &carea, &tpb, flags);
                        if (state & NR_ARENA_ITEM_STATE_INVALID) {
                            /* Clean up and return error */
                            nr_pixblock_release (&tpb);
                            nr_pixblock_release (&mpb);
                            nr_pixblock_release (&ipb);
                            if (dpb != pb)
                                nr_pixblock_release (dpb);
                            item->state |= NR_ARENA_ITEM_STATE_INVALID;
                            return item->state;
                        }
                        /* Composite with clip */
                        if (item->clip) {
                            int x, y;
                            for (y = carea.y0; y < carea.y1; y++) {
                                unsigned char *s, *d;
                                s = NR_PIXBLOCK_PX (&tpb) + (y -
                                                             carea.y0) * tpb.rs;
                                d = NR_PIXBLOCK_PX (&mpb) + (y -
                                                             carea.y0) * mpb.rs;
                                for (x = carea.x0; x < carea.x1; x++) {
                                    unsigned int m;
                                    m = NR_PREMUL_112 (s[0] + s[1] + s[2], s[3]);
                                    d[0] =
                                        FAST_DIV_ROUND < 3 * 255 * 255 >
                                        (NR_PREMUL_123 (d[0], m));
                                    s += 4;
                                    d += 1;
                                }
                            }
                        } else {
                            int x, y;
                            for (y = carea.y0; y < carea.y1; y++) {
                                unsigned char *s, *d;
                                s = NR_PIXBLOCK_PX (&tpb) + (y -
                                                             carea.y0) * tpb.rs;
                                d = NR_PIXBLOCK_PX (&mpb) + (y -
                                                             carea.y0) * mpb.rs;
                                for (x = carea.x0; x < carea.x1; x++) {
                                    unsigned int m;
                                    m = NR_PREMUL_112 (s[0] + s[1] + s[2], s[3]);
                                    d[0] = FAST_DIV_ROUND < 3 * 255 > (m);
                                    s += 4;
                                    d += 1;
                                }
                            }
                            mpb.empty = FALSE;
                        }
                    }
                    nr_pixblock_release (&tpb);
                }
                /* Multiply with opacity if needed */
                if ((item->opacity != 255) && !item->render_opacity
                    ) {
                    int x, y;
                    unsigned int a;
                    a = item->opacity;
                    for (y = carea.y0; y < carea.y1; y++) {
                        unsigned char *d;
                        d = NR_PIXBLOCK_PX (&mpb) + (y - carea.y0) * mpb.rs;
                        for (x = carea.x0; x < carea.x1; x++) {
                            d[0] = NR_PREMUL_111 (d[0], a);
                            d += 1;
                        }
                    }
                }
                /* Compose rendering pixblock int destination */
                nr_blit_pixblock_pixblock_mask (dpb, &ipb, &mpb);
            }
            nr_pixblock_release (&mpb);
        } else {
            if (item->render_opacity) { // opacity was already rendered in, just copy to dpb here
                nr_blit_pixblock_pixblock(dpb, &ipb);
            } else { // copy while multiplying by opacity
                nr_blit_pixblock_pixblock_alpha (dpb, &ipb, item->opacity);
            }
        }
        nr_pixblock_release (&ipb);
        dpb->empty = FALSE;
        /* This pointer wouldn't be valid outside this block, so clear it */
        item->background_pb = NULL;
    } else {
        /* Just render */
        unsigned int state = NR_ARENA_ITEM_VIRTUAL (item, render) (ct, item, &carea, dpb, flags);
        if (state & NR_ARENA_ITEM_STATE_INVALID) {
            /* Clean up and return error */
            if (dpb != pb)
                nr_pixblock_release (dpb);
            item->state |= NR_ARENA_ITEM_STATE_INVALID;
            return item->state;
        }
        dpb->empty = FALSE;
    }

    if (dpb != pb) {
        /* Have to blit from cache */
        nr_blit_pixblock_pixblock (pb, dpb);
        nr_pixblock_release (dpb);
        pb->empty = FALSE;
        item->state |= NR_ARENA_ITEM_STATE_IMAGE;
    }

    return item->state | NR_ARENA_ITEM_STATE_RENDER;
}

unsigned int
nr_arena_item_invoke_clip (NRArenaItem *item, NRRectL *area, NRPixBlock *pb)
{
    nr_return_val_if_fail (item != NULL, NR_ARENA_ITEM_STATE_INVALID);
    nr_return_val_if_fail (NR_IS_ARENA_ITEM (item),
                           NR_ARENA_ITEM_STATE_INVALID);
    /* we originally short-circuited if the object state included
     * NR_ARENA_ITEM_STATE_CLIP (and showed a warning on the console);
     * anyone know why we stopped doing so?
     */
    nr_return_val_if_fail ((pb->area.x1 - pb->area.x0) >=
                           (area->x1 - area->x0),
                           NR_ARENA_ITEM_STATE_INVALID);
    nr_return_val_if_fail ((pb->area.y1 - pb->area.y0) >=
                           (area->y1 - area->y0),
                           NR_ARENA_ITEM_STATE_INVALID);

#ifdef NR_ARENA_ITEM_VERBOSE
    printf ("Invoke clip by %p: %d %d - %d %d, item bbox %d %d - %d %d\n",
            item, area->x0, area->y0, area->x1, area->y1, (&item->bbox)->x0,
            (&item->bbox)->y0, (&item->bbox)->x1, (&item->bbox)->y1);
#endif

    if (item->visible && nr_rect_l_test_intersect_ptr(area, &item->bbox)) {
        /* Need render that item */
        if (((NRArenaItemClass *) NR_OBJECT_GET_CLASS (item))->clip) {
            return ((NRArenaItemClass *) NR_OBJECT_GET_CLASS (item))->
                clip (item, area, pb);
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
        ((y + delta) >= item->bbox.y0) && ((y - delta) < item->bbox.y1)) {
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
nr_arena_item_set_transform (NRArenaItem *item, Geom::Matrix const &transform)
{
    Geom::Matrix const t (transform);
    nr_arena_item_set_transform (item, &t);
}

void
nr_arena_item_set_transform (NRArenaItem *item, Geom::Matrix const *transform)
{
    nr_return_if_fail (item != NULL);
    nr_return_if_fail (NR_IS_ARENA_ITEM (item));

    if (!transform && !item->transform)
        return;

    const Geom::Matrix *md = (item->transform) ? item->transform : &GEOM_MATRIX_IDENTITY;
    const Geom::Matrix *ms = (transform) ? transform : &GEOM_MATRIX_IDENTITY;

    if (!Geom::matrix_equalp(*md, *ms, NR_EPSILON)) {
        nr_arena_item_request_render (item);
        if (!transform || transform->isIdentity()) {
            /* Set to identity affine */
            item->transform = NULL;
        } else {
            if (!item->transform)
                item->transform = new (GC::ATOMIC) Geom::Matrix ();
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
NRPixBlock *
nr_arena_item_get_background (NRArenaItem const *item, int depth)
{
    NRPixBlock *pb;
    if (!item->background_pb)
        return NULL;
    if (item->background_new) {
        pb = new NRPixBlock ();
        nr_pixblock_setup_fast (pb, item->background_pb->mode,
                                item->background_pb->area.x0,
                                item->background_pb->area.y0,
                                item->background_pb->area.x1,
                                item->background_pb->area.y1, true);
        if (pb->size != NR_PIXBLOCK_SIZE_TINY && pb->data.px == NULL) // allocation failed
            return NULL;
    } else if (item->parent) {
        pb = nr_arena_item_get_background (item->parent, depth + 1);
    } else
        return NULL;

    if (depth > 0)
        nr_blit_pixblock_pixblock (pb, item->background_pb);

    return pb;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
