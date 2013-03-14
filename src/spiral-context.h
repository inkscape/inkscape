#ifndef __SP_SPIRAL_CONTEXT_H__
#define __SP_SPIRAL_CONTEXT_H__

/** \file
 * Spiral drawing context
 */
/*
 * Authors:
 *   Mitsuru Oka
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2001 Lauris Kaplinski
 * Copyright (C) 2001-2002 Mitsuru Oka
 *
 * Released under GNU GPL
 */

#include <gtk/gtk.h>
#include <stddef.h>
#include <sigc++/sigc++.h>
#include <2geom/point.h>
#include "event-context.h"

#define SP_TYPE_SPIRAL_CONTEXT            (sp_spiral_context_get_type ())
#define SP_SPIRAL_CONTEXT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_SPIRAL_CONTEXT, SPSpiralContext))
#define SP_SPIRAL_CONTEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_SPIRAL_CONTEXT, SPSpiralContextClass))
#define SP_IS_SPIRAL_CONTEXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_SPIRAL_CONTEXT))
#define SP_IS_SPIRAL_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_SPIRAL_CONTEXT))

struct SPSpiralContext : public SPEventContext {
	SPItem * item;
	Geom::Point center;
	gdouble revo;
	gdouble exp;
	gdouble t0;

    sigc::connection sel_changed_connection;

    Inkscape::MessageContext *_message_context;
};

struct SPSpiralContextClass {
	SPEventContextClass parent_class;
};

/* Standard Gtk function */

GType sp_spiral_context_get_type (void);

#endif
