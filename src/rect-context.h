#ifndef __SP_RECT_CONTEXT_H__
#define __SP_RECT_CONTEXT_H__

/*
 * Rectangle drawing context
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL
 */

#include <sigc++/sigc++.h>
#include "event-context.h"
#include "libnr/nr-point.h"

#define SP_TYPE_RECT_CONTEXT            (sp_rect_context_get_type ())
#define SP_RECT_CONTEXT(obj)            (GTK_CHECK_CAST ((obj), SP_TYPE_RECT_CONTEXT, SPRectContext))
#define SP_RECT_CONTEXT_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_RECT_CONTEXT, SPRectContextClass))
#define SP_IS_RECT_CONTEXT(obj)         (GTK_CHECK_TYPE ((obj), SP_TYPE_RECT_CONTEXT))
#define SP_IS_RECT_CONTEXT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_RECT_CONTEXT))

class SPRectContext;
class SPRectContextClass;

struct SPRectContext : public SPEventContext {
	SPItem *item;
	NR::Point center;

  	gdouble rx;	/* roundness radius (x direction) */
  	gdouble ry;	/* roundness radius (y direction) */

	sigc::connection sel_changed_connection;

	Inkscape::MessageContext *_message_context;
};

struct SPRectContextClass {
	SPEventContextClass parent_class;
};

/* Standard Gtk function */

GtkType sp_rect_context_get_type (void);

#endif
