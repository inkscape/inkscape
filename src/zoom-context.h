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

#define SP_ZOOM_CONTEXT(obj) (dynamic_cast<SPZoomContext*>((SPEventContext*)obj))
#define SP_IS_ZOOM_CONTEXT(obj) (dynamic_cast<const SPZoomContext*>((const SPEventContext*)obj) != NULL)

class SPZoomContext : public SPEventContext {
public:
	SPZoomContext();
	virtual ~SPZoomContext();

	static const std::string prefsPath;

	virtual void setup();
	virtual void finish();
	virtual bool root_handler(GdkEvent* event);

	virtual const std::string& getPrefsPath();

private:
	SPCanvasItem *grabbed;
	bool escaped;
};

#endif
