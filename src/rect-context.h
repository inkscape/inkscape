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

#include <stddef.h>
#include <sigc++/sigc++.h>
#include <2geom/point.h>
#include "event-context.h"

#include "sp-rect.h"

#define SP_RECT_CONTEXT(obj) (dynamic_cast<SPRectContext*>((SPEventContext*)obj))
#define SP_IS_RECT_CONTEXT(obj) (dynamic_cast<const SPRectContext*>((const SPEventContext*)obj) != NULL)

class SPRectContext : public SPEventContext {
public:
	SPRectContext();
	virtual ~SPRectContext();

	static const std::string prefsPath;

	virtual void setup();
	virtual void finish();
	virtual void set(const Inkscape::Preferences::Entry& val);
	virtual bool root_handler(GdkEvent* event);
	virtual bool item_handler(SPItem* item, GdkEvent* event);

	virtual const std::string& getPrefsPath();

private:
	SPRect *rect;
	Geom::Point center;

  	gdouble rx;	/* roundness radius (x direction) */
  	gdouble ry;	/* roundness radius (y direction) */

	sigc::connection sel_changed_connection;

	void drag(Geom::Point const pt, guint state);
	void finishItem();
	void cancel();
	void selection_changed(Inkscape::Selection* selection);
};

#endif
