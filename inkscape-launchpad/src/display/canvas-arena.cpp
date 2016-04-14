/*
 * RGBA display list system for inkscape
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm.h>

#include "display/sp-canvas-util.h"
#include "helper/sp-marshal.h"
#include "display/canvas-arena.h"
#include "display/cairo-utils.h"
#include "display/drawing-context.h"
#include "display/drawing-item.h"
#include "display/drawing-group.h"
#include "display/drawing-surface.h"
#include "preferences.h"

using namespace Inkscape;

enum {
    ARENA_EVENT,
    LAST_SIGNAL
};

static void sp_canvas_arena_destroy(SPCanvasItem *object);

static void sp_canvas_arena_item_deleted(SPCanvasArena *arena, Inkscape::DrawingItem *item);
static void sp_canvas_arena_update (SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags);
static void sp_canvas_arena_render (SPCanvasItem *item, SPCanvasBuf *buf);
static double sp_canvas_arena_point (SPCanvasItem *item, Geom::Point p, SPCanvasItem **actual_item);
static void sp_canvas_arena_viewbox_changed (SPCanvasItem *item, Geom::IntRect const &new_area);
static gint sp_canvas_arena_event (SPCanvasItem *item, GdkEvent *event);

static gint sp_canvas_arena_send_event (SPCanvasArena *arena, GdkEvent *event);

static void sp_canvas_arena_request_update (SPCanvasArena *ca, DrawingItem *item);
static void sp_canvas_arena_request_render (SPCanvasArena *ca, Geom::IntRect const &area);

static guint signals[LAST_SIGNAL] = {0};

struct CachePrefObserver : public Inkscape::Preferences::Observer {
    CachePrefObserver(SPCanvasArena *arena)
        : Inkscape::Preferences::Observer("/options/renderingcache")
        , _arena(arena)
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        std::vector<Inkscape::Preferences::Entry> v = prefs->getAllEntries(observed_path);
        for (unsigned i=0; i<v.size(); ++i) {
            notify(v[i]);
        }
        prefs->addObserver(*this);
    }
    void notify(Preferences::Entry const &v) {
        Glib::ustring name = v.getEntryName();
        if (name == "size") {
            _arena->drawing.setCacheBudget((1 << 20) * v.getIntLimited(64, 0, 4096));
        }
    }
    SPCanvasArena *_arena;
};

G_DEFINE_TYPE(SPCanvasArena, sp_canvas_arena, SP_TYPE_CANVAS_ITEM);

static void
sp_canvas_arena_class_init (SPCanvasArenaClass *klass)
{
    SPCanvasItemClass *item_class = (SPCanvasItemClass *) klass;

    signals[ARENA_EVENT] = g_signal_new ("arena_event",
                                           G_TYPE_FROM_CLASS(item_class),
                                           G_SIGNAL_RUN_LAST,
                                           ((glong)((guint8*)&(klass->arena_event) - (guint8*)klass)),
					   NULL, NULL,
                                           sp_marshal_INT__POINTER_POINTER,
                                           G_TYPE_INT, 2, G_TYPE_POINTER, G_TYPE_POINTER);

    item_class->destroy = sp_canvas_arena_destroy;
    item_class->update = sp_canvas_arena_update;
    item_class->render = sp_canvas_arena_render;
    item_class->point = sp_canvas_arena_point;
    item_class->event = sp_canvas_arena_event;
    item_class->viewbox_changed = sp_canvas_arena_viewbox_changed;
}

static void
sp_canvas_arena_init (SPCanvasArena *arena)
{
    arena->sticky = FALSE;

    new (&arena->drawing) Inkscape::Drawing(arena);

    Inkscape::DrawingGroup *root = new DrawingGroup(arena->drawing);
    root->setPickChildren(true);
    arena->drawing.setRoot(root);

    arena->observer = new CachePrefObserver(arena);

    arena->drawing.signal_request_update.connect(
        sigc::bind<0>(
            sigc::ptr_fun(&sp_canvas_arena_request_update),
            arena));
    arena->drawing.signal_request_render.connect(
        sigc::bind<0>(
            sigc::ptr_fun(&sp_canvas_arena_request_render),
            arena));
    arena->drawing.signal_item_deleted.connect(
        sigc::bind<0>(
            sigc::ptr_fun(&sp_canvas_arena_item_deleted),
            arena));

    arena->active = NULL;
}

static void sp_canvas_arena_destroy(SPCanvasItem *object)
{
    SPCanvasArena *arena = SP_CANVAS_ARENA(object);

    delete arena->observer;
    arena->drawing.~Drawing();

    if (SP_CANVAS_ITEM_CLASS(sp_canvas_arena_parent_class)->destroy)
        SP_CANVAS_ITEM_CLASS(sp_canvas_arena_parent_class)->destroy(object);
}

static void
sp_canvas_arena_update (SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags)
{
    SPCanvasArena *arena = SP_CANVAS_ARENA (item);

    if (SP_CANVAS_ITEM_CLASS(sp_canvas_arena_parent_class)->update)
        SP_CANVAS_ITEM_CLASS(sp_canvas_arena_parent_class)->update(item, affine, flags);

    arena->ctx.ctm = affine;

    unsigned reset = flags & SP_CANVAS_UPDATE_AFFINE ? DrawingItem::STATE_ALL : 0;
    arena->drawing.update(Geom::IntRect::infinite(), arena->ctx, DrawingItem::STATE_ALL, reset);

    Geom::OptIntRect b = arena->drawing.root()->visualBounds();
    if (b) {
        item->x1 = b->left() - 1;
        item->y1 = b->top() - 1;
        item->x2 = b->right() + 1;
        item->y2 = b->bottom() + 1;
    }

    if (arena->cursor) {
        /* Mess with enter/leave notifiers */
        DrawingItem *new_arena = arena->drawing.pick(arena->c, arena->drawing.delta, arena->sticky);
        if (new_arena != arena->active) {
            GdkEventCrossing ec;
            ec.window = gtk_widget_get_window (GTK_WIDGET (item->canvas));
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
            arena->active = new_arena;
            if (arena->active) {
                ec.type = GDK_ENTER_NOTIFY;
                sp_canvas_arena_send_event (arena, (GdkEvent *) &ec);
            }
        }
    }
}

static void
sp_canvas_arena_item_deleted(SPCanvasArena *arena, Inkscape::DrawingItem *item)
{
    if (arena->active == item) {
        arena->active = NULL;
    }
}

static void
sp_canvas_arena_render (SPCanvasItem *item, SPCanvasBuf *buf)
{
    // todo: handle NR_ARENA_ITEM_RENDER_NO_CACHE
    SPCanvasArena *arena = SP_CANVAS_ARENA (item);

    Geom::OptIntRect r = buf->rect;
    if (!r || r->hasZeroArea()) return;

    Inkscape::DrawingContext dc(buf->ct, r->min());

    arena->drawing.update(Geom::IntRect::infinite(), arena->ctx);
    arena->drawing.render(dc, *r);
}

static double
sp_canvas_arena_point (SPCanvasItem *item, Geom::Point p, SPCanvasItem **actual_item)
{
    SPCanvasArena *arena = SP_CANVAS_ARENA (item);

    arena->drawing.update(Geom::IntRect::infinite(), arena->ctx, DrawingItem::STATE_PICK | DrawingItem::STATE_BBOX);
    DrawingItem *picked = arena->drawing.pick(p, arena->drawing.delta, arena->sticky);

    arena->picked = picked;

    if (picked) {
        *actual_item = item;
        return 0.0;
    }

    return 1e18;
}

static void
sp_canvas_arena_viewbox_changed (SPCanvasItem *item, Geom::IntRect const &new_area)
{
    SPCanvasArena *arena = SP_CANVAS_ARENA(item);
    // make the cache limit larger than screen to facilitate smooth scrolling
    Geom::IntRect expanded = new_area;
    Geom::IntPoint expansion(new_area.width()/2, new_area.height()/2);
    expanded.expandBy(expansion);
    arena->drawing.setCacheLimit(expanded);
}

static gint
sp_canvas_arena_event (SPCanvasItem *item, GdkEvent *event)
{
    Inkscape::DrawingItem *new_arena;
    /* fixme: This sucks, we have to handle enter/leave notifiers */

    SPCanvasArena *arena = SP_CANVAS_ARENA (item);

    gint ret = FALSE;

    switch (event->type) {
        case GDK_ENTER_NOTIFY:
            if (!arena->cursor) {
                if (arena->active) {
                    //g_warning ("Cursor entered to arena with already active item");
                }
                arena->cursor = TRUE;

                /* TODO ... event -> arena transform? */
                arena->c = Geom::Point(event->crossing.x, event->crossing.y);

                /* fixme: Not sure abut this, but seems the right thing (Lauris) */
                arena->drawing.update(Geom::IntRect::infinite(), arena->ctx,
                    DrawingItem::STATE_PICK | DrawingItem::STATE_BBOX, 0);
                arena->active = arena->drawing.pick(arena->c, arena->drawing.delta, arena->sticky);
                ret = sp_canvas_arena_send_event (arena, event);
            }
            break;

        case GDK_LEAVE_NOTIFY:
            if (arena->cursor) {
                ret = sp_canvas_arena_send_event (arena, event);
                arena->active = NULL;
                arena->cursor = FALSE;
            }
            break;

        case GDK_MOTION_NOTIFY:
            /* TODO ... event -> arena transform? */
            arena->c = Geom::Point(event->motion.x, event->motion.y);

            /* fixme: Not sure abut this, but seems the right thing (Lauris) */
            arena->drawing.update(Geom::IntRect::infinite(), arena->ctx,
                DrawingItem::STATE_PICK | DrawingItem::STATE_BBOX);
            new_arena = arena->drawing.pick(arena->c, arena->drawing.delta, arena->sticky);
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
                arena->active = new_arena;
                if (arena->active) {
                    ec.type = GDK_ENTER_NOTIFY;
                    ret = sp_canvas_arena_send_event (arena, (GdkEvent *) &ec);
                }
            }
            ret = ret || sp_canvas_arena_send_event (arena, event);
            break;

        case GDK_SCROLL: {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            bool wheelzooms = prefs->getBool("/options/wheelzooms/value");
            bool ctrl = (event->scroll.state & GDK_CONTROL_MASK);
            if ((ctrl && !wheelzooms) || (!ctrl && wheelzooms)) {
                /* Zoom is emitted by the canvas as well, ignore here */
                return FALSE;
            }
            ret = sp_canvas_arena_send_event (arena, event);
            break;
            }

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
sp_canvas_arena_request_update (SPCanvasArena *ca, DrawingItem */*item*/)
{
    sp_canvas_item_request_update (SP_CANVAS_ITEM (ca));
}

static void sp_canvas_arena_request_render(SPCanvasArena *ca, Geom::IntRect const &area)
{
    SPCanvas *canvas = SP_CANVAS_ITEM(ca)->canvas;
    canvas->requestRedraw(area.left(), area.top(), area.right(), area.bottom());
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
sp_canvas_arena_render_surface (SPCanvasArena *ca, cairo_surface_t *surface, Geom::IntRect const &r)
{
    g_return_if_fail (ca != NULL);
    g_return_if_fail (SP_IS_CANVAS_ARENA (ca));

    Inkscape::DrawingContext dc(surface, r.min());
    ca->drawing.update(Geom::IntRect::infinite(), ca->ctx);
    ca->drawing.render(dc, r);
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
