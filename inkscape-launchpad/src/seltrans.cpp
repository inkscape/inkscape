/** @file
 * Helper object for transforming selected items.
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Carl Hetherington <inkscape@carlh.net>
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 1999-2014 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <cstring>
#include <string>

#include <2geom/transforms.h>
#include <gdk/gdkkeysyms.h>
#include "document.h"
#include "document-undo.h"
#include "sp-namedview.h"
#include "desktop.h"

#include "desktop-style.h"
#include "knot.h"
#include "message-stack.h"
#include "snap.h"
#include "pure-transform.h"
#include "selection.h"
#include "ui/tools/select-tool.h"
#include "sp-item.h"
#include "sp-item-transform.h"
#include "sp-root.h"
#include "seltrans-handles.h"
#include "seltrans.h"
#include "selection-chemistry.h"
#include "verbs.h"
#include <glibmm/i18n.h>
#include "display/sp-ctrlline.h"
#include "display/sodipodi-ctrl.h"
#include "preferences.h"
#include "xml/repr.h"
#include "mod360.h"
#include <2geom/angle.h>
#include "display/snap-indicator.h"
#include "ui/control-manager.h"
#include "seltrans-handles.h"

using Inkscape::ControlManager;
using Inkscape::DocumentUndo;

static void sp_sel_trans_handle_grab(SPKnot *knot, guint state, SPSelTransHandle const* data);
static void sp_sel_trans_handle_ungrab(SPKnot *knot, guint state, SPSelTransHandle const* data);
static void sp_sel_trans_handle_click(SPKnot *knot, guint state, SPSelTransHandle const* data);
static void sp_sel_trans_handle_new_event(SPKnot *knot, Geom::Point const &position, guint32 state, SPSelTransHandle const* data);
static gboolean sp_sel_trans_handle_request(SPKnot *knot, Geom::Point *p, guint state, SPSelTransHandle const *data);

extern GdkPixbuf *handles[];

static gboolean sp_sel_trans_handle_event(SPKnot *knot, GdkEvent *event, SPSelTransHandle const*)
{
    switch (event->type) {
        case GDK_MOTION_NOTIFY:
            break;
        case GDK_KEY_PRESS:
            if (Inkscape::UI::Tools::get_group0_keyval (&event->key) == GDK_KEY_space) {
                /* stamping mode: both mode(show content and outline) operation with knot */
                if (!SP_KNOT_IS_GRABBED(knot)) {
                    return FALSE;
                }
                SPDesktop *desktop = knot->desktop;
                Inkscape::SelTrans *seltrans = SP_SELECT_CONTEXT(desktop->event_context)->_seltrans;
                seltrans->stamp();
                return TRUE;
            }
            break;
        default:
            break;
    }

    return FALSE;
}

Inkscape::SelTrans::BoundingBoxPrefsObserver::BoundingBoxPrefsObserver(SelTrans &sel_trans) :
    Observer("/tools/bounding_box"),
    _sel_trans(sel_trans)
{
}

void Inkscape::SelTrans::BoundingBoxPrefsObserver::notify(Preferences::Entry const &val)
{
    _sel_trans._boundingBoxPrefsChanged(static_cast<int>(val.getBool()));
}

Inkscape::SelTrans::SelTrans(SPDesktop *desktop) :
    _desktop(desktop),
    _selcue(desktop),
    _state(STATE_SCALE),
    _show(SHOW_CONTENT),
    _grabbed(false),
    _show_handles(true),
    _bbox(),
    _visual_bbox(),
    _absolute_affine(Geom::Scale(1,1)),
    _opposite(Geom::Point(0,0)),
    _opposite_for_specpoints(Geom::Point(0,0)),
    _opposite_for_bboxpoints(Geom::Point(0,0)),
    _origin_for_specpoints(Geom::Point(0,0)),
    _origin_for_bboxpoints(Geom::Point(0,0)),
    _stamp_cache(std::vector<SPItem*>()),
    _message_context(desktop->messageStack()),
    _bounding_box_prefs_observer(*this)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int prefs_bbox = prefs->getBool("/tools/bounding_box");
    _snap_bbox_type = !prefs_bbox ?
        SPItem::VISUAL_BBOX : SPItem::GEOMETRIC_BBOX;

    g_return_if_fail(desktop != NULL);

    _updateVolatileState();
    _current_relative_affine.setIdentity();

    _center_is_set = false; // reread _center from items, or set to bbox midpoint

    _makeHandles();
    _updateHandles();

    _selection = desktop->getSelection();

    _norm = sp_canvas_item_new(desktop->getControls(),
                               SP_TYPE_CTRL,
                               "anchor", SP_ANCHOR_CENTER,
                               "mode", SP_CTRL_MODE_COLOR,
                               "shape", SP_CTRL_SHAPE_BITMAP,
                               "size", 13.0,
                               "filled", TRUE,
                               "fill_color", 0x00000000,
                               "stroked", TRUE,
                               "stroke_color", 0x000000a0,
                               "pixbuf", handles[12],
                               NULL);

    _grip = sp_canvas_item_new(desktop->getControls(),
                               SP_TYPE_CTRL,
                               "anchor", SP_ANCHOR_CENTER,
                               "mode", SP_CTRL_MODE_XOR,
                               "shape", SP_CTRL_SHAPE_CROSS,
                               "size", 7.0,
                               "filled", TRUE,
                               "fill_color", 0xffffff7f,
                               "stroked", TRUE,
                               "stroke_color", 0xffffffff,
                               "pixbuf", handles[12],
                               NULL);

    sp_canvas_item_hide(_grip);
    sp_canvas_item_hide(_norm);

    for (int i = 0; i < 4; i++) {
        _l[i] = ControlManager::getManager().createControlLine(desktop->getControls());
        sp_canvas_item_hide(_l[i]);
    }

    _sel_changed_connection = _selection->connectChanged(
        sigc::mem_fun(*this, &Inkscape::SelTrans::_selChanged)
        );

    _sel_modified_connection = _selection->connectModified(
        sigc::mem_fun(*this, &Inkscape::SelTrans::_selModified)
        );

    _all_snap_sources_iter = _all_snap_sources_sorted.end();

    prefs->addObserver(_bounding_box_prefs_observer);
}

Inkscape::SelTrans::~SelTrans()
{
    _sel_changed_connection.disconnect();
    _sel_modified_connection.disconnect();

    for (int i = 0; i < NUMHANDS; i++) {
        knot_unref(knots[i]);
        knots[i] = NULL;
    }

    if (_norm) {
        sp_canvas_item_destroy(_norm);
        _norm = NULL;
    }
    if (_grip) {
        sp_canvas_item_destroy(_grip);
        _grip = NULL;
    }
    for (int i = 0; i < 4; i++) {
        if (_l[i]) {
            sp_canvas_item_destroy(_l[i]);
            _l[i] = NULL;
        }
    }

    for (unsigned i = 0; i < _items.size(); i++) {
        sp_object_unref(_items[i], NULL);
    }

    _items.clear();
    _items_const.clear();
    _items_affines.clear();
    _items_centers.clear();
}

void Inkscape::SelTrans::resetState()
{
    _state = STATE_SCALE;
}

void Inkscape::SelTrans::increaseState()
{
    if (_state == STATE_SCALE) {
        _state = STATE_ROTATE;
    } else {
        _state = STATE_SCALE;
    }

    _center_is_set = true; // no need to reread center

    _updateHandles();
}

void Inkscape::SelTrans::setCenter(Geom::Point const &p)
{
    _center = p;
    _center_is_set = true;

    // Write the new center position into all selected items
    std::vector<SPItem*> items=_desktop->selection->itemList();
    for ( std::vector<SPItem*>::const_iterator iter=items.begin();iter!=items.end();iter++ ) {
        SPItem *it = SP_ITEM(*iter);
        it->setCenter(p);
        // only set the value; updating repr and document_done will be done once, on ungrab
    }

    _updateHandles();
}

void Inkscape::SelTrans::grab(Geom::Point const &p, gdouble x, gdouble y, bool show_handles, bool translating)
{
    // While dragging a handle, we will either scale, skew, or rotate and the "translating" parameter will be false
    // When dragging the selected item itself however, we will translate the selection and that parameter will be true
    Inkscape::Selection *selection = _desktop->getSelection();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    g_return_if_fail(!_grabbed);

    _grabbed = true;
    _show_handles = show_handles;
    _updateVolatileState();
    _current_relative_affine.setIdentity();

    _changed = false;

    if (_empty) {
        return;
    }

    std::vector<SPItem*> items=_desktop->selection->itemList();
    for ( std::vector<SPItem*>::const_iterator iter=items.begin();iter!=items.end();iter++ ) {
        SPItem *it = static_cast<SPItem*>(sp_object_ref(*iter, NULL));
        _items.push_back(it);
        _items_const.push_back(it);
        _items_affines.push_back(it->i2dt_affine());
        _items_centers.push_back(it->getCenter()); // for content-dragging, we need to remember original centers
    }

    _handle_x = x;
    _handle_y = y;

    // The selector tool should snap the bbox, special snappoints, and path nodes
    // (The special points are the handles, center, rotation axis, font baseline, ends of spiral, etc.)

    // First, determine the bounding box
    _bbox = selection->bounds(_snap_bbox_type);
    _visual_bbox = selection->visualBounds(); // Used for correctly scaling the strokewidth
    _geometric_bbox = selection->geometricBounds();

    _point = p;
    if (_geometric_bbox) {
        _point_geom = _geometric_bbox->min() + _geometric_bbox->dimensions() * Geom::Scale(x, y);
    } else {
        _point_geom = p;
    }

    // Next, get all points to consider for snapping
    SnapManager const &m = _desktop->namedview->snap_manager;
    _snap_points.clear();
    if (m.someSnapperMightSnap(false)) { // Only search for snap sources when really needed, to avoid unnecessary delays
        _snap_points = selection->getSnapPoints(&m.snapprefs); // This might take some time!
    }
    if (_snap_points.size() > 200 && !(prefs->getBool("/options/snapclosestonly/value", false))) {
        /* Snapping a huge number of nodes will take way too long, so limit the number of snappable nodes
        A typical user would rarely ever try to snap such a large number of nodes anyway, because
        (s)he would hardly be able to discern which node would be snapping */
        std::cout << "Warning: limit of 200 snap sources reached, some will be ignored" << std::endl;
        _snap_points.resize(200);
        // Unfortunately, by now we will have lost the font-baseline snappoints :-(
    }

    // Find bbox hulling all special points, which excludes stroke width. Here we need to include the
    // path nodes, for example because a rectangle which has been converted to a path doesn't have
    // any other special points
    Geom::OptRect snap_points_bbox = selection->bounds(SPItem::GEOMETRIC_BBOX);

    _bbox_points.clear();
    // Collect the bounding box's corners and midpoints for each selected item
    if (m.snapprefs.isTargetSnappable(SNAPTARGET_BBOX_CATEGORY)) {
        bool c = m.snapprefs.isTargetSnappable(SNAPTARGET_BBOX_CORNER);
        bool mp = m.snapprefs.isTargetSnappable(SNAPTARGET_BBOX_MIDPOINT);
        bool emp = m.snapprefs.isTargetSnappable(SNAPTARGET_BBOX_EDGE_MIDPOINT);
        // Preferably we'd use the bbox of each selected item, but for example 50 items will produce at least 200 bbox points,
        // which might make Inkscape crawl(see the comment a few lines above). In that case we will use the bbox of the selection as a whole
        bool c1 = (_items.size() > 0) && (_items.size() < 50);
        bool c2 = prefs->getBool("/options/snapclosestonly/value", false);
        if (translating && (c1 || c2)) {
            // Get the bounding box points for each item in the selection
            for (unsigned i = 0; i < _items.size(); i++) {
                Geom::OptRect b = _items[i]->desktopBounds(_snap_bbox_type);
                getBBoxPoints(b, &_bbox_points, false, c, emp, mp);
            }
        } else {
            // Only get the bounding box points of the selection as a whole
            getBBoxPoints(selection->bounds(_snap_bbox_type), &_bbox_points, false, c, emp, mp);
        }
    }

    if (_bbox) {
        // There are two separate "opposites" (i.e. opposite w.r.t. the handle being dragged):
        //  - one for snapping the boundingbox, which can be either visual or geometric
        //  - one for snapping the special points
        // The "opposite" in case of a geometric boundingbox always coincides with the "opposite" for the special points
        // These distinct "opposites" are needed in the snapmanager to avoid bugs such as LP167905 (in which
        // a box is caught between two guides)
        _opposite_for_bboxpoints = _bbox->min() + _bbox->dimensions() * Geom::Scale(1-x, 1-y);
        if (snap_points_bbox) {
            _opposite_for_specpoints = (*snap_points_bbox).min() + (*snap_points_bbox).dimensions() * Geom::Scale(1-x, 1-y);
        } else {
            _opposite_for_specpoints = _opposite_for_bboxpoints;
        }
        _opposite = _opposite_for_bboxpoints;
    }

    // When snapping the node closest to the mouse pointer is absolutely preferred over the closest snap
    // (i.e. when weight == 1), then we will not even try to snap to other points and disregard those other points

    if (prefs->getBool("/options/snapclosestonly/value", false)) {
        _keepClosestPointOnly(p);
    }

    if ((x != -1) && (y != -1)) {
        sp_canvas_item_show(_norm);
        sp_canvas_item_show(_grip);
    }

    if (_show == SHOW_OUTLINE) {
        for (int i = 0; i < 4; i++)
            sp_canvas_item_show(_l[i]);
    }

    _updateHandles();
    g_return_if_fail(_stamp_cache.empty());
}

void Inkscape::SelTrans::transform(Geom::Affine const &rel_affine, Geom::Point const &norm)
{
    g_return_if_fail(_grabbed);
    g_return_if_fail(!_empty);

    Geom::Affine const affine( Geom::Translate(-norm) * rel_affine * Geom::Translate(norm) );

    if (_show == SHOW_CONTENT) {
        // update the content
        for (unsigned i = 0; i < _items.size(); i++) {
            SPItem &item = *_items[i];
            if( SP_IS_ROOT(&item) ) {
                _desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Cannot transform an embedded SVG."));
                break;
            }
            Geom::Affine const &prev_transform = _items_affines[i];
            item.set_i2d_affine(prev_transform * affine);
            // The new affine will only have been applied if the transformation is different from the previous one, see SPItem::set_item_transform
        }
    } else {
        if (_bbox) {
            Geom::Point p[4];
            /* update the outline */
            for (unsigned i = 0 ; i < 4 ; i++) {
                p[i] = _bbox->corner(i) * affine;
            }
            for (unsigned i = 0 ; i < 4 ; i++) {
                _l[i]->setCoords(p[i], p[(i+1)%4]);
            }
        }
    }

    _current_relative_affine = affine;
    _changed = true;
    _updateHandles();
}

void Inkscape::SelTrans::ungrab()
{
    g_return_if_fail(_grabbed);
    _grabbed = false;
    _show_handles = true;

    _desktop->snapindicator->remove_snapsource();

    Inkscape::Selection *selection = _desktop->getSelection();
    _updateVolatileState();

    for (unsigned i = 0; i < _items.size(); i++) {
        sp_object_unref(_items[i], NULL);
    }

    sp_canvas_item_hide(_norm);
    sp_canvas_item_hide(_grip);

    if (_show == SHOW_OUTLINE) {
        for (int i = 0; i < 4; i++)
            sp_canvas_item_hide(_l[i]);
    }
    if(!_stamp_cache.empty()){
        _stamp_cache.clear();
    }

    _message_context.clear();

    if (!_empty && _changed) {
        if (!_current_relative_affine.isIdentity()) { // we can have a identity affine
            // when trying to stretch a perfectly vertical line in horizontal direction, which will not be allowed by the handles;

            sp_selection_apply_affine(selection, _current_relative_affine, (_show == SHOW_OUTLINE)? true : false);
            if (_center) {
                *_center *= _current_relative_affine;
                _center_is_set = true;
            }

            // If dragging showed content live, sp_selection_apply_affine cannot change the centers
            // appropriately - it does not know the original positions of the centers (all objects already have
            // the new bboxes). So we need to reset the centers from our saved array.
            if (_show != SHOW_OUTLINE && !_current_relative_affine.isTranslation()) {
                for (unsigned i = 0; i < _items_centers.size(); i++) {
                    SPItem *currentItem = _items[i];
                    if (currentItem->isCenterSet()) { // only if it's already set
                        currentItem->setCenter (_items_centers[i] * _current_relative_affine);
                        currentItem->updateRepr();
                    }
                }
            }
        }

        _items.clear();
        _items_const.clear();
        _items_affines.clear();
        _items_centers.clear();

        if (!_current_relative_affine.isIdentity()) { // we can have a identity affine
            // when trying to stretch a perfectly vertical line in horizontal direction, which will not be allowed
            // by the handles; this would be identified as a (zero) translation by isTranslation()
            if (_current_relative_affine.isTranslation()) {
                DocumentUndo::done(_desktop->getDocument(), SP_VERB_CONTEXT_SELECT,
                                   _("Move"));
            } else if (_current_relative_affine.withoutTranslation().isScale()) {
                DocumentUndo::done(_desktop->getDocument(), SP_VERB_CONTEXT_SELECT,
                                   _("Scale"));
            } else if (_current_relative_affine.withoutTranslation().isRotation()) {
                DocumentUndo::done(_desktop->getDocument(), SP_VERB_CONTEXT_SELECT,
                                   _("Rotate"));
            } else {
                DocumentUndo::done(_desktop->getDocument(), SP_VERB_CONTEXT_SELECT,
                                   _("Skew"));
            }
        }

    } else {

        if (_center_is_set) {
            // we were dragging center; update reprs and commit undoable action
        	std::vector<SPItem*> items=_desktop->selection->itemList();
            for ( std::vector<SPItem*>::const_iterator iter=items.begin();iter!=items.end();iter++ ) {
                SPItem *it = *iter;
                it->updateRepr();
            }
            DocumentUndo::done(_desktop->getDocument(), SP_VERB_CONTEXT_SELECT,
                               _("Set center"));
        }

        _items.clear();
        _items_const.clear();
        _items_affines.clear();
        _items_centers.clear();
        _updateHandles();
    }
}

/* fixme: This is really bad, as we compare positions for each stamp (Lauris) */
/* fixme: IMHO the best way to keep sort cache would be to implement timestamping at last */

void Inkscape::SelTrans::stamp()
{
    Inkscape::Selection *selection = _desktop->getSelection();

    bool fixup = !_grabbed;
    if ( fixup && !_stamp_cache.empty() ) {
        // TODO - give a proper fix. Simple temporary work-around for the grab() issue
        _stamp_cache.clear();
    }

    /* stamping mode */
    if (!_empty) {
    	std::vector<SPItem*> l;
        if (!_stamp_cache.empty()) {
            l = _stamp_cache;
        } else {
            /* Build cache */
            l = selection->itemList();
            sort(l.begin(),l.end(),sp_object_compare_position_bool);
            _stamp_cache = l;
        }

        for(std::vector<SPItem*>::const_iterator x=l.begin();x!=l.end();x++) {
            SPItem *original_item = *x;
            Inkscape::XML::Node *original_repr = original_item->getRepr();

            // remember the position of the item
            gint pos = original_repr->position();
            // remember parent
            Inkscape::XML::Node *parent = original_repr->parent();

            Inkscape::XML::Node *copy_repr = original_repr->duplicate(parent->document());

            // add the new repr to the parent
            parent->appendChild(copy_repr);
            // move to the saved position
            copy_repr->setPosition(pos > 0 ? pos : 0);

            SPItem *copy_item = (SPItem *) _desktop->getDocument()->getObjectByRepr(copy_repr);

            Geom::Affine const *new_affine;
            if (_show == SHOW_OUTLINE) {
                Geom::Affine const i2d(original_item->i2dt_affine());
                Geom::Affine const i2dnew( i2d * _current_relative_affine );
                copy_item->set_i2d_affine(i2dnew);
                new_affine = &copy_item->transform;
            } else {
                new_affine = &original_item->transform;
            }

            copy_item->doWriteTransform(copy_repr, *new_affine);

            if ( copy_item->isCenterSet() && _center ) {
                copy_item->setCenter(*_center * _current_relative_affine);
            }

            Inkscape::GC::release(copy_repr);
        }
        DocumentUndo::done(_desktop->getDocument(), SP_VERB_CONTEXT_SELECT,
                           _("Stamp"));
    }

    if ( fixup && !_stamp_cache.empty() ) {
        // TODO - give a proper fix. Simple temporary work-around for the grab() issue
        _stamp_cache.clear();
    }
}

void Inkscape::SelTrans::_updateHandles()
{
    for (int i = 0; i < NUMHANDS; i++)
        knots[i]->hide();

    if ( !_show_handles || _empty )
        return;

    if (!_center_is_set) {
        _center = _desktop->selection->center();
        _center_is_set = true;
    }

    if ( _state == STATE_SCALE ) {
        _showHandles(HANDLE_STRETCH);
        _showHandles(HANDLE_SCALE);
    } else {
        _showHandles(HANDLE_SKEW);
        _showHandles(HANDLE_ROTATE);
        _showHandles(HANDLE_CENTER);
    }
}

void Inkscape::SelTrans::_updateVolatileState()
{
    Inkscape::Selection *selection = _desktop->getSelection();
    _empty = selection->isEmpty();

    if (_empty) {
        return;
    }

    //Update the bboxes
    _bbox = selection->bounds(_snap_bbox_type);
    _visual_bbox = selection->visualBounds();

    if (!_bbox) {
        _empty = true;
        return;
    }

    _strokewidth = stroke_average_width (selection->itemList());
}

void Inkscape::SelTrans::_showHandles(SPSelTransType type)
{
    // shouldn't have nullary bbox, but knots
    g_assert(_bbox);

    for (int i = 0; i < NUMHANDS; i++) {
        if (hands[i].type != type)
            continue;

        // Position knots to scale the selection bbox
        Geom::Point const bpos(hands[i].x, hands[i].y);
        Geom::Point p(_bbox->min() + (_bbox->dimensions() * Geom::Scale(bpos)));
        knots[i]->moveto(p);
        knots[i]->show();

        // This controls the center handle's position, because the default can
        // be moved and needs to be remembered.
        if( type == HANDLE_CENTER && _center )
            knots[i]->moveto(*_center);
    }
}

void Inkscape::SelTrans::_makeHandles()
{
    for (int i = 0; i < NUMHANDS; i++) {
        SPSelTransTypeInfo info = handtypes[hands[i].type];
        knots[i] = new SPKnot(_desktop, _(info.tip));

        knots[i]->setShape(SP_CTRL_SHAPE_BITMAP);
        knots[i]->setSize(13);
        knots[i]->setAnchor(hands[i].anchor);
        knots[i]->setMode(SP_CTRL_MODE_XOR);
        knots[i]->setFill(info.color[0], info.color[1], info.color[2]);
        knots[i]->setStroke(info.color[3], info.color[4], info.color[5]);
        knots[i]->setPixbuf(handles[hands[i].control]);
        knots[i]->updateCtrl();

        knots[i]->request_signal.connect(sigc::bind(sigc::ptr_fun(sp_sel_trans_handle_request), &hands[i]));
        knots[i]->moved_signal.connect(sigc::bind(sigc::ptr_fun(sp_sel_trans_handle_new_event), &hands[i]));
        knots[i]->grabbed_signal.connect(sigc::bind(sigc::ptr_fun(sp_sel_trans_handle_grab), &hands[i]));
        knots[i]->ungrabbed_signal.connect(sigc::bind(sigc::ptr_fun(sp_sel_trans_handle_ungrab), &hands[i]));
        knots[i]->click_signal.connect(sigc::bind(sigc::ptr_fun(sp_sel_trans_handle_click), &hands[i]));
        knots[i]->event_signal.connect(sigc::bind(sigc::ptr_fun(sp_sel_trans_handle_event), &hands[i]));
    }
}

static void sp_sel_trans_handle_grab(SPKnot *knot, guint state, SPSelTransHandle const* data)
{
    SP_SELECT_CONTEXT(knot->desktop->event_context)->_seltrans->handleGrab(
        knot, state, *(SPSelTransHandle const *) data
        );
}

static void sp_sel_trans_handle_ungrab(SPKnot *knot, guint /*state*/, SPSelTransHandle const* /*data*/)
{
    SP_SELECT_CONTEXT(knot->desktop->event_context)->_seltrans->ungrab();
}

static void sp_sel_trans_handle_new_event(SPKnot *knot, Geom::Point const& position, guint state, SPSelTransHandle const *data)
{
    Geom::Point pos = position;

    SP_SELECT_CONTEXT(knot->desktop->event_context)->_seltrans->handleNewEvent(
        knot, &pos, state, *(SPSelTransHandle const *) data
        );
}

static gboolean sp_sel_trans_handle_request(SPKnot *knot, Geom::Point *position, guint state, SPSelTransHandle const *data)
{
    return SP_SELECT_CONTEXT(knot->desktop->event_context)->_seltrans->handleRequest(
        knot, position, state, *(SPSelTransHandle const *) data
        );
}

static void sp_sel_trans_handle_click(SPKnot *knot, guint state, SPSelTransHandle const* data)
{
    SP_SELECT_CONTEXT(knot->desktop->event_context)->_seltrans->handleClick(
        knot, state, *(SPSelTransHandle const *) data
        );
}

void Inkscape::SelTrans::handleClick(SPKnot */*knot*/, guint state, SPSelTransHandle const &handle)
{
    switch (handle.type) {
        case HANDLE_CENTER:
            if (state & GDK_SHIFT_MASK) {
                // Unset the  center position for all selected items
            	std::vector<SPItem*> items=_desktop->selection->itemList();
                for ( std::vector<SPItem*>::const_iterator iter=items.begin();iter!=items.end();iter++ ) {
                    SPItem *it = *iter;
                    it->unsetCenter();
                    it->updateRepr();
                    _center_is_set = false;  // center has changed
                    _updateHandles();
                }
                DocumentUndo::done(_desktop->getDocument(), SP_VERB_CONTEXT_SELECT,
                                   _("Reset center"));
            }
            break;
        default:
            break;
    }
}

void Inkscape::SelTrans::handleGrab(SPKnot *knot, guint /*state*/, SPSelTransHandle const &handle)
{
    switch (handle.type) {
        case HANDLE_CENTER:
            g_object_set(G_OBJECT(_grip),
                         "shape", SP_CTRL_SHAPE_BITMAP,
                         "size", 13.0,
                         NULL);
            sp_canvas_item_show(_grip);
            break;
        default:
            g_object_set(G_OBJECT(_grip),
                         "shape", SP_CTRL_SHAPE_CROSS,
                         "size", 7.0,
                         NULL);
            sp_canvas_item_show(_norm);
            sp_canvas_item_show(_grip);
            break;
    }

    grab(knot->position(), handle.x, handle.y, FALSE, FALSE);
}


void Inkscape::SelTrans::handleNewEvent(SPKnot *knot, Geom::Point *position, guint state, SPSelTransHandle const &handle)
{
    if (!SP_KNOT_IS_GRABBED(knot)) {
        return;
    }

    // in case items have been unhooked from the document, don't
    // try to continue processing events for them.
    for (unsigned int i = 0; i < _items.size(); i++) {
        if ( !_items[i]->document ) {
            return;
        }
    }
    switch (handle.type) {
        case HANDLE_SCALE:
            scale(*position, state);
            break;
        case HANDLE_STRETCH:
            stretch(handle, *position, state);
            break;
        case HANDLE_SKEW:
            skew(handle, *position, state);
            break;
        case HANDLE_ROTATE:
            rotate(*position, state);
            break;
        case HANDLE_CENTER:
            setCenter(*position);
            break;
    }
}


gboolean Inkscape::SelTrans::handleRequest(SPKnot *knot, Geom::Point *position, guint state, SPSelTransHandle const &handle)
{
    if (!SP_KNOT_IS_GRABBED(knot))
        return TRUE;

    // When holding shift while rotating or skewing, the transformation will be
    // relative to the point opposite of the handle; otherwise it will be relative
    // to the center as set for the selection
    if ((!(state & GDK_SHIFT_MASK) == !(_state == STATE_ROTATE)) && (handle.type != HANDLE_CENTER)) {
        _origin = _opposite;
        _origin_for_bboxpoints = _opposite_for_bboxpoints;
        _origin_for_specpoints = _opposite_for_specpoints;
    } else if (_center) {
        _origin = *_center;
        _origin_for_bboxpoints = *_center;
        _origin_for_specpoints = *_center;
    } else {
        // FIXME
        return TRUE;
    }
    if (request(handle, *position, state)) {
        knot->setPosition(*position, state);
        SP_CTRL(_grip)->moveto(*position);
        if (handle.type == HANDLE_CENTER) {
            SP_CTRL(_norm)->moveto(*position);
        } else {
            SP_CTRL(_norm)->moveto(_origin);
        }
    }

    return TRUE;
}


void Inkscape::SelTrans::_selChanged(Inkscape::Selection */*selection*/)
{
    if (!_grabbed) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        // reread in case it changed on the fly:
        int prefs_bbox = prefs->getBool("/tools/bounding_box");
         _snap_bbox_type = !prefs_bbox ?
            SPItem::VISUAL_BBOX : SPItem::GEOMETRIC_BBOX;

        _updateVolatileState();
        _current_relative_affine.setIdentity();
        _center_is_set = false; // center(s) may have changed
        _updateHandles();
    }
}

void Inkscape::SelTrans::_selModified(Inkscape::Selection */*selection*/, guint /*flags*/)
{
    if (!_grabbed) {
        _updateVolatileState();
        _current_relative_affine.setIdentity();

        // reset internal flag
        _changed = false;

        _center_is_set = false;  // center(s) may have changed

        _updateHandles();
    }
}

void Inkscape::SelTrans::_boundingBoxPrefsChanged(int prefs_bbox)
{
    _snap_bbox_type = !prefs_bbox ?
        SPItem::VISUAL_BBOX : SPItem::GEOMETRIC_BBOX;

    _updateVolatileState();
    _updateHandles();
}

/*
 * handlers for handle move-request
 */

/** Returns -1 or 1 according to the sign of x.  Returns 1 for 0 and NaN. */
static double sign(double const x)
{
    return ( x < 0
             ? -1
             : 1 );
}

gboolean Inkscape::SelTrans::scaleRequest(Geom::Point &pt, guint state)
{

    // Calculate the scale factors, which can be either visual or geometric
    // depending on which type of bbox is currently being used (see preferences -> selector tool)
    Geom::Scale default_scale = calcScaleFactors(_point, pt, _origin);

    // Find the scale factors for the geometric bbox
    Geom::Point pt_geom = _getGeomHandlePos(pt);
    Geom::Scale geom_scale = calcScaleFactors(_point_geom, pt_geom, _origin_for_specpoints);

    _absolute_affine = Geom::identity(); //Initialize the scaler

    if (state & GDK_MOD1_MASK) { // scale by an integer multiplier/divider
        // We're scaling either the visual or the geometric bbox here (see the comment above)
        for ( unsigned int i = 0 ; i < 2 ; i++ ) {
            if (fabs(default_scale[i]) > 1) {
                default_scale[i] = round(default_scale[i]);
            } else if (default_scale[i] != 0) {
                default_scale[i] = 1/round(1/(MIN(default_scale[i], 10)));
            }
        }
        // Update the knot position
        pt = _calcAbsAffineDefault(default_scale);
        // When scaling by an integer, snapping is not needed
    } else {
        // In all other cases we should try to snap now
        Inkscape::PureScale  *bb, *sn;

        if ((state & GDK_CONTROL_MASK) || _desktop->isToolboxButtonActive ("lock")) {
            // Scale is locked to a 1:1 aspect ratio, so that s[X] must be made to equal s[Y].
            //
            // The aspect-ratio must be locked before snapping
            if (fabs(default_scale[Geom::X]) > fabs(default_scale[Geom::Y])) {
                default_scale[Geom::X] = fabs(default_scale[Geom::Y]) * sign(default_scale[Geom::X]);
                geom_scale[Geom::X] = fabs(geom_scale[Geom::Y]) * sign(geom_scale[Geom::X]);
            } else {
                default_scale[Geom::Y] = fabs(default_scale[Geom::X]) * sign(default_scale[Geom::Y]);
                geom_scale[Geom::Y] = fabs(geom_scale[Geom::X]) * sign(geom_scale[Geom::Y]);
            }

            // Snap along a suitable constraint vector from the origin.

            bb = new Inkscape::PureScaleConstrained(default_scale, _origin_for_bboxpoints);
            sn = new Inkscape::PureScaleConstrained(geom_scale, _origin_for_specpoints);
        } else {
            /* Scale aspect ratio is unlocked */
            bb = new Inkscape::PureScale(default_scale, _origin_for_bboxpoints, false);
            sn = new Inkscape::PureScale(geom_scale, _origin_for_specpoints, false);
        }
        SnapManager &m = _desktop->namedview->snap_manager;
        m.setup(_desktop, false, _items_const);
        m.snapTransformed(_bbox_points, _point, (*bb));
        m.snapTransformed(_snap_points, _point, (*sn));
        m.unSetup();

        // These lines below are duplicated in stretchRequest
        //TODO: Eliminate this code duplication
        if (bb->best_snapped_point.getSnapped() || sn->best_snapped_point.getSnapped()) {
            if (bb->best_snapped_point.getSnapped()) {
                if (!bb->best_snapped_point.isOtherSnapBetter(sn->best_snapped_point, false)) {
                    // We snapped the bbox (which is either visual or geometric)
                    _desktop->snapindicator->set_new_snaptarget(bb->best_snapped_point);
                    default_scale = bb->getScaleSnapped();
                    // Calculate the new transformation and update the handle position
                    pt = _calcAbsAffineDefault(default_scale);
                }
            } else if (sn->best_snapped_point.getSnapped()) {
                _desktop->snapindicator->set_new_snaptarget(sn->best_snapped_point);
                // We snapped the special points (e.g. nodes), which are not at the visual bbox
                // The handle location however (pt) might however be at the visual bbox, so we
                // will have to calculate pt taking the stroke width into account
                geom_scale = sn->getScaleSnapped();
                pt = _calcAbsAffineGeom(geom_scale);
            }
        } else {
            // We didn't snap at all! Don't update the handle position, just calculate the new transformation
            _calcAbsAffineDefault(default_scale);
            _desktop->snapindicator->remove_snaptarget();
        }

        delete bb;
        delete sn;
    }

    /* Status text */
    _message_context.setF(Inkscape::IMMEDIATE_MESSAGE,
                          _("<b>Scale</b>: %0.2f%% x %0.2f%%; with <b>Ctrl</b> to lock ratio"),
                          100 * _absolute_affine[0], 100 * _absolute_affine[3]);

    return TRUE;
}

gboolean Inkscape::SelTrans::stretchRequest(SPSelTransHandle const &handle, Geom::Point &pt, guint state)
{
    Geom::Dim2 axis, perp;
    switch (handle.cursor) {
        case GDK_TOP_SIDE:
        case GDK_BOTTOM_SIDE:
            axis = Geom::Y;
            perp = Geom::X;
            break;
        case GDK_LEFT_SIDE:
        case GDK_RIGHT_SIDE:
            axis = Geom::X;
            perp = Geom::Y;
            break;
        default:
            g_assert_not_reached();
            return TRUE;
    };

    // Calculate the scale factors, which can be either visual or geometric
    // depending on which type of bbox is currently being used (see preferences -> selector tool)
    Geom::Scale default_scale = calcScaleFactors(_point, pt, _origin);
    default_scale[perp] = 1;

    // Find the scale factors for the geometric bbox
    Geom::Point pt_geom = _getGeomHandlePos(pt);
    Geom::Scale geom_scale = calcScaleFactors(_point_geom, pt_geom, _origin_for_specpoints);
    geom_scale[perp] = 1;

    _absolute_affine = Geom::identity(); //Initialize the scaler

    if (state & GDK_MOD1_MASK) { // stretch by an integer multiplier/divider
        if (fabs(default_scale[axis]) > 1) {
            default_scale[axis] = round(default_scale[axis]);
        } else if (default_scale[axis] != 0) {
            default_scale[axis] = 1/round(1/(MIN(default_scale[axis], 10)));
        }
        // Calculate the new transformation and update the handle position
        pt = _calcAbsAffineDefault(default_scale);
        // When stretching by an integer, snapping is not needed
    } else {
        // In all other cases we should try to snap now

        SnapManager &m = _desktop->namedview->snap_manager;
        m.setup(_desktop, false, _items_const);

        bool symmetrical = state & GDK_CONTROL_MASK;

        Inkscape::PureStretchConstrained bb = Inkscape::PureStretchConstrained(Geom::Coord(default_scale[axis]), _origin_for_bboxpoints, Geom::Dim2(axis), symmetrical);
        Inkscape::PureStretchConstrained sn = Inkscape::PureStretchConstrained(Geom::Coord(geom_scale[axis]), _origin_for_specpoints, Geom::Dim2(axis), symmetrical);

        m.snapTransformed(_bbox_points, _point, bb);
        m.snapTransformed(_snap_points, _point, sn);
        m.unSetup();

        if (bb.best_snapped_point.getSnapped()) {
            // We snapped the bbox (which is either visual or geometric)
            default_scale[axis] = bb.getMagnitude();
        }

        if (sn.best_snapped_point.getSnapped()) {
            geom_scale[axis] = sn.getMagnitude();
        }

        if (symmetrical) {
            // on ctrl, apply symmetrical scaling instead of stretching
            // Preserve aspect ratio, but never flip in the dimension not being edited (by using fabs())
            default_scale[perp] = fabs(default_scale[axis]);
            geom_scale[perp] = fabs(geom_scale[axis]);
        }

        // These lines below are duplicated in scaleRequest
        if (bb.best_snapped_point.getSnapped() || sn.best_snapped_point.getSnapped()) {
            if (bb.best_snapped_point.getSnapped()) {
                if (!bb.best_snapped_point.isOtherSnapBetter(sn.best_snapped_point, false)) {
                    // We snapped the bbox (which is either visual or geometric)
                    _desktop->snapindicator->set_new_snaptarget(bb.best_snapped_point);
                    default_scale = bb.getStretchSnapped();
                    // Calculate the new transformation and update the handle position
                    pt = _calcAbsAffineDefault(default_scale);
                }
            } else if (sn.best_snapped_point.getSnapped()) {
                _desktop->snapindicator->set_new_snaptarget(sn.best_snapped_point);
                // We snapped the special points (e.g. nodes), which are not at the visual bbox
                // The handle location however (pt) might however be at the visual bbox, so we
                // will have to calculate pt taking the stroke width into account
                geom_scale = sn.getStretchSnapped();
                pt = _calcAbsAffineGeom(geom_scale);
            }
        } else {
            // We didn't snap at all! Don't update the handle position, just calculate the new transformation
            _calcAbsAffineDefault(default_scale);
            _desktop->snapindicator->remove_snaptarget();
        }
    }

    // status text
    _message_context.setF(Inkscape::IMMEDIATE_MESSAGE,
                          _("<b>Scale</b>: %0.2f%% x %0.2f%%; with <b>Ctrl</b> to lock ratio"),
                          100 * _absolute_affine[0], 100 * _absolute_affine[3]);

    return TRUE;
}

gboolean Inkscape::SelTrans::request(SPSelTransHandle const &handle, Geom::Point &pt, guint state)
{
    // These _should_ be in the handstype somewhere instead
    switch (handle.type) {
        case HANDLE_SCALE:
            return scaleRequest(pt, state);
        case HANDLE_STRETCH:
            return stretchRequest(handle, pt, state);
        case HANDLE_SKEW:
            return skewRequest(handle, pt, state);
        case HANDLE_ROTATE:
            return rotateRequest(pt, state);
        case HANDLE_CENTER:
            return centerRequest(pt, state);
    }
    return FALSE;
}

gboolean Inkscape::SelTrans::skewRequest(SPSelTransHandle const &handle, Geom::Point &pt, guint state)
{
    /* When skewing (or rotating):
     * 1) the stroke width will not change. This makes life much easier because we don't have to
     *    account for that (like for scaling or stretching). As a consequence, all points will
     *    have the same origin for the transformation and for the snapping.
     * 2) When holding shift, the transformation will be relative to the point opposite of
     *    the handle; otherwise it will be relative to the center as set for the selection
     */

    Geom::Dim2 dim_a;
    Geom::Dim2 dim_b;

    switch (handle.cursor) {
        case GDK_SB_H_DOUBLE_ARROW:
            dim_a = Geom::Y;
            dim_b = Geom::X;
            break;
        case GDK_SB_V_DOUBLE_ARROW:
            dim_a = Geom::X;
            dim_b = Geom::Y;
            break;
        default:
            g_assert_not_reached();
            abort();
            break;
    }

    // _point and _origin are noisy, ranging from 1 to 1e-9 or even smaller; this is due to the
    // limited SVG output precision, which can be arbitrarily set in the preferences
    Geom::Point const initial_delta = _point - _origin;

    // The handle and the origin shouldn't be too close to each other; let's check for that!
    // Due to the limited resolution though (see above), we'd better use a relative error here
    if (_bbox) {
        Geom::Coord d = (*_bbox).dimensions()[dim_a];
        if (fabs(initial_delta[dim_a]/d) < 1e-4) {
            return false;
        }
    }

    // Calculate the scale factors, which can be either visual or geometric
    // depending on which type of bbox is currently being used (see preferences -> selector tool)
    Geom::Scale scale = calcScaleFactors(_point, pt, _origin, false);
    Geom::Scale skew = calcScaleFactors(_point, pt, _origin, true);
    scale[dim_b] = 1;
    skew[dim_b] = 1;

    if (fabs(scale[dim_a]) < 1) {
        // Prevent shrinking of the selected object, while allowing mirroring
        scale[dim_a] = sign(scale[dim_a]);
    } else {
        // Allow expanding of the selected object by integer multiples
        scale[dim_a] = floor(scale[dim_a] + 0.5);
    }

    double radians = atan(skew[dim_a] / scale[dim_a]);

    if (state & GDK_CONTROL_MASK) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        // Snap to defined angle increments
        int snaps = prefs->getInt("/options/rotationsnapsperpi/value", 12);
        if (snaps) {
            double sections = floor(radians * snaps / M_PI + .5);
            if (fabs(sections) >= snaps / 2) {
                sections = sign(sections) * (snaps / 2 - 1);
            }
            radians = (M_PI / snaps) * sections;
        }
        skew[dim_a] = tan(radians) * scale[dim_a];
    } else {
        // Snap to objects, grids, guides

        SnapManager &m = _desktop->namedview->snap_manager;
        m.setup(_desktop, false, _items_const);

        // When skewing, we cannot snap the corners of the bounding box, see the comment in PureSkewConstrained for details
        Inkscape::PureSkewConstrained sn = Inkscape::PureSkewConstrained(skew[dim_a], scale[dim_a], _origin, Geom::Dim2(dim_b));
        m.snapTransformed(_snap_points, _point, sn);

        if (sn.best_snapped_point.getSnapped()) {
            // We snapped something, so change the skew to reflect it
            skew[dim_a] = sn.getSkewSnapped();
             _desktop->snapindicator->set_new_snaptarget(sn.best_snapped_point);
        } else {
            _desktop->snapindicator->remove_snaptarget();
        }

        m.unSetup();
    }

    // Update the handle position
    pt[dim_b] = initial_delta[dim_a] * skew[dim_a] + _point[dim_b];
    pt[dim_a] = initial_delta[dim_a] * scale[dim_a] + _origin[dim_a];

    // Calculate the relative affine
    _relative_affine = Geom::identity();
    _relative_affine[2*dim_a + dim_a] = (pt[dim_a] - _origin[dim_a]) / initial_delta[dim_a];
    _relative_affine[2*dim_a + (dim_b)] = (pt[dim_b] - _point[dim_b]) / initial_delta[dim_a];
    _relative_affine[2*(dim_b) + (dim_a)] = 0;
    _relative_affine[2*(dim_b) + (dim_b)] = 1;

    for (int i = 0; i < 2; i++) {
        if (fabs(_relative_affine[3*i]) < 1e-15) {
            _relative_affine[3*i] = 1e-15;
        }
    }

    // Update the status text
    double degrees = mod360symm(Geom::rad_to_deg(radians));
    _message_context.setF(Inkscape::IMMEDIATE_MESSAGE,
                          // TRANSLATORS: don't modify the first ";"
                          // (it will NOT be displayed as ";" - only the second one will be)
                          _("<b>Skew</b>: %0.2f&#176;; with <b>Ctrl</b> to snap angle"),
                          degrees);

    return TRUE;
}

gboolean Inkscape::SelTrans::rotateRequest(Geom::Point &pt, guint state)
{
    /* When rotating (or skewing):
     * 1) the stroke width will not change. This makes life much easier because we don't have to
     *    account for that (like for scaling or stretching). As a consequence, all points will
     *    have the same origin for the transformation and for the snapping.
     * 2) When holding shift, the transformation will be relative to the point opposite of
     *    the handle; otherwise it will be relative to the center as set for the selection
     */

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int snaps = prefs->getInt("/options/rotationsnapsperpi/value", 12);

    // rotate affine in rotate
    Geom::Point const d1 = _point - _origin;
    Geom::Point const d2 = pt     - _origin;

    Geom::Coord const h1 = Geom::L2(d1); // initial radius
    if (h1 < 1e-15) return FALSE;
    Geom::Point q1 = d1 / h1; // normalized initial vector to handle
    Geom::Coord const h2 = Geom::L2(d2); // new radius
    if (fabs(h2) < 1e-15) return FALSE;
    Geom::Point q2 = d2 / h2; // normalized new vector to handle

    Geom::Rotate r1(q1);
    Geom::Rotate r2(q2);

    double radians = atan2(Geom::dot(Geom::rot90(d1), d2), Geom::dot(d1, d2));;
    if (state & GDK_CONTROL_MASK) {
        // Snap to defined angle increments
        double cos_t = Geom::dot(q1, q2);
        double sin_t = Geom::dot(Geom::rot90(q1), q2);
        radians = atan2(sin_t, cos_t);
        if (snaps) {
            radians = ( M_PI / snaps ) * floor( radians * snaps / M_PI + .5 );
        }
        r1 = Geom::Rotate(0); //q1 = Geom::Point(1, 0);
        r2 = Geom::Rotate(radians); //q2 = Geom::Point(cos(radians), sin(radians));
    } else {
        SnapManager &m = _desktop->namedview->snap_manager;
        m.setup(_desktop, false, _items_const);
        // When rotating, we cannot snap the corners of the bounding box, see the comment in "constrainedSnapRotate" for details
        Inkscape::PureRotateConstrained sn = Inkscape::PureRotateConstrained(radians, _origin);
        m.snapTransformed(_snap_points, _point, sn);
        m.unSetup();

        if (sn.best_snapped_point.getSnapped()) {
            _desktop->snapindicator->set_new_snaptarget(sn.best_snapped_point);
            // We snapped something, so change the rotation to reflect it
            radians = sn.getAngleSnapped();
            r1 = Geom::Rotate(0);
            r2 = Geom::Rotate(radians);
        } else {
            _desktop->snapindicator->remove_snaptarget();
        }

    }


    // Calculate the relative affine
    _relative_affine = r2 * r1.inverse();

    // Update the handle position
    pt = _point * Geom::Translate(-_origin) * _relative_affine * Geom::Translate(_origin);

    // Update the status text
    double degrees = mod360symm(Geom::rad_to_deg(radians));
    _message_context.setF(Inkscape::IMMEDIATE_MESSAGE,
                          // TRANSLATORS: don't modify the first ";"
                          // (it will NOT be displayed as ";" - only the second one will be)
                          _("<b>Rotate</b>: %0.2f&#176;; with <b>Ctrl</b> to snap angle"), degrees);

    return TRUE;
}

// Move the item's transformation center
gboolean Inkscape::SelTrans::centerRequest(Geom::Point &pt, guint state)
{
    // When dragging the transformation center while multiple items have been selected, then those
    // items will share a single center. While dragging that single center, it should never snap to the
    // centers of any of the selected objects. Therefore we will have to pass the list of selected items
    // to the snapper, to avoid self-snapping of the rotation center
	std::vector<SPItem*> items = const_cast<Selection *>(_selection)->itemList();
    SnapManager &m = _desktop->namedview->snap_manager;
    m.setup(_desktop);
    m.setRotationCenterSource(items);

    if (state & GDK_CONTROL_MASK) { // with Ctrl, constrain to axes
        std::vector<Inkscape::Snapper::SnapConstraint> constraints;
        constraints.push_back(Inkscape::Snapper::SnapConstraint(_point, Geom::Point(1, 0)));
        constraints.push_back(Inkscape::Snapper::SnapConstraint(_point, Geom::Point(0, 1)));
        Inkscape::SnappedPoint sp = m.multipleConstrainedSnaps(Inkscape::SnapCandidatePoint(pt, Inkscape::SNAPSOURCE_ROTATION_CENTER), constraints, state & GDK_SHIFT_MASK);
        pt = sp.getPoint();
    }
    else {
        if (!(state & GDK_SHIFT_MASK)) { // Shift disables snapping
            m.freeSnapReturnByRef(pt, Inkscape::SNAPSOURCE_ROTATION_CENTER);
        }
    }

    m.unSetup();

    // status text
    Inkscape::Util::Quantity x_q = Inkscape::Util::Quantity(pt[Geom::X], "px");
    Inkscape::Util::Quantity y_q = Inkscape::Util::Quantity(pt[Geom::Y], "px");
    GString *xs = g_string_new(x_q.string(_desktop->namedview->display_units).c_str());
    GString *ys = g_string_new(y_q.string(_desktop->namedview->display_units).c_str());
    _message_context.setF(Inkscape::NORMAL_MESSAGE, _("Move <b>center</b> to %s, %s"), xs->str, ys->str);
    g_string_free(xs, FALSE);
    g_string_free(ys, FALSE);

    return TRUE;
}

/*
 * handlers for handle movement
 *
 */



void Inkscape::SelTrans::stretch(SPSelTransHandle const &/*handle*/, Geom::Point &/*pt*/, guint /*state*/)
{
    transform(_absolute_affine, Geom::Point(0, 0)); // we have already accounted for origin, so pass 0,0
}

void Inkscape::SelTrans::scale(Geom::Point &/*pt*/, guint /*state*/)
{
    transform(_absolute_affine, Geom::Point(0, 0)); // we have already accounted for origin, so pass 0,0
}

void Inkscape::SelTrans::skew(SPSelTransHandle const &/*handle*/, Geom::Point &/*pt*/, guint /*state*/)
{
    transform(_relative_affine, _origin);
}

void Inkscape::SelTrans::rotate(Geom::Point &/*pt*/, guint /*state*/)
{
    transform(_relative_affine, _origin);
}

void Inkscape::SelTrans::moveTo(Geom::Point const &xy, guint state)
{
    SnapManager &m = _desktop->namedview->snap_manager;

    /* The amount that we've moved by during this drag */
    Geom::Point dxy = xy - _point;

    bool const alt = (state & GDK_MOD1_MASK);
    bool const control = (state & GDK_CONTROL_MASK);
    bool const shift = (state & GDK_SHIFT_MASK);

    if (control) { // constrained to the orthogonal axes
        if (fabs(dxy[Geom::X]) > fabs(dxy[Geom::Y])) {
            dxy[Geom::Y] = 0;
        } else {
            dxy[Geom::X] = 0;
        }
    }

    if (alt) {// Alt pressed means: move only by integer multiples of the grid spacing
        m.setup(_desktop, true, _items_const);
        dxy = m.multipleOfGridPitch(dxy, _point);
        m.unSetup();
    } else if (!shift) { //!shift: with snapping
        /* We're snapping to things, possibly with a constraint to horizontal or
        ** vertical movement.  Obtain a list of possible translations and then
        ** pick the smallest.
        */

        m.setup(_desktop, false, _items_const);

        /* This will be our list of possible translations */
        std::list<Inkscape::SnappedPoint> s;

        Inkscape::PureTranslate *bb, *sn;

        if (control) { // constrained movement with snapping

            /* Snap to things, and also constrain to horizontal or vertical movement */

            Geom::Dim2 dim = fabs(dxy[Geom::X]) > fabs(dxy[Geom::Y]) ? Geom::X : Geom::Y;
            // When doing a constrained translation, all points will move in the same direction, i.e.
            // either horizontally or vertically. Therefore we only have to specify the direction of
            // the constraint-line once. The constraint lines are parallel, but might not be colinear.
            // Therefore we will have to set the point through which the constraint-line runs
            // individually for each point to be snapped; this will be handled however by snapTransformed()
            bb = new Inkscape::PureTranslateConstrained(dxy[dim], dim);
            sn = new Inkscape::PureTranslateConstrained(dxy[dim], dim);
        } else { // !control
            /* Snap to things with no constraint */
            bb = new Inkscape::PureTranslate(dxy);
            sn = new Inkscape::PureTranslate(dxy);
        }
        // Let's leave this timer code here for a while. I'll probably need it in the near future (Diederik van Lierop)
        /* GTimeVal starttime;
        GTimeVal endtime;
        g_get_current_time(&starttime); */

        m.snapTransformed(_bbox_points, _point, (*bb));
        m.snapTransformed(_snap_points, _point, (*sn));
        m.unSetup();

        /*g_get_current_time(&endtime);
        double elapsed = ((((double)endtime.tv_sec - starttime.tv_sec) * G_USEC_PER_SEC + (endtime.tv_usec - starttime.tv_usec))) / 1000.0;
        std::cout << "Time spent snapping: " << elapsed << std::endl; */

        /* Pick one */
        Inkscape::SnappedPoint best_snapped_point;

        bool sn_is_best = sn->best_snapped_point.getSnapped();
        bool bb_is_best = bb->best_snapped_point.getSnapped();

        if (bb_is_best && sn_is_best) {
            sn_is_best = bb->best_snapped_point.isOtherSnapBetter(sn->best_snapped_point, true);
            bb_is_best = !sn_is_best;
        }

        if (sn_is_best) {
            best_snapped_point = sn->best_snapped_point;
            dxy = sn->getTranslationSnapped();
        } else if (bb_is_best) {
            best_snapped_point = bb->best_snapped_point;
            dxy = bb->getTranslationSnapped();
        }

        if (best_snapped_point.getSnapped()) {
            _desktop->snapindicator->set_new_snaptarget(best_snapped_point);
        } else {
            // We didn't snap, so remove any previous snap indicator
            _desktop->snapindicator->remove_snaptarget();
            if (control) {
                // If we didn't snap, then we should still constrain horizontally or vertically
                // (When we did snap, then this constraint has already been enforced by
                // calling constrainedSnapTranslate() above)
                if (fabs(dxy[Geom::X]) > fabs(dxy[Geom::Y])) {
                    dxy[Geom::Y] = 0;
                } else {
                    dxy[Geom::X] = 0;
                }
            }
        }
    }

    Geom::Affine const move((Geom::Translate(dxy)));
    Geom::Point const norm(0, 0);
    transform(move, norm);

    // status text
    Inkscape::Util::Quantity x_q = Inkscape::Util::Quantity(dxy[Geom::X], "px");
    Inkscape::Util::Quantity y_q = Inkscape::Util::Quantity(dxy[Geom::Y], "px");
    GString *xs = g_string_new(x_q.string(_desktop->namedview->display_units).c_str());
    GString *ys = g_string_new(y_q.string(_desktop->namedview->display_units).c_str());
    _message_context.setF(Inkscape::NORMAL_MESSAGE, _("<b>Move</b> by %s, %s; with <b>Ctrl</b> to restrict to horizontal/vertical; with <b>Shift</b> to disable snapping"), xs->str, ys->str);
    g_string_free(xs, TRUE);
    g_string_free(ys, TRUE);
}

// Given a location of a handle at the visual bounding box, find the corresponding location at the
// geometrical bounding box
Geom::Point Inkscape::SelTrans::_getGeomHandlePos(Geom::Point const &visual_handle_pos)
{
    if ( _snap_bbox_type == SPItem::GEOMETRIC_BBOX) {
        // When the selector tool is using geometric bboxes, then the handle is already
        // located at one of the geometric bbox corners
        return visual_handle_pos;
    }

    if (!_geometric_bbox) {
        //_getGeomHandlePos() can only be used after _geometric_bbox has been defined!
        return visual_handle_pos;
    }

    // Using the Geom::Rect constructor below ensures that "min() < max()", which is important
    // because this will also hold for _bbox, and which is required for get_scale_transform_for_stroke()
    Geom::Rect new_bbox = Geom::Rect(_origin_for_bboxpoints, visual_handle_pos); // new visual bounding box
    // Please note that the new_bbox might in fact be just a single line, for example when stretching (in
    // which case the handle and origin will be aligned vertically or horizontally)
    Geom::Point normalized_handle_pos = (visual_handle_pos - new_bbox.min()) * Geom::Scale(new_bbox.dimensions()).inverse();

    // Calculate the absolute affine while taking into account the scaling of the stroke width
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool transform_stroke = prefs->getBool("/options/transform/stroke", true);
    bool preserve = prefs->getBool("/options/preservetransform/value", false);
    Geom::Affine abs_affine = get_scale_transform_for_uniform_stroke (*_bbox, _strokewidth, _strokewidth, transform_stroke, preserve,
                    new_bbox.min()[Geom::X], new_bbox.min()[Geom::Y], new_bbox.max()[Geom::X], new_bbox.max()[Geom::Y]);

    // Calculate the scaled geometrical bbox
    Geom::Rect new_geom_bbox = Geom::Rect(_geometric_bbox->min() * abs_affine, _geometric_bbox->max() * abs_affine);
    // Find the location of the handle on this new geometrical bbox
    return normalized_handle_pos * Geom::Scale(new_geom_bbox.dimensions()) + new_geom_bbox.min(); //new position of the geometric handle
}

Geom::Scale Inkscape::calcScaleFactors(Geom::Point const &initial_point, Geom::Point const &new_point, Geom::Point const &origin, bool const skew)
{
    // Work out the new scale factors for the bbox

    Geom::Point const initial_delta = initial_point - origin;
    Geom::Point const new_delta = new_point - origin;
    Geom::Point const offset = new_point - initial_point;
    Geom::Scale scale(1, 1);

    for ( unsigned int i = 0 ; i < 2 ; i++ ) {
        if ( fabs(initial_delta[i]) > 1e-6 ) {
            if (skew) {
                scale[i] = offset[1-i] / initial_delta[i];
            } else {
                scale[i] = new_delta[i] / initial_delta[i];
            }
        }
    }

    return scale;
}

// Only for scaling/stretching
Geom::Point Inkscape::SelTrans::_calcAbsAffineDefault(Geom::Scale const default_scale)
{
    Geom::Affine abs_affine = Geom::Translate(-_origin) * Geom::Affine(default_scale) * Geom::Translate(_origin);
    Geom::Point new_bbox_min = _visual_bbox->min() * abs_affine;
    Geom::Point new_bbox_max = _visual_bbox->max() * abs_affine;

    bool transform_stroke = false;
    bool preserve = false;
    gdouble stroke_x = 0;
    gdouble stroke_y = 0;

    if ( _snap_bbox_type != SPItem::GEOMETRIC_BBOX) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        transform_stroke = prefs->getBool("/options/transform/stroke", true);
        preserve = prefs->getBool("/options/preservetransform/value", false);
        stroke_x = _visual_bbox->width() - _geometric_bbox->width();
        stroke_y = _visual_bbox->height() - _geometric_bbox->height();
    }

    _absolute_affine = get_scale_transform_for_uniform_stroke (*_visual_bbox, stroke_x, stroke_y, transform_stroke, preserve,
                    new_bbox_min[Geom::X], new_bbox_min[Geom::Y], new_bbox_max[Geom::X], new_bbox_max[Geom::Y]);

    // return the new handle position
    return ( _point - _origin ) * default_scale + _origin;
}

// Only for scaling/stretching
Geom::Point Inkscape::SelTrans::_calcAbsAffineGeom(Geom::Scale const geom_scale)
{
    _relative_affine = Geom::Affine(geom_scale);
    _absolute_affine = Geom::Translate(-_origin_for_specpoints) * _relative_affine * Geom::Translate(_origin_for_specpoints);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool const transform_stroke = prefs->getBool("/options/transform/stroke", true);
    if (_geometric_bbox) {
        Geom::Rect visual_bbox = get_visual_bbox(_geometric_bbox, _absolute_affine, _strokewidth, transform_stroke);
        // return the new handle position
        return visual_bbox.min() + visual_bbox.dimensions() * Geom::Scale(_handle_x, _handle_y);
    }

    // Fall back scenario, in case we don't have a geometric bounding box at hand;
    // (Due to some bugs related to bounding boxes having at least one zero dimension; For more details
    // see https://bugs.launchpad.net/inkscape/+bug/318726)
    g_warning("No geometric bounding box has been calculated; this is a bug that needs fixing!");
    return _calcAbsAffineDefault(geom_scale); // this is bogus, but we must return _something_
}

void Inkscape::SelTrans::_keepClosestPointOnly(Geom::Point const &p)
{
    SnapManager const &m = _desktop->namedview->snap_manager;

    // If we're not going to snap nodes, then we might just as well get rid of their snappoints right away
    if (!(m.snapprefs.isTargetSnappable(SNAPTARGET_NODE_CATEGORY, SNAPTARGET_OTHERS_CATEGORY) || m.snapprefs.isAnyDatumSnappable())) {
        _snap_points.clear();
    }

    // If we're not going to snap bounding boxes, then we might just as well get rid of their snappoints right away
    if (!m.snapprefs.isTargetSnappable(SNAPTARGET_BBOX_CATEGORY)) {
        _bbox_points.clear();
    }

    _all_snap_sources_sorted = _snap_points;
    _all_snap_sources_sorted.insert(_all_snap_sources_sorted.end(), _bbox_points.begin(), _bbox_points.end());

    // Calculate and store the distance to the reference point for each snap candidate point
    for(std::vector<Inkscape::SnapCandidatePoint>::iterator i = _all_snap_sources_sorted.begin(); i != _all_snap_sources_sorted.end(); ++i) {
        (*i).setDistance(Geom::L2((*i).getPoint() - p));
    }

    // Sort them ascending, using the distance calculated above as the single criteria
    std::sort(_all_snap_sources_sorted.begin(), _all_snap_sources_sorted.end());

    // Now get the closest snap source
    _snap_points.clear();
    _bbox_points.clear();
    if (!_all_snap_sources_sorted.empty()) {
        _all_snap_sources_iter = _all_snap_sources_sorted.begin();
        if (_all_snap_sources_sorted.front().getSourceType() & SNAPSOURCE_BBOX_CATEGORY) {
            _bbox_points.push_back(_all_snap_sources_sorted.front());
        } else {
            _snap_points.push_back(_all_snap_sources_sorted.front());
        }
    }

}
// TODO: This code is duplicated in transform-handle-set.cpp; fix this!
void Inkscape::SelTrans::getNextClosestPoint(bool reverse)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/options/snapclosestonly/value", false)) {
        if (!_all_snap_sources_sorted.empty()) {
            if (reverse) { // Shift-tab will find a closer point
                if (_all_snap_sources_iter == _all_snap_sources_sorted.begin()) {
                    _all_snap_sources_iter = _all_snap_sources_sorted.end();
                }
                --_all_snap_sources_iter;
            } else { // Tab will find a point further away
                ++_all_snap_sources_iter;
                if (_all_snap_sources_iter == _all_snap_sources_sorted.end()) {
                    _all_snap_sources_iter = _all_snap_sources_sorted.begin();
                }
            }

            _snap_points.clear();
            _bbox_points.clear();

            if ((*_all_snap_sources_iter).getSourceType() & SNAPSOURCE_BBOX_CATEGORY) {
                _bbox_points.push_back(*_all_snap_sources_iter);
            } else {
                _snap_points.push_back(*_all_snap_sources_iter);
            }

            // Show the updated snap source now; otherwise it won't be shown until the selection is being moved again
            SnapManager &m = _desktop->namedview->snap_manager;
            m.setup(_desktop);
            m.displaySnapsource(*_all_snap_sources_iter);
            m.unSetup();
        }
    }
}

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
