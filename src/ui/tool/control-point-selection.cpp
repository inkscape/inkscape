/** @file
 * Node selection - implementation
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <2geom/transforms.h>
#include "desktop.h"
#include "preferences.h"
#include "ui/tool/control-point-selection.h"
#include "ui/tool/event-utils.h"
#include "ui/tool/selectable-control-point.h"
#include "ui/tool/transform-handle-set.h"

namespace Inkscape {
namespace UI {

/**
 * @class ControlPointSelection
 * @brief Group of selected control points.
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
    signal_point_changed.connect(
        sigc::hide( sigc::hide(
            sigc::bind(
                sigc::mem_fun(*this, &ControlPointSelection::_updateTransformHandles),
                false))));
    _handles->signal_transform.connect(
        sigc::mem_fun(*this, &ControlPointSelection::transform));
    _handles->signal_commit.connect(
        sigc::mem_fun(*this, &ControlPointSelection::_commitTransform));
}

ControlPointSelection::~ControlPointSelection()
{
    clear();
    delete _handles;
}

/** Add a control point to the selection. */
std::pair<ControlPointSelection::iterator, bool> ControlPointSelection::insert(const value_type &x)
{
    iterator found = _points.find(x);
    if (found != _points.end()) {
        return std::pair<iterator, bool>(found, false);
    }

    boost::shared_ptr<connlist_type> clist(new connlist_type());

    // hide event param and always return false
    /*clist->push_back(
        x->signal_grabbed.connect(
            sigc::bind_return(
                sigc::bind<0>(
                    sigc::mem_fun(*this, &ControlPointSelection::_selectionGrabbed),
                    x),
                false)));
    clist->push_back(
        x->signal_dragged.connect(
                sigc::mem_fun(*this, &ControlPointSelection::_selectionDragged)));
    clist->push_back(
        x->signal_ungrabbed.connect(
            sigc::hide(
                sigc::mem_fun(*this, &ControlPointSelection::_selectionUngrabbed))));
    clist->push_back(
        x->signal_clicked.connect(
            sigc::hide(
                sigc::bind<0>(
                    sigc::mem_fun(*this, &ControlPointSelection::_selectionClicked),
                    x))));*/

    found = _points.insert(std::make_pair(x, clist)).first;

    x->updateState();
    _rot_radius.reset();
    signal_point_changed.emit(x, true);

    return std::pair<iterator, bool>(found, true);
}

/** Remove a point from the selection. */
void ControlPointSelection::erase(iterator pos)
{
    SelectableControlPoint *erased = pos->first;
    boost::shared_ptr<connlist_type> clist = pos->second;
    for (connlist_type::iterator i = clist->begin(); i != clist->end(); ++i) {
        i->disconnect();
    }
    _points.erase(pos);
    erased->updateState();
    _rot_radius.reset();
    signal_point_changed.emit(erased, false);
}
ControlPointSelection::size_type ControlPointSelection::erase(const key_type &k)
{
    iterator pos = _points.find(k);
    if (pos == _points.end()) return 0;
    erase(pos);
    return 1;
}
void ControlPointSelection::erase(iterator first, iterator last)
{
    while (first != last) erase(first++);
}

/** Remove all points from the selection, making it empty. */
void ControlPointSelection::clear()
{
    for (iterator i = begin(); i != end(); )
        erase(i++);
}

/** Select all points that this selection can contain. */
void ControlPointSelection::selectAll()
{
    for (set_type::iterator i = _all_points.begin(); i != _all_points.end(); ++i) {
        insert(*i);
    }
}
/** Select all points inside the given rectangle (in desktop coordinates). */
void ControlPointSelection::selectArea(Geom::Rect const &r)
{
    for (set_type::iterator i = _all_points.begin(); i != _all_points.end(); ++i) {
        if (r.contains(**i))
            insert(*i);
    }
}
/** Unselect all selected points and select all unselected points. */
void ControlPointSelection::invertSelection()
{
    for (set_type::iterator i = _all_points.begin(); i != _all_points.end(); ++i) {
        if ((*i)->selected()) erase(*i);
        else insert(*i);
    }
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
    }
}

/** Transform all selected control points by the given affine transformation. */
void ControlPointSelection::transform(Geom::Matrix const &m)
{
    for (iterator i = _points.begin(); i != _points.end(); ++i) {
        SelectableControlPoint *cur = i->first;
        cur->transform(m);
    }
    // TODO preserving the rotation radius needs some rethinking...
    if (_rot_radius) (*_rot_radius) *= m.descrim();
    signal_update.emit();
}

/** Align control points on the specified axis. */
void ControlPointSelection::align(Geom::Dim2 axis)
{
    if (empty()) return;
    Geom::Dim2 d = static_cast<Geom::Dim2>((axis + 1) % 2);

    Geom::OptInterval bound;
    for (iterator i = _points.begin(); i != _points.end(); ++i) {
        bound.unionWith(Geom::OptInterval(i->first->position()[d]));
    }

    double new_coord = bound->middle();
    for (iterator i = _points.begin(); i != _points.end(); ++i) {
        Geom::Point pos = i->first->position();
        pos[d] = new_coord;
        i->first->move(pos);
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
        Geom::Point pos = i->first->position();
        sm.insert(std::make_pair(pos[d], i->first));
        bound.unionWith(Geom::OptInterval(pos[d]));
    }

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
    Geom::OptRect bound;
    for (iterator i = _points.begin(); i != _points.end(); ++i) {
        SelectableControlPoint *cur = i->first;
        Geom::Point p = cur->position();
        if (!bound) {
            bound = Geom::Rect(p, p);
        } else {
            bound->expandTo(p);
        }
    }
    return bound;
}

Geom::OptRect ControlPointSelection::bounds()
{
    Geom::OptRect bound;
    for (iterator i = _points.begin(); i != _points.end(); ++i) {
        SelectableControlPoint *cur = i->first;
        Geom::OptRect r = cur->bounds();
        bound.unionWith(r);
    }
    return bound;
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
        if (size() == 1) _handles->rotationCenter().setVisible(false);
    } else {
        _handles->setMode(TransformHandleSet::MODE_SCALE);
    }
}

void ControlPointSelection::_pointGrabbed()
{
    hideTransformHandles();
    _dragging = true;
}

void ControlPointSelection::_pointDragged(Geom::Point const &old_pos, Geom::Point &new_pos,
                                          GdkEventMotion */*event*/)
{
    Geom::Point delta = new_pos - old_pos;
    for (iterator i = _points.begin(); i != _points.end(); ++i) {
        SelectableControlPoint *cur = i->first;
        cur->move(cur->position() + delta);
    }
    _handles->rotationCenter().move(_handles->rotationCenter().position() + delta);
    signal_update.emit();
}

void ControlPointSelection::_pointUngrabbed()
{
    _dragging = false;
    _grabbed_point = NULL;
    restoreTransformHandles();
    signal_commit.emit(COMMIT_MOUSE_MOVE);
}

bool ControlPointSelection::_pointClicked(SelectableControlPoint *p, GdkEventButton *event)
{
    // clicking a selected node should toggle the transform handles between rotate and scale mode,
    // if they are visible
    if (held_shift(*event)) return false;
    if (_handles_visible && p->selected()) {
        toggleTransformHandlesMode();
        return true;
    }
    return false;
}

void ControlPointSelection::_updateTransformHandles(bool preserve_center)
{
    if (_dragging) return;

    if (_handles_visible && size() > 1) {
        Geom::OptRect b = pointwiseBounds();
        _handles->setBounds(*b, preserve_center);
        _handles->setVisible(true);
    } else if (_one_node_handles && size() == 1) { // only one control point in selection
        SelectableControlPoint *p = begin()->first;
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
    unsigned num = 1 + consume_same_key_events(shortcut_key(event), 0);

    Geom::Point delta = dir * num; 
    if (held_shift(event)) delta *= 10;
    if (held_alt(event)) {
        delta /= _desktop->current_zoom();
    } else {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        double nudge = prefs->getDoubleLimited("/options/nudgedistance/value", 2, 0, 1000);
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

/** Rotates the selected points in the given direction according to the modifier state
 * from the supplied event.
 * @param event Key event to take modifier state from
 * @param dir   Direction of rotation (math convention: 1 = counterclockwise, -1 = clockwise)
 */
bool ControlPointSelection::_keyboardRotate(GdkEventKey const &event, int dir)
{
    if (empty()) return false;

    Geom::Point rc = _handles->rotationCenter();
    if (!_rot_radius) {
        Geom::Rect b = *(size() == 1 ? bounds() : pointwiseBounds());
        double maxlen = 0;
        for (unsigned i = 0; i < 4; ++i) {
            double len = (b.corner(i) - rc).length();
            if (len > maxlen) maxlen = len;
        }
        _rot_radius = maxlen;
    }

    double angle;
    if (held_alt(event)) {
        // Rotate by "one pixel". We interpret this as rotating by an angle that causes
        // the topmost point of a circle circumscribed about the selection's bounding box
        // to move on an arc 1 screen pixel long.
        angle = atan2(1.0 / _desktop->current_zoom(), *_rot_radius) * dir;
    } else {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        int snaps = prefs->getIntLimited("/options/rotationsnapsperpi/value", 12, 1, 1000);
        angle = M_PI * dir / snaps;
    }

    // translate to origin, rotate, translate back to original position
    Geom::Matrix m = Geom::Translate(-rc)
        * Geom::Rotate(angle) * Geom::Translate(rc);
    transform(m);
    signal_commit.emit(COMMIT_KEYBOARD_ROTATE);
    return true;
}


bool ControlPointSelection::_keyboardScale(GdkEventKey const &event, int dir)
{
    if (empty()) return false;

    // TODO should the saved rotation center or the current center be used?
    Geom::Rect bound = (size() == 1 ? *bounds() : *pointwiseBounds());
    double maxext = bound.maxExtent();
    if (Geom::are_near(maxext, 0)) return false;
    Geom::Point center = _handles->rotationCenter().position();

    double length_change;
    if (held_alt(event)) {
        // Scale by "one pixel". It means shrink/grow 1px for the larger dimension
        // of the bounding box.
        length_change = 1.0 / _desktop->current_zoom() * dir;
    } else {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        length_change = prefs->getDoubleLimited("/options/defaultscale/value", 2, 1, 1000);
        length_change *= dir;
    }
    double scale = (maxext + length_change) / maxext;
    
    Geom::Matrix m = Geom::Translate(-center) * Geom::Scale(scale) * Geom::Translate(center);
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

    Geom::Matrix m = Geom::Translate(-center) * scale_transform * Geom::Translate(center);
    transform(m);
    signal_commit.emit(d == Geom::X ? COMMIT_FLIP_X : COMMIT_FLIP_Y);
    return true;
}

void ControlPointSelection::_commitTransform(CommitEvent ce)
{
    _updateTransformHandles(true);
    signal_commit.emit(ce);
}

bool ControlPointSelection::event(GdkEvent *event)
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
        case GDK_Up:
        case GDK_KP_Up:
        case GDK_KP_8:
            return _keyboardMove(event->key, Geom::Point(0, 1));
        case GDK_Down:
        case GDK_KP_Down:
        case GDK_KP_2:
            return _keyboardMove(event->key, Geom::Point(0, -1));
        case GDK_Right:
        case GDK_KP_Right:
        case GDK_KP_6:
            return _keyboardMove(event->key, Geom::Point(1, 0));
        case GDK_Left:
        case GDK_KP_Left:
        case GDK_KP_4:
            return _keyboardMove(event->key, Geom::Point(-1, 0));

        // rotates
        case GDK_bracketleft:
            return _keyboardRotate(event->key, 1);
        case GDK_bracketright:
            return _keyboardRotate(event->key, -1);

        // scaling
        case GDK_less:
        case GDK_comma:
            return _keyboardScale(event->key, -1);
        case GDK_greater:
        case GDK_period:
            return _keyboardScale(event->key, 1);

        // TODO: skewing

        // flipping
        // NOTE: H is horizontal flip, while Shift+H switches transform handle mode!
        case GDK_h:
        case GDK_H:
            if (held_shift(event->key)) {
                toggleTransformHandlesMode();
                return true;
            }
            // any modifiers except shift should cause no action
            if (held_any_modifiers(event->key)) break;
            return _keyboardFlip(Geom::X);
        case GDK_v:
        case GDK_V:
            if (held_any_modifiers(event->key)) break;
            return _keyboardFlip(Geom::Y);
        default: break;
        }
        break;
    default: break;
    }
    return false;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
