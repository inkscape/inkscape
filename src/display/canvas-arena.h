#ifndef SEEN_SP_CANVAS_ARENA_H
#define SEEN_SP_CANVAS_ARENA_H

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

#include <2geom/rect.h>

#include "display/drawing.h"
#include "display/drawing-item.h"
#include "display/sp-canvas.h"
#include "display/sp-canvas-item.h"

#define SP_TYPE_CANVAS_ARENA (sp_canvas_arena_get_type ())
#define SP_CANVAS_ARENA(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_CANVAS_ARENA, SPCanvasArena))
#define SP_CANVAS_ARENA_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_CANVAS_ARENA, SPCanvasArenaClass))
#define SP_IS_CANVAS_ARENA(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_CANVAS_ARENA))
#define SP_IS_CANVAS_ARENA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_CANVAS_ARENA))

typedef struct _SPCanvasArena      SPCanvasArena;
typedef struct _SPCanvasArenaClass SPCanvasArenaClass;
typedef struct _cairo_surface      cairo_surface_t;
struct CachePrefObserver;

namespace Inkscape {

class Drawing;
class DrawingItem;

} // namespace Inkscape

struct _SPCanvasArena {
    SPCanvasItem item;

    guint cursor : 1;
    guint sticky : 1;
    Geom::Point c; // what is this?

    Inkscape::Drawing drawing;
    Inkscape::UpdateContext ctx;

    Inkscape::DrawingItem *active;
    /* fixme: */
    Inkscape::DrawingItem *picked;
    CachePrefObserver *observer;
    double delta;
};

struct _SPCanvasArenaClass {
    SPCanvasItemClass parent_class;

    gint (* arena_event) (SPCanvasArena *carena, Inkscape::DrawingItem *item, GdkEvent *event);
};

GType sp_canvas_arena_get_type (void);

void sp_canvas_arena_set_pick_delta (SPCanvasArena *ca, gdouble delta);
void sp_canvas_arena_set_sticky (SPCanvasArena *ca, gboolean sticky);

void sp_canvas_arena_render_surface (SPCanvasArena *ca, cairo_surface_t *surface, Geom::IntRect const &area);

#endif // SEEN_SP_CANVAS_ARENA_H
