#ifndef __NR_ARENA_H__
#define __NR_ARENA_H__

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

#include <glib/gmacros.h>

#include "display/rendermode.h"

G_BEGIN_DECLS

typedef struct _SPCanvasArena      SPCanvasArena;

G_END_DECLS

#define NR_TYPE_ARENA (nr_arena_get_type ())
#define NR_ARENA(o) (NR_CHECK_INSTANCE_CAST ((o), NR_TYPE_ARENA, NRArena))
#define NR_IS_ARENA(o) (NR_CHECK_INSTANCE_TYPE ((o), NR_TYPE_ARENA))

#include <libnr/nr-forward.h>
#include <libnr/nr-object.h>
#include "nr-arena-forward.h"
#include "sp-paint-server.h"

NRType nr_arena_get_type (void);

struct NRArenaEventVector {
	NRObjectEventVector parent;
	void (* request_update) (NRArena *arena, NRArenaItem *item, void *data);
	void (* request_render) (NRArena *arena, NRRectL *area, void *data);
};

struct NRArena : public NRActiveObject {
	static NRArena *create() {
		return reinterpret_cast<NRArena *>(nr_object_new(NR_TYPE_ARENA));
	}

	double delta;
	Inkscape::RenderMode rendermode;
	guint32 outlinecolor;
	SPCanvasArena *canvasarena; // may be NULL is this arena is not the screen but used for export etc.
};

struct NRArenaClass : public NRActiveObjectClass {
};

void nr_arena_request_update (NRArena *arena, NRArenaItem *item);
void nr_arena_request_render_rect (NRArena *arena, NRRectL *area);

void nr_arena_render_paintserver_fill (NRPixBlock *pb, NRRectL *area, SPPainter *painter, float opacity, NRPixBlock *mask);

#endif
