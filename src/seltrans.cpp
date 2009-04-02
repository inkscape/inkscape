/** @file
 * @brief Helper object for transforming selected items
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Carl Hetherington <inkscape@carlh.net>
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 1999-2008 Authors
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
#include "sp-namedview.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-style.h"
#include "knot.h"
#include "snap.h"
#include "selection.h"
#include "select-context.h"
#include "sp-item.h"
#include "sp-item-transform.h"
#include "seltrans-handles.h"
#include "seltrans.h"
#include "selection-chemistry.h"
#include "sp-metrics.h"
#include "verbs.h"
#include <glibmm/i18n.h>
#include "display/sp-ctrlline.h"
#include "preferences.h"
#include "xml/repr.h"
#include "mod360.h"
#include <2geom/angle.h>
#include "display/snap-indicator.h"


static void sp_remove_handles(SPKnot *knot[], gint num);

static void sp_sel_trans_handle_grab(SPKnot *knot, guint state, gpointer data);
static void sp_sel_trans_handle_ungrab(SPKnot *knot, guint state, gpointer data);
static void sp_sel_trans_handle_click(SPKnot *knot, guint state, gpointer data);
static void sp_sel_trans_handle_new_event(SPKnot *knot, Geom::Point *position, guint32 state, gpointer data);
static gboolean sp_sel_trans_handle_request(SPKnot *knot, Geom::Point *p, guint state, gboolean *data);

extern GdkPixbuf *handles[];

static gboolean sp_seltrans_handle_event(SPKnot *knot, GdkEvent *event, gpointer)
{
    switch (event->type) {
        case GDK_MOTION_NOTIFY:
            break;
        case GDK_KEY_PRESS:
            if (get_group0_keyval (&event->key) == GDK_space) {
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

Inkscape::SelTrans::SelTrans(SPDesktop *desktop) :
    _desktop(desktop),
    _selcue(desktop),
    _state(STATE_SCALE),
    _show(SHOW_CONTENT),
    _grabbed(false),
    _show_handles(true),
    _bbox(),
    _approximate_bbox(),
    _absolute_affine(Geom::Scale(1,1)),
    _opposite(Geom::Point(0,0)),
    _opposite_for_specpoints(Geom::Point(0,0)),
    _opposite_for_bboxpoints(Geom::Point(0,0)),
    _origin_for_specpoints(Geom::Point(0,0)),
    _origin_for_bboxpoints(Geom::Point(0,0)),
    _chandle(NULL),
    _stamp_cache(NULL),
    _message_context(desktop->messageStack())
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int prefs_bbox = prefs->getBool("/tools/bounding_box");
    _snap_bbox_type = !prefs_bbox ?
        SPItem::APPROXIMATE_BBOX : SPItem::GEOMETRIC_BBOX;

    g_return_if_fail(desktop != NULL);

    for (int i = 0; i < 8; i++) {
        _shandle[i] = NULL;
        _rhandle[i] = NULL;
    }

    _updateVolatileState();
    _current_relative_affine.setIdentity();

    _center_is_set = false; // reread _center from items, or set to bbox midpoint

    _updateHandles();

    _selection = sp_desktop_selection(desktop);

    _norm = sp_canvas_item_new(sp_desktop_controls(desktop),
                               SP_TYPE_CTRL,
                               "anchor", GTK_ANCHOR_CENTER,
                               "mode", SP_CTRL_MODE_COLOR,
                               "shape", SP_CTRL_SHAPE_BITMAP,
                               "size", 13.0,
                               "filled", TRUE,
                               "fill_color", 0x00000000,
                               "stroked", TRUE,
                               "stroke_color", 0x000000a0,
                               "pixbuf", handles[12],
                               NULL);

    _grip = sp_canvas_item_new(sp_desktop_controls(desktop),
                               SP_TYPE_CTRL,
                               "anchor", GTK_ANCHOR_CENTER,
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
        _l[i] = sp_canvas_item_new(sp_desktop_controls(desktop), SP_TYPE_CTRLLINE, NULL);
        sp_canvas_item_hide(_l[i]);
    }

    _sel_changed_connection = _selection->connectChanged(
        sigc::mem_fun(*this, &Inkscape::SelTrans::_selChanged)
        );

    _sel_modified_connection = _selection->connectModified(
        sigc::mem_fun(*this, &Inkscape::SelTrans::_selModified)
        );
}

Inkscape::SelTrans::~SelTrans()
{
    _sel_changed_connection.disconnect();
    _sel_modified_connection.disconnect();

    for (unsigned int i = 0; i < 8; i++) {
        if (_shandle[i]) {
            g_object_unref(G_OBJECT(_shandle[i]));
            _shandle[i] = NULL;
        }
        if (_rhandle[i]) {
            g_object_unref(G_OBJECT(_rhandle[i]));
            _rhandle[i] = NULL;
        }
    }
    if (_chandle) {
        g_object_unref(G_OBJECT(_chandle));
        _chandle = NULL;
    }

    if (_norm) {
        gtk_object_destroy(GTK_OBJECT(_norm));
        _norm = NULL;
    }
    if (_grip) {
        gtk_object_destroy(GTK_OBJECT(_grip));
        _grip = NULL;
    }
    for (int i = 0; i < 4; i++) {
        if (_l[i]) {
            gtk_object_destroy(GTK_OBJECT(_l[i]));
            _l[i] = NULL;
        }
    }

    for (unsigned i = 0; i < _items.size(); i++) {
        sp_object_unref(SP_OBJECT(_items[i]), NULL);
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
    for (GSList const *l = _desktop->selection->itemList(); l; l = l->next) {
        SPItem *it = (SPItem*)SP_OBJECT(l->data);
        it->setCenter(p);
        // only set the value; updating repr and document_done will be done once, on ungrab
    }

    _updateHandles();
}

void Inkscape::SelTrans::grab(Geom::Point const &p, gdouble x, gdouble y, bool show_handles)
{
    Inkscape::Selection *selection = sp_desktop_selection(_desktop);

    g_return_if_fail(!_grabbed);

    _grabbed = true;
    _show_handles = show_handles;
    _updateVolatileState();
    _current_relative_affine.setIdentity();

    _changed = false;

    if (_empty) {
        return;
    }

    for (GSList const *l = selection->itemList(); l; l = l->next) {
        SPItem *it = (SPItem *)sp_object_ref(SP_OBJECT(l->data), NULL);
        _items.push_back(it);
        _items_const.push_back(it);
        _items_affines.push_back(sp_item_i2d_affine(it));
        _items_centers.push_back(it->getCenter()); // for content-dragging, we need to remember original centers
    }

    _handle_x = x;
    _handle_y = y;

    // The selector tool should snap the bbox, special snappoints, and path nodes
    // (The special points are the handles, center, rotation axis, font baseline, ends of spiral, etc.)

    // First, determine the bounding box for snapping ...
    _bbox = selection->bounds(_snap_bbox_type);
    _approximate_bbox = selection->bounds(SPItem::APPROXIMATE_BBOX); // Used for correctly scaling the strokewidth
    _geometric_bbox = selection->bounds(SPItem::GEOMETRIC_BBOX);
    _point = p;
    if (_geometric_bbox) {
        _point_geom = _geometric_bbox->min() + _geometric_bbox->dimensions() * Geom::Scale(x, y);
    } else {
        _point_geom = p;
    }

    // Next, get all points to consider for snapping
    SnapManager const &m = _desktop->namedview->snap_manager;
	Inkscape::SnapPreferences local_snapprefs = m.snapprefs;
	local_snapprefs.setSnapToItemNode(true); // We should get at least the cusp nodes here. This might
	// have been turned off because (for example) the user only want paths as a snap target, not nodes
	// but as a snap source we still need some nodes though!
    _snap_points.clear();
	_snap_points = selection->getSnapPoints(&local_snapprefs);
	std::vector<std::pair<Geom::Point, int> > snap_points_hull = selection->getSnapPointsConvexHull(&local_snapprefs);
	if (_snap_points.size() > 100) {
		/* Snapping a huge number of nodes will take way too long, so limit the number of snappable nodes
		An average user would rarely ever try to snap such a large number of nodes anyway, because
		(s)he could hardly discern which node would be snapping */
		_snap_points = snap_points_hull;
		// Unfortunately, by now we will have lost the font-baseline snappoints :-(
	}

    // Find bbox hulling all special points, which excludes stroke width. Here we need to include the
    // path nodes, for example because a rectangle which has been converted to a path doesn't have
    // any other special points
    Geom::Rect snap_points_bbox;
    if ( snap_points_hull.empty() == false ) {
    	std::vector<std::pair<Geom::Point, int> >::iterator i = snap_points_hull.begin();
        snap_points_bbox = Geom::Rect((*i).first, (*i).first);
        i++;
        while (i != snap_points_hull.end()) {
            snap_points_bbox.expandTo((*i).first);
            i++;
        }
    }

    _bbox_points.clear();
    if (_bbox) {
    	if (m.snapprefs.getSnapModeBBox()) {
    		getBBoxPoints(_bbox, &_bbox_points, false, true, m.snapprefs.getSnapBBoxEdgeMidpoints(), m.snapprefs.getSnapBBoxMidpoints());
    	}
    	// There are two separate "opposites" (i.e. opposite w.r.t. the handle being dragged):
        //  - one for snapping the boundingbox, which can be either visual or geometric
        //  - one for snapping the special points
        // The "opposite" in case of a geometric boundingbox always coincides with the "opposite" for the special points
        // These distinct "opposites" are needed in the snapmanager to avoid bugs such as #sf1540195 (in which
        // a box is caught between two guides)
        _opposite_for_bboxpoints = _bbox->min() + _bbox->dimensions() * Geom::Scale(1-x, 1-y);
        _opposite_for_specpoints = snap_points_bbox.min() + snap_points_bbox.dimensions() * Geom::Scale(1-x, 1-y);
        _opposite = _opposite_for_bboxpoints;
    }

    // When snapping the node closest to the mouse pointer is absolutely preferred over the closest snap
    // (i.e. when weight == 1), then we will not even try to snap to other points and discard those other
    // points immediately.
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
	if (prefs->getBool("/options/snapclosestonly/value", false)) {
    	if (m.snapprefs.getSnapModeNode()) {
			_keepClosestPointOnly(_snap_points, p);
    	} else {
    		_snap_points.clear(); // don't keep any point
    	}

    	if (m.snapprefs.getSnapModeBBox()) {
			_keepClosestPointOnly(_bbox_points, p);
		} else {
			_bbox_points.clear(); // don't keep any point
		}

    	g_assert(_bbox_points.size() < 2 && _snap_points.size() < 2);
    	if (_snap_points.size() == 1 && _bbox_points.size() == 1) { //both vectors can only have either one or zero elements
    		// So we have exactly one bbox corner and one node left; now find out which is closest and delete the other one
    		if (Geom::L2((_snap_points.at(0)).first - p) < Geom::L2((_bbox_points.at(0)).first - p)) {
    			_bbox_points.clear();
    		} else {
    			_snap_points.clear();
    		}
		}


    	// Optionally, show the snap source
    	if (!(_state == STATE_ROTATE && x != 0.5 && y != 0.5)) { // but not when we're draging a rotation handle, because that won't snap
			// Now either _bbox_points or _snap_points has a single element, the other one has zero..... or both have zero elements
			g_assert((_bbox_points.size() + _snap_points.size()) < 2);
			if (m.snapprefs.getSnapEnabledGlobally()) {
				if (_bbox_points.size() == 1) {
					_desktop->snapindicator->set_new_snapsource(_bbox_points.at(0));
				} else if (_snap_points.size() == 1){
					_desktop->snapindicator->set_new_snapsource(_snap_points.at(0));
				}
			}
    	}
    }

	//sp_event_context_snap_window_open(_desktop->event_context);

    if ((x != -1) && (y != -1)) {
        sp_canvas_item_show(_norm);
        sp_canvas_item_show(_grip);
    }

    if (_show == SHOW_OUTLINE) {
        for (int i = 0; i < 4; i++)
            sp_canvas_item_show(_l[i]);
    }

    _updateHandles();
    g_return_if_fail(_stamp_cache == NULL);
}

void Inkscape::SelTrans::transform(Geom::Matrix const &rel_affine, Geom::Point const &norm)
{
    g_return_if_fail(_grabbed);
    g_return_if_fail(!_empty);

    Geom::Matrix const affine( Geom::Translate(-norm) * rel_affine * Geom::Translate(norm) );

    if (_show == SHOW_CONTENT) {
        // update the content
        for (unsigned i = 0; i < _items.size(); i++) {
            SPItem &item = *_items[i];
            Geom::Matrix const &prev_transform = _items_affines[i];
            sp_item_set_i2d_affine(&item, prev_transform * affine);
        }
    } else {
        if (_bbox) {
            Geom::Point p[4];
            /* update the outline */
            for (unsigned i = 0 ; i < 4 ; i++) {
                p[i] = _bbox->corner(i) * affine;
            }
            for (unsigned i = 0 ; i < 4 ; i++) {
                sp_ctrlline_set_coords(SP_CTRLLINE(_l[i]), p[i], p[(i+1)%4]);
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

    //sp_event_context_snap_window_closed(_desktop->event_context);

    _desktop->snapindicator->remove_snapsource();

    Inkscape::Selection *selection = sp_desktop_selection(_desktop);
    _updateVolatileState();

    for (unsigned i = 0; i < _items.size(); i++) {
        sp_object_unref(SP_OBJECT(_items[i]), NULL);
    }

    sp_canvas_item_hide(_norm);
    sp_canvas_item_hide(_grip);

    if (_show == SHOW_OUTLINE) {
        for (int i = 0; i < 4; i++)
            sp_canvas_item_hide(_l[i]);
    }

    if (_stamp_cache) {
        g_slist_free(_stamp_cache);
        _stamp_cache = NULL;
    }

    _message_context.clear();

    if (!_empty && _changed) {
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
                    SP_OBJECT(currentItem)->updateRepr();
                }
            }
        }

        _items.clear();
        _items_const.clear();
        _items_affines.clear();
        _items_centers.clear();

        if (_current_relative_affine.isTranslation()) {
            sp_document_done(sp_desktop_document(_desktop), SP_VERB_CONTEXT_SELECT,
                             _("Move"));
        } else if (_current_relative_affine.isScale()) {
            sp_document_done(sp_desktop_document(_desktop), SP_VERB_CONTEXT_SELECT,
                             _("Scale"));
        } else if (_current_relative_affine.isRotation()) {
            sp_document_done(sp_desktop_document(_desktop), SP_VERB_CONTEXT_SELECT,
                             _("Rotate"));
        } else {
            sp_document_done(sp_desktop_document(_desktop), SP_VERB_CONTEXT_SELECT,
                             _("Skew"));
        }

    } else {

        if (_center_is_set) {
            // we were dragging center; update reprs and commit undoable action
            for (GSList const *l = _desktop->selection->itemList(); l; l = l->next) {
                SPItem *it = (SPItem*)SP_OBJECT(l->data);
                SP_OBJECT(it)->updateRepr();
            }
            sp_document_done (sp_desktop_document(_desktop), SP_VERB_CONTEXT_SELECT,
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
    Inkscape::Selection *selection = sp_desktop_selection(_desktop);

    bool fixup = !_grabbed;
    if ( fixup && _stamp_cache ) {
        // TODO - give a proper fix. Simple temproary work-around for the grab() issue
        g_slist_free(_stamp_cache);
        _stamp_cache = NULL;
    }

    /* stamping mode */
    if (!_empty) {
        GSList *l;
        if (_stamp_cache) {
            l = _stamp_cache;
        } else {
            /* Build cache */
            l  = g_slist_copy((GSList *) selection->itemList());
            l  = g_slist_sort(l, (GCompareFunc) sp_object_compare_position);
            _stamp_cache = l;
        }

        while (l) {
            SPItem *original_item = SP_ITEM(l->data);
            Inkscape::XML::Node *original_repr = SP_OBJECT_REPR(original_item);

            // remember the position of the item
            gint pos = original_repr->position();
            // remember parent
            Inkscape::XML::Node *parent = sp_repr_parent(original_repr);

            Inkscape::XML::Node *copy_repr = original_repr->duplicate(parent->document());

            // add the new repr to the parent
            parent->appendChild(copy_repr);
            // move to the saved position
            copy_repr->setPosition(pos > 0 ? pos : 0);

            SPItem *copy_item = (SPItem *) sp_desktop_document(_desktop)->getObjectByRepr(copy_repr);

            Geom::Matrix const *new_affine;
            if (_show == SHOW_OUTLINE) {
                Geom::Matrix const i2d(sp_item_i2d_affine(original_item));
                Geom::Matrix const i2dnew( i2d * _current_relative_affine );
                sp_item_set_i2d_affine(copy_item, i2dnew);
                new_affine = &copy_item->transform;
            } else {
                new_affine = &original_item->transform;
            }

            sp_item_write_transform(copy_item, copy_repr, *new_affine);

            if ( copy_item->isCenterSet() && _center ) {
                copy_item->setCenter(*_center * _current_relative_affine);
            }

            Inkscape::GC::release(copy_repr);
            l = l->next;
        }
        sp_document_done(sp_desktop_document(_desktop), SP_VERB_CONTEXT_SELECT,
                         _("Stamp"));
    }

    if ( fixup && _stamp_cache ) {
        // TODO - give a proper fix. Simple temproary work-around for the grab() issue
        g_slist_free(_stamp_cache);
        _stamp_cache = NULL;
    }
}

void Inkscape::SelTrans::_updateHandles()
{
    if ( !_show_handles || _empty )
    {
        sp_remove_handles(_shandle, 8);
        sp_remove_handles(_rhandle, 8);
        sp_remove_handles(&_chandle, 1);
        return;
    }

    // center handle
    if ( _chandle == NULL ) {
        _chandle = sp_knot_new(_desktop, _("<b>Center</b> of rotation and skewing: drag to reposition; scaling with Shift also uses this center"));

        _chandle->setShape (SP_CTRL_SHAPE_BITMAP);
        _chandle->setSize (13);
        _chandle->setAnchor (handle_center.anchor);
        _chandle->setMode (SP_CTRL_MODE_XOR);
        _chandle->setFill(0x00000000, 0x00000000, 0x00000000);
        _chandle->setStroke(0x000000ff, 0xff0000b0, 0xff0000b0);
        _chandle->setPixbuf(handles[handle_center.control]);
        sp_knot_update_ctrl(_chandle);

        g_signal_connect(G_OBJECT(_chandle), "request",
                         G_CALLBACK(sp_sel_trans_handle_request), (gpointer) &handle_center);
        g_signal_connect(G_OBJECT(_chandle), "moved",
                         G_CALLBACK(sp_sel_trans_handle_new_event), (gpointer) &handle_center);
        g_signal_connect(G_OBJECT(_chandle), "grabbed",
                         G_CALLBACK(sp_sel_trans_handle_grab), (gpointer) &handle_center);
        g_signal_connect(G_OBJECT(_chandle), "ungrabbed",
                         G_CALLBACK(sp_sel_trans_handle_ungrab), (gpointer) &handle_center);
        g_signal_connect(G_OBJECT(_chandle), "clicked",
                         G_CALLBACK(sp_sel_trans_handle_click), (gpointer) &handle_center);
    }

    sp_remove_handles(&_chandle, 1);
    if ( _state == STATE_SCALE ) {
        sp_remove_handles(_rhandle, 8);
        _showHandles(_shandle, handles_scale, 8,
                    _("<b>Squeeze or stretch</b> selection; with <b>Ctrl</b> to scale uniformly; with <b>Shift</b> to scale around rotation center"),
                    _("<b>Scale</b> selection; with <b>Ctrl</b> to scale uniformly; with <b>Shift</b> to scale around rotation center"));
    } else {
        sp_remove_handles(_shandle, 8);
        _showHandles(_rhandle, handles_rotate, 8,
                    _("<b>Skew</b> selection; with <b>Ctrl</b> to snap angle; with <b>Shift</b> to skew around the opposite side"),
                    _("<b>Rotate</b> selection; with <b>Ctrl</b> to snap angle; with <b>Shift</b> to rotate around the opposite corner"));
    }

    if (!_center_is_set) {
        _center = _desktop->selection->center();
        _center_is_set = true;
    }

    if ( _state == STATE_SCALE || !_center ) {
        sp_knot_hide(_chandle);
    } else {
        sp_knot_show(_chandle);
        sp_knot_moveto(_chandle, *_center);
    }
}

void Inkscape::SelTrans::_updateVolatileState()
{
    Inkscape::Selection *selection = sp_desktop_selection(_desktop);
    _empty = selection->isEmpty();

    if (_empty) {
        return;
    }

    //Update the bboxes
    _bbox = selection->bounds(_snap_bbox_type);
    _approximate_bbox = selection->bounds(SPItem::APPROXIMATE_BBOX);

    if (!_bbox) {
        _empty = true;
        return;
    }

    _strokewidth = stroke_average_width (selection->itemList());
}

static void sp_remove_handles(SPKnot *knot[], gint num)
{
    for (int i = 0; i < num; i++) {
        if (knot[i] != NULL) {
            sp_knot_hide(knot[i]);
        }
    }
}

void Inkscape::SelTrans::_showHandles(SPKnot *knot[], SPSelTransHandle const handle[], gint num,
                             gchar const *even_tip, gchar const *odd_tip)
{
    g_return_if_fail( !_empty );

    for (int i = 0; i < num; i++) {
        if (knot[i] == NULL) {
            knot[i] = sp_knot_new(_desktop, i % 2 ? even_tip : odd_tip);

            knot[i]->setShape (SP_CTRL_SHAPE_BITMAP);
            knot[i]->setSize (13);
            knot[i]->setAnchor (handle[i].anchor);
            knot[i]->setMode (SP_CTRL_MODE_XOR);
            knot[i]->setFill(0x000000ff, 0x00ff6600, 0x00ff6600); // inversion, green, green
            knot[i]->setStroke(0x000000ff, 0x000000ff, 0x000000ff); // inversion
            knot[i]->setPixbuf(handles[handle[i].control]);
            sp_knot_update_ctrl(knot[i]);

            g_signal_connect(G_OBJECT(knot[i]), "request",
                             G_CALLBACK(sp_sel_trans_handle_request), (gpointer) &handle[i]);
            g_signal_connect(G_OBJECT(knot[i]), "moved",
                             G_CALLBACK(sp_sel_trans_handle_new_event), (gpointer) &handle[i]);
            g_signal_connect(G_OBJECT(knot[i]), "grabbed",
                             G_CALLBACK(sp_sel_trans_handle_grab), (gpointer) &handle[i]);
            g_signal_connect(G_OBJECT(knot[i]), "ungrabbed",
                             G_CALLBACK(sp_sel_trans_handle_ungrab), (gpointer) &handle[i]);
            g_signal_connect(G_OBJECT(knot[i]), "event", G_CALLBACK(sp_seltrans_handle_event), (gpointer) &handle[i]);
        }
        sp_knot_show(knot[i]);

        Geom::Point const handle_pt(handle[i].x, handle[i].y);
        // shouldn't have nullary bbox, but knots
        g_assert(_bbox);
        Geom::Point p( _bbox->min()
                     + ( _bbox->dimensions()
                         * Geom::Scale(handle_pt) ) );

        sp_knot_moveto(knot[i], p);
    }
}

static void sp_sel_trans_handle_grab(SPKnot *knot, guint state, gpointer data)
{
    SP_SELECT_CONTEXT(knot->desktop->event_context)->_seltrans->handleGrab(
        knot, state, *(SPSelTransHandle const *) data
        );
}

static void sp_sel_trans_handle_ungrab(SPKnot *knot, guint /*state*/, gpointer /*data*/)
{
    SP_SELECT_CONTEXT(knot->desktop->event_context)->_seltrans->ungrab();
}

static void sp_sel_trans_handle_new_event(SPKnot *knot, Geom::Point *position, guint state, gpointer data)
{
    SP_SELECT_CONTEXT(knot->desktop->event_context)->_seltrans->handleNewEvent(
        knot, position, state, *(SPSelTransHandle const *) data
        );
}

static gboolean sp_sel_trans_handle_request(SPKnot *knot, Geom::Point *position, guint state, gboolean *data)
{
    return SP_SELECT_CONTEXT(knot->desktop->event_context)->_seltrans->handleRequest(
        knot, position, state, *(SPSelTransHandle const *) data
        );
}

static void sp_sel_trans_handle_click(SPKnot *knot, guint state, gpointer data)
{
    SP_SELECT_CONTEXT(knot->desktop->event_context)->_seltrans->handleClick(
        knot, state, *(SPSelTransHandle const *) data
        );
}

void Inkscape::SelTrans::handleClick(SPKnot */*knot*/, guint state, SPSelTransHandle const &handle)
{
    switch (handle.anchor) {
        case GTK_ANCHOR_CENTER:
            if (state & GDK_SHIFT_MASK) {
                // Unset the  center position for all selected items
                for (GSList const *l = _desktop->selection->itemList(); l; l = l->next) {
                    SPItem *it = (SPItem*)(SP_OBJECT(l->data));
                    it->unsetCenter();
                    SP_OBJECT(it)->updateRepr();
                    _center_is_set = false;  // center has changed
                    _updateHandles();
                }
                sp_document_done (sp_desktop_document(_desktop), SP_VERB_CONTEXT_SELECT,
                                        _("Reset center"));
            }
            break;
        default:
            break;
    }
}

void Inkscape::SelTrans::handleGrab(SPKnot *knot, guint /*state*/, SPSelTransHandle const &handle)
{
    switch (handle.anchor) {
        case GTK_ANCHOR_CENTER:
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

    grab(sp_knot_position(knot), handle.x, handle.y, FALSE);
}


void Inkscape::SelTrans::handleNewEvent(SPKnot *knot, Geom::Point *position, guint state, SPSelTransHandle const &handle)
{
    if (!SP_KNOT_IS_GRABBED(knot)) {
        return;
    }

    // in case items have been unhooked from the document, don't
    // try to continue processing events for them.
    for (unsigned int i = 0; i < _items.size(); i++) {
        if (!SP_OBJECT_DOCUMENT(SP_OBJECT(_items[i])) ) {
            return;
        }
    }

    handle.action(this, handle, *position, state);
}


gboolean Inkscape::SelTrans::handleRequest(SPKnot *knot, Geom::Point *position, guint state, SPSelTransHandle const &handle)
{
    if (!SP_KNOT_IS_GRABBED(knot)) {
        return TRUE;
    }

    knot->desktop->setPosition(*position);

    // When holding shift while rotating or skewing, the transformation will be
    // relative to the point opposite of the handle; otherwise it will be relative
    // to the center as set for the selection
    if ((!(state & GDK_SHIFT_MASK) == !(_state == STATE_ROTATE)) && (&handle != &handle_center)) {
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
    if (handle.request(this, handle, *position, state)) {
        sp_knot_set_position(knot, *position, state);
        SP_CTRL(_grip)->moveto(*position);
        SP_CTRL(_norm)->moveto(_origin);
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
            SPItem::APPROXIMATE_BBOX : SPItem::GEOMETRIC_BBOX;
        //SPItem::APPROXIMATE_BBOX will be replaced by SPItem::VISUAL_BBOX, as soon as the latter is implemented properly

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

gboolean sp_sel_trans_scale_request(Inkscape::SelTrans *seltrans,
                                    SPSelTransHandle const &, Geom::Point &pt, guint state)
{
    return seltrans->scaleRequest(pt, state);
}

gboolean sp_sel_trans_stretch_request(Inkscape::SelTrans *seltrans,
                                      SPSelTransHandle const &handle, Geom::Point &pt, guint state)
{
    return seltrans->stretchRequest(handle, pt, state);
}

gboolean sp_sel_trans_skew_request(Inkscape::SelTrans *seltrans,
                                   SPSelTransHandle const &handle, Geom::Point &pt, guint state)
{
    return seltrans->skewRequest(handle, pt, state);
}

gboolean sp_sel_trans_rotate_request(Inkscape::SelTrans *seltrans,
                                     SPSelTransHandle const &, Geom::Point &pt, guint state)
{
    return seltrans->rotateRequest(pt, state);
}

gboolean sp_sel_trans_center_request(Inkscape::SelTrans *seltrans,
                                     SPSelTransHandle const &, Geom::Point &pt, guint state)
{
    return seltrans->centerRequest(pt, state);
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
        SnapManager &m = _desktop->namedview->snap_manager;
        m.setup(_desktop, false, _items_const);

        Inkscape::SnappedPoint bb, sn;
        Geom::Coord bd(NR_HUGE);
        Geom::Coord sd(NR_HUGE);

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
            bb = m.constrainedSnapScale(SnapPreferences::SNAPPOINT_BBOX, _bbox_points, _point, default_scale, _origin_for_bboxpoints);
            sn = m.constrainedSnapScale(SnapPreferences::SNAPPOINT_NODE, _snap_points, _point, geom_scale, _origin_for_specpoints);

            /* Choose the smaller difference in scale.  Since s[X] == s[Y] we can
            ** just compare difference in s[X].
            */
            bd = bb.getSnapped() ? fabs(bb.getTransformation()[Geom::X] - default_scale[Geom::X]) : NR_HUGE;
            sd = sn.getSnapped() ? fabs(sn.getTransformation()[Geom::X] - geom_scale[Geom::X]) : NR_HUGE;
        } else {
            /* Scale aspect ratio is unlocked */
            bb = m.freeSnapScale(SnapPreferences::SNAPPOINT_BBOX, _bbox_points, _point, default_scale, _origin_for_bboxpoints);
            sn = m.freeSnapScale(SnapPreferences::SNAPPOINT_NODE, _snap_points, _point, geom_scale, _origin_for_specpoints);

            /* Pick the snap that puts us closest to the original scale */
            bd = bb.getSnapped() ? fabs(Geom::L2(bb.getTransformation()) - Geom::L2(Geom::Point(default_scale[Geom::X], default_scale[Geom::Y]))) : NR_HUGE;
            sd = sn.getSnapped() ? fabs(Geom::L2(sn.getTransformation()) - Geom::L2(Geom::Point(geom_scale[Geom::X], geom_scale[Geom::Y]))) : NR_HUGE;
        }

        if (!(bb.getSnapped() || sn.getSnapped())) {
            // We didn't snap at all! Don't update the handle position, just calculate the new transformation
            _calcAbsAffineDefault(default_scale);
            _desktop->snapindicator->remove_snaptarget();
        } else if (bd < sd) {
            // We snapped the bbox (which is either visual or geometric)
            _desktop->snapindicator->set_new_snaptarget(bb);
            default_scale = Geom::Scale(bb.getTransformation());
            // Calculate the new transformation and update the handle position
            pt = _calcAbsAffineDefault(default_scale);
        } else {
            _desktop->snapindicator->set_new_snaptarget(sn);
            // We snapped the special points (e.g. nodes), which are not at the visual bbox
            // The handle location however (pt) might however be at the visual bbox, so we
            // will have to calculate pt taking the stroke width into account
            geom_scale = Geom::Scale(sn.getTransformation());
            pt = _calcAbsAffineGeom(geom_scale);
        }
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

        Inkscape::SnappedPoint bb, sn;
        g_assert(bb.getSnapped() == false); // Check initialization to catch any regression
        Geom::Coord bd(NR_HUGE);
        Geom::Coord sd(NR_HUGE);

        bool symmetrical = state & GDK_CONTROL_MASK;

        bb = m.constrainedSnapStretch(SnapPreferences::SNAPPOINT_BBOX, _bbox_points, _point, Geom::Coord(default_scale[axis]), _origin_for_bboxpoints, Geom::Dim2(axis), symmetrical);
        sn = m.constrainedSnapStretch(SnapPreferences::SNAPPOINT_NODE, _snap_points, _point, Geom::Coord(geom_scale[axis]), _origin_for_specpoints, Geom::Dim2(axis), symmetrical);

        if (bb.getSnapped()) {
            // We snapped the bbox (which is either visual or geometric)
            bd = fabs(bb.getTransformation()[axis] - default_scale[axis]);
            default_scale[axis] = bb.getTransformation()[axis];
        }

        if (sn.getSnapped()) {
            sd = fabs(sn.getTransformation()[axis] - geom_scale[axis]);
            geom_scale[axis] = sn.getTransformation()[axis];
        }

        if (symmetrical) {
            // on ctrl, apply symmetrical scaling instead of stretching
            // Preserve aspect ratio, but never flip in the dimension not being edited (by using fabs())
            default_scale[perp] = fabs(default_scale[axis]);
            geom_scale[perp] = fabs(geom_scale[axis]);
        }

        if (!(bb.getSnapped() || sn.getSnapped())) {
            // We didn't snap at all! Don't update the handle position, just calculate the new transformation
            _calcAbsAffineDefault(default_scale);
            _desktop->snapindicator->remove_snaptarget();
        } else if (bd < sd) {
            _desktop->snapindicator->set_new_snaptarget(bb);
            // Calculate the new transformation and update the handle position
            pt = _calcAbsAffineDefault(default_scale);
        } else {
            _desktop->snapindicator->set_new_snaptarget(sn);
            // We snapped the special points (e.g. nodes), which are not at the visual bbox
            // The handle location however (pt) might however be at the visual bbox, so we
            // will have to calculate pt taking the stroke width into account
            pt = _calcAbsAffineGeom(geom_scale);
        }
    }

    // status text
    _message_context.setF(Inkscape::IMMEDIATE_MESSAGE,
                          _("<b>Scale</b>: %0.2f%% x %0.2f%%; with <b>Ctrl</b> to lock ratio"),
                          100 * _absolute_affine[0], 100 * _absolute_affine[3]);

    return TRUE;
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

    Geom::Point const initial_delta = _point - _origin;

    if (fabs(initial_delta[dim_a]) < 1e-15) {
        return false;
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

        Inkscape::Snapper::ConstraintLine const constraint(component_vectors[dim_b]);
        // When skewing, we cannot snap the corners of the bounding box, see the comment in "constrainedSnapSkew" for details
        Geom::Point const s(skew[dim_a], scale[dim_a]);
        Inkscape::SnappedPoint sn = m.constrainedSnapSkew(Inkscape::SnapPreferences::SNAPPOINT_NODE, _snap_points, _point, constraint, s, _origin, Geom::Dim2(dim_b));

        if (sn.getSnapped()) {
            // We snapped something, so change the skew to reflect it
            Geom::Coord const sd = sn.getSnapped() ? sn.getTransformation()[0] : NR_HUGE;
             _desktop->snapindicator->set_new_snaptarget(sn);
            skew[dim_a] = sd;
        } else {
            _desktop->snapindicator->remove_snaptarget();
        }
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

    double radians;
    if (state & GDK_CONTROL_MASK) {
        // Snap to defined angle increments
        double cos_t = Geom::dot(q1, q2);
        double sin_t = Geom::dot(Geom::rot90(q1), q2);
        radians = atan2(sin_t, cos_t);
        if (snaps) {
            radians = ( M_PI / snaps ) * floor( radians * snaps / M_PI + .5 );
        }
        q1 = Geom::Point(1, 0);
        q2 = Geom::Point(cos(radians), sin(radians));
    } else {
        radians = atan2(Geom::dot(Geom::rot90(d1), d2),
                        Geom::dot(d1, d2));
    }

    Geom::Rotate const r1(q1);
    Geom::Rotate const r2(q2);

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
    SnapManager &m = _desktop->namedview->snap_manager;
    m.setup(_desktop);
    m.freeSnapReturnByRef(SnapPreferences::SNAPPOINT_NODE, pt, Inkscape::SNAPSOURCE_HANDLE);

    if (state & GDK_CONTROL_MASK) {
        if ( fabs(_point[Geom::X] - pt[Geom::X]) > fabs(_point[Geom::Y] - pt[Geom::Y]) ) {
            pt[Geom::Y] = _point[Geom::Y];
        } else {
            pt[Geom::X] = _point[Geom::X];
        }
    }

    if ( !(state & GDK_SHIFT_MASK) && _bbox ) {
        // screen pixels to snap center to bbox
#define SNAP_DIST 5
        // FIXME: take from prefs
        double snap_dist = SNAP_DIST / _desktop->current_zoom();

        for (int i = 0; i < 2; i++) {
            if (fabs(pt[i] - _bbox->min()[i]) < snap_dist) {
                pt[i] = _bbox->min()[i];
            }
            if (fabs(pt[i] - _bbox->midpoint()[i]) < snap_dist) {
                pt[i] = _bbox->midpoint()[i];
            }
            if (fabs(pt[i] - _bbox->max()[i]) < snap_dist) {
                pt[i] = _bbox->max()[i];
            }
        }
    }

    // status text
    GString *xs = SP_PX_TO_METRIC_STRING(pt[Geom::X], _desktop->namedview->getDefaultMetric());
    GString *ys = SP_PX_TO_METRIC_STRING(pt[Geom::Y], _desktop->namedview->getDefaultMetric());
    _message_context.setF(Inkscape::NORMAL_MESSAGE, _("Move <b>center</b> to %s, %s"), xs->str, ys->str);
    g_string_free(xs, FALSE);
    g_string_free(ys, FALSE);

    return TRUE;
}

/*
 * handlers for handle movement
 *
 */

void sp_sel_trans_stretch(Inkscape::SelTrans *seltrans, SPSelTransHandle const &handle, Geom::Point &pt, guint state)
{
    seltrans->stretch(handle, pt, state);
}

void sp_sel_trans_scale(Inkscape::SelTrans *seltrans, SPSelTransHandle const &, Geom::Point &pt, guint state)
{
    seltrans->scale(pt, state);
}

void sp_sel_trans_skew(Inkscape::SelTrans *seltrans, SPSelTransHandle const &handle, Geom::Point &pt, guint state)
{
    seltrans->skew(handle, pt, state);
}

void sp_sel_trans_rotate(Inkscape::SelTrans *seltrans, SPSelTransHandle const &, Geom::Point &pt, guint state)
{
    seltrans->rotate(pt, state);
}

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

void sp_sel_trans_center(Inkscape::SelTrans *seltrans, SPSelTransHandle const &, Geom::Point &pt, guint /*state*/)
{
    seltrans->setCenter(pt);
}


void Inkscape::SelTrans::moveTo(Geom::Point const &xy, guint state)
{
    SnapManager &m = _desktop->namedview->snap_manager;

    /* The amount that we've moved by during this drag */
    Geom::Point dxy = xy - _point;

    bool const alt = (state & GDK_MOD1_MASK);
    bool const control = (state & GDK_CONTROL_MASK);
    bool const shift = (state & GDK_SHIFT_MASK);

    if (alt) {

        /* Alt pressed means keep offset: snap the moved distance to the grid.
        ** FIXME: this will snap to more than just the grid, nowadays.
        */

    	m.setup(_desktop, true, _items_const);
    	m.freeSnapReturnByRef(SnapPreferences::SNAPPOINT_NODE, dxy, Inkscape::SNAPSOURCE_UNDEFINED);

    } else if (!shift) {

        /* We're snapping to things, possibly with a constraint to horizontal or
        ** vertical movement.  Obtain a list of possible translations and then
        ** pick the smallest.
        */

    	m.setup(_desktop, false, _items_const);

    	/* This will be our list of possible translations */
        std::list<Inkscape::SnappedPoint> s;

        if (control) {

            /* Snap to things, and also constrain to horizontal or vertical movement */

            for (unsigned int dim = 0; dim < 2; dim++) {
                // When doing a constrained translation, all points will move in the same direction, i.e.
                // either horizontally or vertically. Therefore we only have to specify the direction of
                // the constraint-line once. The constraint lines are parallel, but might not be colinear.
                // Therefore we will have to set the point through which the constraint-line runs
                // individually for each point to be snapped; this will be handled however by _snapTransformed()
                s.push_back(m.constrainedSnapTranslation(Inkscape::SnapPreferences::SNAPPOINT_BBOX,
                                                         _bbox_points,
                                                         _point,
                                                         Inkscape::Snapper::ConstraintLine(component_vectors[dim]),
                                                         dxy));

                s.push_back(m.constrainedSnapTranslation(Inkscape::SnapPreferences::SNAPPOINT_NODE,
                                                         _snap_points,
                                                         _point,
                                                         Inkscape::Snapper::ConstraintLine(component_vectors[dim]),
                                                         dxy));
            }

        } else {

            // Let's leave this timer code here for a while. I'll probably need it in the near future (Diederik van Lierop)
            /* GTimeVal starttime;
            GTimeVal endtime;
    		g_get_current_time(&starttime); */

            /* Snap to things with no constraint */
			s.push_back(m.freeSnapTranslation(Inkscape::SnapPreferences::SNAPPOINT_BBOX, _bbox_points, _point, dxy));
        	s.push_back(m.freeSnapTranslation(Inkscape::SnapPreferences::SNAPPOINT_NODE, _snap_points, _point, dxy));

          	/*g_get_current_time(&endtime);
          	double elapsed = ((((double)endtime.tv_sec - starttime.tv_sec) * G_USEC_PER_SEC + (endtime.tv_usec - starttime.tv_usec))) / 1000.0;
          	std::cout << "Time spent snapping: " << elapsed << std::endl; */
        }

        /* Pick one */
        Inkscape::SnappedPoint best_snapped_point;
        for (std::list<Inkscape::SnappedPoint>::const_iterator i = s.begin(); i != s.end(); i++) {
            if (i->getSnapped()) {
                if (best_snapped_point.isOtherSnapBetter(*i, true)) {
                    best_snapped_point = *i;
                    dxy = i->getTransformation();
                }
            }
        }
        if (best_snapped_point.getSnapped()) {
            _desktop->snapindicator->set_new_snaptarget(best_snapped_point);
        } else {
            // We didn't snap, so remove any previous snap indicator
            _desktop->snapindicator->remove_snaptarget();
            if (control) {
                // If we didn't snap, then we should still constrain horizontally or vertically
                // (When we did snap, then this constraint has already been enforced by
                // calling constrainedSnapTranslation() above)
                if (fabs(dxy[Geom::X]) > fabs(dxy[Geom::Y])) {
                    dxy[Geom::Y] = 0;
                } else {
                    dxy[Geom::X] = 0;
                }
            }
        }
    }

    Geom::Matrix const move((Geom::Translate(dxy)));
    Geom::Point const norm(0, 0);
    transform(move, norm);

    // status text
    GString *xs = SP_PX_TO_METRIC_STRING(dxy[Geom::X], _desktop->namedview->getDefaultMetric());
    GString *ys = SP_PX_TO_METRIC_STRING(dxy[Geom::Y], _desktop->namedview->getDefaultMetric());
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
    // because this will also hold for _bbox, and which is required for get_scale_transform_with_stroke()
    Geom::Rect new_bbox = Geom::Rect(_origin_for_bboxpoints, visual_handle_pos); // new visual bounding box
    // Please note that the new_bbox might in fact be just a single line, for example when stretching (in
    // which case the handle and origin will be aligned vertically or horizontally)
    Geom::Point normalized_handle_pos = (visual_handle_pos - new_bbox.min()) * Geom::Scale(new_bbox.dimensions()).inverse();

    // Calculate the absolute affine while taking into account the scaling of the stroke width
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool transform_stroke = prefs->getBool("/options/transform/stroke", true);
    Geom::Matrix abs_affine = get_scale_transform_with_stroke (*_bbox, _strokewidth, transform_stroke,
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
    Geom::Matrix abs_affine = Geom::Translate(-_origin) * Geom::Matrix(default_scale) * Geom::Translate(_origin);
    Geom::Point new_bbox_min = _approximate_bbox->min() * abs_affine;
    Geom::Point new_bbox_max = _approximate_bbox->max() * abs_affine;

    bool transform_stroke = false;
    gdouble strokewidth = 0;

    if ( _snap_bbox_type != SPItem::GEOMETRIC_BBOX) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        transform_stroke = prefs->getBool("/options/transform/stroke", true);
        strokewidth = _strokewidth;
    }

    _absolute_affine = get_scale_transform_with_stroke (*_approximate_bbox, strokewidth, transform_stroke,
                    new_bbox_min[Geom::X], new_bbox_min[Geom::Y], new_bbox_max[Geom::X], new_bbox_max[Geom::Y]);

    // return the new handle position
    return ( _point - _origin ) * default_scale + _origin;
}

// Only for scaling/stretching
Geom::Point Inkscape::SelTrans::_calcAbsAffineGeom(Geom::Scale const geom_scale)
{
    _relative_affine = Geom::Matrix(geom_scale);
    _absolute_affine = Geom::Translate(-_origin_for_specpoints) * _relative_affine * Geom::Translate(_origin_for_specpoints);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool const transform_stroke = prefs->getBool("/options/transform/stroke", true);
    Geom::Rect visual_bbox = get_visual_bbox(_geometric_bbox, _absolute_affine, _strokewidth, transform_stroke);

    // return the new handle position
    return visual_bbox.min() + visual_bbox.dimensions() * Geom::Scale(_handle_x, _handle_y);
}

void Inkscape::SelTrans::_keepClosestPointOnly(std::vector<std::pair<Geom::Point, int> > &points, const Geom::Point &reference)
{
	if (points.size() < 2) return;

	std::pair<Geom::Point, int> closest_point = std::make_pair(Geom::Point(NR_HUGE, NR_HUGE), SNAPSOURCE_UNDEFINED);
	Geom::Coord closest_dist = NR_HUGE;

	for(std::vector<std::pair<Geom::Point, int> >::const_iterator i = points.begin(); i != points.end(); i++) {
		Geom::Coord dist = Geom::L2((*i).first - reference);
		if (i == points.begin() || dist < closest_dist) {
			closest_point = *i;
			closest_dist = dist;
		}
    }

	points.clear();
	points.push_back(closest_point);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
