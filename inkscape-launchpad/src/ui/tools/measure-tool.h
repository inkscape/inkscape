#ifndef SEEN_SP_MEASURING_CONTEXT_H
#define SEEN_SP_MEASURING_CONTEXT_H

/*
 * Our fine measuring tool
 *
 * Authors:
 *   Felipe Correa da Silva Sanches <juca@members.fsf.org>
 *   Jabiertxo Arraiza <jabier.arraiza@marker.es>
 * Copyright (C) 2011 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stddef.h>
#include <sigc++/sigc++.h>
#include "ui/tools/tool-base.h"
#include <2geom/point.h>
#include "display/canvas-text.h"
#include "display/canvas-temporary-item.h"
#include "ui/control-manager.h"
#include <boost/optional.hpp>

#define SP_MEASURE_CONTEXT(obj) (dynamic_cast<Inkscape::UI::Tools::MeasureTool*>((Inkscape::UI::Tools::ToolBase*)obj))
#define SP_IS_MEASURE_CONTEXT(obj) (dynamic_cast<const Inkscape::UI::Tools::MeasureTool*>((const Inkscape::UI::Tools::ToolBase*)obj) != NULL)

class SPKnot;

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
    virtual void showCanvasItems(bool to_guides = false, bool to_item = false, bool to_phantom = false, Inkscape::XML::Node *measure_repr = NULL);
    virtual void reverseKnots();
    virtual void toGuides();
    virtual void toPhantom();
    virtual void toMarkDimension();
    virtual void toItem();
    virtual void reset();
    virtual void setMarkers();
    virtual void setMarker(bool isStart);
    virtual const std::string& getPrefsPath();
    Geom::Point readMeasurePoint(bool is_start);
    void writeMeasurePoint(Geom::Point point, bool is_start);
    void setGuide(Geom::Point origin, double angle, const char *label);
    void setPoint(Geom::Point origin, Inkscape::XML::Node *measure_repr);
    void setLine(Geom::Point start_point,Geom::Point end_point, bool markers, guint32 color, Inkscape::XML::Node *measure_repr = NULL);
    void setMeasureCanvasText(bool is_angle, double precision, double amount, double fontsize, Glib::ustring unit_name, Geom::Point position, guint32 background, CanvasTextAnchorPositionEnum text_anchor, bool to_item, bool to_phantom, Inkscape::XML::Node *measure_repr);
    void setMeasureCanvasItem(Geom::Point position, bool to_item, bool to_phantom, Inkscape::XML::Node *measure_repr);
    void setMeasureCanvasControlLine(Geom::Point start, Geom::Point end, bool to_item, bool to_phantom, Inkscape::CtrlLineType ctrl_line_type, Inkscape::XML::Node *measure_repr);
    void setLabelText(const char *value, Geom::Point pos, double fontsize, Geom::Coord angle, guint32 background , Inkscape::XML::Node *measure_repr = NULL, CanvasTextAnchorPositionEnum text_anchor = TEXT_ANCHOR_CENTER );
    void knotStartMovedHandler(SPKnot */*knot*/, Geom::Point const &ppointer, guint state);
    void knotEndMovedHandler(SPKnot */*knot*/, Geom::Point const &ppointer, guint state);
    void knotClickHandler(SPKnot *knot, guint state);
    void knotUngrabbedHandler(SPKnot */*knot*/,  unsigned int /*state*/);
private:
    SPCanvasItem* grabbed;
    boost::optional<Geom::Point> explicit_base;
    boost::optional<Geom::Point> last_end;
    SPKnot *knot_start;
    SPKnot *knot_end;
    gint dimension_offset;
    Geom::Point start_p;
    Geom::Point end_p;
    std::vector<SPCanvasItem *> measure_tmp_items;
    std::vector<SPCanvasItem *> measure_phantom_items;
    sigc::connection _knot_start_moved_connection;
    sigc::connection _knot_start_ungrabbed_connection;
    sigc::connection _knot_start_click_connection;
    sigc::connection _knot_end_moved_connection;
    sigc::connection _knot_end_click_connection;
    sigc::connection _knot_end_ungrabbed_connection;
};

}
}
}

#endif // SEEN_SP_MEASURING_CONTEXT_H

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
