#define __NR_ARENA_C__

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

#include "nr-arena-item.h"
#include "nr-arena.h"
#include <libnr/nr-blit.h>

static void nr_arena_class_init (NRArenaClass *klass);
static void nr_arena_init (NRArena *arena);
static void nr_arena_finalize (NRObject *object);

static NRActiveObjectClass *parent_class;

NRType
nr_arena_get_type (void)
{
    static NRType type = 0;
    if (!type) {
        type = nr_object_register_type (NR_TYPE_ACTIVE_OBJECT,
                                        "NRArena",
                                        sizeof (NRArenaClass),
                                        sizeof (NRArena),
                                        (void (*) (NRObjectClass *)) nr_arena_class_init,
                                        (void (*) (NRObject *)) nr_arena_init);
    }
    return type;
}

static void
nr_arena_class_init (NRArenaClass *klass)
{
    NRObjectClass *object_class = (NRObjectClass *) klass;

    parent_class = (NRActiveObjectClass *) (((NRObjectClass *) klass)->parent);

    object_class->finalize = nr_arena_finalize;
    object_class->cpp_ctor = NRObject::invoke_ctor<NRArena>;
}

static void
nr_arena_init (NRArena *arena)
{
    arena->delta = 0; // to be set by desktop from prefs
    arena->rendermode = Inkscape::RENDERMODE_NORMAL; // default is normal render
    arena->outlinecolor = 0xff; // black; to be set by desktop from bg color
    arena->canvasarena = NULL;
}

static void
nr_arena_finalize (NRObject *object)
{
    ((NRObjectClass *) (parent_class))->finalize (object);
}

void
nr_arena_request_update (NRArena *arena, NRArenaItem *item)
{
    NRActiveObject *aobject = (NRActiveObject *) arena;

    nr_return_if_fail (arena != NULL);
    nr_return_if_fail (NR_IS_ARENA (arena));
    nr_return_if_fail (item != NULL);
    nr_return_if_fail (NR_IS_ARENA_ITEM (item));

    if (aobject->callbacks) {
        for (unsigned int i = 0; i < aobject->callbacks->length; i++) {
            NRObjectListener *listener = aobject->callbacks->listeners + i;
            NRArenaEventVector *avector = (NRArenaEventVector *) listener->vector;
            if ((listener->size >= sizeof (NRArenaEventVector)) && avector->request_update) {
                avector->request_update (arena, item, listener->data);
            }
        }
    }
}

void
nr_arena_request_render_rect (NRArena *arena, NRRectL *area)
{
    NRActiveObject *aobject = (NRActiveObject *) arena;

    nr_return_if_fail (arena != NULL);
    nr_return_if_fail (NR_IS_ARENA (arena));
    nr_return_if_fail (area != NULL);

    if (aobject->callbacks && area && !nr_rect_l_test_empty_ptr(area)) {
        for (unsigned int i = 0; i < aobject->callbacks->length; i++) {
            NRObjectListener *listener = aobject->callbacks->listeners + i;
            NRArenaEventVector *avector = (NRArenaEventVector *) listener->vector;
            if ((listener->size >= sizeof (NRArenaEventVector)) && avector->request_render) {
                avector->request_render (arena, area, listener->data);
            }
        }
    }
}

void
nr_arena_render_paintserver_fill (NRPixBlock *pb, NRRectL *area, SPPainter *painter, float opacity, NRPixBlock *mask)
{
    NRPixBlock cb, cb_opa;
    nr_pixblock_setup_fast (&cb, NR_PIXBLOCK_MODE_R8G8B8A8N, area->x0, area->y0, area->x1, area->y1, TRUE);
    nr_pixblock_setup_fast (&cb_opa, NR_PIXBLOCK_MODE_R8G8B8A8N, area->x0, area->y0, area->x1, area->y1, TRUE);

    // if memory allocation failed, abort
    if ((cb.size != NR_PIXBLOCK_SIZE_TINY && cb.data.px == NULL) || (cb_opa.size != NR_PIXBLOCK_SIZE_TINY && cb_opa.data.px == NULL)) {
        return;
    }

    cb.visible_area = pb->visible_area;
    cb_opa.visible_area = pb->visible_area;

    /* Need separate gradient buffer (lauris)*/
    // do the filling
    painter->fill (painter, &cb);
    cb.empty = FALSE;

    // do the fill-opacity and mask composite
    if (opacity < 1.0) {
        nr_blit_pixblock_pixblock_alpha (&cb_opa, &cb, (int) floor (255 * opacity));
        cb_opa.empty = FALSE;
        nr_blit_pixblock_pixblock_mask (pb, &cb_opa, mask);
    } else {
        nr_blit_pixblock_pixblock_mask (pb, &cb, mask);
    }

    pb->empty = FALSE;

    nr_pixblock_release (&cb);
    nr_pixblock_release (&cb_opa);
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
