/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/tool/curve-drag-point.h"
#include <glib/gi18n.h>
#include <2geom/bezier-curve.h>
#include "desktop.h"
#include "ui/tool/control-point-selection.h"
#include "ui/tool/event-utils.h"
#include "ui/tool/multi-path-manipulator.h"
#include "ui/tool/path-manipulator.h"
#include "ui/tool/node.h"
#include "sp-namedview.h"
#include "snap.h"

namespace Inkscape {
namespace UI {


bool CurveDragPoint::_drags_stroke = false;
bool CurveDragPoint::_segment_was_degenerate = false;

CurveDragPoint::CurveDragPoint(PathManipulator &pm) :
    ControlPoint(pm._multi_path_manipulator._path_data.node_data.desktop, Geom::Point(), SP_ANCHOR_CENTER,
                 CTRL_TYPE_INVISIPOINT,
                 invisible_cset, pm._multi_path_manipulator._path_data.dragpoint_group),
      _pm(pm)
{
    setVisible(false);
}

bool CurveDragPoint::_eventHandler(Inkscape::UI::Tools::ToolBase *event_context, GdkEvent *event)
{
    // do not process any events when the manipulator is empty
    if (_pm.empty()) {
        setVisible(false);
        return false;
    }
    return ControlPoint::_eventHandler(event_context, event);
}

bool CurveDragPoint::grabbed(GdkEventMotion */*event*/)
{
    _pm._selection.hideTransformHandles();
    NodeList::iterator second = first.next();

    // move the handles to 1/3 the length of the segment for line segments
    if (first->front()->isDegenerate() && second->back()->isDegenerate()) {
        _segment_was_degenerate = true;

        // delta is a vector equal 1/3 of distance from first to second
        Geom::Point delta = (second->position() - first->position()) / 3.0;
        // only update the nodes if the mode is bspline
        if(!_pm._isBSpline()){
            first->front()->move(first->front()->position() + delta);
            second->back()->move(second->back()->position() - delta);
        }
        _pm.update();
    } else {
        _segment_was_degenerate = false;
    }
    return false;
}

void CurveDragPoint::dragged(Geom::Point &new_pos, GdkEventMotion *event)
{
    NodeList::iterator second = first.next();

    // special cancel handling - retract handles when if the segment was degenerate
    if (_is_drag_cancelled(event) && _segment_was_degenerate) {
        first->front()->retract();
        second->back()->retract();
        _pm.update();
        return;
    }

    if (_drag_initiated && !(event->state & GDK_SHIFT_MASK)) {
        SnapManager &m = _desktop->namedview->snap_manager;
        SPItem *path = static_cast<SPItem *>(_pm._path);
        m.setup(_desktop, true, path); // We will not try to snap to "path" itself
        Inkscape::SnapCandidatePoint scp(new_pos, Inkscape::SNAPSOURCE_OTHER_HANDLE);
        Inkscape::SnappedPoint sp = m.freeSnap(scp, Geom::OptRect(), false);
        new_pos = sp.getPoint();
        m.unSetup();
    }

    // Magic Bezier Drag Equations follow!
    // "weight" describes how the influence of the drag should be distributed
    // among the handles; 0 = front handle only, 1 = back handle only.
    double weight, t = _t;
    if (t <= 1.0 / 6.0) weight = 0;
    else if (t <= 0.5) weight = (pow((6 * t - 1) / 2.0, 3)) / 2;
    else if (t <= 5.0 / 6.0) weight = (1 - pow((6 * (1-t) - 1) / 2.0, 3)) / 2 + 0.5;
    else weight = 1;

    Geom::Point delta = new_pos - position();
    Geom::Point offset0 = ((1-weight)/(3*t*(1-t)*(1-t))) * delta;
    Geom::Point offset1 = (weight/(3*t*t*(1-t))) * delta;

    //modified so that, if the trace is bspline, it only acts if the SHIFT key is pressed
    if(!_pm._isBSpline()){
        first->front()->move(first->front()->position() + offset0);
        second->back()->move(second->back()->position() + offset1);
    }else if(weight>=0.8){
        if(held_shift(*event)){
            second->back()->move(new_pos);
        } else {
            second->move(second->position() + delta);
        }
    }else if(weight<=0.2){
        if(held_shift(*event)){
            first->back()->move(new_pos);
        } else {
            first->move(first->position() + delta);
        }
    }else{
        first->move(first->position() + delta);
        second->move(second->position() + delta);
    }
    _pm.update();
}

void CurveDragPoint::ungrabbed(GdkEventButton *)
{
    _pm._updateDragPoint(_desktop->d2w(position()));
    _pm._commit(_("Drag curve"));
    _pm._selection.restoreTransformHandles();
}

bool CurveDragPoint::clicked(GdkEventButton *event)
{
    // This check is probably redundant
    if (!first || event->button != 1) return false;
    // the next iterator can be invalid if we click very near the end of path
    NodeList::iterator second = first.next();
    if (!second) return false;

    // insert nodes on Ctrl+Alt+click
    if (held_control(*event) && held_alt(*event)) {
        _insertNode(false);
        return true;
    }

    if (held_shift(*event)) {
        // if both nodes of the segment are selected, deselect;
        // otherwise add to selection
        if (first->selected() && second->selected())  {
            _pm._selection.erase(first.ptr());
            _pm._selection.erase(second.ptr());
        } else {
            _pm._selection.insert(first.ptr());
            _pm._selection.insert(second.ptr());
        }
    } else {
        // without Shift, take selection
        _pm._selection.clear();
        _pm._selection.insert(first.ptr());
        _pm._selection.insert(second.ptr());
    }
    return true;
}

bool CurveDragPoint::doubleclicked(GdkEventButton *event)
{
    if (event->button != 1 || !first || !first.next()) return false;
    _insertNode(true);
    return true;
}

void CurveDragPoint::_insertNode(bool take_selection)
{
    // The purpose of this call is to make way for the just created node.
    // Otherwise clicks on the new node would only work after the user moves the mouse a bit.
    // PathManipulator will restore visibility when necessary.
    setVisible(false);

    _pm.insertNode(first, _t, take_selection);
}

Glib::ustring CurveDragPoint::_getTip(unsigned state) const
{
    if (_pm.empty()) return "";
    if (!first || !first.next()) return "";
    bool linear = first->front()->isDegenerate() && first.next()->back()->isDegenerate();
    if(state_held_shift(state) && _pm._isBSpline()){
        return C_("Path segment tip",
            "<b>Shift</b>: drag to open or move BSpline handles");
    }
    if (state_held_shift(state)) {
        return C_("Path segment tip",
            "<b>Shift</b>: click to toggle segment selection");
    }
    if (state_held_control(state) && state_held_alt(state)) {
        return C_("Path segment tip",
            "<b>Ctrl+Alt</b>: click to insert a node");
    }
    if(_pm._isBSpline()){
        return C_("Path segment tip",
            "<b>BSpline segment</b>: drag to shape the segment, doubleclick to insert node, "
            "click to select (more: Shift, Ctrl+Alt)");
    }
    if (linear) {
        return C_("Path segment tip",
            "<b>Linear segment</b>: drag to convert to a Bezier segment, "
            "doubleclick to insert node, click to select (more: Shift, Ctrl+Alt)");
    } else {
        return C_("Path segment tip",
            "<b>Bezier segment</b>: drag to shape the segment, doubleclick to insert node, "
            "click to select (more: Shift, Ctrl+Alt)");
    }
}

} // namespace UI
} // namespace Inkscape

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
