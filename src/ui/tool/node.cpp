/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <iostream>
#include <stdexcept>
#include <boost/utility.hpp>
#include "multi-path-manipulator.h"
#include <glib/gi18n.h>
#include <2geom/bezier-utils.h>
#include <2geom/transforms.h>
#include "display/sp-ctrlline.h"
#include "display/sp-canvas.h"
#include "display/sp-canvas-util.h"
#include "desktop.h"

#include "preferences.h"
#include "snap.h"
#include "snap-preferences.h"
#include "sp-namedview.h"
#include "ui/control-manager.h"
#include "ui/tool/control-point-selection.h"
#include "ui/tool/event-utils.h"
#include "ui/tool/node.h"
#include "ui/tool/path-manipulator.h"
#include "ui/tools/node-tool.h"
#include "ui/tools-switch.h"
#include <gdk/gdkkeysyms.h>
#include <cmath>

namespace {

Inkscape::ControlType nodeTypeToCtrlType(Inkscape::UI::NodeType type)
{
    Inkscape::ControlType result = Inkscape::CTRL_TYPE_NODE_CUSP;
    switch(type) {
        case Inkscape::UI::NODE_SMOOTH:
            result = Inkscape::CTRL_TYPE_NODE_SMOOTH;
            break;
        case Inkscape::UI::NODE_AUTO:
            result = Inkscape::CTRL_TYPE_NODE_AUTO;
            break;
        case Inkscape::UI::NODE_SYMMETRIC:
            result = Inkscape::CTRL_TYPE_NODE_SYMETRICAL;
            break;
        case Inkscape::UI::NODE_CUSP:
        default:
            result = Inkscape::CTRL_TYPE_NODE_CUSP;
            break;
    }
    return result;
}

} // namespace

namespace Inkscape {
namespace UI {

const double NO_POWER = 0.0;
const double DEFAULT_START_POWER = 1.0/3.0;

ControlPoint::ColorSet Node::node_colors = {
    {0xbfbfbf00, 0x000000ff}, // normal fill, stroke
    {0xff000000, 0x000000ff}, // mouseover fill, stroke
    {0xff000000, 0x000000ff}, // clicked fill, stroke
    //
    {0x0000ffff, 0x000000ff}, // normal fill, stroke when selected
    {0xff000000, 0x000000ff}, // mouseover fill, stroke when selected
    {0xff000000, 0x000000ff}  // clicked fill, stroke when selected
};

ControlPoint::ColorSet Handle::_handle_colors = {
    {0xffffffff, 0x000000ff}, // normal fill, stroke
    {0xff000000, 0x000000ff}, // mouseover fill, stroke
    {0xff000000, 0x000000ff}, // clicked fill, stroke
    //
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

Geom::Point Handle::_saved_other_pos(0, 0);

double Handle::_saved_length = 0.0;

bool Handle::_drag_out = false;

Handle::Handle(NodeSharedData const &data, Geom::Point const &initial_pos, Node *parent) :
    ControlPoint(data.desktop, initial_pos, SP_ANCHOR_CENTER,
                 CTRL_TYPE_ADJ_HANDLE,
                 _handle_colors, data.handle_group),
    _parent(parent),
    _handle_line(ControlManager::getManager().createControlLine(data.handle_line_group)),
    _degenerate(true)
{
    setVisible(false);
}

Handle::~Handle()
{
    //sp_canvas_item_hide(_handle_line);
    sp_canvas_item_destroy(_handle_line);
}

void Handle::setVisible(bool v)
{
    ControlPoint::setVisible(v);
    if (v) {
        sp_canvas_item_show(_handle_line);
    } else {
        sp_canvas_item_hide(_handle_line);
    }
}

void Handle::move(Geom::Point const &new_pos)
{
    Handle *other = this->other();
    Node *node_towards = _parent->nodeToward(this); // node in direction of this handle
    Node *node_away = _parent->nodeAwayFrom(this); // node in the opposite direction
    Handle *towards = node_towards ? node_towards->handleAwayFrom(_parent) : NULL;
    Handle *towards_second = node_towards ? node_towards->handleToward(_parent) : NULL;
    double bspline_weight = 0.0;

    if (Geom::are_near(new_pos, _parent->position())) {
        // The handle becomes degenerate.
        // Adjust node type as necessary.
        if (other->isDegenerate()) {
            // If both handles become degenerate, convert to parent cusp node
            _parent->setType(NODE_CUSP, false);
        } else {
            // Only 1 handle becomes degenerate
            switch (_parent->type()) {
            case NODE_AUTO:
            case NODE_SYMMETRIC:
                _parent->setType(NODE_SMOOTH, false);
                break;
            default:
                // do nothing for other node types
                break;
            }
        }
        // If the segment between the handle and the node
        // in its direction becomes linear and there are smooth nodes
        // at its ends, make their handles colinear with the segment
        if (towards && towards_second->isDegenerate()) {
            if (node_towards->type() == NODE_SMOOTH) {
                towards->setDirection(*_parent, *node_towards);
            }
            if (_parent->type() == NODE_SMOOTH) {
                other->setDirection(*node_towards, *_parent);
            }
        }
        setPosition(new_pos);

        //move the handler and its oposite the same proportion
        if(_pm()._isBSpline()){
            setPosition(_pm()._bsplineHandleReposition(this, false));
            bspline_weight = _pm()._bsplineHandlePosition(this, false);
            this->other()->setPosition(_pm()._bsplineHandleReposition(this->other(), bspline_weight));
        }
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

        //move the handler and its oposite the same proportion
        if(_pm()._isBSpline()){ 
            setPosition(_pm()._bsplineHandleReposition(this, false));
            bspline_weight = _pm()._bsplineHandlePosition(this, false);
            this->other()->setPosition(_pm()._bsplineHandleReposition(this->other(), bspline_weight));
        }
        
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

    // moves the handler and its oposite the same proportion
    if(_pm()._isBSpline()){
        setPosition(_pm()._bsplineHandleReposition(this, false));
        bspline_weight = _pm()._bsplineHandlePosition(this, false);
        this->other()->setPosition(_pm()._bsplineHandleReposition(this->other(), bspline_weight));
    }
}

void Handle::setPosition(Geom::Point const &p)
{
    ControlPoint::setPosition(p);
    _handle_line->setCoords(_parent->position(), position());

    // update degeneration info and visibility
    if (Geom::are_near(position(), _parent->position()))
        _degenerate = true;
    else _degenerate = false;

    if (_parent->_handles_shown && _parent->visible() && !_degenerate) {
        setVisible(true);
    } else {
        setVisible(false);
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
    move(_parent->position());
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

bool Handle::_eventHandler(Inkscape::UI::Tools::ToolBase *event_context, GdkEvent *event)
{
    switch (event->type)
    {
    case GDK_KEY_PRESS:
        switch (shortcut_key(event->key))
        {
        case GDK_KEY_s:
        case GDK_KEY_S:
            if (held_only_shift(event->key) && _parent->_type == NODE_CUSP) {
                // when Shift+S is pressed when hovering over a handle belonging to a cusp node,
                // hold this handle in place; otherwise process normally
                // this handle is guaranteed not to be degenerate
                other()->move(_parent->position() - (position() - _parent->position()));
                _parent->setType(NODE_SMOOTH, false);
                _parent->_pm().update(); // magic triple combo to add undo event
                _parent->_pm().writeXML();
                _parent->_pm()._commit(_("Change node type"));
                return true;
            }
            break;
        default: break;
        }
        break;
    // new double click event to set the handlers of a node to the default proportion, DEFAULT_START_POWER% 
    case GDK_2BUTTON_PRESS:
        handle_2button_press();
        break;
    
    default: break;
    }

    return ControlPoint::_eventHandler(event_context, event);
}

//this function moves the handler and its oposite to the default proportion of DEFAULT_START_POWER
void Handle::handle_2button_press(){
    if(_pm()._isBSpline()){
        setPosition(_pm()._bsplineHandleReposition(this, DEFAULT_START_POWER));
        this->other()->setPosition(_pm()._bsplineHandleReposition(this->other(), DEFAULT_START_POWER));
        _pm().update();
    }
}

bool Handle::grabbed(GdkEventMotion *)
{
    _saved_other_pos = other()->position();
    _saved_length = _drag_out ? 0 : length();
    _pm()._handleGrabbed();
    return false;
}

void Handle::dragged(Geom::Point &new_pos, GdkEventMotion *event)
{
    if (tools_isactive(_desktop, TOOLS_NODES)) {
        Inkscape::UI::Tools::NodeTool *nt = static_cast<Inkscape::UI::Tools::NodeTool*>(_desktop->event_context);
        nt->update_helperpath();
    }
    Geom::Point parent_pos = _parent->position();
    Geom::Point origin = _last_drag_origin();
    SnapManager &sm = _desktop->namedview->snap_manager;
    bool snap = held_shift(*event) ? false : sm.someSnapperMightSnap();
    boost::optional<Inkscape::Snapper::SnapConstraint> ctrl_constraint;

    // with Alt, preserve length
    if (held_alt(*event)) {
        new_pos = parent_pos + Geom::unit_vector(new_pos - parent_pos) * _saved_length;
        snap = false;
    }
    // with Ctrl, constrain to M_PI/rotationsnapsperpi increments from vertical
    // and the original position.
    if (held_control(*event)) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        int snaps = 2 * prefs->getIntLimited("/options/rotationsnapsperpi/value", 12, 1, 1000);

        // note: if snapping to the original position is only desired in the original
        // direction of the handle, change to Ray instead of Line
        Geom::Line original_line(parent_pos, origin);
        Geom::Line perp_line(parent_pos, parent_pos + Geom::rot90(origin - parent_pos));
        Geom::Point snap_pos = parent_pos + Geom::constrain_angle(
            Geom::Point(0,0), new_pos - parent_pos, snaps, Geom::Point(1,0));
        Geom::Point orig_pos = original_line.pointAt(original_line.nearestTime(new_pos));
        Geom::Point perp_pos = perp_line.pointAt(perp_line.nearestTime(new_pos));

        Geom::Point result = snap_pos;
        ctrl_constraint = Inkscape::Snapper::SnapConstraint(parent_pos, parent_pos - snap_pos);
        if (Geom::distance(orig_pos, new_pos) < Geom::distance(result, new_pos)) {
            result = orig_pos;
            ctrl_constraint = Inkscape::Snapper::SnapConstraint(parent_pos, parent_pos - orig_pos);
        }
        if (Geom::distance(perp_pos, new_pos) < Geom::distance(result, new_pos)) {
            result = perp_pos;
            ctrl_constraint = Inkscape::Snapper::SnapConstraint(parent_pos, parent_pos - perp_pos);
        }
        new_pos = result;
        // moves the handler and its oposite in X fixed positions depending on parameter "steps with control" 
        // by default in live BSpline
        if(_pm()._isBSpline()){
            setPosition(new_pos);
            int steps = _pm()._bsplineGetSteps();
            new_pos=_pm()._bsplineHandleReposition(this,ceilf(_pm()._bsplineHandlePosition(this, false)*steps)/steps);
        }
    }

    std::vector<Inkscape::SnapCandidatePoint> unselected;
    //if the snap adjustment is activated and it is not bspline
    if (snap && !_pm()._isBSpline()) {
        ControlPointSelection::Set &nodes = _parent->_selection.allPoints();
        for (ControlPointSelection::Set::iterator i = nodes.begin(); i != nodes.end(); ++i) {
            Node *n = static_cast<Node*>(*i);
            unselected.push_back(n->snapCandidatePoint());
        }
        sm.setupIgnoreSelection(_desktop, true, &unselected);

        Node *node_away = _parent->nodeAwayFrom(this);
        if (_parent->type() == NODE_SMOOTH && Node::_is_line_segment(_parent, node_away)) {
            Inkscape::Snapper::SnapConstraint cl(_parent->position(),
                _parent->position() - node_away->position());
            Inkscape::SnappedPoint p;
            p = sm.constrainedSnap(Inkscape::SnapCandidatePoint(new_pos, SNAPSOURCE_NODE_HANDLE), cl);
            new_pos = p.getPoint();
        } else if (ctrl_constraint) {
            // NOTE: this is subtly wrong.
            // We should get all possible constraints and snap along them using
            // multipleConstrainedSnaps, instead of first snapping to angle and then to objects
            Inkscape::SnappedPoint p;
            p = sm.constrainedSnap(Inkscape::SnapCandidatePoint(new_pos, SNAPSOURCE_NODE_HANDLE), *ctrl_constraint);
            new_pos = p.getPoint();
        } else {
            sm.freeSnapReturnByRef(new_pos, SNAPSOURCE_NODE_HANDLE);
        }
        sm.unSetup();
    }


    // with Shift, if the node is cusp, rotate the other handle as well
    if (_parent->type() == NODE_CUSP && !_drag_out) {
        if (held_shift(*event)) {
            Geom::Point other_relpos = _saved_other_pos - parent_pos;
            other_relpos *= Geom::Rotate(Geom::angle_between(origin - parent_pos, new_pos - parent_pos));
            other()->setRelativePos(other_relpos);
        } else {
            // restore the position
            other()->setPosition(_saved_other_pos);
        }
    }
    //if it is bspline but SHIFT or CONTROL are not pressed it fixes it in the original position
    if(_pm()._isBSpline() && !held_shift(*event) && !held_control(*event)){
        new_pos=_last_drag_origin();
    }
    move(new_pos); // needed for correct update, even though it's redundant
    _pm().update();
}

void Handle::ungrabbed(GdkEventButton *event)
{
    // hide the handle if it's less than dragtolerance away from the node
    // however, never do this for cancelled drag / broken grab
    // TODO is this actually a good idea?
    if (event) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        int drag_tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

        Geom::Point dist = _desktop->d2w(_parent->position()) - _desktop->d2w(position());
        if (dist.length() <= drag_tolerance) {
            move(_parent->position());
        }
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

Handle const *Handle::other() const
{
    return const_cast<Handle *>(this)->other();
}

Handle *Handle::other()
{
    if (this == &_parent->_front) {
        return &_parent->_back;
    } else {
        return &_parent->_front;
    }
}

static double snap_increment_degrees() {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int snaps = prefs->getIntLimited("/options/rotationsnapsperpi/value", 12, 1, 1000);
    return 180.0 / snaps;
}

Glib::ustring Handle::_getTip(unsigned state) const
{
    char const *more;
    // a trick to mark as bspline if the node has no strength, we are going to use it later
    // to show the appropiate messages. We cannot do it in any different way becasue the function is constant
    Handle *h = const_cast<Handle *>(this);
    bool isBSpline = _pm()._isBSpline();
    bool can_shift_rotate = _parent->type() == NODE_CUSP && !other()->isDegenerate();
    if (can_shift_rotate && !isBSpline) {
        more = C_("Path handle tip", "more: Shift, Ctrl, Alt");
    } else if(isBSpline){
        more = C_("Path handle tip", "more: Ctrl");
    }else {
        more = C_("Path handle tip", "more: Ctrl, Alt");
    }
    if (state_held_alt(state) && !isBSpline) {
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
                    "<b>Shift+Alt</b>: preserve handle length and rotate both handles");
            } else {
                return C_("Path handle tip",
                    "<b>Alt</b>: preserve handle length while dragging");
            }
        }
    } else {
        if (state_held_control(state)) {
            if (state_held_shift(state) && can_shift_rotate && !isBSpline) {
                return format_tip(C_("Path handle tip",
                    "<b>Shift+Ctrl</b>: snap rotation angle to %g° increments and rotate both handles"),
                    snap_increment_degrees());
            } else if(isBSpline){
                return format_tip(C_("Path handle tip",
                    "<b>Ctrl</b>: Snap handle to steps defined in BSpline Live Path Effect"));
            }else{
                return format_tip(C_("Path handle tip",
                    "<b>Ctrl</b>: snap rotation angle to %g° increments, click to retract"),
                    snap_increment_degrees());
            }
        } else if (state_held_shift(state) && can_shift_rotate && !isBSpline) {
            return C_("Path hande tip",
                "<b>Shift</b>: rotate both handles by the same angle");
        } else if(state_held_shift(state) && isBSpline){
            return C_("Path hande tip",
                "<b>Shift</b>: move handle");
        }
    }

    switch (_parent->type()) {
    case NODE_AUTO:
        return format_tip(C_("Path handle tip",
            "<b>Auto node handle</b>: drag to convert to smooth node (%s)"), more);
    default:
        if(!isBSpline){
            return format_tip(C_("Path handle tip",
                "<b>Auto node handle</b>: drag to convert to smooth node (%s)"), more);
        }else{
            return format_tip(C_("Path handle tip",
                "<b>BSpline node handle</b>: Shift to drag, double click to reset (%s). %g power"),more,_pm()._bsplineHandlePosition(h));
        }
    }
}

Glib::ustring Handle::_getDragTip(GdkEventMotion */*event*/) const
{
    Geom::Point dist = position() - _last_drag_origin();
    // report angle in mathematical convention
    double angle = Geom::angle_between(Geom::Point(-1,0), position() - _parent->position());
    angle += M_PI; // angle is (-M_PI...M_PI] - offset by +pi and scale to 0...360
    angle *= 360.0 / (2 * M_PI);
    
    Inkscape::Util::Quantity x_q = Inkscape::Util::Quantity(dist[Geom::X], "px");
    Inkscape::Util::Quantity y_q = Inkscape::Util::Quantity(dist[Geom::Y], "px");
    Inkscape::Util::Quantity len_q = Inkscape::Util::Quantity(length(), "px");
    GString *x = g_string_new(x_q.string(_desktop->namedview->display_units).c_str());
    GString *y = g_string_new(y_q.string(_desktop->namedview->display_units).c_str());
    GString *len = g_string_new(len_q.string(_desktop->namedview->display_units).c_str());
    Glib::ustring ret = format_tip(C_("Path handle tip",
        "Move handle by %s, %s; angle %.2f°, length %s"), x->str, y->str, angle, len->str);
    g_string_free(x, TRUE);
    g_string_free(y, TRUE);
    g_string_free(len, TRUE);
    return ret;
}

Node::Node(NodeSharedData const &data, Geom::Point const &initial_pos) :
    SelectableControlPoint(data.desktop, initial_pos, SP_ANCHOR_CENTER,
                           CTRL_TYPE_NODE_CUSP,
                           *data.selection,
                           node_colors, data.node_group),
    _front(data, initial_pos, this),
    _back(data, initial_pos, this),
    _type(NODE_CUSP),
    _handles_shown(false)
{
    // NOTE we do not set type here, because the handles are still degenerate
}

Node const *Node::_next() const
{
    return const_cast<Node*>(this)->_next();
}

// NOTE: not using iterators won't make this much quicker because iterators can be 100% inlined.
Node *Node::_next()
{
    NodeList::iterator n = NodeList::get_iterator(this).next();
    if (n) {
        return n.ptr();
    } else {
        return NULL;
    }
}

Node const *Node::_prev() const
{
    return const_cast<Node *>(this)->_prev();
}

Node *Node::_prev()
{
    NodeList::iterator p = NodeList::get_iterator(this).prev();
    if (p) {
        return p.ptr();
    } else {
        return NULL;
    }
}

void Node::move(Geom::Point const &new_pos)
{
    // move handles when the node moves.
    Geom::Point old_pos = position();
    Geom::Point delta = new_pos - position();

    // save the previous nodes strength to apply it again once the node is moved 
    double nodeWeight = NO_POWER;
    double nextNodeWeight = NO_POWER;
    double prevNodeWeight = NO_POWER;
    Node *n = this;
    Node * nextNode = n->nodeToward(n->front());
    Node * prevNode = n->nodeToward(n->back());
    nodeWeight = fmax(_pm()._bsplineHandlePosition(n->front(), false),_pm()._bsplineHandlePosition(n->back(), false));
    if(prevNode){
        prevNodeWeight = _pm()._bsplineHandlePosition(prevNode->front());
    }
    if(nextNode){
        nextNodeWeight = _pm()._bsplineHandlePosition(nextNode->back());
    }

    setPosition(new_pos);

    _front.setPosition(_front.position() + delta);
    _back.setPosition(_back.position() + delta);

    // if the node has a smooth handle after a line segment, it should be kept colinear
    // with the segment
    _fixNeighbors(old_pos, new_pos);

    // move the affected handlers. First the node ones, later the adjoining ones.
    if(_pm()._isBSpline()){
        _front.setPosition(_pm()._bsplineHandleReposition(this->front(),nodeWeight));
        _back.setPosition(_pm()._bsplineHandleReposition(this->back(),nodeWeight));
        if(prevNode){
            prevNode->front()->setPosition(_pm()._bsplineHandleReposition(prevNode->front(), prevNodeWeight));
        }
        if(nextNode){
            nextNode->back()->setPosition(_pm()._bsplineHandleReposition(nextNode->back(), nextNodeWeight));
        }
    }
}

void Node::transform(Geom::Affine const &m)
{

    Geom::Point old_pos = position();

    // save the previous nodes strength to apply it again once the node is moved 
    double nodeWeight = NO_POWER;
    double nextNodeWeight = NO_POWER;
    double prevNodeWeight = NO_POWER;
    Node *n = this;
    Node * nextNode = n->nodeToward(n->front());
    Node * prevNode = n->nodeToward(n->back());
    nodeWeight = _pm()._bsplineHandlePosition(n->front());
    if(prevNode){
        prevNodeWeight = _pm()._bsplineHandlePosition(prevNode->front());
    }
    if(nextNode){
        nextNodeWeight = _pm()._bsplineHandlePosition(nextNode->back());
    }

    setPosition(position() * m);
    _front.setPosition(_front.position() * m);
    _back.setPosition(_back.position() * m);

    /* Affine transforms keep handle invariants for smooth and symmetric nodes,
     * but smooth nodes at ends of linear segments and auto nodes need special treatment */
    _fixNeighbors(old_pos, position());

    // move the involved handlers, first the node ones, later the adjoining ones 
    if(_pm()._isBSpline()){
        _front.setPosition(_pm()._bsplineHandleReposition(this->front(), nodeWeight));
        _back.setPosition(_pm()._bsplineHandleReposition(this->back(), nodeWeight));
        if(prevNode){
            prevNode->front()->setPosition(_pm()._bsplineHandleReposition(prevNode->front(), prevNodeWeight));
        }
        if(nextNode){
            nextNode->back()->setPosition(_pm()._bsplineHandleReposition(nextNode->back(), nextNodeWeight));
        }
    }
}

Geom::Rect Node::bounds() const
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
    if (!_front.isDegenerate()) {
        _front.setVisible(v);
    }
    if (!_back.isDegenerate()) {
        _back.setVisible(v);
    }

}

void Node::updateHandles()
{
    _handleControlStyling();

    _front._handleControlStyling();
    _back._handleControlStyling();
}


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
            // nothing to do
            break;
        case NODE_AUTO:
            // auto handles make no sense for endnodes
            if (isEndNode()) return;
            _updateAutoHandles();
            break;
        case NODE_SMOOTH: {
            // ignore attempts to make smooth endnodes.
            if (isEndNode()) return;
            // rotate handles to be colinear
            // for degenerate nodes set positions like auto handles
            bool prev_line = _is_line_segment(_prev(), this);
            bool next_line = _is_line_segment(this, _next());
            if (_type == NODE_SMOOTH) {
                // For a node that is already smooth and has a degenerate handle,
                // drag out the second handle without changing the direction of the first one.
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
        /* in node type changes, about bspline traces, we can mantain them with NO_POWER power in border mode,
           or we give them the default power in curve mode */
        if(_pm()._isBSpline()){
            double weight = NO_POWER;
            if(_pm()._bsplineHandlePosition(this->front()) != NO_POWER ){
                weight = DEFAULT_START_POWER;
            }
            _front.setPosition(_pm()._bsplineHandleReposition(this->front(), weight));
            _back.setPosition(_pm()._bsplineHandleReposition(this->back(), weight));
        }
    }
    _type = type;
    _setControlType(nodeTypeToCtrlType(_type));
    updateState();
}

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
    _setControlType(nodeTypeToCtrlType(_type));
    updateState();
}

bool Node::isEndNode() const
{
    return !_prev() || !_next();
}

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

bool Node::_eventHandler(Inkscape::UI::Tools::ToolBase *event_context, GdkEvent *event)
{
    int dir = 0;

    switch (event->type)
    {
    case GDK_SCROLL:
        if (event->scroll.direction == GDK_SCROLL_UP) {
            dir = 1;
        } else if (event->scroll.direction == GDK_SCROLL_DOWN) {
            dir = -1;
        } else break;
        if (held_control(event->scroll)) {
            _linearGrow(dir);
        } else {
            _selection.spatialGrow(this, dir);
        }
        return true;
    case GDK_KEY_PRESS:
        switch (shortcut_key(event->key))
        {
        case GDK_KEY_Page_Up:
            dir = 1;
            break;
        case GDK_KEY_Page_Down:
            dir = -1;
            break;
        default: goto bail_out;
        }

        if (held_control(event->key)) {
            _linearGrow(dir);
        } else {
            _selection.spatialGrow(this, dir);
        }
        return true;

    default:
        break;
    }
    
    bail_out:
    return ControlPoint::_eventHandler(event_context, event);
}

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
            distance_front += Geom::bezier_length(*fwd, fwd->_front, n->_back, *n);
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
            distance_back += Geom::bezier_length(*rev, rev->_back, p->_front, *p);
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
                if (n) distance_front += Geom::bezier_length(*fwd, fwd->_front, n->_back, *n);
                fwd = n;
            } else if (rev && (!fwd || distance_front > distance_back)) {
                if (rev->selected()) {
                    last_rev = rev;
                    last_distance_back = distance_back;
                }
                NodeList::iterator p = rev.prev();
                if (p) distance_back += Geom::bezier_length(*rev, rev->_back, p->_front, *p);
                rev = p;
            }
            // Check whether we walked the entire cyclic subpath.
            // This is initially true because both iterators start from this node,
            // so this check cannot go in the while condition.
            // When this happens, we need to check the last node, pointed to by the iterators.
            if (fwd && fwd == rev) {
                if (!fwd->selected()) break;
                NodeList::iterator fwdp = fwd.prev(), revn = rev.next();
                double df = distance_front + Geom::bezier_length(*fwdp, fwdp->_front, fwd->_back, *fwd);
                double db = distance_back + Geom::bezier_length(*revn, revn->_back, rev->_front, *rev);
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
    ControlManager &mgr = ControlManager::getManager();
    mgr.setSelected(_canvas_item, selected());
    switch (state) {
        case STATE_NORMAL:
            mgr.setActive(_canvas_item, false);
            mgr.setPrelight(_canvas_item, false);
            break;
        case STATE_MOUSEOVER:
            mgr.setActive(_canvas_item, false);
            mgr.setPrelight(_canvas_item, true);
            break;
        case STATE_CLICKED:
            mgr.setActive(_canvas_item, true);
            mgr.setPrelight(_canvas_item, false);
            //this shows the handlers when selecting the nodes
            if(_pm()._isBSpline()){
                this->front()->setPosition(_pm()._bsplineHandleReposition(this->front()));
                this->back()->setPosition(_pm()._bsplineHandleReposition(this->back()));
            }
            break;
    }
    SelectableControlPoint::_setState(state);
}

bool Node::grabbed(GdkEventMotion *event)
{
    if (SelectableControlPoint::grabbed(event)) {
        return true;
    }

    // Dragging out handles with Shift + drag on a node.
    if (!held_shift(*event)) {
        return false;
    }

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
    if (!has_degenerate) {
        return false;
    }

    Handle *h = angle_next < angle_prev ? &_front : &_back;

    h->setPosition(_desktop->w2d(evp));
    h->setVisible(true);
    h->transferGrab(this, event);
    Handle::_drag_out = true;
    return true;
}

void Node::dragged(Geom::Point &new_pos, GdkEventMotion *event)
{
    if (tools_isactive(_desktop, TOOLS_NODES)) {
        Inkscape::UI::Tools::NodeTool *nt = static_cast<Inkscape::UI::Tools::NodeTool*>(_desktop->event_context);
        nt->update_helperpath();
    }
    // For a note on how snapping is implemented in Inkscape, see snap.h.
    SnapManager &sm = _desktop->namedview->snap_manager;
    // even if we won't really snap, we might still call the one of the
    // constrainedSnap() methods to enforce the constraints, so we need
    // to setup the snapmanager anyway; this is also required for someSnapperMightSnap()
    sm.setup(_desktop);

    // do not snap when Shift is pressed
    bool snap = !held_shift(*event) && sm.someSnapperMightSnap();

    Inkscape::SnappedPoint sp;
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
        sm.unSetup();
        sm.setupIgnoreSelection(_desktop, true, &unselected);
    }

    // Snap candidate point for free snapping; this will consider snapping tangentially
    // and perpendicularly and therefore the origin or direction vector must be set
    Inkscape::SnapCandidatePoint scp_free(new_pos, _snapSourceType());

    boost::optional<Geom::Point> front_point, back_point;
    Geom::Point origin = _last_drag_origin();
    Geom::Point dummy_cp;
    if (_front.isDegenerate()) {
        if (_is_line_segment(this, _next())) {
            front_point = _next()->position() - origin;
            if (_next()->selected()) {
                dummy_cp = _next()->position() - position();
                scp_free.addVector(dummy_cp);
            } else {
                dummy_cp = _next()->position();
                scp_free.addOrigin(dummy_cp);
            }
        }
    } else {
        front_point = _front.relativePos();
        scp_free.addVector(*front_point);
    }
    if (_back.isDegenerate()) {
        if (_is_line_segment(_prev(), this)) {
            back_point = _prev()->position() - origin;
            if (_prev()->selected()) {
                dummy_cp = _prev()->position() - position();
                scp_free.addVector(dummy_cp);
            } else {
                dummy_cp = _prev()->position();
                scp_free.addOrigin(dummy_cp);
            }
        }
    } else {
        back_point = _back.relativePos();
        scp_free.addVector(*back_point);
    }

    if (held_control(*event)) {
        // We're about to consider a constrained snap, which is already limited to 1D
        // Therefore tangential or perpendicular snapping will not be considered, and therefore
        // all calls above to scp_free.addVector() and scp_free.addOrigin() can be neglected
        std::vector<Inkscape::Snapper::SnapConstraint> constraints;
        if (held_alt(*event)) {
            // with Ctrl+Alt, constrain to handle lines
            // project the new position onto a handle line that is closer;
            // also snap to perpendiculars of handle lines

            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            int snaps = prefs->getIntLimited("/options/rotationsnapsperpi/value", 12, 1, 1000);
            double min_angle = M_PI / snaps;

            boost::optional<Geom::Point> fperp_point, bperp_point;
            if (front_point) {
                constraints.push_back(Inkscape::Snapper::SnapConstraint(origin, *front_point));
                fperp_point = Geom::rot90(*front_point);
            }
            if (back_point) {
                constraints.push_back(Inkscape::Snapper::SnapConstraint(origin, *back_point));
                bperp_point = Geom::rot90(*back_point);
            }
            // perpendiculars only snap when they are further than snap increment away
            // from the second handle constraint
            if (fperp_point && (!back_point ||
                (fabs(Geom::angle_between(*fperp_point, *back_point)) > min_angle &&
                 fabs(Geom::angle_between(*fperp_point, *back_point)) < M_PI - min_angle)))
            {
                constraints.push_back(Inkscape::Snapper::SnapConstraint(origin, *fperp_point));
            }
            if (bperp_point && (!front_point ||
                (fabs(Geom::angle_between(*bperp_point, *front_point)) > min_angle &&
                 fabs(Geom::angle_between(*bperp_point, *front_point)) < M_PI - min_angle)))
            {
                constraints.push_back(Inkscape::Snapper::SnapConstraint(origin, *bperp_point));
            }

            sp = sm.multipleConstrainedSnaps(Inkscape::SnapCandidatePoint(new_pos, _snapSourceType()), constraints, held_shift(*event));
        } else {
            // with Ctrl, constrain to axes
            constraints.push_back(Inkscape::Snapper::SnapConstraint(origin, Geom::Point(1, 0)));
            constraints.push_back(Inkscape::Snapper::SnapConstraint(origin, Geom::Point(0, 1)));
            sp = sm.multipleConstrainedSnaps(Inkscape::SnapCandidatePoint(new_pos, _snapSourceType()), constraints, held_shift(*event));
        }
        new_pos = sp.getPoint();
    } else if (snap) {
        Inkscape::SnappedPoint sp = sm.freeSnap(scp_free);
        new_pos = sp.getPoint();
    }

    sm.unSetup();

    SelectableControlPoint::dragged(new_pos, event);
}

bool Node::clicked(GdkEventButton *event)
{
    if(_pm()._nodeClicked(this, event))
        return true;
    return SelectableControlPoint::clicked(event);
}

Inkscape::SnapSourceType Node::_snapSourceType() const
{
    if (_type == NODE_SMOOTH || _type == NODE_AUTO)
        return SNAPSOURCE_NODE_SMOOTH;
    return SNAPSOURCE_NODE_CUSP;
}
Inkscape::SnapTargetType Node::_snapTargetType() const
{
    if (_type == NODE_SMOOTH || _type == NODE_AUTO)
        return SNAPTARGET_NODE_SMOOTH;
    return SNAPTARGET_NODE_CUSP;
}

Inkscape::SnapCandidatePoint Node::snapCandidatePoint()
{
    return SnapCandidatePoint(position(), _snapSourceType(), _snapTargetType());
}

Handle *Node::handleToward(Node *to)
{
    if (_next() == to) {
        return front();
    }
    if (_prev() == to) {
        return back();
    }
    g_error("Node::handleToward(): second node is not adjacent!");
    return NULL;
}

Node *Node::nodeToward(Handle *dir)
{
    if (front() == dir) {
        return _next();
    }
    if (back() == dir) {
        return _prev();
    }
    g_error("Node::nodeToward(): handle is not a child of this node!");
    return NULL;
}

Handle *Node::handleAwayFrom(Node *to)
{
    if (_next() == to) {
        return back();
    }
    if (_prev() == to) {
        return front();
    }
    g_error("Node::handleAwayFrom(): second node is not adjacent!");
    return NULL;
}

Node *Node::nodeAwayFrom(Handle *h)
{
    if (front() == h) {
        return _prev();
    }
    if (back() == h) {
        return _next();
    }
    g_error("Node::nodeAwayFrom(): handle is not a child of this node!");
    return NULL;
}

Glib::ustring Node::_getTip(unsigned state) const
{
    bool isBSpline = _pm()._isBSpline();
    Handle *h = const_cast<Handle *>(&_front);
    if (state_held_shift(state)) {
        bool can_drag_out = (_next() && _front.isDegenerate()) || (_prev() && _back.isDegenerate());
        if (can_drag_out) {
            /*if (state_held_control(state)) {
                return format_tip(C_("Path node tip",
                    "<b>Shift+Ctrl:</b> drag out a handle and snap its angle "
                    "to %f° increments"), snap_increment_degrees());
            }*/
            return C_("Path node tip",
                "<b>Shift</b>: drag out a handle, click to toggle selection");
        }
        return C_("Path node tip", "<b>Shift</b>: click to toggle selection");
    }

    if (state_held_control(state)) {
        if (state_held_alt(state)) {
            return C_("Path node tip", "<b>Ctrl+Alt</b>: move along handle lines, click to delete node");
        }
        return C_("Path node tip",
            "<b>Ctrl</b>: move along axes, click to change node type");
    }

    if (state_held_alt(state)) {
        return C_("Path node tip", "<b>Alt</b>: sculpt nodes");
    }

    // No modifiers: assemble tip from node type
    char const *nodetype = node_type_to_localized_string(_type);
    double power = _pm()._bsplineHandlePosition(h);
    if (_selection.transformHandlesEnabled() && selected()) {
        if (_selection.size() == 1 && !isBSpline) {
            return format_tip(C_("Path node tip",
                "<b>%s</b>: drag to shape the path (more: Shift, Ctrl, Alt)"), nodetype);
        }else if(_selection.size() == 1){
            return format_tip(C_("Path node tip",
                "<b>BSpline node</b>: drag to shape the path (more: Shift, Ctrl, Alt). %g power"), power);
        }
        return format_tip(C_("Path node tip",
            "<b>%s</b>: drag to shape the path, click to toggle scale/rotation handles (more: Shift, Ctrl, Alt)"), nodetype);
    }
    if (!isBSpline) {
        return format_tip(C_("Path node tip",
            "<b>%s</b>: drag to shape the path, click to select only this node (more: Shift, Ctrl, Alt)"), nodetype);
    }else{
        return format_tip(C_("Path node tip",
            "<b>BSpline node</b>: drag to shape the path, click to select only this node (more: Shift, Ctrl, Alt). %g power"), power);
    
    }
}

Glib::ustring Node::_getDragTip(GdkEventMotion */*event*/) const
{
    Geom::Point dist = position() - _last_drag_origin();
    
    Inkscape::Util::Quantity x_q = Inkscape::Util::Quantity(dist[Geom::X], "px");
    Inkscape::Util::Quantity y_q = Inkscape::Util::Quantity(dist[Geom::Y], "px");
    GString *x = g_string_new(x_q.string(_desktop->namedview->display_units).c_str());
    GString *y = g_string_new(y_q.string(_desktop->namedview->display_units).c_str());
    Glib::ustring ret = format_tip(C_("Path node tip", "Move node by %s, %s"), x->str, y->str);
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

bool Node::_is_line_segment(Node *first, Node *second)
{
    if (!first || !second) return false;
    if (first->_next() == second)
        return first->_front.isDegenerate() && second->_back.isDegenerate();
    if (second->_next() == first)
        return second->_front.isDegenerate() && first->_back.isDegenerate();
    return false;
}

NodeList::NodeList(SubpathList &splist)
    : _list(splist)
    , _closed(false)
{
    this->ln_list = this;
    this->ln_next = this;
    this->ln_prev = this;
}

NodeList::~NodeList()
{
    clear();
}

bool NodeList::empty()
{
    return ln_next == this;
}

NodeList::size_type NodeList::size()
{
    size_type sz = 0;
    for (ListNode *ln = ln_next; ln != this; ln = ln->ln_next) ++sz;
    return sz;
}

bool NodeList::closed()
{
    return _closed;
}

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

NodeList::iterator NodeList::before(Geom::PathTime const &pvp)
{
    iterator ret = begin();
    std::advance(ret, pvp.curve_index);
    return ret;
}

NodeList::iterator NodeList::insert(iterator pos, Node *x)
{
    ListNode *ins = pos._node;
    x->ln_next = ins;
    x->ln_prev = ins->ln_prev;
    ins->ln_prev->ln_next = x;
    ins->ln_prev = x;
    x->ln_list = this;
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

void NodeList::splice(iterator pos, NodeList &/*list*/, iterator first, iterator last)
{
    ListNode *ins_beg = first._node, *ins_end = last._node, *at = pos._node;
    for (ListNode *ln = ins_beg; ln != ins_end; ln = ln->ln_next) {
        ln->ln_list = this;
    }
    ins_beg->ln_prev->ln_next = ins_end;
    ins_end->ln_prev->ln_next = at;
    at->ln_prev->ln_next = ins_beg;

    ListNode *atprev = at->ln_prev;
    at->ln_prev = ins_end->ln_prev;
    ins_end->ln_prev = ins_beg->ln_prev;
    ins_beg->ln_prev = atprev;
}

void NodeList::shift(int n)
{
    // 1. make the list perfectly cyclic
    ln_next->ln_prev = ln_prev;
    ln_prev->ln_next = ln_next;
    // 2. find new begin
    ListNode *new_begin = ln_next;
    if (n > 0) {
        for (; n > 0; --n) new_begin = new_begin->ln_next;
    } else {
        for (; n < 0; ++n) new_begin = new_begin->ln_prev;
    }
    // 3. relink begin to list
    ln_next = new_begin;
    ln_prev = new_begin->ln_prev;
    new_begin->ln_prev->ln_next = this;
    new_begin->ln_prev = this;
}

void NodeList::reverse()
{
    for (ListNode *ln = ln_next; ln != this; ln = ln->ln_prev) {
        std::swap(ln->ln_next, ln->ln_prev);
        Node *node = static_cast<Node*>(ln);
        Geom::Point save_pos = node->front()->position();
        node->front()->setPosition(node->back()->position());
        node->back()->setPosition(save_pos);
    }
    std::swap(ln_next, ln_prev);
}

void NodeList::clear()
{
    // ugly but more efficient clearing mechanism
    std::vector<ControlPointSelection *> to_clear;
    std::vector<std::pair<SelectableControlPoint *, long> > nodes;
    long in = -1;
    for (iterator i = begin(); i != end(); ++i) {
        SelectableControlPoint *rm = static_cast<Node*>(i._node);
        if (std::find(to_clear.begin(), to_clear.end(), &rm->_selection) == to_clear.end()) {
            to_clear.push_back(&rm->_selection);
            ++in;
        }
        nodes.push_back(std::make_pair(rm, in));
    }
    for (size_t i = 0, e = nodes.size(); i != e; ++i) {
        to_clear[nodes[i].second]->erase(nodes[i].first, false);
    }
    std::vector<std::vector<SelectableControlPoint *> > emission;
    for (long i = 0, e = to_clear.size(); i != e; ++i) {
        emission.push_back(std::vector<SelectableControlPoint *>());
        for (size_t j = 0, f = nodes.size(); j != f; ++j) {
            if (nodes[j].second != i)
                break;
            emission[i].push_back(nodes[j].first);
        }
    }

    for (size_t i = 0, e = emission.size(); i != e; ++i) {
        to_clear[i]->signal_selection_changed.emit(emission[i], false);
    }

    for (iterator i = begin(); i != end();)
        erase (i++);
}

NodeList::iterator NodeList::erase(iterator i)
{
    // some gymnastics are required to ensure that the node is valid when deleted;
    // otherwise the code that updates handle visibility will break
    Node *rm = static_cast<Node*>(i._node);
    ListNode *rmnext = rm->ln_next, *rmprev = rm->ln_prev;
    ++i;
    delete rm;
    rmprev->ln_next = rmnext;
    rmnext->ln_prev = rmprev;
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
    return n->nodeList();
}
NodeList &NodeList::get(iterator const &i) {
    return *(i._node->ln_list);
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
