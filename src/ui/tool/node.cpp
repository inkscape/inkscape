/** @file
 * Editable node - implementation
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <iostream>
#include <stdexcept>
#include <boost/utility.hpp>
#include <glib.h>
#include <glib/gi18n.h>
#include <2geom/bezier-utils.h>
#include <2geom/transforms.h>

#include "display/sp-ctrlline.h"
#include "display/sp-canvas.h"
#include "display/sp-canvas-util.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "preferences.h"
#include "snap.h"
#include "snap-preferences.h"
#include "sp-metrics.h"
#include "sp-namedview.h"
#include "ui/tool/control-point-selection.h"
#include "ui/tool/event-utils.h"
#include "ui/tool/multi-path-manipulator.h"
#include "ui/tool/node.h"
#include "ui/tool/path-manipulator.h"

namespace Inkscape {
namespace UI {

static SelectableControlPoint::ColorSet node_colors = {
    {
        {0xbfbfbf00, 0x000000ff}, // normal fill, stroke
        {0xff000000, 0x000000ff}, // mouseover fill, stroke
        {0xff000000, 0x000000ff}  // clicked fill, stroke
    },
    {0x0000ffff, 0x000000ff}, // normal fill, stroke when selected
    {0xff000000, 0x000000ff}, // mouseover fill, stroke when selected
    {0xff000000, 0x000000ff}  // clicked fill, stroke when selected
};

static ControlPoint::ColorSet handle_colors = {
    {0xffffffff, 0x000000ff}, // normal fill, stroke
    {0xff000000, 0x000000ff}, // mouseover fill, stroke
    {0xff000000, 0x000000ff}  // clicked fill, stroke
};

std::ostream &operator<<(std::ostream &out, NodeType type)
{
    switch(type) {
    case NODE_CUSP: out << 'c'; break;
    case NODE_SMOOTH: out << 's'; break;
    case NODE_AUTO: out << 'a'; break;
    case NODE_SYMMETRIC: out << 'z'; break;
    default: out << 'b'; break;
    }
    return out;
}

/** Computes an unit vector of the direction from first to second control point */
static Geom::Point direction(Geom::Point const &first, Geom::Point const &second) {
    return Geom::unit_vector(second - first);
}

/**
 * @class Handle
 * @brief Control point of a cubic Bezier curve in a path.
 *
 * Handle keeps the node type invariant only for the opposite handle of the same node.
 * Keeping the invariant on node moves is left to the %Node class.
 */

Geom::Point Handle::_saved_other_pos(0, 0);
double Handle::_saved_length = 0.0;
bool Handle::_drag_out = false;

Handle::Handle(NodeSharedData const &data, Geom::Point const &initial_pos, Node *parent)
    : ControlPoint(data.desktop, initial_pos, Gtk::ANCHOR_CENTER, SP_CTRL_SHAPE_CIRCLE, 7.0,
        &handle_colors, data.handle_group)
    , _parent(parent)
    , _degenerate(true)
{
    _cset = &handle_colors;
    _handle_line = sp_canvas_item_new(data.handle_line_group, SP_TYPE_CTRLLINE, NULL);
    setVisible(false);
}
Handle::~Handle()
{
    //sp_canvas_item_hide(_handle_line);
    gtk_object_destroy(GTK_OBJECT(_handle_line));
}

void Handle::setVisible(bool v)
{
    ControlPoint::setVisible(v);
    if (v) sp_canvas_item_show(_handle_line);
    else sp_canvas_item_hide(_handle_line);
}

void Handle::move(Geom::Point const &new_pos)
{
    Handle *other, *towards, *towards_second;
    Node *node_towards; // node in direction of this handle
    Node *node_away; // node in the opposite direction
    if (this == &_parent->_front) {
        other = &_parent->_back;
        node_towards = _parent->_next();
        node_away = _parent->_prev();
        towards = node_towards ? &node_towards->_back : 0;
        towards_second = node_towards ? &node_towards->_front : 0;
    } else {
        other = &_parent->_front;
        node_towards = _parent->_prev();
        node_away = _parent->_next();
        towards = node_towards ? &node_towards->_front : 0;
        towards_second = node_towards ? &node_towards->_back : 0;
    }

    if (Geom::are_near(new_pos, _parent->position())) {
        // The handle becomes degenerate. If the segment between it and the node
        // in its direction becomes linear and there are smooth nodes
        // at its ends, make their handles colinear with the segment
        if (towards && towards->isDegenerate()) {
            if (node_towards->type() == NODE_SMOOTH) {
                towards_second->setDirection(*_parent, *node_towards);
            }
            if (_parent->type() == NODE_SMOOTH) {
                other->setDirection(*node_towards, *_parent);
            }
        }
        setPosition(new_pos);
        return;
    }

    if (_parent->type() == NODE_SMOOTH && Node::_is_line_segment(_parent, node_away)) {
        // restrict movement to the line joining the nodes
        Geom::Point direction = _parent->position() - node_away->position();
        Geom::Point delta = new_pos - _parent->position();
        // project the relative position on the direction line
        Geom::Point new_delta = (Geom::dot(delta, direction)
            / Geom::L2sq(direction)) * direction;
        setRelativePos(new_delta);
        return;
    }

    switch (_parent->type()) {
    case NODE_AUTO:
        _parent->setType(NODE_SMOOTH, false);
        // fall through - auto nodes degrade into smooth nodes
    case NODE_SMOOTH: {
        /* for smooth nodes, we need to rotate the other handle so that it's colinear
         * with the dragged one while conserving length. */
        other->setDirection(new_pos, *_parent);
        } break;
    case NODE_SYMMETRIC:
        // for symmetric nodes, place the other handle on the opposite side
        other->setRelativePos(-(new_pos - _parent->position()));
        break;
    default: break;
    }

    setPosition(new_pos);
}

void Handle::setPosition(Geom::Point const &p)
{
    ControlPoint::setPosition(p);
    sp_ctrlline_set_coords(SP_CTRLLINE(_handle_line), _parent->position(), position());

    // update degeneration info and visibility
    if (Geom::are_near(position(), _parent->position()))
        _degenerate = true;
    else _degenerate = false;
    if (_parent->_handles_shown && _parent->visible() && !_degenerate) {
        setVisible(true);
    } else {
        setVisible(false);
    }
    // If both handles become degenerate, convert to parent cusp node
    if (_parent->isDegenerate()) {
        _parent->setType(NODE_CUSP, false);
    }
}

void Handle::setLength(double len)
{
    if (isDegenerate()) return;
    Geom::Point dir = Geom::unit_vector(relativePos());
    setRelativePos(dir * len);
}

void Handle::retract()
{
    setPosition(_parent->position());
}

void Handle::setDirection(Geom::Point const &from, Geom::Point const &to)
{
    setDirection(to - from);
}

void Handle::setDirection(Geom::Point const &dir)
{
    Geom::Point unitdir = Geom::unit_vector(dir);
    setRelativePos(unitdir * length());
}

char const *Handle::handle_type_to_localized_string(NodeType type)
{
    switch(type) {
    case NODE_CUSP: return _("Cusp node handle");
    case NODE_SMOOTH: return _("Smooth node handle");
    case NODE_SYMMETRIC: return _("Symmetric node handle");
    case NODE_AUTO: return _("Auto-smooth node handle");
    default: return "";
    }
}

bool Handle::grabbed(GdkEventMotion *)
{
    _saved_other_pos = other().position();
    _saved_length = _drag_out ? 0 : length();
    _pm()._handleGrabbed();
    return false;
}

void Handle::dragged(Geom::Point &new_pos, GdkEventMotion *event)
{
    Geom::Point parent_pos = _parent->position();
    Geom::Point origin = _last_drag_origin();
    // with Alt, preserve length
    if (held_alt(*event)) {
        new_pos = parent_pos + Geom::unit_vector(new_pos - parent_pos) * _saved_length;
    }
    // with Ctrl, constrain to M_PI/rotationsnapsperpi increments from vertical
    // and the original position.
    if (held_control(*event)) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        int snaps = 2 * prefs->getIntLimited("/options/rotationsnapsperpi/value", 12, 1, 1000);

        // note: if snapping to the original position is only desired in the original
        // direction of the handle, change 2nd line below to Ray instead of Line
        Geom::Line original_line(parent_pos, origin);
        Geom::Point snap_pos = parent_pos + Geom::constrain_angle(
            Geom::Point(0,0), new_pos - parent_pos, snaps, Geom::Point(1,0));
        Geom::Point orig_pos = original_line.pointAt(original_line.nearestPoint(new_pos));

        if (Geom::distance(snap_pos, new_pos) < Geom::distance(orig_pos, new_pos)) {
            new_pos = snap_pos;
        } else {
            new_pos = orig_pos;
        }
    }
    // with Shift, if the node is cusp, rotate the other handle as well
    if (_parent->type() == NODE_CUSP && !_drag_out) {
        if (held_shift(*event)) {
            Geom::Point other_relpos = _saved_other_pos - parent_pos;
            other_relpos *= Geom::Rotate(Geom::angle_between(origin - parent_pos, new_pos - parent_pos));
            other().setRelativePos(other_relpos);
        } else {
            // restore the position
            other().setPosition(_saved_other_pos);
        }
    }
    _pm().update();
}

void Handle::ungrabbed(GdkEventButton *event)
{
    // hide the handle if it's less than dragtolerance away from the node
    // TODO is this actually desired?
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int drag_tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

    Geom::Point dist = _desktop->d2w(_parent->position()) - _desktop->d2w(position());
    if (dist.length() <= drag_tolerance) {
        move(_parent->position());
    }

    // HACK: If the handle was dragged out, call parent's ungrabbed handler,
    // so that transform handles reappear
    if (_drag_out) {
        _parent->ungrabbed(event);
    }
    _drag_out = false;

    _pm()._handleUngrabbed();
}

bool Handle::clicked(GdkEventButton *event)
{
    _pm()._handleClicked(this, event);
    return true;
}

Handle &Handle::other()
{
    if (this == &_parent->_front) return _parent->_back;
    return _parent->_front;
}

static double snap_increment_degrees() {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int snaps = prefs->getIntLimited("/options/rotationsnapsperpi/value", 12, 1, 1000);
    return 180.0 / snaps;
}

Glib::ustring Handle::_getTip(unsigned state)
{
    char const *more;
    bool can_shift_rotate = _parent->type() == NODE_CUSP && !other().isDegenerate();
    if (can_shift_rotate) {
        more = C_("Path handle tip", "more: Shift, Ctrl, Alt");
    } else {
        more = C_("Path handle tip", "more: Ctrl, Alt");
    }
    if (state_held_alt(state)) {
        if (state_held_control(state)) {
            if (state_held_shift(state) && can_shift_rotate) {
                return format_tip(C_("Path handle tip",
                    "<b>Shift+Ctrl+Alt</b>: preserve length and snap rotation angle to %g° "
                    "increments while rotating both handles"),
                    snap_increment_degrees());
            } else {
                return format_tip(C_("Path handle tip",
                    "<b>Ctrl+Alt</b>: preserve length and snap rotation angle to %g° increments"),
                    snap_increment_degrees());
            }
        } else {
            if (state_held_shift(state) && can_shift_rotate) {
                return C_("Path handle tip",
                    "<b>Shift+Alt:</b> preserve handle length and rotate both handles");
            } else {
                return C_("Path handle tip",
                    "<b>Alt:</b> preserve handle length while dragging");
            }
        }
    } else {
        if (state_held_control(state)) {
            if (state_held_shift(state) && can_shift_rotate) {
                return format_tip(C_("Path handle tip",
                    "<b>Ctrl:</b> snap rotation angle to %g° increments, click to retract"),
                    snap_increment_degrees());
            } else {
                return format_tip(C_("Path handle tip",
                    "<b>Shift+Ctrl:</b> snap rotation angle to %g° increments and rotate both handles"),
                    snap_increment_degrees());
            }
        } else if (state_held_shift(state) && can_shift_rotate) {
            return C_("Path hande tip",
                "<b>Shift</b>: rotate both handles by the same angle");
        }
    }

    switch (_parent->type()) {
    case NODE_AUTO:
        return format_tip(C_("Path handle tip",
            "<b>Auto node handle:</b> drag to convert to smooth node (%s)"), more);
    default:
        return format_tip(C_("Path handle tip",
            "<b>%s:</b> drag to shape the segment (%s)"),
            handle_type_to_localized_string(_parent->type()), more);
    }
}

Glib::ustring Handle::_getDragTip(GdkEventMotion */*event*/)
{
    Geom::Point dist = position() - _last_drag_origin();
    // report angle in mathematical convention
    double angle = Geom::angle_between(Geom::Point(-1,0), position() - _parent->position());
    angle += M_PI; // angle is (-M_PI...M_PI] - offset by +pi and scale to 0...360
    angle *= 360.0 / (2 * M_PI);
    GString *x = SP_PX_TO_METRIC_STRING(dist[Geom::X], _desktop->namedview->getDefaultMetric());
    GString *y = SP_PX_TO_METRIC_STRING(dist[Geom::Y], _desktop->namedview->getDefaultMetric());
    GString *len = SP_PX_TO_METRIC_STRING(length(), _desktop->namedview->getDefaultMetric());
    Glib::ustring ret = format_tip(C_("Path handle tip",
        "Move handle by %s, %s; angle %.2f°, length %s"), x->str, y->str, angle, len->str);
    g_string_free(x, TRUE);
    g_string_free(y, TRUE);
    g_string_free(len, TRUE);
    return ret;
}

/**
 * @class Node
 * @brief Curve endpoint in an editable path.
 *
 * The method move() keeps node type invariants during translations.
 */

Node::Node(NodeSharedData const &data, Geom::Point const &initial_pos)
    : SelectableControlPoint(data.desktop, initial_pos, Gtk::ANCHOR_CENTER,
        SP_CTRL_SHAPE_DIAMOND, 9.0, *data.selection, &node_colors, data.node_group)
    , _front(data, initial_pos, this)
    , _back(data, initial_pos, this)
    , _type(NODE_CUSP)
    , _handles_shown(false)
{
    // NOTE we do not set type here, because the handles are still degenerate
}

// NOTE: not using iterators won't make this much quicker because iterators can be 100% inlined.
Node *Node::_next()
{
    NodeList::iterator n = NodeList::get_iterator(this).next();
    if (n) return n.ptr();
    return NULL;
}
Node *Node::_prev()
{
    NodeList::iterator p = NodeList::get_iterator(this).prev();
    if (p) return p.ptr();
    return NULL;
}

void Node::move(Geom::Point const &new_pos)
{
    // move handles when the node moves.
    Geom::Point old_pos = position();
    Geom::Point delta = new_pos - position();
    setPosition(new_pos);
    _front.setPosition(_front.position() + delta);
    _back.setPosition(_back.position() + delta);

    // if the node has a smooth handle after a line segment, it should be kept colinear
    // with the segment
    _fixNeighbors(old_pos, new_pos);
}

void Node::transform(Geom::Matrix const &m)
{
    Geom::Point old_pos = position();
    setPosition(position() * m);
    _front.setPosition(_front.position() * m);
    _back.setPosition(_back.position() * m);

    /* Affine transforms keep handle invariants for smooth and symmetric nodes,
     * but smooth nodes at ends of linear segments and auto nodes need special treatment */
    _fixNeighbors(old_pos, position());
}

Geom::Rect Node::bounds()
{
    Geom::Rect b(position(), position());
    b.expandTo(_front.position());
    b.expandTo(_back.position());
    return b;
}

void Node::_fixNeighbors(Geom::Point const &old_pos, Geom::Point const &new_pos)
{
    /* This method restores handle invariants for neighboring nodes,
     * and invariants that are based on positions of those nodes for this one. */

    /* Fix auto handles */
    if (_type == NODE_AUTO) _updateAutoHandles();
    if (old_pos != new_pos) {
        if (_next() && _next()->_type == NODE_AUTO) _next()->_updateAutoHandles();
        if (_prev() && _prev()->_type == NODE_AUTO) _prev()->_updateAutoHandles();
    }

    /* Fix smooth handles at the ends of linear segments.
     * Rotate the appropriate handle to be colinear with the segment.
     * If there is a smooth node at the other end of the segment, rotate it too. */
    Handle *handle, *other_handle;
    Node *other;
    if (_is_line_segment(this, _next())) {
        handle = &_back;
        other = _next();
        other_handle = &_next()->_front;
    } else if (_is_line_segment(_prev(), this)) {
        handle = &_front;
        other = _prev();
        other_handle = &_prev()->_back;
    } else return;

    if (_type == NODE_SMOOTH && !handle->isDegenerate()) {
        handle->setDirection(other->position(), new_pos);
    }
    // also update the handle on the other end of the segment
    if (other->_type == NODE_SMOOTH && !other_handle->isDegenerate()) {
        other_handle->setDirection(new_pos, other->position());
    }
}

void Node::_updateAutoHandles()
{
    // Recompute the position of automatic handles.
    // For endnodes, retract both handles. (It's only possible to create an end auto node
    // through the XML editor.)
    if (isEndNode()) {
        _front.retract();
        _back.retract();
        return;
    }

    // Auto nodes automaticaly adjust their handles to give an appearance of smoothness,
    // no matter what their surroundings are.
    Geom::Point vec_next = _next()->position() - position();
    Geom::Point vec_prev = _prev()->position() - position();
    double len_next = vec_next.length(), len_prev = vec_prev.length();
    if (len_next > 0 && len_prev > 0) {
        // "dir" is an unit vector perpendicular to the bisector of the angle created
        // by the previous node, this auto node and the next node.
        Geom::Point dir = Geom::unit_vector((len_prev / len_next) * vec_next - vec_prev);
        // Handle lengths are equal to 1/3 of the distance from the adjacent node.
        _back.setRelativePos(-dir * (len_prev / 3));
        _front.setRelativePos(dir * (len_next / 3));
    } else {
        // If any of the adjacent nodes coincides, retract both handles.
        _front.retract();
        _back.retract();
    }
}

void Node::showHandles(bool v)
{
    _handles_shown = v;
    if (!_front.isDegenerate()) _front.setVisible(v);
    if (!_back.isDegenerate()) _back.setVisible(v);
}

/** Sets the node type and optionally restores the invariants associated with the given type.
 * @param type The type to set
 * @param update_handles Whether to restore invariants associated with the given type.
 *                       Passing false is useful e.g. wen initially creating the path,
 *                       and when making cusp nodes during some node algorithms.
 *                       Pass true when used in response to an UI node type button.
 */
void Node::setType(NodeType type, bool update_handles)
{
    if (type == NODE_PICK_BEST) {
        pickBestType();
        updateState(); // The size of the control might have changed
        return;
    }

    // if update_handles is true, adjust handle positions to match the node type
    // handle degenerate handles appropriately
    if (update_handles) {
        switch (type) {
        case NODE_CUSP:
            // if the existing type is also NODE_CUSP, retract handles
            if (_type == NODE_CUSP) {
                _front.retract();
                _back.retract();
            }
            break;
        case NODE_AUTO:
            // auto handles make no sense for endnodes
            if (isEndNode()) return;
            _updateAutoHandles();
            break;
        case NODE_SMOOTH: {
            // rotate handles to be colinear
            // for degenerate nodes set positions like auto handles
            bool prev_line = _is_line_segment(_prev(), this);
            bool next_line = _is_line_segment(this, _next());
            if (_type == NODE_SMOOTH) {
                // for a node that is already smooth and has a degenerate handle,
                // drag out the second handle to 1/3 the length of the linear segment
                if (_front.isDegenerate()) {
                    double dist = Geom::distance(_next()->position(), position());
                    _front.setRelativePos(Geom::unit_vector(-_back.relativePos()) * dist / 3);
                }
                if (_back.isDegenerate()) {
                    double dist = Geom::distance(_prev()->position(), position());
                    _back.setRelativePos(Geom::unit_vector(-_front.relativePos()) * dist / 3);
                }
            } else if (isDegenerate()) {
                _updateAutoHandles();
            } else if (_front.isDegenerate()) {
                // if the front handle is degenerate and this...next is a line segment,
                // make back colinear; otherwise pull out the other handle
                // to 1/3 of distance to prev
                if (next_line) {
                    _back.setDirection(*_next(), *this);
                } else if (_prev()) {
                    Geom::Point dir = direction(_back, *this);
                    _front.setRelativePos(Geom::distance(_prev()->position(), position()) / 3 * dir);
                }
            } else if (_back.isDegenerate()) {
                if (prev_line) {
                    _front.setDirection(*_prev(), *this);
                } else if (_next()) {
                    Geom::Point dir = direction(_front, *this);
                    _back.setRelativePos(Geom::distance(_next()->position(), position()) / 3 * dir);
                }
            } else {
                // both handles are extended. make colinear while keeping length
                // first make back colinear with the vector front ---> back,
                // then make front colinear with back ---> node
                // (not back ---> front because back's position was changed in the first call)
                _back.setDirection(_front, _back);
                _front.setDirection(_back, *this);
            }
            } break;
        case NODE_SYMMETRIC:
            if (isEndNode()) return; // symmetric handles make no sense for endnodes
            if (isDegenerate()) {
                // similar to auto handles but set the same length for both
                Geom::Point vec_next = _next()->position() - position();
                Geom::Point vec_prev = _prev()->position() - position();
                double len_next = vec_next.length(), len_prev = vec_prev.length();
                double len = (len_next + len_prev) / 6; // take 1/3 of average
                if (len == 0) return;

                Geom::Point dir = Geom::unit_vector((len_prev / len_next) * vec_next - vec_prev);
                _back.setRelativePos(-dir * len);
                _front.setRelativePos(dir * len);
            } else {
                // Both handles are extended. Compute average length, use direction from
                // back handle to front handle. This also works correctly for degenerates
                double len = (_front.length() + _back.length()) / 2;
                Geom::Point dir = direction(_back, _front);
                _front.setRelativePos(dir * len);
                _back.setRelativePos(-dir * len);
            }
            break;
        default: break;
        }
    }
    _type = type;
    _setShape(_node_type_to_shape(type));
    updateState();
}

/** Pick the best type for this node, based on the position of its handles.
 * This is what assigns types to nodes created using the pen tool. */
void Node::pickBestType()
{
    _type = NODE_CUSP;
    bool front_degen = _front.isDegenerate();
    bool back_degen = _back.isDegenerate();
    bool both_degen = front_degen && back_degen;
    bool neither_degen = !front_degen && !back_degen;
    do {
        // if both handles are degenerate, do nothing
        if (both_degen) break;
        // if neither are degenerate, check their respective positions
        if (neither_degen) {
            Geom::Point front_delta = _front.position() - position();
            Geom::Point back_delta = _back.position() - position();
            // for now do not automatically make nodes symmetric, it can be annoying
            /*if (Geom::are_near(front_delta, -back_delta)) {
                _type = NODE_SYMMETRIC;
                break;
            }*/
            if (Geom::are_near(Geom::unit_vector(front_delta),
                Geom::unit_vector(-back_delta)))
            {
                _type = NODE_SMOOTH;
                break;
            }
        }
        // check whether the handle aligns with the previous line segment.
        // we know that if front is degenerate, back isn't, because
        // both_degen was false
        if (front_degen && _next() && _next()->_back.isDegenerate()) {
            Geom::Point segment_delta = Geom::unit_vector(_next()->position() - position());
            Geom::Point handle_delta = Geom::unit_vector(_back.position() - position());
            if (Geom::are_near(segment_delta, -handle_delta)) {
                _type = NODE_SMOOTH;
                break;
            }
        } else if (back_degen && _prev() && _prev()->_front.isDegenerate()) {
            Geom::Point segment_delta = Geom::unit_vector(_prev()->position() - position());
            Geom::Point handle_delta = Geom::unit_vector(_front.position() - position());
            if (Geom::are_near(segment_delta, -handle_delta)) {
                _type = NODE_SMOOTH;
                break;
            }
        }
    } while (false);
    _setShape(_node_type_to_shape(_type));
    updateState();
}

bool Node::isEndNode()
{
    return !_prev() || !_next();
}

/** Move the node to the bottom of its canvas group. Useful for node break, to ensure that
 * the selected nodes are above the unselected ones. */
void Node::sink()
{
    sp_canvas_item_move_to_z(_canvas_item, 0);
}

NodeType Node::parse_nodetype(char x)
{
    switch (x) {
    case 'a': return NODE_AUTO;
    case 'c': return NODE_CUSP;
    case 's': return NODE_SMOOTH;
    case 'z': return NODE_SYMMETRIC;
    default: return NODE_PICK_BEST;
    }
}

/** Customized event handler to catch scroll events needed for selection grow/shrink. */
bool Node::_eventHandler(GdkEvent *event)
{
    static NodeList::iterator origin;
    static int dir;

    switch (event->type)
    {
    case GDK_SCROLL:
        if (event->scroll.direction == GDK_SCROLL_UP) {
            dir = 1;
        } else if (event->scroll.direction == GDK_SCROLL_DOWN) {
            dir = -1;
        } else break;
        if (held_control(event->scroll)) {
            _selection.spatialGrow(this, dir);
        } else {
            _linearGrow(dir);
        }
        return true;
    default:
        break;
    }
    return ControlPoint::_eventHandler(event);
}

// TODO Move this to 2Geom!
static double bezier_length (Geom::Point a0, Geom::Point a1, Geom::Point a2, Geom::Point a3)
{
    double lower = Geom::distance(a0, a3);
    double upper = Geom::distance(a0, a1) + Geom::distance(a1, a2) + Geom::distance(a2, a3);

    if (upper - lower < Geom::EPSILON) return (lower + upper)/2;

    Geom::Point // Casteljau subdivision
        b0 = a0,
        c0 = a3,
        b1 = 0.5*(a0 + a1),
        t0 = 0.5*(a1 + a2),
        c1 = 0.5*(a2 + a3),
        b2 = 0.5*(b1 + t0),
        c2 = 0.5*(t0 + c1),
        b3 = 0.5*(b2 + c2); // == c3
    return bezier_length(b0, b1, b2, b3) + bezier_length(b3, c2, c1, c0);
}

/** Select or deselect a node in this node's subpath based on its path distance from this node.
 * @param dir If negative, shrink selection by one node; if positive, grow by one node */
void Node::_linearGrow(int dir)
{
    // Interestingly, we do not need any help from PathManipulator when doing linear grow.
    // First handle the trivial case of growing over an unselected node.
    if (!selected() && dir > 0) {
        _selection.insert(this);
        return;
    }

    NodeList::iterator this_iter = NodeList::get_iterator(this);
    NodeList::iterator fwd = this_iter, rev = this_iter;
    double distance_back = 0, distance_front = 0;

    // Linear grow is simple. We find the first unselected nodes in each direction
    // and compare the linear distances to them.
    if (dir > 0) {
        if (!selected()) {
            _selection.insert(this);
            return;
        }

        // find first unselected nodes on both sides
        while (fwd && fwd->selected()) {
            NodeList::iterator n = fwd.next();
            distance_front += bezier_length(*fwd, fwd->_front, n->_back, *n);
            fwd = n;
            if (fwd == this_iter)
                // there is no unselected node in this cyclic subpath
                return;
        }
        // do the same for the second direction. Do not check for equality with
        // this node, because there is at least one unselected node in the subpath,
        // so we are guaranteed to stop.
        while (rev && rev->selected()) {
            NodeList::iterator p = rev.prev();
            distance_back += bezier_length(*rev, rev->_back, p->_front, *p);
            rev = p;
        }

        NodeList::iterator t; // node to select
        if (fwd && rev) {
            if (distance_front <= distance_back) t = fwd;
            else t = rev;
        } else {
            if (fwd) t = fwd;
            if (rev) t = rev;
        }
        if (t) _selection.insert(t.ptr());

    // Linear shrink is more complicated. We need to find the farthest selected node.
    // This means we have to check the entire subpath. We go in the direction in which
    // the distance we traveled is lower. We do this until we run out of nodes (ends of path)
    // or the two iterators meet. On the way, we store the last selected node and its distance
    // in each direction (if any). At the end, we choose the one that is farther and deselect it.
    } else {
        // both iterators that store last selected nodes are initially empty
        NodeList::iterator last_fwd, last_rev;
        double last_distance_back = 0, last_distance_front = 0;

        while (rev || fwd) {
            if (fwd && (!rev || distance_front <= distance_back)) {
                if (fwd->selected()) {
                    last_fwd = fwd;
                    last_distance_front = distance_front;
                }
                NodeList::iterator n = fwd.next();
                if (n) distance_front += bezier_length(*fwd, fwd->_front, n->_back, *n);
                fwd = n;
            } else if (rev && (!fwd || distance_front > distance_back)) {
                if (rev->selected()) {
                    last_rev = rev;
                    last_distance_back = distance_back;
                }
                NodeList::iterator p = rev.prev();
                if (p) distance_back += bezier_length(*rev, rev->_back, p->_front, *p);
                rev = p;
            }
            // Check whether we walked the entire cyclic subpath.
            // This is initially true because both iterators start from this node,
            // so this check cannot go in the while condition.
            // When this happens, we need to check the last node, pointed to by the iterators.
            if (fwd && fwd == rev) {
                if (!fwd->selected()) break;
                NodeList::iterator fwdp = fwd.prev(), revn = rev.next();
                double df = distance_front + bezier_length(*fwdp, fwdp->_front, fwd->_back, *fwd);
                double db = distance_back + bezier_length(*revn, revn->_back, rev->_front, *rev);
                if (df > db) {
                    last_fwd = fwd;
                    last_distance_front = df;
                } else {
                    last_rev = rev;
                    last_distance_back = db;
                }
                break;
            }
        }

        NodeList::iterator t;
        if (last_fwd && last_rev) {
            if (last_distance_front >= last_distance_back) t = last_fwd;
            else t = last_rev;
        } else {
            if (last_fwd) t = last_fwd;
            if (last_rev) t = last_rev;
        }
        if (t) _selection.erase(t.ptr());
    }
}

void Node::_setState(State state)
{
    // change node size to match type and selection state
    switch (_type) {
    case NODE_AUTO:
    case NODE_CUSP:
        if (selected()) _setSize(11);
        else _setSize(9);
        break;
    default:
        if(selected()) _setSize(9);
        else _setSize(7);
        break;
    }
    SelectableControlPoint::_setState(state);
}

bool Node::grabbed(GdkEventMotion *event)
{
    if (SelectableControlPoint::grabbed(event))
        return true;

    // Dragging out handles with Shift + drag on a node.
    if (!held_shift(*event)) return false;

    Handle *h;
    Geom::Point evp = event_point(*event);
    Geom::Point rel_evp = evp - _last_click_event_point();

    // This should work even if dragtolerance is zero and evp coincides with node position.
    double angle_next = HUGE_VAL;
    double angle_prev = HUGE_VAL;
    bool has_degenerate = false;
    // determine which handle to drag out based on degeneration and the direction of drag
    if (_front.isDegenerate() && _next()) {
        Geom::Point next_relpos = _desktop->d2w(_next()->position())
            - _desktop->d2w(position());
        angle_next = fabs(Geom::angle_between(rel_evp, next_relpos));
        has_degenerate = true;
    }
    if (_back.isDegenerate() && _prev()) {
        Geom::Point prev_relpos = _desktop->d2w(_prev()->position())
            - _desktop->d2w(position());
        angle_prev = fabs(Geom::angle_between(rel_evp, prev_relpos));
        has_degenerate = true;
    }
    if (!has_degenerate) return false;
    h = angle_next < angle_prev ? &_front : &_back;

    h->setPosition(_desktop->w2d(evp));
    h->setVisible(true);
    h->transferGrab(this, event);
    Handle::_drag_out = true;
    return true;
}

void Node::dragged(Geom::Point &new_pos, GdkEventMotion *event)
{
    // For a note on how snapping is implemented in Inkscape, see snap.h.
    SnapManager &sm = _desktop->namedview->snap_manager;
    bool snap = sm.someSnapperMightSnap();
    std::vector<Inkscape::SnapCandidatePoint> unselected;
    if (snap) {
        /* setup
         * TODO We are doing this every time a snap happens. It should once be done only once
         *      per drag - maybe in the grabbed handler?
         * TODO Unselected nodes vector must be valid during the snap run, because it is not
         *      copied. Fix this in snap.h and snap.cpp, then the above.
         * TODO Snapping to unselected segments of selected paths doesn't work yet. */

        // Build the list of unselected nodes.
        typedef ControlPointSelection::Set Set;
        Set &nodes = _selection.allPoints();
        for (Set::iterator i = nodes.begin(); i != nodes.end(); ++i) {
            if (!(*i)->selected()) {
                Node *n = static_cast<Node*>(*i);
                Inkscape::SnapCandidatePoint p(n->position(), n->_snapSourceType(), n->_snapTargetType());
                unselected.push_back(p);
            }
        }
        sm.setupIgnoreSelection(_desktop, true, &unselected);
    }

    if (held_control(*event)) {
        Geom::Point origin = _last_drag_origin();
        Inkscape::SnappedPoint fp, bp;
        if (held_alt(*event)) {
            // with Ctrl+Alt, constrain to handle lines
            // project the new position onto a handle line that is closer
            boost::optional<Geom::Point> front_point, back_point;
            boost::optional<Inkscape::Snapper::ConstraintLine> line_front, line_back;
            if (_front.isDegenerate()) {
                if (_is_line_segment(this, _next()))
                    front_point = _next()->position() - origin;
            } else {
                front_point = _front.relativePos();
            }
            if (_back.isDegenerate()) {
                if (_is_line_segment(_prev(), this))
                    back_point = _prev()->position() - origin;
            } else {
                back_point = _back.relativePos();
            }
            if (front_point)
                line_front = Inkscape::Snapper::ConstraintLine(origin, *front_point);
            if (back_point)
                line_back = Inkscape::Snapper::ConstraintLine(origin, *back_point);

            // TODO: combine the snap and non-snap branches by modifying snap.h / snap.cpp
            if (snap) {
                if (line_front) {
                    fp = sm.constrainedSnap(Inkscape::SnapCandidatePoint(position(),
                        _snapSourceType()), *line_front);
                }
                if (line_back) {
                    bp = sm.constrainedSnap(Inkscape::SnapCandidatePoint(position(),
                        _snapSourceType()), *line_back);
                }
            }
            if (fp.getSnapped() || bp.getSnapped()) {
                if (fp.isOtherSnapBetter(bp, false)) {
                    bp.getPoint(new_pos);
                } else {
                    fp.getPoint(new_pos);
                }
            } else {
                boost::optional<Geom::Point> pos;
                if (line_front) {
                    pos = line_front->projection(new_pos);
                }
                if (line_back) {
                    Geom::Point pos2 = line_back->projection(new_pos);
                    if (!pos || (pos && Geom::distance(new_pos, *pos) > Geom::distance(new_pos, pos2)))
                        pos = pos2;
                }
                if (pos) {
                    new_pos = *pos;
                } else {
                    new_pos = origin;
                }
            }
        } else {
            // with Ctrl, constrain to axes
            // TODO combine the two branches
            if (snap) {
                Inkscape::Snapper::ConstraintLine line_x(origin, Geom::Point(1, 0));
                Inkscape::Snapper::ConstraintLine line_y(origin, Geom::Point(0, 1));
                fp = sm.constrainedSnap(Inkscape::SnapCandidatePoint(position(), _snapSourceType()), line_x);
                bp = sm.constrainedSnap(Inkscape::SnapCandidatePoint(position(), _snapSourceType()), line_y);
            }
            if (fp.getSnapped() || bp.getSnapped()) {
                if (fp.isOtherSnapBetter(bp, false)) {
                    fp = bp;
                }
                fp.getPoint(new_pos);
            } else {
                Geom::Point origin = _last_drag_origin();
                Geom::Point delta = new_pos - origin;
                Geom::Dim2 d = (fabs(delta[Geom::X]) < fabs(delta[Geom::Y])) ? Geom::X : Geom::Y;
                new_pos[d] = origin[d];
            }
        }
    } else if (snap) {
        sm.freeSnapReturnByRef(new_pos, _snapSourceType());
    }

    SelectableControlPoint::dragged(new_pos, event);
}

bool Node::clicked(GdkEventButton *event)
{
    if(_pm()._nodeClicked(this, event))
        return true;
    return SelectableControlPoint::clicked(event);
}

Inkscape::SnapSourceType Node::_snapSourceType()
{
    if (_type == NODE_SMOOTH || _type == NODE_AUTO)
        return SNAPSOURCE_NODE_SMOOTH;
    return SNAPSOURCE_NODE_CUSP;
}
Inkscape::SnapTargetType Node::_snapTargetType()
{
    if (_type == NODE_SMOOTH || _type == NODE_AUTO)
        return SNAPTARGET_NODE_SMOOTH;
    return SNAPTARGET_NODE_CUSP;
}

Glib::ustring Node::_getTip(unsigned state)
{
    if (state_held_shift(state)) {
        bool can_drag_out = (_next() && _front.isDegenerate()) || (_prev() && _back.isDegenerate());
        if (can_drag_out) {
            /*if (state_held_control(state)) {
                return format_tip(C_("Path node tip",
                    "<b>Shift+Ctrl:</b> drag out a handle and snap its angle "
                    "to %f° increments"), snap_increment_degrees());
            }*/
            return C_("Path node tip",
                "<b>Shift:</b> drag out a handle, click to toggle selection");
        }
        return C_("Path node tip", "<b>Shift:</b> click to toggle selection");
    }

    if (state_held_control(state)) {
        if (state_held_alt(state)) {
            return C_("Path node tip", "<b>Ctrl+Alt:</b> move along handle lines, click to delete node");
        }
        return C_("Path node tip",
            "<b>Ctrl:</b> move along axes, click to change node type");
    }

    // assemble tip from node name
    char const *nodetype = node_type_to_localized_string(_type);
    if (_selection.transformHandlesEnabled() && selected()) {
        if (_selection.size() == 1) {
            return format_tip(C_("Path node tip",
                "<b>%s:</b> drag to shape the path (more: Shift, Ctrl, Alt)"), nodetype);
        }
        return format_tip(C_("Path node tip",
            "<b>%s:</b> drag to shape the path, click to toggle scale/rotation handles (more: Shift, Ctrl, Alt)"), nodetype);
    }
    return format_tip(C_("Path node tip",
        "<b>%s:</b> drag to shape the path, click to select only this node (more: Shift, Ctrl, Alt)"), nodetype);
}

Glib::ustring Node::_getDragTip(GdkEventMotion */*event*/)
{
    Geom::Point dist = position() - _last_drag_origin();
    GString *x = SP_PX_TO_METRIC_STRING(dist[Geom::X], _desktop->namedview->getDefaultMetric());
    GString *y = SP_PX_TO_METRIC_STRING(dist[Geom::Y], _desktop->namedview->getDefaultMetric());
    Glib::ustring ret = format_tip(C_("Path node tip", "Move node by %s, %s"),
        x->str, y->str);
    g_string_free(x, TRUE);
    g_string_free(y, TRUE);
    return ret;
}

char const *Node::node_type_to_localized_string(NodeType type)
{
    switch (type) {
    case NODE_CUSP: return _("Cusp node");
    case NODE_SMOOTH: return _("Smooth node");
    case NODE_SYMMETRIC: return _("Symmetric node");
    case NODE_AUTO: return _("Auto-smooth node");
    default: return "";
    }
}

/** Determine whether two nodes are joined by a linear segment. */
bool Node::_is_line_segment(Node *first, Node *second)
{
    if (!first || !second) return false;
    if (first->_next() == second)
        return first->_front.isDegenerate() && second->_back.isDegenerate();
    if (second->_next() == first)
        return second->_front.isDegenerate() && first->_back.isDegenerate();
    return false;
}

SPCtrlShapeType Node::_node_type_to_shape(NodeType type)
{
    switch(type) {
    case NODE_CUSP: return SP_CTRL_SHAPE_DIAMOND;
    case NODE_SMOOTH: return SP_CTRL_SHAPE_SQUARE;
    case NODE_AUTO: return SP_CTRL_SHAPE_CIRCLE;
    case NODE_SYMMETRIC: return SP_CTRL_SHAPE_SQUARE;
    default: return SP_CTRL_SHAPE_DIAMOND;
    }
}


/**
 * @class NodeList
 * @brief An editable list of nodes representing a subpath.
 *
 * It can optionally be cyclic to represent a closed path.
 * The list has iterators that act like plain node iterators, but can also be used
 * to obtain shared pointers to nodes.
 */

NodeList::NodeList(SubpathList &splist)
    : _list(splist)
    , _closed(false)
{
    this->list = this;
    this->next = this;
    this->prev = this;
}

NodeList::~NodeList()
{
    clear();
}

bool NodeList::empty()
{
    return next == this;
}

NodeList::size_type NodeList::size()
{
    size_type sz = 0;
    for (ListNode *ln = next; ln != this; ln = ln->next) ++sz;
    return sz;
}

bool NodeList::closed()
{
    return _closed;
}

/** A subpath is degenerate if it has no segments - either one node in an open path
 * or no nodes in a closed path */
bool NodeList::degenerate()
{
    return closed() ? empty() : ++begin() == end();
}

NodeList::iterator NodeList::before(double t, double *fracpart)
{
    double intpart;
    *fracpart = std::modf(t, &intpart);
    int index = intpart;

    iterator ret = begin();
    std::advance(ret, index);
    return ret;
}

// insert a node before i
NodeList::iterator NodeList::insert(iterator i, Node *x)
{
    ListNode *ins = i._node;
    x->next = ins;
    x->prev = ins->prev;
    ins->prev->next = x;
    ins->prev = x;
    x->ListNode::list = this;
    return iterator(x);
}

void NodeList::splice(iterator pos, NodeList &list)
{
    splice(pos, list, list.begin(), list.end());
}

void NodeList::splice(iterator pos, NodeList &list, iterator i)
{
    NodeList::iterator j = i;
    ++j;
    splice(pos, list, i, j);
}

void NodeList::splice(iterator pos, NodeList &list, iterator first, iterator last)
{
    ListNode *ins_beg = first._node, *ins_end = last._node, *at = pos._node;
    for (ListNode *ln = ins_beg; ln != ins_end; ln = ln->next) {
        ln->list = this;
    }
    ins_beg->prev->next = ins_end;
    ins_end->prev->next = at;
    at->prev->next = ins_beg;

    ListNode *atprev = at->prev;
    at->prev = ins_end->prev;
    ins_end->prev = ins_beg->prev;
    ins_beg->prev = atprev;
}

void NodeList::shift(int n)
{
    // 1. make the list perfectly cyclic
    next->prev = prev;
    prev->next = next;
    // 2. find new begin
    ListNode *new_begin = next;
    if (n > 0) {
        for (; n > 0; --n) new_begin = new_begin->next;
    } else {
        for (; n < 0; ++n) new_begin = new_begin->prev;
    }
    // 3. relink begin to list
    next = new_begin;
    prev = new_begin->prev;
    new_begin->prev->next = this;
    new_begin->prev = this;
}

void NodeList::reverse()
{
    for (ListNode *ln = next; ln != this; ln = ln->prev) {
        std::swap(ln->next, ln->prev);
        Node *node = static_cast<Node*>(ln);
        Geom::Point save_pos = node->front()->position();
        node->front()->setPosition(node->back()->position());
        node->back()->setPosition(save_pos);
    }
    std::swap(next, prev);
}

void NodeList::clear()
{
    for (iterator i = begin(); i != end();) erase (i++);
}

NodeList::iterator NodeList::erase(iterator i)
{
    // some gymnastics are required to ensure that the node is valid when deleted;
    // otherwise the code that updates handle visibility will break
    Node *rm = static_cast<Node*>(i._node);
    ListNode *rmnext = rm->next, *rmprev = rm->prev;
    ++i;
    delete rm;
    rmprev->next = rmnext;
    rmnext->prev = rmprev;
    return i;
}

// TODO this method is very ugly!
// converting SubpathList to an intrusive list might allow us to get rid of it
void NodeList::kill()
{
    for (SubpathList::iterator i = _list.begin(); i != _list.end(); ++i) {
        if (i->get() == this) {
            _list.erase(i);
            return;
        }
    }
}

NodeList &NodeList::get(Node *n) {
    return *(n->list());
}
NodeList &NodeList::get(iterator const &i) {
    return *(i._node->list);
}


/**
 * @class SubpathList
 * @brief Editable path composed of one or more subpaths
 */

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
