#ifndef __SP_BOX3D_CONTEXT_H__
#define __SP_BOX3D_CONTEXT_H__

/*
 * 3D box drawing context
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 * Copyright (C) 2007 Maximilian Albert <Anhalter42@gmx.de>
 *
 * Released under GNU GPL
 */

#include <stddef.h>

#include <2geom/point.h>
#include <sigc++/connection.h>

#include "proj_pt.h"
#include "vanishing-point.h"

#include "ui/tools/tool-base.h"

class SPItem;
class SPBox3D;

namespace Box3D {
    struct VPDrag;
}

namespace Inkscape {
    class Selection;
}

#define SP_BOX3D_CONTEXT(obj) (dynamic_cast<Inkscape::UI::Tools::Box3dTool*>((Inkscape::UI::Tools::ToolBase*)obj))
#define SP_IS_BOX3D_CONTEXT(obj) (dynamic_cast<const Inkscape::UI::Tools::Box3dTool*>((const Inkscape::UI::Tools::ToolBase*)obj) != NULL)

namespace Inkscape {
namespace UI {
namespace Tools {

class Box3dTool : public ToolBase {
public:
	Box3dTool();
	virtual ~Box3dTool();

	Box3D::VPDrag * _vpdrag;

	static const std::string prefsPath;

	virtual void setup();
	virtual void finish();
	virtual bool root_handler(GdkEvent* event);
	virtual bool item_handler(SPItem* item, GdkEvent* event);

	virtual const std::string& getPrefsPath();

private:
    SPBox3D* box3d;
    Geom::Point center;

    /**
     * save three corners while dragging:
     * 1) the starting point (already done by the event_context)
     * 2) drag_ptB --> the opposite corner of the front face (before pressing shift)
     * 3) drag_ptC --> the "extruded corner" (which coincides with the mouse pointer location
     *    if we are ctrl-dragging but is constrained to the perspective line from drag_ptC
     *    to the vanishing point Y otherwise)
     */
    Geom::Point drag_origin;
    Geom::Point drag_ptB;
    Geom::Point drag_ptC;

    Proj::Pt3 drag_origin_proj;
    Proj::Pt3 drag_ptB_proj;
    Proj::Pt3 drag_ptC_proj;

    bool ctrl_dragged; /* whether we are ctrl-dragging */
    bool extruded; /* whether shift-dragging already occurred (i.e. the box is already extruded) */

    sigc::connection sel_changed_connection;

	void selection_changed(Inkscape::Selection* selection);

	void drag(guint state);
	void finishItem();
};

}
}
}

#endif /* __SP_BOX3D_CONTEXT_H__ */

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
