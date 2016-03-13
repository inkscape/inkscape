/**
 * @file
 * Node selection - implementation.
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <boost/none.hpp>
#include "ui/tool/selectable-control-point.h"
#include <2geom/transforms.h>
#include "desktop.h"
#include "preferences.h"
#include "ui/tool/control-point-selection.h"
#include "ui/tool/event-utils.h"
#include "ui/tool/transform-handle-set.h"
#include "ui/tool/node.h"



#include <gdk/gdkkeysyms.h>

namespace Inkscape {
namespace UI {

/**
 * @class ControlPointSelection
 * Group of selected control points.
 *
 * Some operations can be performed on all selected points regardless of their type, therefore
 * this class is also a Manipulator. It handles the transformations of points using
 * the keyboard.
 *
 * The exposed interface is similar to that of an STL set. Internally, a hash map is used.
 * @todo Correct iterators (that don't expose the connection list)
 */

/** @var ControlPointSelection::signal_update
 * Fires when the display needs to be updated to reflect changes.
 */
/** @var ControlPointSelection::signal_point_changed
 * Fires when a control point is added to or removed from the selection.
 * The first param contains a pointer to the control point that changed sel. state. 
 * The second says whether the point is currently selected.
 */
/** @var ControlPointSelection::signal_commit
 * Fires when a change that needs to be committed to XML happens.
 */

ControlPointSelection::ControlPointSelection(SPDesktop *d, SPCanvasGroup *th_group)
    : Manipulator(d)
    , _handles(new TransformHandleSet(d, th_group))
    , _dragging(false)
    , _handles_visible(true)
    , _one_node_handles(false)
{
    signal_update.connect( sigc::bind(
        sigc::mem_fun(*this, &ControlPointSelection::_updateTransformHandles),
        true));
    ControlPoint::signal_mouseover_change.connect(
        sigc::hide(
            sigc::mem_fun(*this, &ControlPointSelection::_mouseoverChanged)));
    _handles->signal_transform.connect(
        sigc::mem_fun(*this, &ControlPointSelection::transform));
    _handles->signal_commit.connect(
        sigc::mem_fun(*this, &ControlPointSelection::_commitHandlesTransform));
}

ControlPointSelection::~ControlPointSelection()
{
    clear();
    delete _handles;
}

/** Add a control point to the selection. */
std::pair<ControlPointSelection::iterator, bool> ControlPointSelection::insert(const value_type &x, bool notify)
{
    iterator found = _points.find(x);
    if (found != _points.end()) {
        return std::pair<iterator, bool>(found, false);
    }

    found = _points.insert(x).first;
    _points_list.push_back(x);

    x->updateState();
    _pointChanged(x, true);

    if (notify) {
        signal_selection_changed.emit(std::vector<key_type>(1, x), true);
    }

    return std::pair<iterator, bool>(found, true);
}

/** Remove a point from the selection. */
void ControlPointSelection::erase(iterator pos)
{
    SelectableControlPoint *erased = *pos;
    _points_list.remove(*pos);
    _points.erase(pos);
    erased->updateState();
    _pointChanged(erased, false);
}
ControlPointSelection::size_type ControlPointSelection::erase(const key_type &k, bool notify)
{
    iterator pos = _points.find(k);
    if (pos == _points.end()) return 0;
    erase(pos);

    if (notify) {
        signal_selection_changed.emit(std::vector<key_type>(1, k), false);
    }
    return 1;
}
void ControlPointSelection::erase(iterator first, iterator last)
{
    std::vector<SelectableControlPoint *> out(first, last);
    while (first != last) erase(first++);
    signal_selection_changed.emit(out, false);
}

/** Remove all points from the selection, making it empty. */
void ControlPointSelection::clear()
{
    std::vector<SelectableControlPoint *> out(begin(), end());
    for (iterator i = begin(); i != end(); )
        erase(i++);
    if (!out.empty())
        signal_selection_changed.emit(out, false);
}

/** Select all points that this selection can contain. */
void ControlPointSelection::selectAll()
{
    for (set_type::iterator i = _all_points.begin(); i != _all_points.end(); ++i) {
        insert(*i, false);
    }
    std::vector<SelectableControlPoint *> out(_all_points.begin(), _all_points.end());
    if (!out.empty())
        signal_selection_changed.emit(out, true);
}
/** Select all points inside the given rectangle (in desktop coordinates). */
void ControlPointSelection::selectArea(Geom::Rect const &r)
{
    std::vector<SelectableControlPoint *> out;
    for (set_type::iterator i = _all_points.begin(); i != _all_points.end(); ++i) {
        if (r.contains(**i)) {
            insert(*i, false);
            out.push_back(*i);
        }
    }
    if (!out.empty())
        signal_selection_changed.emit(out, true);
}
/** Unselect all selected points and select all unselected points. */
void ControlPointSelection::invertSelection()
{
    std::vector<SelectableControlPoint *> in, out;
    for (set_type::iterator i = _all_points.begin(); i != _all_points.end(); ++i) {
        if ((*i)->selected()) {
            in.push_back(*i);
            erase(*i); 
        }
        else {
            out.push_back(*i);
            insert(*i, false); 
        }
    }
    if (!in.empty())
        signal_selection_changed.emit(in, false);
    if (!out.empty())
        signal_selection_changed.emit(out, true);
}
void ControlPointSelection::spatialGrow(SelectableControlPoint *origin, int dir)
{
    bool grow = (dir > 0);
    Geom::Point p = origin->position();
    double best_dist = grow ? HUGE_VAL : 0;
    SelectableControlPoint *match = NULL;
    for (set_type::iterator i = _all_points.begin(); i != _all_points.end(); ++i) {
        bool selected = (*i)->selected();
        if (grow && !selected) {
            double dist = Geom::distance((*i)->position(), p);
            if (dist < best_dist) {
                best_dist = dist;
                match = *i;
            }
        }
        if (!grow && selected) {
            double dist = Geom::distance((*i)->position(), p);
            // use >= to also deselect the origin node when it's the last one selected
            if (dist >= best_dist) {
                best_dist = dist;
                match = *i;
            }
        }
    }
    if (match) {
        if (grow) insert(match);
        else erase(match);
        signal_selection_changed.emit(std::vector<value_type>(1, match), grow);
    }
}

/** Transform all selected control points by the given affine transformation. */
void ControlPointSelection::transform(Geom::Affine const &m)
{
    for (iterator i = _points.begin(); i != _points.end(); ++i) {
        SelectableControlPoint *cur = *i;
        cur->transform(m);
    }
    _updateBounds();
    // TODO preserving the rotation radius needs some rethinking...
    if (_rot_radius) (*_rot_radius) *= m.descrim();
    if (_mouseover_rot_radius) (*_mouseover_rot_radius) *= m.descrim();
    signal_update.emit();
}

/** Align control points on the specified axis. */
void ControlPointSelection::align(Geom::Dim2 axis)
{
    enum AlignTargetNode { LAST_NODE=0, FIRST_NODE, MID_NODE, MIN_NODE, MAX_NODE };
    if (empty()) return;
    Geom::Dim2 d = static_cast<Geom::Dim2>((axis + 1) % 2);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();


    Geom::OptInterval bound;
    for (iterator i = _points.begin(); i != _points.end(); ++i) {
        bound.unionWith(Geom::OptInterval((*i)->position()[d]));
    }

    if (!bound) { return; }

    double new_coord;
    switch (AlignTargetNode(prefs->getInt("/dialogs/align/align-nodes-to", 2))){
        case FIRST_NODE:
            new_coord=(_points_list.front())->position()[d];
            break;
        case LAST_NODE:
            new_coord=(_points_list.back())->position()[d];
            break;
        case MID_NODE:
            new_coord=bound->middle();
            break;
        case MIN_NODE:
            new_coord=bound->min();
            break;
        case MAX_NODE:
            new_coord=bound->max();
            break;
        default:
            return;
    }

    for (iterator i = _points.begin(); i != _points.end(); ++i) {
        Geom::Point pos = (*i)->position();
        pos[d] = new_coord;
        (*i)->move(pos);
    }
}

/** Equdistantly distribute control points by moving them in the specified dimension. */
void ControlPointSelection::distribute(Geom::Dim2 d)
{
    if (empty()) return;

    // this needs to be a multimap, otherwise it will fail when some points have the same coord
    typedef std::multimap<double, SelectableControlPoint*> SortMap;

    SortMap sm;
    Geom::OptInterval bound;
    // first we insert all points into a multimap keyed by the aligned coord to sort them
    // simultaneously we compute the extent of selection
    for (iterator i = _points.begin(); i != _points.end(); ++i) {
        Geom::Point pos = (*i)->position();
        sm.insert(std::make_pair(pos[d], (*i)));
        bound.unionWith(Geom::OptInterval(pos[d]));
    }

    if (!bound) { return; }

    // now we iterate over the multimap and set aligned positions.
    double step = size() == 1 ? 0 : bound->extent() / (size() - 1);
    double start = bound->min();
    unsigned num = 0;
    for (SortMap::iterator i = sm.begin(); i != sm.end(); ++i, ++num) {
        Geom::Point pos = i->second->position();
        pos[d] = start + num * step;
        i->second->move(pos);
    }
}

/** Get the bounds of the selection.
 * @return Smallest rectangle containing the positions of all selected points,
 *         or nothing if the selection is empty */
Geom::OptRect ControlPointSelection::pointwiseBounds()
{
    return _bounds;
}

Geom::OptRect ControlPointSelection::bounds()
{
    return size() == 1 ? (*_points.begin())->bounds() : _bounds;
}

void ControlPointSelection::showTransformHandles(bool v, bool one_node)
{
    _one_node_handles = one_node;
    _handles_visible = v;
    _updateTransformHandles(false);
}

void ControlPointSelection::hideTransformHandles()
{
    _handles->setVisible(false);
}
void ControlPointSelection::restoreTransformHandles()
{
    _updateTransformHandles(true);
}

void ControlPointSelection::toggleTransformHandlesMode()
{
    if (_handles->mode() == TransformHandleSet::MODE_SCALE) {
        _handles->setMode(TransformHandleSet::MODE_ROTATE_SKEW);
        if (size() == 1) {
            _handles->rotationCenter().setVisible(false);
        }
    } else {
        _handles->setMode(TransformHandleSet::MODE_SCALE);
    }
}

void ControlPointSelection::_pointGrabbed(SelectableControlPoint *point)
{
    hideTransformHandles();
    _dragging = true;
    _grabbed_point = point;
    _farthest_point = point;
    double maxdist = 0;
    Geom::Affine m;
    m.setIdentity();
    for (iterator i = _points.begin(); i != _points.end(); ++i) {
        _original_positions.insert(std::make_pair(*i, (*i)->position()));
        _last_trans.insert(std::make_pair(*i, m));
        double dist = Geom::distance(*_grabbed_point, **i);
        if (dist > maxdist) {
            maxdist = dist;
            _farthest_point = *i;
        }
    }
}

void ControlPointSelection::_pointDragged(Geom::Point &new_pos, GdkEventMotion *event)
{
    Geom::Point abs_delta = new_pos - _original_positions[_grabbed_point];
    double fdist = Geom::distance(_original_positions[_grabbed_point], _original_positions[_farthest_point]);
    if (held_only_alt(*event) && fdist > 0) {
        // Sculpting
        for (iterator i = _points.begin(); i != _points.end(); ++i) {
            SelectableControlPoint *cur = (*i);
            Geom::Affine trans;
            trans.setIdentity();
            double dist = Geom::distance(_original_positions[cur], _original_positions[_grabbed_point]);
            double deltafrac = 0.5 + 0.5 * cos(M_PI * dist/fdist);
            if (dist != 0.0) {
                // The sculpting transformation is not affine, but it can be
                // locally approximated by one. Here we compute the local
                // affine approximation of the sculpting transformation near
                // the currently transformed point. We then transform the point
                // by this approximation. This gives us sensible behavior for node handles.
                // NOTE: probably it would be better to transform the node handles,
                // but ControlPointSelection is supposed to work for any
                // SelectableControlPoints, not only Nodes. We could create a specialized
                // NodeSelection class that inherits from this one and move sculpting there.
                Geom::Point origdx(Geom::EPSILON, 0);
                Geom::Point origdy(0, Geom::EPSILON);
                Geom::Point origp = _original_positions[cur];
                Geom::Point origpx = _original_positions[cur] + origdx;
                Geom::Point origpy = _original_positions[cur] + origdy;
                double distdx = Geom::distance(origpx, _original_positions[_grabbed_point]);
                double distdy = Geom::distance(origpy, _original_positions[_grabbed_point]);
                double deltafracdx = 0.5 + 0.5 * cos(M_PI * distdx/fdist);
                double deltafracdy = 0.5 + 0.5 * cos(M_PI * distdy/fdist);
                Geom::Point newp = origp + abs_delta * deltafrac;
                Geom::Point newpx = origpx + abs_delta * deltafracdx;
                Geom::Point newpy = origpy + abs_delta * deltafracdy;
                Geom::Point newdx = (newpx - newp) / Geom::EPSILON;
                Geom::Point newdy = (newpy - newp) / Geom::EPSILON;

                Geom::Affine itrans(newdx[Geom::X], newdx[Geom::Y], newdy[Geom::X], newdy[Geom::Y], 0, 0);
                if (itrans.isSingular())
                    itrans.setIdentity();

                trans *= Geom::Translate(-cur->position());
                trans *= _last_trans[cur].inverse();
                trans *= itrans;
                trans *= Geom::Translate(_original_positions[cur] + abs_delta * deltafrac);
                _last_trans[cur] = itrans;
            } else {
                trans *= Geom::Translate(-cur->position() + _original_positions[cur] + abs_delta * deltafrac);
            }
            cur->transform(trans);
            //cur->move(_original_positions[cur] + abs_delta * deltafrac);
        }
    } else {
        Geom::Point delta = new_pos - _grabbed_point->position();
        for (iterator i = _points.begin(); i != _points.end(); ++i) {
            SelectableControlPoint *cur = (*i);
            cur->move(_original_positions[cur] + abs_delta);
        }
        _handles->rotationCenter().move(_handles->rotationCenter().position() + delta);
    }
    signal_update.emit();
}

void ControlPointSelection::_pointUngrabbed()
{
    _original_positions.clear();
    _last_trans.clear();
    _dragging = false;
    _grabbed_point = _farthest_point = NULL;
    _updateBounds();
    restoreTransformHandles();
    signal_commit.emit(COMMIT_MOUSE_MOVE);
}

bool ControlPointSelection::_pointClicked(SelectableControlPoint *p, GdkEventButton *event)
{
    // clicking a selected node should toggle the transform handles between rotate and scale mode,
    // if they are visible
    if (held_no_modifiers(*event) && _handles_visible && p->selected()) {
        toggleTransformHandlesMode();
        return true;
    }
    return false;
}

void ControlPointSelection::_pointChanged(SelectableControlPoint *p, bool selected)
{
    _updateBounds();
    _updateTransformHandles(false);
    if (_bounds) {
        _handles->rotationCenter().move(_bounds->midpoint());
    }

    //signal_point_changed.emit(p, selected);
}

void ControlPointSelection::_mouseoverChanged()
{
    _mouseover_rot_radius = boost::none;
}

void ControlPointSelection::_updateBounds()
{
    _rot_radius = boost::none;
    _bounds = Geom::OptRect();
    for (iterator i = _points.begin(); i != _points.end(); ++i) {
        SelectableControlPoint *cur = (*i);
        Geom::Point p = cur->position();
        if (!_bounds) {
            _bounds = Geom::Rect(p, p);
        } else {
            _bounds->expandTo(p);
        }
    }
}

void ControlPointSelection::_updateTransformHandles(bool preserve_center)
{
    if (_dragging) return;

    if (_handles_visible && size() > 1) {
        _handles->setBounds(*bounds(), preserve_center);
        _handles->setVisible(true);
    } else if (_one_node_handles && size() == 1) { // only one control point in selection
        SelectableControlPoint *p = *begin();
        _handles->setBounds(p->bounds());
        _handles->rotationCenter().move(p->position());
        _handles->rotationCenter().setVisible(false);
        _handles->setVisible(true);
    } else {
        _handles->setVisible(false);
    }
}

/** Moves the selected points along the supplied unit vector according to
 * the modifier state of the supplied event. */
bool ControlPointSelection::_keyboardMove(GdkEventKey const &event, Geom::Point const &dir)
{
    if (held_control(event)) return false;
    unsigned num = 1 + combine_key_events(shortcut_key(event), 0);

    Geom::Point delta = dir * num; 
    if (held_shift(event)) delta *= 10;
    if (held_alt(event)) {
        delta /= _desktop->current_zoom();
    } else {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        double nudge = prefs->getDoubleLimited("/options/nudgedistance/value", 2, 0, 1000, "px");
        delta *= nudge;
    }

    transform(Geom::Translate(delta));
    if (fabs(dir[Geom::X]) > 0) {
        signal_commit.emit(COMMIT_KEYBOARD_MOVE_X);
    } else {
        signal_commit.emit(COMMIT_KEYBOARD_MOVE_Y);
    }
    return true;
}

/**
 * Computes the distance to the farthest corner of the bounding box.
 * Used to determine what it means to "rotate by one pixel".
 */
double ControlPointSelection::_rotationRadius(Geom::Point const &rc)
{
    if (empty()) return 1.0; // some safe value
    Geom::Rect b = *bounds();
    double maxlen = 0;
    for (unsigned i = 0; i < 4; ++i) {
        double len = Geom::distance(b.corner(i), rc);
        if (len > maxlen) maxlen = len;
    }
    return maxlen;
}

/**
 * Rotates the selected points in the given direction according to the modifier state
 * from the supplied event.
 * @param event Key event to take modifier state from
 * @param dir   Direction of rotation (math convention: 1 = counterclockwise, -1 = clockwise)
 */
bool ControlPointSelection::_keyboardRotate(GdkEventKey const &event, int dir)
{
    if (empty()) return false;

    Geom::Point rc;

    // rotate around the mouseovered point, or the selection's rotation center
    // if nothing is mouseovered
    double radius;
    SelectableControlPoint *scp =
        dynamic_cast<SelectableControlPoint*>(ControlPoint::mouseovered_point);
    if (scp) {
        rc = scp->position();
        if (!_mouseover_rot_radius) {
            _mouseover_rot_radius = _rotationRadius(rc);
        }
        radius = *_mouseover_rot_radius;
    } else {
        rc = _handles->rotationCenter();
        if (!_rot_radius) {
            _rot_radius = _rotationRadius(rc);
        }
        radius = *_rot_radius;
    }

    double angle;
    if (held_alt(event)) {
        // Rotate by "one pixel". We interpret this as rotating by an angle that causes
        // the topmost point of a circle circumscribed about the selection's bounding box
        // to move on an arc 1 screen pixel long.
        angle = atan2(1.0 / _desktop->current_zoom(), radius) * dir;
    } else {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        int snaps = prefs->getIntLimited("/options/rotationsnapsperpi/value", 12, 1, 1000);
        angle = M_PI * dir / snaps;
    }

    // translate to origin, rotate, translate back to original position
    Geom::Affine m = Geom::Translate(-rc)
        * Geom::Rotate(angle) * Geom::Translate(rc);
    transform(m);
    signal_commit.emit(COMMIT_KEYBOARD_ROTATE);
    return true;
}


bool ControlPointSelection::_keyboardScale(GdkEventKey const &event, int dir)
{
    if (empty()) return false;

    double maxext = bounds()->maxExtent();
    if (Geom::are_near(maxext, 0)) return false;

    Geom::Point center;
    SelectableControlPoint *scp =
        dynamic_cast<SelectableControlPoint*>(ControlPoint::mouseovered_point);
    if (scp) {
        center = scp->position();
    } else {
        center = _handles->rotationCenter().position();
    }

    double length_change;
    if (held_alt(event)) {
        // Scale by "one pixel". It means shrink/grow 1px for the larger dimension
        // of the bounding box.
        length_change = 1.0 / _desktop->current_zoom() * dir;
    } else {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        length_change = prefs->getDoubleLimited("/options/defaultscale/value", 2, 1, 1000, "px");
        length_change *= dir;
    }
    double scale = (maxext + length_change) / maxext;
    
    Geom::Affine m = Geom::Translate(-center) * Geom::Scale(scale) * Geom::Translate(center);
    transform(m);
    signal_commit.emit(COMMIT_KEYBOARD_SCALE_UNIFORM);
    return true;
}

bool ControlPointSelection::_keyboardFlip(Geom::Dim2 d)
{
    if (empty()) return false;

    Geom::Scale scale_transform(1, 1);
    if (d == Geom::X) {
        scale_transform = Geom::Scale(-1, 1);
    } else {
        scale_transform = Geom::Scale(1, -1);
    }

    SelectableControlPoint *scp =
        dynamic_cast<SelectableControlPoint*>(ControlPoint::mouseovered_point);
    Geom::Point center = scp ? scp->position() : _handles->rotationCenter().position();

    Geom::Affine m = Geom::Translate(-center) * scale_transform * Geom::Translate(center);
    transform(m);
    signal_commit.emit(d == Geom::X ? COMMIT_FLIP_X : COMMIT_FLIP_Y);
    return true;
}

void ControlPointSelection::_commitHandlesTransform(CommitEvent ce)
{
    _updateBounds();
    _updateTransformHandles(true);
    signal_commit.emit(ce);
}

bool ControlPointSelection::event(Inkscape::UI::Tools::ToolBase * /*event_context*/, GdkEvent *event)
{
    // implement generic event handling that should apply for all control point selections here;
    // for example, keyboard moves and transformations. This way this functionality doesn't need
    // to be duplicated in many places
    // Later split out so that it can be reused in object selection

    switch (event->type) {
    case GDK_KEY_PRESS:
        // do not handle key events if the selection is empty
        if (empty()) break;

        switch(shortcut_key(event->key)) {
        // moves
        case GDK_KEY_Up:
        case GDK_KEY_KP_Up:
        case GDK_KEY_KP_8:
            return _keyboardMove(event->key, Geom::Point(0, 1));
        case GDK_KEY_Down:
        case GDK_KEY_KP_Down:
        case GDK_KEY_KP_2:
            return _keyboardMove(event->key, Geom::Point(0, -1));
        case GDK_KEY_Right:
        case GDK_KEY_KP_Right:
        case GDK_KEY_KP_6:
            return _keyboardMove(event->key, Geom::Point(1, 0));
        case GDK_KEY_Left:
        case GDK_KEY_KP_Left:
        case GDK_KEY_KP_4:
            return _keyboardMove(event->key, Geom::Point(-1, 0));

        // rotates
        case GDK_KEY_bracketleft:
            return _keyboardRotate(event->key, 1);
        case GDK_KEY_bracketright:
            return _keyboardRotate(event->key, -1);

        // scaling
        case GDK_KEY_less:
        case GDK_KEY_comma:
            return _keyboardScale(event->key, -1);
        case GDK_KEY_greater:
        case GDK_KEY_period:
            return _keyboardScale(event->key, 1);

        // TODO: skewing

        // flipping
        // NOTE: H is horizontal flip, while Shift+H switches transform handle mode!
        case GDK_KEY_h:
        case GDK_KEY_H:
            if (held_shift(event->key)) {
                toggleTransformHandlesMode();
                return true;
            }
            // any modifiers except shift should cause no action
            if (held_any_modifiers(event->key)) break;
            return _keyboardFlip(Geom::X);
        case GDK_KEY_v:
        case GDK_KEY_V:
            if (held_any_modifiers(event->key)) break;
            return _keyboardFlip(Geom::Y);
        default: break;
        }
        break;
    default: break;
    }
    return false;
}

void ControlPointSelection::getOriginalPoints(std::vector<Inkscape::SnapCandidatePoint> &pts)
{
    pts.clear();
    for (iterator i = _points.begin(); i != _points.end(); ++i) {
        pts.push_back(Inkscape::SnapCandidatePoint(_original_positions[*i], SNAPSOURCE_NODE_HANDLE));
    }
}

void ControlPointSelection::getUnselectedPoints(std::vector<Inkscape::SnapCandidatePoint> &pts)
{
    pts.clear();
    ControlPointSelection::Set &nodes = this->allPoints();
    for (ControlPointSelection::Set::iterator i = nodes.begin(); i != nodes.end(); ++i) {
        if (!(*i)->selected()) {
            Node *n = static_cast<Node*>(*i);
            pts.push_back(n->snapCandidatePoint());
        }
    }
}

void ControlPointSelection::setOriginalPoints()
{
    _original_positions.clear();
    for (iterator i = _points.begin(); i != _points.end(); ++i) {
        _original_positions.insert(std::make_pair(*i, (*i)->position()));
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
