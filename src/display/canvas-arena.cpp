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

#include <gtk/gtk.h>

#include "display/display-forward.h"
#include "display/sp-canvas-util.h"
#include "helper/sp-marshal.h"
#include "display/nr-arena.h"
#include "display/nr-arena-group.h"
#include "display/canvas-arena.h"
#include "display/cairo-utils.h"

enum {
    ARENA_EVENT,
    LAST_SIGNAL
};

static void sp_canvas_arena_class_init(SPCanvasArenaClass *klass);
static void sp_canvas_arena_init(SPCanvasArena *group);
static void sp_canvas_arena_destroy(GtkObject *object);

static void sp_canvas_arena_update (SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags);
static void sp_canvas_arena_render (SPCanvasItem *item, SPCanvasBuf *buf);
static void sp_canvas_arena_render_cache (SPCanvasItem *item, Geom::IntRect const &area);
static void sp_canvas_arena_dirty_cache (SPCanvasArena *arena, NRRectL *area);
static double sp_canvas_arena_point (SPCanvasItem *item, Geom::Point p, SPCanvasItem **actual_item);
static void sp_canvas_arena_visible_area_changed (SPCanvasItem *item, Geom::IntRect const &old_area, Geom::IntRect const &new_area);
static gint sp_canvas_arena_event (SPCanvasItem *item, GdkEvent *event);

static gint sp_canvas_arena_send_event (SPCanvasArena *arena, GdkEvent *event);

static void sp_canvas_arena_request_update (NRArena *arena, NRArenaItem *item, void *data);
static void sp_canvas_arena_request_render (NRArena *arena, NRRectL *area, void *data);

NRArenaEventVector carenaev = {
    {NULL},
    sp_canvas_arena_request_update,
    sp_canvas_arena_request_render
};

static SPCanvasItemClass *parent_class;
static guint signals[LAST_SIGNAL] = {0};

GType
sp_canvas_arena_get_type (void)
{
    static GType type = 0;
    if (!type) {
	GTypeInfo info = {
            sizeof (SPCanvasArenaClass),
	    NULL, NULL,
            (GClassInitFunc) sp_canvas_arena_class_init,
	    NULL, NULL,
            sizeof (SPCanvasArena),
	    0,
            (GInstanceInitFunc) sp_canvas_arena_init,
	    NULL
	};
        type = g_type_register_static (SP_TYPE_CANVAS_ITEM, "SPCanvasArena", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_canvas_arena_class_init (SPCanvasArenaClass *klass)
{
    GtkObjectClass *object_class;
    SPCanvasItemClass *item_class;

    object_class = (GtkObjectClass *) klass;
    item_class = (SPCanvasItemClass *) klass;

    parent_class = (SPCanvasItemClass*)g_type_class_peek_parent (klass);

    signals[ARENA_EVENT] = g_signal_new ("arena_event",
                                           G_TYPE_FROM_CLASS(object_class),
                                           G_SIGNAL_RUN_LAST,
                                           ((glong)((guint8*)&(klass->arena_event) - (guint8*)klass)),
					   NULL, NULL,
                                           sp_marshal_INT__POINTER_POINTER,
                                           GTK_TYPE_INT, 2, GTK_TYPE_POINTER, GTK_TYPE_POINTER);

    object_class->destroy = sp_canvas_arena_destroy;

    item_class->update = sp_canvas_arena_update;
    item_class->render = sp_canvas_arena_render;
    item_class->point = sp_canvas_arena_point;
    item_class->event = sp_canvas_arena_event;
    item_class->visible_area_changed = sp_canvas_arena_visible_area_changed;
}

static void
sp_canvas_arena_init (SPCanvasArena *arena)
{
    arena->sticky = FALSE;

    arena->arena = NRArena::create();
    arena->arena->canvasarena = arena;
    arena->root = NRArenaGroup::create(arena->arena);
    nr_arena_group_set_transparent (NR_ARENA_GROUP (arena->root), TRUE);

    arena->active = NULL;
    arena->cache = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    arena->cache_area = Geom::IntRect::from_xywh(0,0,1,1);
    arena->dirty = cairo_region_create();

    nr_active_object_add_listener ((NRActiveObject *) arena->arena, (NRObjectEventVector *) &carenaev, sizeof (carenaev), arena);
}

static void
sp_canvas_arena_destroy (GtkObject *object)
{
    SPCanvasArena *arena = SP_CANVAS_ARENA (object);

    if (arena->active) {
        nr_object_unref ((NRObject *) arena->active);
        arena->active = NULL;
    }

    if (arena->root) {
        nr_arena_item_unref (arena->root);
        arena->root = NULL;
    }

    if (arena->arena) {
        nr_active_object_remove_listener_by_data ((NRActiveObject *) arena->arena, arena);

        nr_object_unref ((NRObject *) arena->arena);
        arena->arena = NULL;
    }
    if (arena->cache) {
        cairo_surface_destroy(arena->cache);
        arena->cache = NULL;
    }
    cairo_region_destroy(arena->dirty);

    if (GTK_OBJECT_CLASS (parent_class)->destroy)
        (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
sp_canvas_arena_update (SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags)
{
    SPCanvasArena *arena = SP_CANVAS_ARENA (item);

    if (((SPCanvasItemClass *) parent_class)->update)
        (* ((SPCanvasItemClass *) parent_class)->update) (item, affine, flags);

    arena->gc.transform = affine;

    guint reset;
    reset = (flags & SP_CANVAS_UPDATE_AFFINE)? NR_ARENA_ITEM_STATE_ALL : NR_ARENA_ITEM_STATE_NONE;

    nr_arena_item_invoke_update (arena->root, NULL, &arena->gc, NR_ARENA_ITEM_STATE_ALL, reset);

    item->x1 = arena->root->bbox.x0 - 1;
    item->y1 = arena->root->bbox.y0 - 1;
    item->x2 = arena->root->bbox.x1 + 1;
    item->y2 = arena->root->bbox.y1 + 1;

    if (arena->cursor) {
        /* Mess with enter/leave notifiers */
        NRArenaItem *new_arena = nr_arena_item_invoke_pick (arena->root, arena->c, arena->arena->delta, arena->sticky);
        if (new_arena != arena->active) {
            GdkEventCrossing ec;
            ec.window = GTK_WIDGET (item->canvas)->window;
            ec.send_event = TRUE;
            ec.subwindow = ec.window;
            ec.time = GDK_CURRENT_TIME;
            ec.x = arena->c[Geom::X];
            ec.y = arena->c[Geom::Y];
            /* fixme: */
            if (arena->active) {
                ec.type = GDK_LEAVE_NOTIFY;
                sp_canvas_arena_send_event (arena, (GdkEvent *) &ec);
            }
            /* fixme: This is not optimal - better track ::destroy (Lauris) */
            if (arena->active) nr_object_unref ((NRObject *) arena->active);
            arena->active = new_arena;
            if (arena->active) nr_object_ref ((NRObject *) arena->active);
            if (arena->active) {
                ec.type = GDK_ENTER_NOTIFY;
                sp_canvas_arena_send_event (arena, (GdkEvent *) &ec);
            }
        }
    }
}

static void
sp_canvas_arena_render (SPCanvasItem *item, SPCanvasBuf *buf)
{
    SPCanvasArena *arena = SP_CANVAS_ARENA (item);
    //SPCanvas *canvas = item->canvas;

    //nr_arena_item_invoke_update (arena->root, NULL, &arena->gc,
    //                             NR_ARENA_ITEM_STATE_BBOX | NR_ARENA_ITEM_STATE_RENDER,
    //                             NR_ARENA_ITEM_STATE_NONE);

    Geom::OptIntRect r = buf->rect;
    if (!r || r->hasZeroArea()) return;
    
    cairo_rectangle_int_t crect;
    crect.x = r->left();
    crect.y = r->top();
    crect.width = r->width();
    crect.height = r->height();
    if (cairo_region_contains_rectangle(arena->dirty, &crect) != CAIRO_REGION_OVERLAP_OUT) {
        sp_canvas_arena_render_cache(item, *r);
        cairo_region_subtract_rectangle(arena->dirty, &crect);
    }

    cairo_save(buf->ct);
    cairo_translate(buf->ct, -r->left(), -r->top());
    //cairo_rectangle(buf->ct, r->left(), r->top(), r->width(), r->height());
    //cairo_clip(buf->ct);
    cairo_set_source_surface(buf->ct, arena->cache, arena->cache_area.left(), arena->cache_area.top());
    cairo_paint(buf->ct);
    //nr_arena_item_invoke_render (buf->ct, arena->root, &area, NULL, 0);
    cairo_restore(buf->ct);
}

static void sp_canvas_arena_render_cache (SPCanvasItem *item, Geom::IntRect const &area)
{
    SPCanvasArena *arena = SP_CANVAS_ARENA (item);
    
    Geom::OptIntRect r = Geom::intersect(arena->cache_area, area);
    if (!r || r->hasZeroArea()) return; // nothing to do
    
    cairo_t *ct = cairo_create(arena->cache);
    cairo_translate(ct, -arena->cache_area.left(), -arena->cache_area.top());
    
    // clear area to paint
    cairo_rectangle(ct, area.left(), area.top(), area.width(), area.height());
    cairo_clip(ct);
    cairo_save(ct);
    cairo_set_source_rgba(ct, 0,0,0,0);
    cairo_set_operator(ct, CAIRO_OPERATOR_SOURCE);
    cairo_paint(ct);
    cairo_restore(ct);
    
    NRRectL nr_area(r);

    nr_arena_item_invoke_update (arena->root, NULL, &arena->gc,
                                 NR_ARENA_ITEM_STATE_BBOX | NR_ARENA_ITEM_STATE_RENDER,
                                 NR_ARENA_ITEM_STATE_NONE);
    nr_arena_item_invoke_render (ct, arena->root, &nr_area, NULL, 0);

    cairo_destroy(ct);
}

static void
sp_canvas_arena_dirty_cache (SPCanvasArena *arena, NRRectL *area)
{
    cairo_rectangle_int_t rect;
    rect.x = area->x0;
    rect.y = area->y0;
    rect.width = area->x1 - area->x0;
    rect.height = area->y1 - area->y0;
    cairo_region_union_rectangle(arena->dirty, &rect);
}

static double
sp_canvas_arena_point (SPCanvasItem *item, Geom::Point p, SPCanvasItem **actual_item)
{
    SPCanvasArena *arena = SP_CANVAS_ARENA (item);

    nr_arena_item_invoke_update (arena->root, NULL, &arena->gc,
                                 NR_ARENA_ITEM_STATE_BBOX | NR_ARENA_ITEM_STATE_PICK,
                                 NR_ARENA_ITEM_STATE_NONE);

    NRArenaItem *picked = nr_arena_item_invoke_pick (arena->root, p, arena->arena->delta, arena->sticky);

    arena->picked = picked;

    if (picked) {
        *actual_item = item;
        return 0.0;
    }

    return 1e18;
}

static void
sp_canvas_arena_visible_area_changed (SPCanvasItem *item, Geom::IntRect const &old_area, Geom::IntRect const &new_area)
{
    SPCanvasArena *arena = SP_CANVAS_ARENA(item);

    cairo_surface_t *new_cache = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
        new_area.width(), new_area.height());
    cairo_t *ct = cairo_create(new_cache);
    cairo_set_source_surface(ct, arena->cache, old_area.left() - new_area.left(), old_area.top() - new_area.top());
    cairo_set_operator(ct, CAIRO_OPERATOR_SOURCE);
    cairo_paint(ct);
    cairo_destroy(ct);
    cairo_surface_destroy(arena->cache);
    arena->cache = new_cache;
    arena->cache_area = new_area;

    cairo_rectangle_int_t crect;
    crect.x = new_area.left();
    crect.y = new_area.top();
    crect.width = new_area.width();
    crect.height = new_area.height();
    cairo_region_intersect_rectangle(arena->dirty, &crect);

    // invalidate newly exposed areas
    /*
     * +----------------------+
     * |      top strip       |
     * +-------+------+-------+
     * |       |      |       |
     * | left  | old  | right |
     * | strip | area | strip |
     * |       |      |       |
     * +-------+------+-------+
     * |     bottom strip     |
     * +----------------------+
     */
    
    // top strip
    if (new_area.top() < old_area.top()) {
        NRRectL top_strip(new_area.left(), new_area.top(), new_area.right(), old_area.top());
        sp_canvas_arena_dirty_cache(arena, &top_strip);
    }
    // left strip
    if (new_area.left() < old_area.left()) {
        NRRectL left_strip(new_area.left(), std::max(new_area.top(), old_area.top()),
            old_area.left(), std::min(new_area.bottom(), old_area.bottom()));
        sp_canvas_arena_dirty_cache(arena, &left_strip);
    }
    // right strip
    if (new_area.right() > old_area.right()) {
        NRRectL right_strip(old_area.right(), std::max(new_area.top(), old_area.top()),
            new_area.right(), std::min(new_area.bottom(), old_area.bottom()));
        sp_canvas_arena_dirty_cache(arena, &right_strip);
    }
    // bottom strip
    if (new_area.bottom() > old_area.bottom()) {
        NRRectL bottom_strip(new_area.left(), old_area.bottom(), new_area.right(), new_area.bottom());
        sp_canvas_arena_dirty_cache(arena, &bottom_strip);
    }
}

static gint
sp_canvas_arena_event (SPCanvasItem *item, GdkEvent *event)
{
    NRArenaItem *new_arena;
    /* fixme: This sucks, we have to handle enter/leave notifiers */

    SPCanvasArena *arena = SP_CANVAS_ARENA (item);

    gint ret = FALSE;

    switch (event->type) {
        case GDK_ENTER_NOTIFY:
            if (!arena->cursor) {
                if (arena->active) {
                    //g_warning ("Cursor entered to arena with already active item");
                    nr_object_unref ((NRObject *) arena->active);
                }
                arena->cursor = TRUE;

                /* TODO ... event -> arena transform? */
                arena->c = Geom::Point(event->crossing.x, event->crossing.y);

                /* fixme: Not sure abut this, but seems the right thing (Lauris) */
                nr_arena_item_invoke_update (arena->root, NULL, &arena->gc, NR_ARENA_ITEM_STATE_PICK, NR_ARENA_ITEM_STATE_NONE);
                arena->active = nr_arena_item_invoke_pick (arena->root, arena->c, arena->arena->delta, arena->sticky);
                if (arena->active) nr_object_ref ((NRObject *) arena->active);
                ret = sp_canvas_arena_send_event (arena, event);
            }
            break;

        case GDK_LEAVE_NOTIFY:
            if (arena->cursor) {
                ret = sp_canvas_arena_send_event (arena, event);
                if (arena->active) nr_object_unref ((NRObject *) arena->active);
                arena->active = NULL;
                arena->cursor = FALSE;
            }
            break;

        case GDK_MOTION_NOTIFY:
            /* TODO ... event -> arena transform? */
            arena->c = Geom::Point(event->motion.x, event->motion.y);

            /* fixme: Not sure abut this, but seems the right thing (Lauris) */
            nr_arena_item_invoke_update (arena->root, NULL, &arena->gc, NR_ARENA_ITEM_STATE_PICK, NR_ARENA_ITEM_STATE_NONE);
            new_arena = nr_arena_item_invoke_pick (arena->root, arena->c, arena->arena->delta, arena->sticky);
            if (new_arena != arena->active) {
                GdkEventCrossing ec;
                ec.window = event->motion.window;
                ec.send_event = event->motion.send_event;
                ec.subwindow = event->motion.window;
                ec.time = event->motion.time;
                ec.x = event->motion.x;
                ec.y = event->motion.y;
                /* fixme: */
                if (arena->active) {
                    ec.type = GDK_LEAVE_NOTIFY;
                    ret = sp_canvas_arena_send_event (arena, (GdkEvent *) &ec);
                }
                if (arena->active) nr_object_unref ((NRObject *) arena->active);
                arena->active = new_arena;
                if (arena->active) nr_object_ref ((NRObject *) arena->active);
                if (arena->active) {
                    ec.type = GDK_ENTER_NOTIFY;
                    ret = sp_canvas_arena_send_event (arena, (GdkEvent *) &ec);
                }
            }
            ret = sp_canvas_arena_send_event (arena, event);
            break;

        default:
            /* Just send event */
            ret = sp_canvas_arena_send_event (arena, event);
            break;
    }

    return ret;
}

static gint
sp_canvas_arena_send_event (SPCanvasArena *arena, GdkEvent *event)
{
    gint ret = FALSE;

    /* Send event to arena */
    g_signal_emit (G_OBJECT (arena), signals[ARENA_EVENT], 0, arena->active, event, &ret);

    return ret;
}

static void
sp_canvas_arena_request_update (NRArena */*arena*/, NRArenaItem */*item*/, void *data)
{
    sp_canvas_item_request_update (SP_CANVAS_ITEM (data));
}

static void
sp_canvas_arena_request_render (NRArena */*arena*/, NRRectL *area, void *data)
{
    if (!area) return;
    sp_canvas_arena_dirty_cache (SP_CANVAS_ARENA(data), area);
    sp_canvas_request_redraw (SP_CANVAS_ITEM (data)->canvas, area->x0, area->y0, area->x1, area->y1);
}

void
sp_canvas_arena_set_pick_delta (SPCanvasArena *ca, gdouble delta)
{
    g_return_if_fail (ca != NULL);
    g_return_if_fail (SP_IS_CANVAS_ARENA (ca));

    /* fixme: repick? */
    ca->delta = delta;
}

void
sp_canvas_arena_set_sticky (SPCanvasArena *ca, gboolean sticky)
{
    g_return_if_fail (ca != NULL);
    g_return_if_fail (SP_IS_CANVAS_ARENA (ca));

    /* fixme: repick? */
    ca->sticky = sticky;
}

void
sp_canvas_arena_render_surface (SPCanvasArena *ca, cairo_surface_t *surface, NRRectL const &r)
{
    g_return_if_fail (ca != NULL);
    g_return_if_fail (SP_IS_CANVAS_ARENA (ca));

    cairo_t *ct = cairo_create(surface);
    cairo_translate(ct, -r.x0, -r.y0);
    nr_arena_item_invoke_render (ct, ca->root, &r, NULL, 0);
    cairo_destroy(ct);
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
