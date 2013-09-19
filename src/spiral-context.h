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

#include "sp-spiral.h"

#define SP_SPIRAL_CONTEXT(obj) (dynamic_cast<SPSpiralContext*>((SPEventContext*)obj))
#define SP_IS_SPIRAL_CONTEXT(obj) (dynamic_cast<const SPSpiralContext*>((const SPEventContext*)obj) != NULL)

class SPSpiralContext : public SPEventContext {
public:
	SPSpiralContext();
	virtual ~SPSpiralContext();

	static const std::string prefsPath;

	virtual void setup();
	virtual void finish();
	virtual void set(const Inkscape::Preferences::Entry& val);
	virtual bool root_handler(GdkEvent* event);

	virtual const std::string& getPrefsPath();

private:
	SPSpiral * spiral;
	Geom::Point center;
	gdouble revo;
	gdouble exp;
	gdouble t0;

    sigc::connection sel_changed_connection;

	void drag(Geom::Point const &p, guint state);
	void finishItem();
	void cancel();
	void selection_changed(Inkscape::Selection *selection);
};

#endif
