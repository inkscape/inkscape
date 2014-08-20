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

#include "ui/tools/tool-base.h"
#include <2geom/point.h>
#include <boost/optional.hpp>

#define SP_MEASURE_CONTEXT(obj) (dynamic_cast<Inkscape::UI::Tools::MeasureTool*>((Inkscape::UI::Tools::ToolBase*)obj))
#define SP_IS_MEASURE_CONTEXT(obj) (dynamic_cast<const Inkscape::UI::Tools::MeasureTool*>((const Inkscape::UI::Tools::ToolBase*)obj) != NULL)

namespace Inkscape {
namespace UI {
namespace Tools {

class MeasureTool : public ToolBase {
public:
	MeasureTool();
	virtual ~MeasureTool();

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

}
}
}

#endif // SEEN_SP_MEASURING_CONTEXT_H
