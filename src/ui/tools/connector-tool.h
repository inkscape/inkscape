#ifndef SEEN_CONNECTOR_CONTEXT_H
#define SEEN_CONNECTOR_CONTEXT_H

/*
 * Connector creation tool
 *
 * Authors:
 *   Michael Wybrow <mjwybrow@users.sourceforge.net>
 *
 * Copyright (C) 2005 Michael Wybrow
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <map>
#include <string>

#include <2geom/point.h>
#include <sigc++/connection.h>

#include "ui/tools/tool-base.h"

class SPItem;
class SPCurve;
class SPKnot;
struct SPCanvasItem;

namespace Avoid {
    class ConnRef;
}

namespace Inkscape {
    class Selection;

    namespace XML {
        class Node;
    }
}

#define SP_CONNECTOR_CONTEXT(obj) (dynamic_cast<Inkscape::UI::Tools::ConnectorTool*>((Inkscape::UI::Tools::ToolBase*)obj))
//#define SP_IS_CONNECTOR_CONTEXT(obj) (dynamic_cast<const ConnectorTool*>((const ToolBase*)obj) != NULL)

enum {
    SP_CONNECTOR_CONTEXT_IDLE,
    SP_CONNECTOR_CONTEXT_DRAGGING,
    SP_CONNECTOR_CONTEXT_CLOSE,
    SP_CONNECTOR_CONTEXT_STOP,
    SP_CONNECTOR_CONTEXT_REROUTING,
    SP_CONNECTOR_CONTEXT_NEWCONNPOINT
};

typedef std::map<SPKnot *, int>  SPKnotList;

namespace Inkscape {
namespace UI {
namespace Tools {

class ConnectorTool : public ToolBase {
public:
	ConnectorTool();
	virtual ~ConnectorTool();

    Inkscape::Selection *selection;
    Geom::Point p[5];

    /** \invar npoints in {0, 2}. */
    gint npoints;
    unsigned int state : 4;

    // Red curve
    SPCanvasItem *red_bpath;
    SPCurve *red_curve;
    guint32 red_color;

    // Green curve
    SPCurve *green_curve;

    // The new connector
    SPItem *newconn;
    Avoid::ConnRef *newConnRef;
    gdouble curvature;
    bool isOrthogonal;

    // The active shape
    SPItem *active_shape;
    Inkscape::XML::Node *active_shape_repr;
    Inkscape::XML::Node *active_shape_layer_repr;

    // Same as above, but for the active connector
    SPItem *active_conn;
    Inkscape::XML::Node *active_conn_repr;
    sigc::connection sel_changed_connection;

    // The activehandle
    SPKnot *active_handle;

    // The selected handle, used in editing mode
    SPKnot *selected_handle;

    SPItem *clickeditem;
    SPKnot *clickedhandle;

    SPKnotList knots;
    SPKnot *endpt_handle[2];
    guint  endpt_handler_id[2];
    gchar *shref;
    gchar *ehref;
    SPCanvasItem *c0, *c1, *cl0, *cl1;

	static const std::string prefsPath;

	virtual void setup();
	virtual void finish();
	virtual void set(const Inkscape::Preferences::Entry& val);
	virtual bool root_handler(GdkEvent* event);
	virtual bool item_handler(SPItem* item, GdkEvent* event);

	virtual const std::string& getPrefsPath();

    void cc_clear_active_shape();
    void cc_set_active_conn(SPItem *item);
    void cc_clear_active_conn();

private:
	void _selectionChanged(Inkscape::Selection *selection);

	bool _handleButtonPress(GdkEventButton const &bevent);
	bool _handleMotionNotify(GdkEventMotion const &mevent);
	bool _handleButtonRelease(GdkEventButton const &revent);
	bool _handleKeyPress(guint const keyval);

	void _setInitialPoint(Geom::Point const p);
	void _setSubsequentPoint(Geom::Point const p);
	void _finishSegment(Geom::Point p);
	void _resetColors();
	void _finish();
	void _concatColorsAndFlush();
	void _flushWhite(SPCurve *gc);

	void _activeShapeAddKnot(SPItem* item);
	void _setActiveShape(SPItem *item);
	bool _ptHandleTest(Geom::Point& p, gchar **href);

	void _reroutingFinish(Geom::Point *const p);
};

void cc_selection_set_avoid(bool const set_ignore);
void cc_create_connection_point(ConnectorTool* cc);
void cc_remove_connection_point(ConnectorTool* cc);
bool cc_item_is_connector(SPItem *item);

}
}
}

#endif /* !SEEN_CONNECTOR_CONTEXT_H */

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
