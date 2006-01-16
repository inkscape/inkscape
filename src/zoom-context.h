#ifndef __SP_ZOOM_CONTEXT_H__
#define __SP_ZOOM_CONTEXT_H__

/*
 * Handy zooming tool
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "event-context.h"

#define SP_TYPE_ZOOM_CONTEXT (sp_zoom_context_get_type ())
#define SP_ZOOM_CONTEXT(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_ZOOM_CONTEXT, SPZoomContext))
#define SP_IS_ZOOM_CONTEXT(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_ZOOM_CONTEXT))

class SPZoomContext;
class SPZoomContextClass;

struct SPZoomContext {
	SPEventContext event_context;
};

struct SPZoomContextClass {
	SPEventContextClass parent_class;
};

GType sp_zoom_context_get_type (void);

#endif
