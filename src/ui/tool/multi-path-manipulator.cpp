/** @file
 * Multi path manipulator - implementation
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <tr1/unordered_set>
#include <boost/shared_ptr.hpp>
#include <glib.h>
#include <glibmm/i18n.h>
#include "desktop.h"
#include "desktop-handles.h"
#include "document.h"
#include "live_effects/lpeobject.h"
#include "message-stack.h"
#include "preferences.h"
#include "sp-path.h"
#include "ui/tool/control-point-selection.h"
#include "ui/tool/event-utils.h"
#include "ui/tool/node.h"
#include "ui/tool/multi-path-manipulator.h"
#include "ui/tool/path-manipulator.h"

namespace std { using namespace tr1; }

namespace Inkscape {
namespace UI {

namespace {
typedef std::pair<NodeList::iterator, NodeList::iterator> IterPair;
typedef std::vector<IterPair> IterPairList;
typedef std::unordered_set<NodeList::iterator> IterSet;
typedef std::multimap<double, IterPair> DistanceMap;
typedef std::pair<double, IterPair> DistanceMapItem;

/** Find pairs of selected endnodes suitable for joining. */
void find_join_iterators(ControlPointSelection &sel, IterPairList &pairs)
{
    IterSet join_iters;
    DistanceMap dists;

    // find all endnodes in selection
    for (ControlPointSelection::iterator i = sel.begin(); i != sel.end(); ++i) {
        Node *node = dynamic_cast<Node*>(i->first);
        if (!node) continue;
        NodeList::iterator iter = NodeList::get_iterator(node);
        if (!iter.next() || !iter.prev()) join_iters.insert(iter);
    }

    if (join_iters.size() < 2) return;

    // Below we find the closest pairs. The algorithm is O(N^3).
    // We can go down to O(N^2 log N) by using O(N^2) memory, by putting all pairs
    // with their distances in a multimap (not worth it IMO).
    while (join_iters.size() >= 2) {
        double closest = DBL_MAX;
        IterPair closest_pair;
        for (IterSet::iterator i = join_iters.begin(); i != join_iters.end(); ++i) {
            for (IterSet::iterator j = join_iters.begin(); j != i; ++j) {
                double dist = Geom::distance(**i, **j);
                if (dist < closest) {
                    closest = dist;
                    closest_pair = std::make_pair(*i, *j);
                }
            }
        }
        pairs.push_back(closest_pair);
        join_iters.erase(closest_pair.first);
        join_iters.erase(closest_pair.second);
    }
}

/** After this function, first should be at the end of path and second at the beginnning.
 * @returns True if the nodes are in the same subpath */
bool prepare_join(IterPair &join_iters)
{
    if (&NodeList::get(join_iters.first) == &NodeList::get(join_iters.second)) {
        if (join_iters.first.next()) // if first is begin, swap the iterators
            std::swap(join_iters.first, join_iters.second);
        return true;
    }

    NodeList &sp_first = NodeList::get(join_iters.first);
    NodeList &sp_second = NodeList::get(join_iters.second);
    if (join_iters.first.next()) { // first is begin
        if (join_iters.second.next()) { // second is begin
            sp_first.reverse();
        } else { // second is end
            std::swap(join_iters.first, join_iters.second);
        }
    } else { // first is end
        if (join_iters.second.next()) { // second is begin
            // do nothing
        } else { // second is end
            sp_second.reverse();
        }
    }
    return false;
}
} // anonymous namespace


MultiPathManipulator::MultiPathManipulator(PathSharedData &data, sigc::connection &chg)
    : PointManipulator(data.node_data.desktop, *data.node_data.selection)
    , _path_data(data)
    , _changed(chg)
{
    _selection.signal_commit.connect(
        sigc::mem_fun(*this, &MultiPathManipulator::_commit));
    _selection.signal_point_changed.connect(
        sigc::hide( sigc::hide(
            signal_coords_changed.make_slot())));
}

MultiPathManipulator::~MultiPathManipulator()
{
    _mmap.clear();
}

/** Remove empty manipulators. */
void MultiPathManipulator::cleanup()
{
    for (MapType::iterator i = _mmap.begin(); i != _mmap.end(); ) {
        if (i->second->empty()) _mmap.erase(i++);
        else ++i;
    }
}

/** @brief Change the set of items to edit.
 *
 * This method attempts to preserve as much of the state as possible. */
void MultiPathManipulator::setItems(std::set<ShapeRecord> const &s)
{
    std::set<ShapeRecord> shapes(s);

    // iterate over currently edited items, modifying / removing them as necessary
    for (MapType::iterator i = _mmap.begin(); i != _mmap.end();) {
        std::set<ShapeRecord>::iterator si = shapes.find(i->first);
        if (si == shapes.end()) {
            // This item is no longer supposed to be edited - remove its manipulator
            _mmap.erase(i++);
        } else {
            ShapeRecord const &sr = i->first;
            ShapeRecord const &sr_new = *si;
            // if the shape record differs, replace the key only and modify other values
            if (sr.edit_transform != sr_new.edit_transform ||
                sr.role != sr_new.role)
            {
                boost::shared_ptr<PathManipulator> hold(i->second);
                if (sr.edit_transform != sr_new.edit_transform)
                    hold->setControlsTransform(sr_new.edit_transform);
                if (sr.role != sr_new.role) {
                    //hold->setOutlineColor(_getOutlineColor(sr_new.role));
                }
                _mmap.erase(sr);
                _mmap.insert(std::make_pair(sr_new, hold));
            }
            shapes.erase(si); // remove the processed record
            ++i;
        }
    }

    // add newly selected items
    for (std::set<ShapeRecord>::iterator i = shapes.begin(); i != shapes.end(); ++i) {
        ShapeRecord const &r = *i;
        if (!SP_IS_PATH(r.item) && !IS_LIVEPATHEFFECT(r.item)) continue;
        boost::shared_ptr<PathManipulator> newpm(new PathManipulator(*this, (SPPath*) r.item,
            r.edit_transform, _getOutlineColor(r.role), r.lpe_key));
        newpm->showHandles(_show_handles);
        // always show outlines for clips and masks
        newpm->showOutline(_show_outline || r.role != SHAPE_ROLE_NORMAL);
        newpm->showPathDirection(_show_path_direction);
        newpm->setLiveOutline(_live_outline);
        newpm->setLiveObjects(_live_objects);
        _mmap.insert(std::make_pair(r, newpm));
    }
}

void MultiPathManipulator::selectSubpaths()
{
    if (_selection.empty()) {
        _selection.selectAll();
    } else {
        invokeForAll(&PathManipulator::selectSubpaths);
    }
}

void MultiPathManipulator::shiftSelection(int dir)
{
    invokeForAll(&PathManipulator::shiftSelection, dir);
}

void MultiPathManipulator::invertSelectionInSubpaths()
{
    invokeForAll(&PathManipulator::invertSelectionInSubpaths);
}

void MultiPathManipulator::setNodeType(NodeType type)
{
    if (_selection.empty()) return;
    for (ControlPointSelection::iterator i = _selection.begin(); i != _selection.end(); ++i) {
        Node *node = dynamic_cast<Node*>(i->first);
        if (node) node->setType(type);
    }
    _done(_("Change node type"));
}

void MultiPathManipulator::setSegmentType(SegmentType type)
{
    if (_selection.empty()) return;
    invokeForAll(&PathManipulator::setSegmentType, type);
    if (type == SEGMENT_STRAIGHT) {
        _done(_("Straighten segments"));
    } else {
        _done(_("Make segments curves"));
    }
}

void MultiPathManipulator::insertNodes()
{
    invokeForAll(&PathManipulator::insertNodes);
    _done(_("Add nodes"));
}

void MultiPathManipulator::joinNodes()
{
    invokeForAll(&PathManipulator::hideDragPoint);
    // Node join has two parts. In the first one we join two subpaths by fusing endpoints
    // into one. In the second we fuse nodes in each subpath.
    IterPairList joins;
    NodeList::iterator preserve_pos;
    Node *mouseover_node = dynamic_cast<Node*>(ControlPoint::mouseovered_point);
    if (mouseover_node) {
        preserve_pos = NodeList::get_iterator(mouseover_node);
    }
    find_join_iterators(_selection, joins);

    for (IterPairList::iterator i = joins.begin(); i != joins.end(); ++i) {
        bool same_path = prepare_join(*i);
        bool mouseover = true;
        NodeList &sp_first = NodeList::get(i->first);
        NodeList &sp_second = NodeList::get(i->second);
        i->first->setType(NODE_CUSP, false);

        Geom::Point joined_pos, pos_front, pos_back;
        pos_front = *i->second->front();
        pos_back = *i->first->back();
        if (i->first == preserve_pos) {
            joined_pos = *i->first;
        } else if (i->second == preserve_pos) {
            joined_pos = *i->second;
        } else {
            joined_pos = Geom::middle_point(pos_back, pos_front);
            mouseover = false;
        }

        // if the handles aren't degenerate, don't move them
        i->first->move(joined_pos);
        Node *joined_node = i->first.ptr();
        if (!i->second->front()->isDegenerate()) {
            joined_node->front()->setPosition(pos_front);
        }
        if (!i->first->back()->isDegenerate()) {
            joined_node->back()->setPosition(pos_back);
        }
        if (mouseover) {
            // Second node could be mouseovered, but it will be deleted, so we must change
            // the preserve_pos iterator to the first node.
            preserve_pos = i->first;
        }
        sp_second.erase(i->second);

        if (same_path) {
            sp_first.setClosed(true);
        } else {
            sp_first.splice(sp_first.end(), sp_second);
            sp_second.kill();
        }
        _selection.insert(i->first.ptr());
    }

    if (joins.empty()) {
        // Second part replaces contiguous selections of nodes with single nodes
        invokeForAll(&PathManipulator::weldNodes, preserve_pos);
    }

    _doneWithCleanup(_("Join nodes"));
}

void MultiPathManipulator::breakNodes()
{
    if (_selection.empty()) return;
    invokeForAll(&PathManipulator::breakNodes);
    _done(_("Break nodes"));
}

void MultiPathManipulator::deleteNodes(bool keep_shape)
{
    if (_selection.empty()) return;
    invokeForAll(&PathManipulator::deleteNodes, keep_shape);
    _doneWithCleanup(_("Delete nodes"));
}

/** Join selected endpoints to create segments. */
void MultiPathManipulator::joinSegments()
{
    IterPairList joins;
    find_join_iterators(_selection, joins);

    for (IterPairList::iterator i = joins.begin(); i != joins.end(); ++i) {
        bool same_path = prepare_join(*i);
        NodeList &sp_first = NodeList::get(i->first);
        NodeList &sp_second = NodeList::get(i->second);
        i->first->setType(NODE_CUSP, false);
        i->second->setType(NODE_CUSP, false);
        if (same_path) {
            sp_first.setClosed(true);
        } else {
            sp_first.splice(sp_first.end(), sp_second);
            sp_second.kill();
        }
    }

    if (joins.empty()) {
        invokeForAll(&PathManipulator::weldSegments);
    }
    _doneWithCleanup("Join segments");
}

void MultiPathManipulator::deleteSegments()
{
    if (_selection.empty()) return;
    invokeForAll(&PathManipulator::deleteSegments);
    _doneWithCleanup("Delete segments");
}

void MultiPathManipulator::alignNodes(Geom::Dim2 d)
{
    _selection.align(d);
    if (d == Geom::X) {
        _done("Align nodes to a horizontal line");
    } else {
        _done("Align nodes to a vertical line");
    }
}

void MultiPathManipulator::distributeNodes(Geom::Dim2 d)
{
    _selection.distribute(d);
    if (d == Geom::X) {
        _done("Distrubute nodes horizontally");
    } else {
        _done("Distribute nodes vertically");
    }
}

void MultiPathManipulator::reverseSubpaths()
{
    if (_selection.empty()) {
        invokeForAll(&PathManipulator::reverseSubpaths, false);
        _done("Reverse subpaths");
    } else {
        invokeForAll(&PathManipulator::reverseSubpaths, true);
        _done("Reverse selected subpaths");
    }
}

void MultiPathManipulator::move(Geom::Point const &delta)
{
    _selection.transform(Geom::Translate(delta));
    _done("Move nodes");
}

void MultiPathManipulator::showOutline(bool show)
{
    for (MapType::iterator i = _mmap.begin(); i != _mmap.end(); ++i) {
        // always show outlines for clipping paths and masks
        i->second->showOutline(show || i->first.role != SHAPE_ROLE_NORMAL);
    }
    _show_outline = show;
}

void MultiPathManipulator::showHandles(bool show)
{
    invokeForAll(&PathManipulator::showHandles, show);
    _show_handles = show;
}

void MultiPathManipulator::showPathDirection(bool show)
{
    invokeForAll(&PathManipulator::showPathDirection, show);
    _show_path_direction = show;
}

/** @brief Set live outline update status
 * When set to true, outline will be updated continuously when dragging
 * or transforming nodes. Otherwise it will only update when changes are committed
 * to XML. */
void MultiPathManipulator::setLiveOutline(bool set)
{
    invokeForAll(&PathManipulator::setLiveOutline, set);
    _live_outline = set;
}

/** @brief Set live object update status
 * When set to true, objects will be updated continuously when dragging
 * or transforming nodes. Otherwise they will only update when changes are committed
 * to XML. */
void MultiPathManipulator::setLiveObjects(bool set)
{
    invokeForAll(&PathManipulator::setLiveObjects, set);
    _live_objects = set;
}

void MultiPathManipulator::updateOutlineColors()
{
    //for (MapType::iterator i = _mmap.begin(); i != _mmap.end(); ++i) {
    //    i->second->setOutlineColor(_getOutlineColor(i->first.role));
    //}
}

bool MultiPathManipulator::event(GdkEvent *event)
{
    switch (event->type) {
    case GDK_KEY_PRESS:
        switch (shortcut_key(event->key)) {
        case GDK_Insert:
        case GDK_KP_Insert:
            // Insert - insert nodes in the middle of selected segments
            insertNodes();
            return true;
        case GDK_i:
        case GDK_I:
            if (held_only_shift(event->key)) {
                // Shift+I - insert nodes (alternate keybinding for Mac keyboards
                //           that don't have the Insert key)
                insertNodes();
                return true;
            }
            break;
        case GDK_j:
        case GDK_J:
            if (held_only_shift(event->key)) {
                // Shift+J - join nodes
                joinNodes();
                return true;
            }
            if (held_only_alt(event->key)) {
                // Alt+J - join segments
                joinSegments();
                return true;
            }
            break;
        case GDK_b:
        case GDK_B:
            if (held_only_shift(event->key)) {
                // Shift+B - break nodes
                breakNodes();
                return true;
            }
            break;
        case GDK_Delete:
        case GDK_KP_Delete:
        case GDK_BackSpace:
            if (held_shift(event->key)) break;
            if (held_alt(event->key)) {
                // Alt+Delete - delete segments
                deleteSegments();
            } else {
                // Control+Delete - delete nodes
                // Delete - delete nodes preserving shape
                deleteNodes(!held_control(event->key));
            }
            return true;
        case GDK_c:
        case GDK_C:
            if (held_only_shift(event->key)) {
                // Shift+C - make nodes cusp
                setNodeType(NODE_CUSP);
                return true;
            }
            break;
        case GDK_s:
        case GDK_S:
            if (held_only_shift(event->key)) {
                // Shift+S - make nodes smooth
                setNodeType(NODE_SMOOTH);
                return true;
            }
            break;
        case GDK_a:
        case GDK_A:
            if (held_only_shift(event->key)) {
                // Shift+A - make nodes auto-smooth
                setNodeType(NODE_AUTO);
                return true;
            }
            break;
        case GDK_y:
        case GDK_Y:
            if (held_only_shift(event->key)) {
                // Shift+Y - make nodes symmetric
                setNodeType(NODE_SYMMETRIC);
                return true;
            }
            break;
        case GDK_r:
        case GDK_R:
            if (held_only_shift(event->key)) {
                // Shift+R - reverse subpaths
                reverseSubpaths();
            }
            break;
        default:
            break;
        }
        break;
    default: break;
    }

    for (MapType::iterator i = _mmap.begin(); i != _mmap.end(); ++i) {
        if (i->second->event(event)) return true;
    }
    return false;
}

/** Commit changes to XML and add undo stack entry based on the action that was done. Invoked
 * by sub-manipulators, for example TransformHandleSet and ControlPointSelection. */
void MultiPathManipulator::_commit(CommitEvent cps)
{
    gchar const *reason = NULL;
    gchar const *key = NULL;
    switch(cps) {
    case COMMIT_MOUSE_MOVE:
        reason = _("Move nodes");
        break;
    case COMMIT_KEYBOARD_MOVE_X:
        reason = _("Move nodes horizontally");
        key = "node:move:x";
        break;
    case COMMIT_KEYBOARD_MOVE_Y:
        reason = _("Move nodes vertically");
        key = "node:move:y";
        break;
    case COMMIT_MOUSE_ROTATE:
        reason = _("Rotate nodes");
        break;
    case COMMIT_KEYBOARD_ROTATE:
        reason = _("Rotate nodes");
        key = "node:rotate";
        break;
    case COMMIT_MOUSE_SCALE_UNIFORM:
        reason = _("Scale nodes uniformly");
        break;
    case COMMIT_MOUSE_SCALE:
        reason = _("Scale nodes");
        break;
    case COMMIT_KEYBOARD_SCALE_UNIFORM:
        reason = _("Scale nodes uniformly");
        key = "node:scale:uniform";
        break;
    case COMMIT_KEYBOARD_SCALE_X:
        reason = _("Scale nodes horizontally");
        key = "node:scale:x";
        break;
    case COMMIT_KEYBOARD_SCALE_Y:
        reason = _("Scale nodes vertically");
        key = "node:scale:y";
        break;
    case COMMIT_FLIP_X:
        reason = _("Flip nodes horizontally");
        break;
    case COMMIT_FLIP_Y:
        reason = _("Flip nodes vertically");
        break;
    default: return;
    }
    
    _selection.signal_update.emit();
    invokeForAll(&PathManipulator::writeXML);
    if (key) {
        sp_document_maybe_done(sp_desktop_document(_desktop), key, SP_VERB_CONTEXT_NODE, reason);
    } else {
        sp_document_done(sp_desktop_document(_desktop), SP_VERB_CONTEXT_NODE, reason);
    }
    signal_coords_changed.emit();
}

/** Commits changes to XML and adds undo stack entry. */
void MultiPathManipulator::_done(gchar const *reason) {
    invokeForAll(&PathManipulator::update);
    invokeForAll(&PathManipulator::writeXML);
    sp_document_done(sp_desktop_document(_desktop), SP_VERB_CONTEXT_NODE, reason);
    signal_coords_changed.emit();
}

/** Commits changes to XML, adds undo stack entry and removes empty manipulators. */
void MultiPathManipulator::_doneWithCleanup(gchar const *reason) {
    _changed.block();
    _done(reason);
    cleanup();
    _changed.unblock();
}

/** Get an outline color based on the shape's role (normal, mask, LPE parameter, etc.). */
guint32 MultiPathManipulator::_getOutlineColor(ShapeRole role)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    switch(role) {
    case SHAPE_ROLE_CLIPPING_PATH:
        return prefs->getColor("/tools/nodes/clipping_path_color", 0x00ff00ff);
    case SHAPE_ROLE_MASK:
        return prefs->getColor("/tools/nodes/mask_color", 0x0000ffff);
    case SHAPE_ROLE_LPE_PARAM:
        return prefs->getColor("/tools/nodes/lpe_param_color", 0x009000ff);
    case SHAPE_ROLE_NORMAL:
    default:
        return prefs->getColor("/tools/nodes/outline_color", 0xff0000ff);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
