/** @file
 * Path manipulator - implementation
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string>
#include <sstream>
#include <deque>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <2geom/bezier-curve.h>
#include <2geom/bezier-utils.h>
#include <2geom/svg-path.h>
#include <glibmm.h>
#include <glibmm/i18n.h>
#include "ui/tool/path-manipulator.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "display/sp-canvas.h"
#include "display/sp-canvas-util.h"
#include "display/curve.h"
#include "display/canvas-bpath.h"
#include "document.h"
#include "live_effects/effect.h"
#include "live_effects/lpeobject.h"
#include "live_effects/parameter/path.h"
#include "sp-path.h"
#include "helper/geom.h"
#include "preferences.h"
#include "style.h"
#include "ui/tool/control-point-selection.h"
#include "ui/tool/curve-drag-point.h"
#include "ui/tool/event-utils.h"
#include "ui/tool/multi-path-manipulator.h"
#include "xml/node.h"
#include "xml/node-observer.h"

namespace Inkscape {
namespace UI {

namespace {
/// Types of path changes that we must react to.
enum PathChange {
    PATH_CHANGE_D,
    PATH_CHANGE_TRANSFORM
};

} // anonymous namespace

/**
 * Notifies the path manipulator when something changes the path being edited
 * (e.g. undo / redo)
 */
class PathManipulatorObserver : public Inkscape::XML::NodeObserver {
public:
    PathManipulatorObserver(PathManipulator *p) : _pm(p), _blocked(false) {}
    virtual void notifyAttributeChanged(Inkscape::XML::Node &, GQuark attr,
        Util::ptr_shared<char>, Util::ptr_shared<char>)
    {
        // do nothing if blocked
        if (_blocked) return;

        GQuark path_d = g_quark_from_static_string("d");
        GQuark path_transform = g_quark_from_static_string("transform");
        GQuark lpe_quark = _pm->_lpe_key.empty() ? 0 : g_quark_from_string(_pm->_lpe_key.data());

        // only react to "d" (path data) and "transform" attribute changes
        if (attr == lpe_quark || attr == path_d) {
            _pm->_externalChange(PATH_CHANGE_D);
        } else if (attr == path_transform) {
            _pm->_externalChange(PATH_CHANGE_TRANSFORM);
        }
    }
    void block() { _blocked = true; }
    void unblock() { _blocked = false; }
private:
    PathManipulator *_pm;
    bool _blocked;
};

void build_segment(Geom::PathBuilder &, Node *, Node *);

PathManipulator::PathManipulator(MultiPathManipulator &mpm, SPPath *path,
        Geom::Matrix const &et, guint32 outline_color, Glib::ustring lpe_key)
    : PointManipulator(mpm._path_data.node_data.desktop, *mpm._path_data.node_data.selection)
    , _subpaths(*this)
    , _multi_path_manipulator(mpm)
    , _path(path)
    , _spcurve(NULL)
    , _dragpoint(new CurveDragPoint(*this))
    , _observer(new PathManipulatorObserver(this))
    , _edit_transform(et)
    , _num_selected(0)
    , _show_handles(true)
    , _show_outline(false)
    , _lpe_key(lpe_key)
{
    if (_lpe_key.empty()) {
        _i2d_transform = sp_item_i2d_affine(SP_ITEM(path));
    } else {
        _i2d_transform = Geom::identity();
    }
    _d2i_transform = _i2d_transform.inverse();
    _dragpoint->setVisible(false);

    _getGeometry();

    _outline = sp_canvas_bpath_new(_multi_path_manipulator._path_data.outline_group, NULL);
    sp_canvas_item_hide(_outline);
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(_outline), outline_color, 1.0,
        SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
    sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(_outline), 0, SP_WIND_RULE_NONZERO);

    _subpaths.signal_insert_node.connect(
        sigc::mem_fun(*this, &PathManipulator::_attachNodeHandlers));
    // NOTE: signal_remove_node is called just before destruction. Nodes are trackable,
    // so removing the signals manually is not necessary.
    /*_subpaths.signal_remove_node.connect(
        sigc::mem_fun(*this, &PathManipulator::_removeNodeHandlers));*/

    _selection.signal_update.connect(
        sigc::mem_fun(*this, &PathManipulator::update));
    _selection.signal_point_changed.connect(
        sigc::mem_fun(*this, &PathManipulator::_selectionChanged));
    _dragpoint->signal_update.connect(
        sigc::mem_fun(*this, &PathManipulator::update));
    _desktop->signal_zoom_changed.connect(
        sigc::hide( sigc::mem_fun(*this, &PathManipulator::_updateOutlineOnZoomChange)));

    _createControlPointsFromGeometry();

    _path->repr->addObserver(*_observer);
}

PathManipulator::~PathManipulator()
{
    delete _dragpoint;
    if (_path) _path->repr->removeObserver(*_observer);
    delete _observer;
    gtk_object_destroy(_outline);
    if (_spcurve) _spcurve->unref();
    clear();
}

/** Handle motion events to update the position of the curve drag point. */
bool PathManipulator::event(GdkEvent *event)
{
    if (empty()) return false;

    switch (event->type)
    {
    case GDK_MOTION_NOTIFY:
        _updateDragPoint(event_point(event->motion));
        break;
    default: break;
    }
    return false;
}

/** Check whether the manipulator has any nodes. */
bool PathManipulator::empty() {
    return !_path || _subpaths.empty();
}

/** Update the display and the outline of the path. */
void PathManipulator::update()
{
    _createGeometryFromControlPoints();
}

/** Store the changes to the path in XML. */
void PathManipulator::writeXML()
{
    if (!_path) return;
    _observer->block();
    if (!empty()) {
        SP_OBJECT(_path)->updateRepr();
        _getXMLNode()->setAttribute(_nodetypesKey().data(), _createTypeString().data());
    } else {
        // this manipulator will have to be destroyed right after this call
        _getXMLNode()->removeObserver(*_observer);
        sp_object_ref(_path);
        _path->deleteObject(true, true);
        sp_object_unref(_path);
        _path = 0;
    }
    _observer->unblock();
}

/** Remove all nodes from the path. */
void PathManipulator::clear()
{
    // no longer necessary since nodes remove themselves from selection on destruction
    //_removeNodesFromSelection();
    _subpaths.clear();
}

/** Select all nodes in subpaths that have something selected. */
void PathManipulator::selectSubpaths()
{
    for (std::list<SubpathPtr>::iterator i = _subpaths.begin(); i != _subpaths.end(); ++i) {
        NodeList::iterator sp_start = (*i)->begin(), sp_end = (*i)->end();
        for (NodeList::iterator j = sp_start; j != sp_end; ++j) {
            if (j->selected()) {
                // if at least one of the nodes from this subpath is selected,
                // select all nodes from this subpath
                for (NodeList::iterator ins = sp_start; ins != sp_end; ++ins)
                    _selection.insert(ins.ptr());
                continue;
            }
        }
    }
}

/** Move the selection forward or backward by one node in each subpath, based on the sign
 * of the parameter. */
void PathManipulator::shiftSelection(int dir)
{
    if (dir == 0) return;
    if (_num_selected == 0) {
        // select the first node of the path.
        SubpathList::iterator s = _subpaths.begin();
        if (s == _subpaths.end()) return;
        NodeList::iterator n = (*s)->begin();
        if (n != (*s)->end())
            _selection.insert(n.ptr());
        return;
    }
    // We cannot do any tricks here, like iterating in different directions based on
    // the sign and only setting the selection of nodes behind us, because it would break
    // for closed paths.
    for (SubpathList::iterator i = _subpaths.begin(); i != _subpaths.end(); ++i) {
        std::deque<bool> sels; // I hope this is specialized for bools!
        unsigned num = 0;
        
        for (NodeList::iterator j = (*i)->begin(); j != (*i)->end(); ++j) {
            sels.push_back(j->selected());
            _selection.erase(j.ptr());
            ++num;
        }
        if (num == 0) continue; // should never happen! zero-node subpaths are not allowed

        num = 0;
        // In closed subpath, shift the selection cyclically. In an open one,
        // let the selection 'slide into nothing' at ends.
        if (dir > 0) {
            if ((*i)->closed()) {
                bool last = sels.back();
                sels.pop_back();
                sels.push_front(last);
            } else {
                sels.push_front(false);
            }
        } else {
            if ((*i)->closed()) {
                bool first = sels.front();
                sels.pop_front();
                sels.push_back(first);
            } else {
                sels.push_back(false);
                num = 1;
            }
        }

        for (NodeList::iterator j = (*i)->begin(); j != (*i)->end(); ++j) {
            if (sels[num]) _selection.insert(j.ptr());
            ++num;
        }
    }
}

/** Invert selection in the selected subpaths. */
void PathManipulator::invertSelectionInSubpaths()
{
    for (SubpathList::iterator i = _subpaths.begin(); i != _subpaths.end(); ++i) {
        for (NodeList::iterator j = (*i)->begin(); j != (*i)->end(); ++j) {
            if (j->selected()) {
                // found selected node - invert selection in this subpath
                for (NodeList::iterator k = (*i)->begin(); k != (*i)->end(); ++k) {
                    if (k->selected()) _selection.erase(k.ptr());
                    else _selection.insert(k.ptr());
                }
                // next subpath
                break;
            }
        }
    }
}

/** Insert a new node in the middle of each selected segment. */
void PathManipulator::insertNodes()
{
    if (_num_selected < 2) return;

    for (SubpathList::iterator i = _subpaths.begin(); i != _subpaths.end(); ++i) {
        for (NodeList::iterator j = (*i)->begin(); j != (*i)->end(); ++j) {
            NodeList::iterator k = j.next();
            if (k && j->selected() && k->selected()) {
                j = subdivideSegment(j, 0.5);
                _selection.insert(j.ptr());
            }
        }
    }
}

/** Replace contiguous selections of nodes in each subpath with one node. */
void PathManipulator::weldNodes(NodeList::iterator preserve_pos)
{
    if (_num_selected < 2) return;
    hideDragPoint();

    bool pos_valid = preserve_pos;
    for (SubpathList::iterator i = _subpaths.begin(); i != _subpaths.end(); ++i) {
        SubpathPtr sp = *i;
        unsigned num_selected = 0, num_unselected = 0;
        for (NodeList::iterator j = sp->begin(); j != sp->end(); ++j) {
            if (j->selected()) ++num_selected;
            else ++num_unselected;
        }
        if (num_selected < 2) continue;
        if (num_unselected == 0) {
            // if all nodes in a subpath are selected, the operation doesn't make much sense
            continue;
        }

        // Start from unselected node in closed paths, so that we don't start in the middle
        // of a selection
        NodeList::iterator sel_beg = sp->begin(), sel_end;
        if (sp->closed()) {
            while (sel_beg->selected()) ++sel_beg;
        }

        // Work loop
        while (num_selected > 0) {
            // Find selected node
            while (sel_beg && !sel_beg->selected()) sel_beg = sel_beg.next();
            if (!sel_beg) throw std::logic_error("Join nodes: end of open path reached, "
                "but there are still nodes to process!");

            // note: this is initialized to zero, because the loop below counts sel_beg as well
            // the loop conditions are simpler that way
            unsigned num_points = 0;
            bool use_pos = false;
            Geom::Point back_pos, front_pos;
            back_pos = *sel_beg->back();

            for (sel_end = sel_beg; sel_end && sel_end->selected(); sel_end = sel_end.next()) {
                ++num_points;
                front_pos = *sel_end->front();
                if (pos_valid && sel_end == preserve_pos) use_pos = true;
            }
            if (num_points > 1) {
                Geom::Point joined_pos;
                if (use_pos) {
                    joined_pos = preserve_pos->position();
                    pos_valid = false;
                } else {
                    joined_pos = Geom::middle_point(back_pos, front_pos);
                }
                sel_beg->setType(NODE_CUSP, false);
                sel_beg->move(joined_pos);
                // do not move handles if they aren't degenerate
                if (!sel_beg->back()->isDegenerate()) {
                    sel_beg->back()->setPosition(back_pos);
                }
                if (!sel_end.prev()->front()->isDegenerate()) {
                    sel_beg->front()->setPosition(front_pos);
                }
                sel_beg = sel_beg.next();
                while (sel_beg != sel_end) {
                    NodeList::iterator next = sel_beg.next();
                    sp->erase(sel_beg);
                    sel_beg = next;
                    --num_selected;
                }
            }
            --num_selected; // for the joined node or single selected node
        }
    }
}

/** Remove nodes in the middle of selected segments. */
void PathManipulator::weldSegments()
{
    if (_num_selected < 2) return;
    hideDragPoint();

    for (SubpathList::iterator i = _subpaths.begin(); i != _subpaths.end(); ++i) {
        SubpathPtr sp = *i;
        unsigned num_selected = 0, num_unselected = 0;
        for (NodeList::iterator j = sp->begin(); j != sp->end(); ++j) {
            if (j->selected()) ++num_selected;
            else ++num_unselected;
        }
        if (num_selected < 3) continue;
        if (num_unselected == 0 && sp->closed()) {
            // if all nodes in a closed subpath are selected, the operation doesn't make much sense
            continue;
        }

        // Start from unselected node in closed paths, so that we don't start in the middle
        // of a selection
        NodeList::iterator sel_beg = sp->begin(), sel_end;
        if (sp->closed()) {
            while (sel_beg->selected()) ++sel_beg;
        }

        // Work loop
        while (num_selected > 0) {
            // Find selected node
            while (sel_beg && !sel_beg->selected()) sel_beg = sel_beg.next();
            if (!sel_beg) throw std::logic_error("Join nodes: end of open path reached, "
                "but there are still nodes to process!");

            // note: this is initialized to zero, because the loop below counts sel_beg as well
            // the loop conditions are simpler that way
            unsigned num_points = 0;

            // find the end of selected segment
            for (sel_end = sel_beg; sel_end && sel_end->selected(); sel_end = sel_end.next()) {
                ++num_points;
            }
            if (num_points > 2) {
                // remove nodes in the middle
                sel_beg = sel_beg.next();
                while (sel_beg != sel_end.prev()) {
                    NodeList::iterator next = sel_beg.next();
                    sp->erase(sel_beg);
                    sel_beg = next;
                }
                sel_beg = sel_end;
            }
            num_selected -= num_points;
        }
    }
}

/** Break the subpath at selected nodes. It also works for single node closed paths. */
void PathManipulator::breakNodes()
{
    for (SubpathList::iterator i = _subpaths.begin(); i != _subpaths.end(); ++i) {
        SubpathPtr sp = *i;
        NodeList::iterator cur = sp->begin(), end = sp->end();
        if (!sp->closed()) {
            // Each open path must have at least two nodes so no checks are required.
            // For 2-node open paths, cur == end
            ++cur;
            --end;
        }
        for (; cur != end; ++cur) {
            if (!cur->selected()) continue;
            SubpathPtr ins;
            bool becomes_open = false;

            if (sp->closed()) {
                // Move the node to break at to the beginning of path
                if (cur != sp->begin())
                    sp->splice(sp->begin(), *sp, cur, sp->end());
                sp->setClosed(false);
                ins = sp;
                becomes_open = true;
            } else {
                SubpathPtr new_sp(new NodeList(_subpaths));
                new_sp->splice(new_sp->end(), *sp, sp->begin(), cur);
                _subpaths.insert(i, new_sp);
                ins = new_sp;
            }

            Node *n = new Node(_multi_path_manipulator._path_data.node_data, cur->position());
            ins->insert(ins->end(), n);
            cur->setType(NODE_CUSP, false);
            n->back()->setRelativePos(cur->back()->relativePos());
            cur->back()->retract();
            n->sink();

            if (becomes_open) {
                cur = sp->begin(); // this will be increased to ++sp->begin()
                end = --sp->end();
            }
        }
    }
}

/** Delete selected nodes in the path, optionally substituting deleted segments with bezier curves
 * in a way that attempts to preserve the original shape of the curve. */
void PathManipulator::deleteNodes(bool keep_shape)
{
    if (_num_selected == 0) return;
    hideDragPoint();

    for (SubpathList::iterator i = _subpaths.begin(); i != _subpaths.end();) {
        SubpathPtr sp = *i;

        // If there are less than 2 unselected nodes in an open subpath or no unselected nodes
        // in a closed one, delete entire subpath.
        unsigned num_unselected = 0, num_selected = 0;
        for (NodeList::iterator j = sp->begin(); j != sp->end(); ++j) {
            if (j->selected()) ++num_selected;
            else ++num_unselected;
        }
        if (num_selected == 0) {
            ++i;
            continue;
        }
        if (sp->closed() ? (num_unselected < 1) : (num_unselected < 2)) {
            _subpaths.erase(i++);
            continue;
        }

        // In closed paths, start from an unselected node - otherwise we might start in the middle
        // of a selected stretch and the resulting bezier fit would be suboptimal
        NodeList::iterator sel_beg = sp->begin(), sel_end;
        if (sp->closed()) {
            while (sel_beg->selected()) ++sel_beg;
        }
        sel_end = sel_beg;
        
        while (num_selected > 0) {
            while (!sel_beg->selected()) {
                sel_beg = sel_beg.next();
            }
            sel_end = sel_beg;

            while (sel_end && sel_end->selected()) {
                sel_end = sel_end.next();
            }
            
            num_selected -= _deleteStretch(sel_beg, sel_end, keep_shape);
        }
        ++i;
    }
}

/** @brief Delete nodes between the two iterators.
 * The given range can cross the beginning of the subpath in closed subpaths.
 * @param start      Beginning of the range to delete
 * @param end        End of the range
 * @param keep_shape Whether to fit the handles at surrounding nodes to approximate
 *                   the shape before deletion
 * @return Number of deleted nodes */
unsigned PathManipulator::_deleteStretch(NodeList::iterator start, NodeList::iterator end, bool keep_shape)
{
    unsigned const samples_per_segment = 10;
    double const t_step = 1.0 / samples_per_segment;

    unsigned del_len = 0;
    for (NodeList::iterator i = start; i != end; i = i.next()) {
        ++del_len;
    }
    if (del_len == 0) return 0;

    // set surrounding node types to cusp if:
    // 1. keep_shape is on, or
    // 2. we are deleting at the end or beginning of an open path
    if ((keep_shape || !end) && start.prev()) start.prev()->setType(NODE_CUSP, false);
    if ((keep_shape || !start.prev()) && end) end->setType(NODE_CUSP, false);

    if (keep_shape && start.prev() && end) {
        unsigned num_samples = (del_len + 1) * samples_per_segment + 1;
        Geom::Point *bezier_data = new Geom::Point[num_samples];
        Geom::Point result[4];
        unsigned seg = 0;

        for (NodeList::iterator cur = start.prev(); cur != end; cur = cur.next()) {
            Geom::CubicBezier bc(*cur, *cur->front(), *cur.next(), *cur.next()->back());
            for (unsigned s = 0; s < samples_per_segment; ++s) {
                bezier_data[seg * samples_per_segment + s] = bc.pointAt(t_step * s);
            }
            ++seg;
        }
        // Fill last point
        bezier_data[num_samples - 1] = end->position();
        // Compute replacement bezier curve
        // TODO the fitting algorithm sucks - rewrite it to be awesome
        bezier_fit_cubic(result, bezier_data, num_samples, 0.5);
        delete[] bezier_data;

        start.prev()->front()->setPosition(result[1]);
        end->back()->setPosition(result[2]);
    }

    // We can't use nl->erase(start, end), because it would break when the stretch
    // crosses the beginning of a closed subpath
    NodeList *nl = start->list();
    while (start != end) {
        NodeList::iterator next = start.next();
        nl->erase(start);
        start = next;
    }

    return del_len;
}

/** Removes selected segments */
void PathManipulator::deleteSegments()
{
    if (_num_selected == 0) return;
    hideDragPoint();

    for (SubpathList::iterator i = _subpaths.begin(); i != _subpaths.end();) {
        SubpathPtr sp = *i;
        bool has_unselected = false;
        unsigned num_selected = 0;
        for (NodeList::iterator j = sp->begin(); j != sp->end(); ++j) {
            if (j->selected()) {
                ++num_selected;
            } else {
                has_unselected = true;
            }
        }
        if (!has_unselected) {
            _subpaths.erase(i++);
            continue;
        }

        NodeList::iterator sel_beg = sp->begin();
        if (sp->closed()) {
            while (sel_beg && sel_beg->selected()) ++sel_beg;
        }
        while (num_selected > 0) {
            if (!sel_beg->selected()) {
                sel_beg = sel_beg.next();
                continue;
            }
            NodeList::iterator sel_end = sel_beg;
            unsigned num_points = 0;
            while (sel_end && sel_end->selected()) {
                sel_end = sel_end.next();
                ++num_points;
            }
            if (num_points >= 2) {
                // Retract end handles
                sel_end.prev()->setType(NODE_CUSP, false);
                sel_end.prev()->back()->retract();
                sel_beg->setType(NODE_CUSP, false);
                sel_beg->front()->retract();
                if (sp->closed()) {
                    // In closed paths, relocate the beginning of the path to the last selected
                    // node and then unclose it. Remove the nodes from the first selected node
                    // to the new end of path.
                    if (sel_end.prev() != sp->begin())
                        sp->splice(sp->begin(), *sp, sel_end.prev(), sp->end());
                    sp->setClosed(false);
                    sp->erase(sel_beg.next(), sp->end());
                } else {
                    // for open paths:
                    // 1. At end or beginning, delete including the node on the end or beginning
                    // 2. In the middle, delete only inner nodes
                    if (sel_beg == sp->begin()) {
                        sp->erase(sp->begin(), sel_end.prev());
                    } else if (sel_end == sp->end()) {
                        sp->erase(sel_beg.next(), sp->end());
                    } else {
                        SubpathPtr new_sp(new NodeList(_subpaths));
                        new_sp->splice(new_sp->end(), *sp, sp->begin(), sel_beg.next());
                        _subpaths.insert(i, new_sp);
                        if (sel_end.prev())
                            sp->erase(sp->begin(), sel_end.prev());
                    }
                }
            }
            sel_beg = sel_end;
            num_selected -= num_points;
        }
        ++i;
    }
}

/** Reverse subpaths of the path.
 * @param selected_only If true, only paths that have at least one selected node
 *                      will be reversed. Otherwise all subpaths will be reversed. */
void PathManipulator::reverseSubpaths(bool selected_only)
{
    for (SubpathList::iterator i = _subpaths.begin(); i != _subpaths.end(); ++i) {
        if (selected_only) {
            for (NodeList::iterator j = (*i)->begin(); j != (*i)->end(); ++j) {
                if (j->selected()) {
                    (*i)->reverse();
                    break; // continue with the next subpath
                }
            }
        } else {
            (*i)->reverse();
        }
    }
}

/** Make selected segments curves / lines. */
void PathManipulator::setSegmentType(SegmentType type)
{
    if (_num_selected == 0) return;
    for (SubpathList::iterator i = _subpaths.begin(); i != _subpaths.end(); ++i) {
        for (NodeList::iterator j = (*i)->begin(); j != (*i)->end(); ++j) {
            NodeList::iterator k = j.next();
            if (!(k && j->selected() && k->selected())) continue;
            switch (type) {
            case SEGMENT_STRAIGHT:
                if (j->front()->isDegenerate() && k->back()->isDegenerate())
                    break;
                j->front()->move(*j);
                k->back()->move(*k);
                break;
            case SEGMENT_CUBIC_BEZIER:
                if (!j->front()->isDegenerate() || !k->back()->isDegenerate())
                    break;
                j->front()->move(j->position() + (k->position() - j->position()) / 3);
                k->back()->move(k->position() + (j->position() - k->position()) / 3);
                break;
            }
        }
    }
}

/** Set the visibility of handles. */
void PathManipulator::showHandles(bool show)
{
    if (show == _show_handles) return;
    if (show) {
        for (SubpathList::iterator i = _subpaths.begin(); i != _subpaths.end(); ++i) {
            for (NodeList::iterator j = (*i)->begin(); j != (*i)->end(); ++j) {
                if (!j->selected()) continue;
                j->showHandles(true);
                if (j.prev()) j.prev()->showHandles(true);
                if (j.next()) j.next()->showHandles(true);
            }
        }
    } else {
        for (SubpathList::iterator i = _subpaths.begin(); i != _subpaths.end(); ++i) {
            for (NodeList::iterator j = (*i)->begin(); j != (*i)->end(); ++j) {
                j->showHandles(false);
            }
        }
    }
    _show_handles = show;
}

/** Set the visibility of outline. */
void PathManipulator::showOutline(bool show)
{
    if (show == _show_outline) return;
    _show_outline = show;
    _updateOutline();
}

void PathManipulator::showPathDirection(bool show)
{
    if (show == _show_path_direction) return;
    _show_path_direction = show;
    _updateOutline();
}

void PathManipulator::setControlsTransform(Geom::Matrix const &tnew)
{
    Geom::Matrix delta = _i2d_transform.inverse() * _edit_transform.inverse() * tnew * _i2d_transform;
    _edit_transform = tnew;
    for (SubpathList::iterator i = _subpaths.begin(); i != _subpaths.end(); ++i) {
        for (NodeList::iterator j = (*i)->begin(); j != (*i)->end(); ++j) {
            j->transform(delta);
        }
    }
    _createGeometryFromControlPoints();
}

/** Hide the curve drag point until the next motion event.
 * This should be called at the beginning of every method that can delete nodes.
 * Otherwise the invalidated iterator in the dragpoint can cause crashes. */
void PathManipulator::hideDragPoint()
{
    _dragpoint->setVisible(false);
    _dragpoint->setIterator(NodeList::iterator());
}

/** Insert a node in the segment beginning with the supplied iterator,
 * at the given time value */
NodeList::iterator PathManipulator::subdivideSegment(NodeList::iterator first, double t)
{
    if (!first) throw std::invalid_argument("Subdivide after invalid iterator");
    NodeList &list = NodeList::get(first);
    NodeList::iterator second = first.next();
    if (!second) throw std::invalid_argument("Subdivide after last node in open path");

    // We need to insert the segment after 'first'. We can't simply use 'second'
    // as the point of insertion, because when 'first' is the last node of closed path,
    // the new node will be inserted as the first node instead.
    NodeList::iterator insert_at = first;
    ++insert_at;

    NodeList::iterator inserted;
    if (first->front()->isDegenerate() && second->back()->isDegenerate()) {
        // for a line segment, insert a cusp node
        Node *n = new Node(_multi_path_manipulator._path_data.node_data,
            Geom::lerp(t, first->position(), second->position()));
        n->setType(NODE_CUSP, false);
        inserted = list.insert(insert_at, n);
    } else {
        // build bezier curve and subdivide
        Geom::CubicBezier temp(first->position(), first->front()->position(),
            second->back()->position(), second->position());
        std::pair<Geom::CubicBezier, Geom::CubicBezier> div = temp.subdivide(t);
        std::vector<Geom::Point> seg1 = div.first.points(), seg2 = div.second.points();

        // set new handle positions
        Node *n = new Node(_multi_path_manipulator._path_data.node_data, seg2[0]);
        n->back()->setPosition(seg1[2]);
        n->front()->setPosition(seg2[1]);
        n->setType(NODE_SMOOTH, false);
        inserted = list.insert(insert_at, n);

        first->front()->move(seg1[1]);
        second->back()->move(seg2[2]);
    }
    return inserted;
}

/** Find the node that is closest/farthest from the origin
 * @param origin Point of reference
 * @param search_selected Consider selected nodes
 * @param search_unselected Consider unselected nodes
 * @param closest If true, return closest node, if false, return farthest
 * @return The matching node, or an empty iterator if none found
 */
NodeList::iterator PathManipulator::extremeNode(NodeList::iterator origin, bool search_selected,
    bool search_unselected, bool closest)
{
    NodeList::iterator match;
    double extr_dist = closest ? HUGE_VAL : -HUGE_VAL;
    if (_num_selected == 0 && !search_unselected) return match;

    for (SubpathList::iterator i = _subpaths.begin(); i != _subpaths.end(); ++i) {
        for (NodeList::iterator j = (*i)->begin(); j != (*i)->end(); ++j) {
            if(j->selected()) {
                if (!search_selected) continue;
            } else {
                if (!search_unselected) continue;
            }
            double dist = Geom::distance(*j, *origin);
            bool cond = closest ? (dist < extr_dist) : (dist > extr_dist);
            if (cond) {
                match = j;
                extr_dist = dist;
            }
        }
    }
    return match;
}

/** Called by the XML observer when something else than us modifies the path. */
void PathManipulator::_externalChange(unsigned type)
{
    switch (type) {
    case PATH_CHANGE_D: {
        _getGeometry();

        // ugly: stored offsets of selected nodes in a vector
        // vector<bool> should be specialized so that it takes only 1 bit per value
        std::vector<bool> selpos;
        for (SubpathList::iterator i = _subpaths.begin(); i != _subpaths.end(); ++i) {
            for (NodeList::iterator j = (*i)->begin(); j != (*i)->end(); ++j) {
                selpos.push_back(j->selected());
            }
        }
        unsigned size = selpos.size(), curpos = 0;

        _createControlPointsFromGeometry();

        for (SubpathList::iterator i = _subpaths.begin(); i != _subpaths.end(); ++i) {
            for (NodeList::iterator j = (*i)->begin(); j != (*i)->end(); ++j) {
                if (curpos >= size) goto end_restore;
                if (selpos[curpos]) _selection.insert(j.ptr());
                ++curpos;
            }
        }
        end_restore:

        _updateOutline();
        } break;
    case PATH_CHANGE_TRANSFORM: {
        Geom::Matrix i2d_change = _d2i_transform;
        _i2d_transform = sp_item_i2d_affine(SP_ITEM(_path));
        _d2i_transform = _i2d_transform.inverse();
        i2d_change *= _i2d_transform;
        for (SubpathList::iterator i = _subpaths.begin(); i != _subpaths.end(); ++i) {
            for (NodeList::iterator j = (*i)->begin(); j != (*i)->end(); ++j) {
                j->transform(i2d_change);
            }
        }
        _updateOutline();
        } break;
    default: break;
    }
}

/** Create nodes and handles based on the XML of the edited path. */
void PathManipulator::_createControlPointsFromGeometry()
{
    clear();

    // sanitize pathvector and store it in SPCurve,
    // so that _updateDragPoint doesn't crash on paths with naked movetos
    Geom::PathVector pathv = pathv_to_linear_and_cubic_beziers(_spcurve->get_pathvector());
    for (Geom::PathVector::iterator i = pathv.begin(); i != pathv.end(); ) {
        if (i->empty()) pathv.erase(i++);
        else ++i;
    }
    _spcurve->set_pathvector(pathv);

    pathv *= (_edit_transform * _i2d_transform);

    // in this loop, we know that there are no zero-segment subpaths
    for (Geom::PathVector::const_iterator pit = pathv.begin(); pit != pathv.end(); ++pit) {
        // prepare new subpath
        SubpathPtr subpath(new NodeList(_subpaths));
        _subpaths.push_back(subpath);

        Node *previous_node = new Node(_multi_path_manipulator._path_data.node_data, pit->initialPoint());
        subpath->push_back(previous_node);
        Geom::Curve const &cseg = pit->back_closed();
        bool fuse_ends = pit->closed()
            && Geom::are_near(cseg.initialPoint(), cseg.finalPoint());

        for (Geom::Path::const_iterator cit = pit->begin(); cit != pit->end_open(); ++cit) {
            Geom::Point pos = cit->finalPoint();
            Node *current_node;
            // if the closing segment is degenerate and the path is closed, we need to move
            // the handle of the first node instead of creating a new one
            if (fuse_ends && cit == --(pit->end_open())) {
                current_node = subpath->begin().get_pointer();
            } else {
                /* regardless of segment type, create a new node at the end
                 * of this segment (unless this is the last segment of a closed path
                 * with a degenerate closing segment */
                current_node = new Node(_multi_path_manipulator._path_data.node_data, pos);
                subpath->push_back(current_node);
            }
            // if this is a bezier segment, move handles appropriately
            if (Geom::CubicBezier const *cubic_bezier =
                dynamic_cast<Geom::CubicBezier const*>(&*cit))
            {
                std::vector<Geom::Point> points = cubic_bezier->points();

                previous_node->front()->setPosition(points[1]);
                current_node ->back() ->setPosition(points[2]);
            }
            previous_node = current_node;
        }
        // If the path is closed, make the list cyclic
        if (pit->closed()) subpath->setClosed(true);
    }

    // we need to set the nodetypes after all the handles are in place,
    // so that pickBestType works correctly
    // TODO maybe migrate to inkscape:node-types?
    gchar const *nts_raw = _path ? _path->repr->attribute(_nodetypesKey().data()) : 0;
    std::string nodetype_string = nts_raw ? nts_raw : "";
    /* Calculate the needed length of the nodetype string.
     * For closed paths, the entry is duplicated for the starting node,
     * so we can just use the count of segments including the closing one
     * to include the extra end node. */
    std::string::size_type nodetype_len = 0;
    for (Geom::PathVector::const_iterator i = pathv.begin(); i != pathv.end(); ++i) {
        if (i->empty()) continue;
        nodetype_len += i->size_closed();
    }
    /* pad the string to required length with a bogus value.
     * 'b' and any other letter not recognized by the parser causes the best fit to be set
     * as the node type */
    if (nodetype_len > nodetype_string.size()) {
        nodetype_string.append(nodetype_len - nodetype_string.size(), 'b');
    }
    std::string::iterator tsi = nodetype_string.begin();
    for (std::list<SubpathPtr>::iterator i = _subpaths.begin(); i != _subpaths.end(); ++i) {
        for (NodeList::iterator j = (*i)->begin(); j != (*i)->end(); ++j) {
            j->setType(Node::parse_nodetype(*tsi++), false);
        }
        if ((*i)->closed()) {
            // STUPIDITY ALERT: it seems we need to use the duplicate type symbol instead of
            // the first one to remain backward compatible.
            (*i)->begin()->setType(Node::parse_nodetype(*tsi++), false);
        }
    }
}

/** Construct the geometric representation of nodes and handles, update the outline
 * and display */
void PathManipulator::_createGeometryFromControlPoints()
{
    Geom::PathBuilder builder;
    for (std::list<SubpathPtr>::iterator spi = _subpaths.begin(); spi != _subpaths.end(); ) {
        SubpathPtr subpath = *spi;
        if (subpath->empty()) {
            _subpaths.erase(spi++);
            continue;
        }
        NodeList::iterator prev = subpath->begin();
        builder.moveTo(prev->position());

        for (NodeList::iterator i = ++subpath->begin(); i != subpath->end(); ++i) {
            build_segment(builder, prev.ptr(), i.ptr());
            prev = i;
        }
        if (subpath->closed()) {
            // Here we link the last and first node if the path is closed.
            // If the last segment is Bezier, we add it.
            if (!prev->front()->isDegenerate() || !subpath->begin()->back()->isDegenerate()) {
                build_segment(builder, prev.ptr(), subpath->begin().ptr());
            }
            // if that segment is linear, we just call closePath().
            builder.closePath();
        }
        ++spi;
    }
    builder.finish();
    _spcurve->set_pathvector(builder.peek() * (_edit_transform * _i2d_transform).inverse());
    _updateOutline();
    _setGeometry();
}

/** Build one segment of the geometric representation.
 * @relates PathManipulator */
void build_segment(Geom::PathBuilder &builder, Node *prev_node, Node *cur_node)
{
    if (cur_node->back()->isDegenerate() && prev_node->front()->isDegenerate())
    {
        // NOTE: It seems like the renderer cannot correctly handle vline / hline segments,
        // and trying to display a path using them results in funny artifacts.
        builder.lineTo(cur_node->position());
    } else {
        // this is a bezier segment
        builder.curveTo(
            prev_node->front()->position(),
            cur_node->back()->position(),
            cur_node->position());
    }
}

/** Construct a node type string to store in the sodipodi:nodetypes attribute. */
std::string PathManipulator::_createTypeString()
{
    // precondition: no single-node subpaths
    std::stringstream tstr;
    for (std::list<SubpathPtr>::iterator i = _subpaths.begin(); i != _subpaths.end(); ++i) {
        for (NodeList::iterator j = (*i)->begin(); j != (*i)->end(); ++j) {
            tstr << j->type();
        }
        // nodestring format peculiarity: first node is counted twice for closed paths
        if ((*i)->closed()) tstr << (*i)->begin()->type();
    }
    return tstr.str();
}

/** Update the path outline. */
void PathManipulator::_updateOutline()
{
    if (!_show_outline) {
        sp_canvas_item_hide(_outline);
        return;
    }

    Geom::PathVector pv = _spcurve->get_pathvector();
    pv *= (_edit_transform * _i2d_transform);
    // This SPCurve thing has to be killed with extreme prejudice
    SPCurve *_hc = new SPCurve();
    if (_show_path_direction) {
        // To show the direction, we append additional subpaths which consist of a single
        // linear segment that starts at the time value of 0.5 and extends for 10 pixels
        // at an angle 150 degrees from the unit tangent. This creates the appearance
        // of little 'harpoons' that show the direction of the subpaths.
        Geom::PathVector arrows;
        for (Geom::PathVector::iterator i = pv.begin(); i != pv.end(); ++i) {
            Geom::Path &path = *i;
            for (Geom::Path::const_iterator j = path.begin(); j != path.end_default(); ++j) {
                Geom::Point at = j->pointAt(0.5);
                Geom::Point ut = j->unitTangentAt(0.5);
                // rotate the point 
                ut *= Geom::Rotate(150.0 / 180.0 * M_PI);
                Geom::Point arrow_end = _desktop->w2d(
                    _desktop->d2w(at) + Geom::unit_vector(_desktop->d2w(ut)) * 10.0);

                Geom::Path arrow(at);
                arrow.appendNew<Geom::LineSegment>(arrow_end);
                arrows.push_back(arrow);
            }
        }
        pv.insert(pv.end(), arrows.begin(), arrows.end());
    }
    _hc->set_pathvector(pv);
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(_outline), _hc);
    sp_canvas_item_show(_outline);
    _hc->unref();
}

/** Retrieve the geometry of the edited object from the object tree */
void PathManipulator::_getGeometry()
{
    using namespace Inkscape::LivePathEffect;
    if (!_lpe_key.empty()) {
        Effect *lpe = LIVEPATHEFFECT(_path)->get_lpe();
        if (lpe) {
            PathParam *pathparam = dynamic_cast<PathParam *>(lpe->getParameter(_lpe_key.data()));
            if (!_spcurve)
                _spcurve = new SPCurve(pathparam->get_pathvector());
            else
                _spcurve->set_pathvector(pathparam->get_pathvector());
        }
    } else {
        if (_spcurve) _spcurve->unref();
        _spcurve = sp_path_get_curve_for_edit(_path);
    }
}

/** Set the geometry of the edited object in the object tree, but do not commit to XML */
void PathManipulator::_setGeometry()
{
    using namespace Inkscape::LivePathEffect;
    if (empty()) return;

    if (!_lpe_key.empty()) {
        // copied from nodepath.cpp
        // NOTE: if we are editing an LPE param, _path is not actually an SPPath, it is
        // a LivePathEffectObject. (mad laughter)
        Effect *lpe = LIVEPATHEFFECT(_path)->get_lpe();
        if (lpe) {
            PathParam *pathparam = dynamic_cast<PathParam *>(lpe->getParameter(_lpe_key.data()));
            pathparam->set_new_value(_spcurve->get_pathvector(), false);
            LIVEPATHEFFECT(_path)->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
    } else {
        if (_path->repr->attribute("inkscape:original-d"))
            sp_path_set_original_curve(_path, _spcurve, true, false);
        else
            sp_shape_set_curve(SP_SHAPE(_path), _spcurve, false);
    }
}

/** Figure out in what attribute to store the nodetype string. */
Glib::ustring PathManipulator::_nodetypesKey()
{
    if (_lpe_key.empty()) return "sodipodi:nodetypes";
    return _lpe_key + "-nodetypes";
}

/** Return the XML node we are editing.
 * This method is wrong but necessary at the moment. */
Inkscape::XML::Node *PathManipulator::_getXMLNode()
{
    if (_lpe_key.empty()) return _path->repr;
    return LIVEPATHEFFECT(_path)->repr;
}

void PathManipulator::_attachNodeHandlers(Node *node)
{
    Handle *handles[2] = { node->front(), node->back() };
    for (int i = 0; i < 2; ++i) {
        handles[i]->signal_update.connect(
            sigc::mem_fun(*this, &PathManipulator::update));
        handles[i]->signal_ungrabbed.connect(
            sigc::hide(
                sigc::mem_fun(*this, &PathManipulator::_handleUngrabbed)));
        handles[i]->signal_grabbed.connect(
            sigc::bind_return(
                sigc::hide(
                    sigc::mem_fun(*this, &PathManipulator::_handleGrabbed)),
                false));
        handles[i]->signal_clicked.connect(
            sigc::bind<0>(
                sigc::mem_fun(*this, &PathManipulator::_handleClicked),
                handles[i]));
    }
    node->signal_clicked.connect(
        sigc::bind<0>(
            sigc::mem_fun(*this, &PathManipulator::_nodeClicked),
            node));
}

bool PathManipulator::_nodeClicked(Node *n, GdkEventButton *event)
{
    // cycle between node types on ctrl+click
    if (event->button != 1) return false;
    if (held_alt(*event) && held_control(*event)) {
        // Ctrl+Alt+click: delete nodes
        hideDragPoint();
        NodeList::iterator iter = NodeList::get_iterator(n);
        NodeList *nl = iter->list();

        if (nl->size() <= 1 || (nl->size() <= 2 && !nl->closed())) {
            // Removing last node of closed path - delete it
            nl->kill();
        } else {
            // In other cases, delete the node under cursor
            _deleteStretch(iter, iter.next(), true);
        }

        if (!empty()) { 
            update();
        }
        // We need to call MPM's method because it could have been our last node
        _multi_path_manipulator._doneWithCleanup(_("Delete node"));

        return true;
    } else if (held_control(*event)) {
        // Ctrl+click: cycle between node types
        if (n->isEndNode()) {
            if (n->type() == NODE_CUSP) {
                n->setType(NODE_SMOOTH);
            } else {
                n->setType(NODE_CUSP);
            }
        } else {
            n->setType(static_cast<NodeType>((n->type() + 1) % NODE_LAST_REAL_TYPE));
        }
        update();
        _commit(_("Cycle node type"));
        return true;
    }
    return false;
}

void PathManipulator::_handleGrabbed()
{
    _selection.hideTransformHandles();
}

void PathManipulator::_handleUngrabbed()
{
    _selection.restoreTransformHandles();
    _commit(_("Drag handle"));
}

bool PathManipulator::_handleClicked(Handle *h, GdkEventButton *event)
{
    // retracting by Ctrl+click
    if (event->button == 1 && held_control(*event)) {
        h->move(h->parent()->position());
        update();
        _commit(_("Retract handle"));
        return true;
    }
    return false;
}

void PathManipulator::_selectionChanged(SelectableControlPoint *p, bool selected)
{
    // don't do anything if we do not show handles
    if (!_show_handles) return;

    // only do something if a node changed selection state
    Node *node = dynamic_cast<Node*>(p);
    if (!node) return;

    // update handle display
    NodeList::iterator iters[5];
    iters[2] = NodeList::get_iterator(node);
    iters[1] = iters[2].prev();
    iters[3] = iters[2].next();
    if (selected) {
        // selection - show handles on this node and adjacent ones
        node->showHandles(true);
        if (iters[1]) iters[1]->showHandles(true);
        if (iters[3]) iters[3]->showHandles(true);
    } else {
        /* Deselection is more complex.
         * The change might affect 3 nodes - this one and two adjacent.
         * If the node and both its neighbors are deselected, hide handles.
         * Otherwise, leave as is. */
        if (iters[1]) iters[0] = iters[1].prev();
        if (iters[3]) iters[4] = iters[3].next();
        bool nodesel[5];
        for (int i = 0; i < 5; ++i) {
            nodesel[i] = iters[i] && iters[i]->selected();
        }
        for (int i = 1; i < 4; ++i) {
            if (iters[i] && !nodesel[i-1] && !nodesel[i] && !nodesel[i+1]) {
                iters[i]->showHandles(false);
            }
        }
    }

    if (selected) ++_num_selected;
    else --_num_selected;
}

/** Removes all nodes belonging to this manipulator from the control pont selection */
void PathManipulator::_removeNodesFromSelection()
{
    // remove this manipulator's nodes from selection
    for (std::list<SubpathPtr>::iterator i = _subpaths.begin(); i != _subpaths.end(); ++i) {
        for (NodeList::iterator j = (*i)->begin(); j != (*i)->end(); ++j) {
            _selection.erase(j.get_pointer());
        }
    }
}

/** Update the XML representation and put the specified annotation on the undo stack */
void PathManipulator::_commit(Glib::ustring const &annotation)
{
    writeXML();
    sp_document_done(sp_desktop_document(_desktop), SP_VERB_CONTEXT_NODE, annotation.data());
}

/** Update the position of the curve drag point such that it is over the nearest
 * point of the path. */
void PathManipulator::_updateDragPoint(Geom::Point const &evp)
{
    // TODO find a way to make this faster (no transform required)
    Geom::PathVector pv = _spcurve->get_pathvector() * (_edit_transform * _i2d_transform);
    boost::optional<Geom::PathVectorPosition> pvp
        = Geom::nearestPoint(pv, _desktop->w2d(evp));
    if (!pvp) return;
    Geom::Point nearest_point = _desktop->d2w(pv.at(pvp->path_nr).pointAt(pvp->t));
    
    double fracpart;
    std::list<SubpathPtr>::iterator spi = _subpaths.begin();
    for (unsigned i = 0; i < pvp->path_nr; ++i, ++spi) {}
    NodeList::iterator first = (*spi)->before(pvp->t, &fracpart);
    
    double stroke_tolerance = _getStrokeTolerance();
    if (Geom::distance(evp, nearest_point) < stroke_tolerance) {
        _dragpoint->setVisible(true);
        _dragpoint->setPosition(_desktop->w2d(nearest_point));
        _dragpoint->setSize(2 * stroke_tolerance);
        _dragpoint->setTimeValue(fracpart);
        _dragpoint->setIterator(first);
    } else {
        _dragpoint->setVisible(false);
    }
}

/// This is called on zoom change to update the direction arrows
void PathManipulator::_updateOutlineOnZoomChange()
{
    if (_show_path_direction) _updateOutline();
}

/** Compute the radius from the edge of the path where clicks chould initiate a curve drag
 * or segment selection, in window coordinates. */
double PathManipulator::_getStrokeTolerance()
{
    /* Stroke event tolerance is equal to half the stroke's width plus the global
     * drag tolerance setting.  */
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    double ret = prefs->getIntLimited("/options/dragtolerance/value", 2, 0, 100);
    if (_path && !SP_OBJECT_STYLE(_path)->stroke.isNone()) {
        ret += SP_OBJECT_STYLE(_path)->stroke_width.computed * 0.5
            * (_edit_transform * _i2d_transform).descrim() // scale to desktop coords
            * _desktop->current_zoom(); // == _d2w.descrim() - scale to window coords
    }
    return ret;
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
