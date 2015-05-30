/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UI_TOOL_CURVE_DRAG_POINT_H
#define SEEN_UI_TOOL_CURVE_DRAG_POINT_H

#include "ui/tool/control-point.h"
#include "ui/tool/node.h"

class SPDesktop;
namespace Inkscape {
namespace UI {

class PathManipulator;
struct PathSharedData;

// This point should be invisible to the user - use the invisible_cset from control-point.h
// TODO make some methods from path-manipulator.cpp public so that this point doesn't have
// to be declared as a friend
/**
 * An invisible point used to drag curves. This point is used by PathManipulator to allow editing
 * of path segments by dragging them. It is defined in a separate file so that the node tool
 * can check if the mouseovered control point is a curve drag point and update the cursor
 * accordingly, without the need to drag in the full PathManipulator header.
 */
class CurveDragPoint : public ControlPoint {
public:

    CurveDragPoint(PathManipulator &pm);
    void setSize(double sz) { _setSize(sz); }
    void setTimeValue(double t) { _t = t; }
    double getTimeValue() { return _t; }
    void setIterator(NodeList::iterator i) { first = i; }
    NodeList::iterator getIterator() { return first; }
    virtual bool _eventHandler(Inkscape::UI::Tools::ToolBase *event_context, GdkEvent *event);

protected:

    virtual Glib::ustring _getTip(unsigned state) const;
    virtual void dragged(Geom::Point &, GdkEventMotion *);
    virtual bool grabbed(GdkEventMotion *);
    virtual void ungrabbed(GdkEventButton *);
    virtual bool clicked(GdkEventButton *);
    virtual bool doubleclicked(GdkEventButton *);

private:
    double _t;
    PathManipulator &_pm;
    NodeList::iterator first;

    static bool _drags_stroke;
    static bool _segment_was_degenerate;
    static Geom::Point _stroke_drag_origin;
    void _insertNode(bool take_selection);
};

} // namespace UI
} // namespace Inkscape

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
