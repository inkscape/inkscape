#ifndef __SP_DROPPER_CONTEXT_H__
#define __SP_DROPPER_CONTEXT_H__

/*
 * Tool for picking colors from drawing
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <2geom/point.h>

#include "ui/tools/tool-base.h"

struct SPCanvasItem;

#define SP_DROPPER_CONTEXT(obj) (dynamic_cast<Inkscape::UI::Tools::DropperTool*>((Inkscape::UI::Tools::ToolBase*)obj))
#define SP_IS_DROPPER_CONTEXT(obj) (dynamic_cast<const Inkscape::UI::Tools::DropperTool*>((const Inkscape::UI::Tools::ToolBase*)obj) != NULL)

enum {
      SP_DROPPER_PICK_VISIBLE,
      SP_DROPPER_PICK_ACTUAL  
};

namespace Inkscape {
namespace UI {
namespace Tools {

class DropperTool : public ToolBase {
public:
	DropperTool();
	virtual ~DropperTool();

	static const std::string prefsPath;

	virtual const std::string& getPrefsPath();

	guint32 get_color();

protected:
	virtual void setup();
	virtual void finish();
	virtual bool root_handler(GdkEvent* event);

private:
    double        R;
    double        G;
    double        B;
    double        alpha;

    bool dragging;

    SPCanvasItem* grabbed;
    SPCanvasItem* area;
    Geom::Point centre;
};

}
}
}

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
