/** @file
 * Control point that is dragged during path drag
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
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

class CurveDragPoint : public ControlPoint {
public:
    CurveDragPoint(PathManipulator &pm);
    void setSize(double sz) { _setSize(sz); }
    void setTimeValue(double t) { _t = t; }
    void setIterator(NodeList::iterator i) { first = i; }
    sigc::signal<void> signal_update;
protected:
    virtual Glib::ustring _getTip(unsigned state);
private:
    void _grabbedHandler(GdkEventMotion *);
    void _draggedHandler(Geom::Point const &, Geom::Point const &);
    bool _clickedHandler(GdkEventButton *);
    bool _doubleclickedHandler(GdkEventButton *);
    void _ungrabbedHandler();
    void _insertNode(bool take_selection);
    double _t;
    PathManipulator &_pm;
    NodeList::iterator first;
    static bool _drags_stroke;
    static Geom::Point _stroke_drag_origin;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
