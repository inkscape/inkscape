#ifndef SEEN_ARC_CONTEXT_H
#define SEEN_ARC_CONTEXT_H

/*
 * Ellipse drawing context
 *
 * Authors:
 *   Mitsuru Oka
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2000-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Mitsuru Oka
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <stddef.h>
#include <sigc++/connection.h>

#include <2geom/point.h>
#include "event-context.h"

#include "sp-ellipse.h"

#define SP_ARC_CONTEXT(obj) (dynamic_cast<SPArcContext*>((SPEventContext*)obj))
#define SP_IS_ARC_CONTEXT(obj) (dynamic_cast<const SPArcContext*>(const SPEventContext*(obj)) != NULL)

class SPArcContext : public SPEventContext {
public:
	SPArcContext();
	virtual ~SPArcContext();

	static const std::string prefsPath;

	virtual void setup();
	virtual void finish();
	virtual bool root_handler(GdkEvent* event);
	virtual bool item_handler(SPItem* item, GdkEvent* event);

	virtual const std::string& getPrefsPath();

private:
	SPGenericEllipse *arc;

    Geom::Point center;

    sigc::connection sel_changed_connection;

	void selection_changed(Inkscape::Selection* selection);

	void drag(Geom::Point pt, guint state);
	void finishItem();
	void cancel();
};

#endif /* !SEEN_ARC_CONTEXT_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
