#ifndef SEEN_SP_MEASURING_CONTEXT_H
#define SEEN_SP_MEASURING_CONTEXT_H

/*
 * Our fine measuring tool
 *
 * Authors:
 *   Felipe Correa da Silva Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2011 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "event-context.h"
#include <2geom/point.h>
#include <boost/optional.hpp>

#define SP_MEASURE_CONTEXT(obj) (dynamic_cast<SPMeasureContext*>((SPEventContext*)obj))
#define SP_IS_MEASURE_CONTEXT(obj) (dynamic_cast<const SPMeasureContext*>((const SPEventContext*)obj) != NULL)

class SPMeasureContext : public SPEventContext {
public:
	SPMeasureContext();
	virtual ~SPMeasureContext();

	static const std::string prefsPath;

	virtual void finish();
	virtual bool root_handler(GdkEvent* event);

	virtual const std::string& getPrefsPath();

private:
	SPCanvasItem* grabbed;

    Geom::Point start_point;
    boost::optional<Geom::Point> explicitBase;
    boost::optional<Geom::Point> lastEnd;
};

#endif // SEEN_SP_MEASURING_CONTEXT_H
