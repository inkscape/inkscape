/** \file
 * Per-desktop selection container
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *   Andrius R. <knutux@gmail.com>
 *
 * Copyright (C)      2006 Andrius R.
 * Copyright (C) 2004-2005 MenTaLguY
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "macros.h"
#include "inkscape-private.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "document.h"
#include "selection.h"
#include "helper/recthull.h"
#include "xml/repr.h"

#include "sp-shape.h"
#include "sp-path.h"
#include "sp-item-group.h"
#include "box3d.h"
#include "box3d.h"
#include "persp3d.h"

#include <sigc++/functors/mem_fun.h>

#define SP_SELECTION_UPDATE_PRIORITY (G_PRIORITY_HIGH_IDLE + 1)

namespace Inkscape {

Selection::Selection(SPDesktop *desktop) :
    _objs(NULL),
    _reprs(NULL),
    _items(NULL),
    _desktop(desktop),
    _selection_context(NULL),
    _flags(0),
    _idle(0)
{
}

Selection::~Selection() {
    _clear();
    _desktop = NULL;
    if (_idle) {
        g_source_remove(_idle);
        _idle = 0;
    }
}

/* Handler for selected objects "modified" signal */

void Selection::_schedule_modified(SPObject */*obj*/, guint flags) {
    if (!this->_idle) {
        /* Request handling to be run in _idle loop */
        this->_idle = g_idle_add_full(SP_SELECTION_UPDATE_PRIORITY, GSourceFunc(&Selection::_emit_modified), this, NULL);
    }

    /* Collect all flags */
    this->_flags |= flags;
}

gboolean
Selection::_emit_modified(Selection *selection)
{
    /* force new handler to be created if requested before we return */
    selection->_idle = 0;
    guint flags = selection->_flags;
    selection->_flags = 0;

    selection->_emitModified(flags);

    /* drop this handler */
    return FALSE;
}

void Selection::_emitModified(guint flags) {
    inkscape_selection_modified(this, flags);
    _modified_signal.emit(this, flags);
}

void Selection::_emitChanged(bool persist_selection_context/* = false */) {
    if (persist_selection_context) {
        if (NULL == _selection_context) {
            _selection_context = desktop()->currentLayer();
            sp_object_ref(_selection_context, NULL);
            _context_release_connection = _selection_context->connectRelease(sigc::mem_fun(*this, &Selection::_releaseContext));
        }
    } else {
        _releaseContext(_selection_context);
    }

    inkscape_selection_changed(this);
    _changed_signal.emit(this);
}

void
Selection::_releaseContext(SPObject *obj)
{
    if (NULL == _selection_context || _selection_context != obj)
        return;

    _context_release_connection.disconnect();

    sp_object_unref(_selection_context, NULL);
    _selection_context = NULL;
}

void Selection::_invalidateCachedLists() {
    g_slist_free(_items);
    _items = NULL;

    g_slist_free(_reprs);
    _reprs = NULL;
}

void Selection::_clear() {
    _invalidateCachedLists();
    while (_objs) {
        SPObject *obj=reinterpret_cast<SPObject *>(_objs->data);
        _remove(obj);
    }
}

SPObject *Selection::activeContext() {
    if (NULL != _selection_context)
        return _selection_context;
    return desktop()->currentLayer();
    }

bool Selection::includes(SPObject *obj) const {
    if (obj == NULL)
        return FALSE;

    g_return_val_if_fail(SP_IS_OBJECT(obj), FALSE);

    return ( g_slist_find(_objs, obj) != NULL );
}

void Selection::add(SPObject *obj, bool persist_selection_context/* = false */) {
    g_return_if_fail(obj != NULL);
    g_return_if_fail(SP_IS_OBJECT(obj));

    if (includes(obj)) {
        return;
    }

    _invalidateCachedLists();
    _add(obj);
    _emitChanged(persist_selection_context);
}

void Selection::add_box_perspective(SPBox3D *box) {
    Persp3D *persp = box3d_get_perspective(box);
    std::map<Persp3D *, unsigned int>::iterator p = _persps.find(persp);
    if (p != _persps.end()) {
        (*p).second++;
    } else {
        _persps[persp] = 1;
    }
}

void Selection::add_3D_boxes_recursively(SPObject *obj) {
    std::list<SPBox3D *> boxes = box3d_extract_boxes(obj);

    for (std::list<SPBox3D *>::iterator i = boxes.begin(); i != boxes.end(); ++i) {
        SPBox3D *box = *i;
        box3d_add_to_selection(box);
        _3dboxes.push_back(box);
        add_box_perspective(box);
    }
}

void Selection::_add(SPObject *obj) {
    // unselect any of the item's ancestors and descendants which may be selected
    // (to prevent double-selection)
    _removeObjectDescendants(obj);
    _removeObjectAncestors(obj);

    _objs = g_slist_prepend(_objs, obj);

    add_3D_boxes_recursively(obj);

    _release_connections[obj] = obj->connectRelease(sigc::mem_fun(*this, (void (Selection::*)(SPObject *))&Selection::remove));
    _modified_connections[obj] = obj->connectModified(sigc::mem_fun(*this, &Selection::_schedule_modified));
}

void Selection::set(SPObject *object, bool persist_selection_context) {
    _clear();
    add(object, persist_selection_context);
}

void Selection::toggle(SPObject *obj) {
    if (includes (obj)) {
        remove (obj);
    } else {
        add(obj);
    }
}

void Selection::remove(SPObject *obj) {
    g_return_if_fail(obj != NULL);
    g_return_if_fail(SP_IS_OBJECT(obj));
    g_return_if_fail(includes(obj));

    _invalidateCachedLists();
    _remove(obj);
    _emitChanged();
}

void Selection::remove_box_perspective(SPBox3D *box) {
    Persp3D *persp = box3d_get_perspective(box);
    std::map<Persp3D *, unsigned int>::iterator p = _persps.find(persp);
    if (p == _persps.end()) {
        g_print ("Warning! Trying to remove unselected perspective from selection!\n");
        return;
    }
    if ((*p).second > 1) {
        _persps[persp]--;
    } else {
        _persps.erase(p);
    }
}

void Selection::remove_3D_boxes_recursively(SPObject *obj) {
    std::list<SPBox3D *> boxes = box3d_extract_boxes(obj);

    for (std::list<SPBox3D *>::iterator i = boxes.begin(); i != boxes.end(); ++i) {
        SPBox3D *box = *i;
        box3d_remove_from_selection(box);
        std::list<SPBox3D *>::iterator b = std::find(_3dboxes.begin(), _3dboxes.end(), box);
        if (b == _3dboxes.end()) {
            g_print ("Warning! Trying to remove unselected box from selection.\n");
            return;
        }
        _3dboxes.erase(b);
        remove_box_perspective(box);
    }
}

void Selection::_remove(SPObject *obj) {
    _modified_connections[obj].disconnect();
    _modified_connections.erase(obj);

    _release_connections[obj].disconnect();
    _release_connections.erase(obj);

    remove_3D_boxes_recursively(obj);

    _objs = g_slist_remove(_objs, obj);
}

void Selection::setList(GSList const *list) {
    _clear();

    if ( list != NULL ) {
        for ( GSList const *iter = list ; iter != NULL ; iter = iter->next ) {
            _add(reinterpret_cast<SPObject *>(iter->data));
        }
    }

    _emitChanged();
}

void Selection::addList(GSList const *list) {

    if (list == NULL)
        return;

    _invalidateCachedLists();

    for ( GSList const *iter = list ; iter != NULL ; iter = iter->next ) {
        SPObject *obj = reinterpret_cast<SPObject *>(iter->data);
        if (includes(obj)) {
            continue;
        }
        _add (obj);
    }

    _emitChanged();
}

void Selection::setReprList(GSList const *list) {
    _clear();

    for ( GSList const *iter = list ; iter != NULL ; iter = iter->next ) {
        SPObject *obj=_objectForXMLNode(reinterpret_cast<Inkscape::XML::Node *>(iter->data));
        if (obj) {
            _add(obj);
        }
    }

    _emitChanged();
}

void Selection::clear() {
    _clear();
    _emitChanged();
}

GSList const *Selection::list() {
    return _objs;
}

GSList const *Selection::itemList() {
    if (_items) {
        return _items;
    }

    for ( GSList const *iter=_objs ; iter != NULL ; iter = iter->next ) {
        SPObject *obj=reinterpret_cast<SPObject *>(iter->data);
        if (SP_IS_ITEM(obj)) {
            _items = g_slist_prepend(_items, SP_ITEM(obj));
        }
    }
    _items = g_slist_reverse(_items);

    return _items;
}

GSList const *Selection::reprList() {
    if (_reprs) { return _reprs; }

    for ( GSList const *iter=itemList() ; iter != NULL ; iter = iter->next ) {
        SPObject *obj=reinterpret_cast<SPObject *>(iter->data);
        _reprs = g_slist_prepend(_reprs, SP_OBJECT_REPR(obj));
    }
    _reprs = g_slist_reverse(_reprs);

    return _reprs;
}

std::list<Persp3D *> const Selection::perspList() {
    std::list<Persp3D *> pl;
    for (std::map<Persp3D *, unsigned int>::iterator p = _persps.begin(); p != _persps.end(); ++p) {
        pl.push_back((*p).first);
    }
    return pl;
}

std::list<SPBox3D *> const Selection::box3DList() {
    return _3dboxes;
}

SPObject *Selection::single() {
    if ( _objs != NULL && _objs->next == NULL ) {
        return reinterpret_cast<SPObject *>(_objs->data);
    } else {
        return NULL;
    }
}

SPItem *Selection::singleItem() {
    GSList const *items=itemList();
    if ( items != NULL && items->next == NULL ) {
        return reinterpret_cast<SPItem *>(items->data);
    } else {
        return NULL;
    }
}

Inkscape::XML::Node *Selection::singleRepr() {
    SPObject *obj=single();
    return obj ? SP_OBJECT_REPR(obj) : NULL;
}

NRRect *Selection::bounds(NRRect *bbox, SPItem::BBoxType type) const
{
    g_return_val_if_fail (bbox != NULL, NULL);
    *bbox = NRRect(bounds(type));
    return bbox;
}

boost::optional<Geom::Rect> Selection::bounds(SPItem::BBoxType type) const
{
    GSList const *items = const_cast<Selection *>(this)->itemList();

    boost::optional<Geom::Rect> bbox;
    for ( GSList const *i = items ; i != NULL ; i = i->next ) {
        bbox = unify(bbox, sp_item_bbox_desktop(SP_ITEM(i->data), type));
    }
    return bbox;
}

NRRect *Selection::boundsInDocument(NRRect *bbox, SPItem::BBoxType type) const {
    g_return_val_if_fail (bbox != NULL, NULL);

    GSList const *items=const_cast<Selection *>(this)->itemList();
    if (!items) {
        bbox->x0 = bbox->y0 = bbox->x1 = bbox->y1 = 0.0;
        return bbox;
    }

    bbox->x0 = bbox->y0 = 1e18;
    bbox->x1 = bbox->y1 = -1e18;

    for ( GSList const *iter=items ; iter != NULL ; iter = iter->next ) {
        SPItem *item=SP_ITEM(iter->data);
        Geom::Matrix i2doc(sp_item_i2doc_affine(item));
        sp_item_invoke_bbox(item, bbox, i2doc, FALSE, type);
    }

    return bbox;
}

boost::optional<Geom::Rect> Selection::boundsInDocument(SPItem::BBoxType type) const {
    NRRect r;
    return to_2geom(boundsInDocument(&r, type)->upgrade());
}

/** Extract the position of the center from the first selected object */
boost::optional<Geom::Point> Selection::center() const {
    GSList *items = (GSList *) const_cast<Selection *>(this)->itemList();
    Geom::Point center;
    if (items) {
        SPItem *first = reinterpret_cast<SPItem*>(g_slist_last(items)->data); // from the first item in selection
        if (first->isCenterSet()) { // only if set explicitly
            return first->getCenter();
        }
    }
    boost::optional<Geom::Rect> bbox = bounds();
    if (bbox) {
        return bounds()->midpoint();
    } else {
        return boost::optional<Geom::Point>();
    }
}

/**
 * Compute the list of points in the selection that are to be considered for snapping.
 */
std::vector<Geom::Point> Selection::getSnapPoints(SnapPreferences const *snapprefs) const {
    GSList const *items = const_cast<Selection *>(this)->itemList();
    
    SnapPreferences snapprefs_dummy = *snapprefs; // create a local copy of the snapping prefs
    snapprefs_dummy.setIncludeItemCenter(false); // locally disable snapping to the item center
    
    std::vector<Geom::Point> p;
    for (GSList const *iter = items; iter != NULL; iter = iter->next) {
        SPItem *this_item = SP_ITEM(iter->data);
        sp_item_snappoints(this_item, SnapPointsIter(p), &snapprefs_dummy);
       
        //Include the transformation origin for snapping
        //For a selection or group only the overall origin is considered
        if (snapprefs != NULL && snapprefs->getIncludeItemCenter()) {
        	p.push_back(this_item->getCenter());
        }  
    }

    return p;
}

std::vector<Geom::Point> Selection::getSnapPointsConvexHull(SnapPreferences const *snapprefs) const {
    GSList const *items = const_cast<Selection *>(this)->itemList();

    std::vector<Geom::Point> p;
    for (GSList const *iter = items; iter != NULL; iter = iter->next) {
		sp_item_snappoints(SP_ITEM(iter->data), SnapPointsIter(p), snapprefs);
    }

    std::vector<Geom::Point> pHull;
    if (!p.empty()) {
        std::vector<Geom::Point>::iterator i;
        Geom::RectHull cvh(p.front());
        for (i = p.begin(); i != p.end(); i++) {
            // these are the points we get back
            cvh.add(*i);
        }

        boost::optional<Geom::Rect> rHull = cvh.bounds();
        if (rHull) {
            for ( unsigned i = 0 ; i < 4 ; ++i ) {
                pHull.push_back(rHull->corner(i));
            }
        }
    }

    return pHull;
}

void Selection::_removeObjectDescendants(SPObject *obj) {
    GSList *iter, *next;
    for ( iter = _objs ; iter ; iter = next ) {
        next = iter->next;
        SPObject *sel_obj=reinterpret_cast<SPObject *>(iter->data);
        SPObject *parent=SP_OBJECT_PARENT(sel_obj);
        while (parent) {
            if ( parent == obj ) {
                _remove(sel_obj);
                break;
            }
            parent = SP_OBJECT_PARENT(parent);
        }
    }
}

void Selection::_removeObjectAncestors(SPObject *obj) {
        SPObject *parent=SP_OBJECT_PARENT(obj);
        while (parent) {
            if (includes(parent)) {
                _remove(parent);
            }
            parent = SP_OBJECT_PARENT(parent);
        }
}

SPObject *Selection::_objectForXMLNode(Inkscape::XML::Node *repr) const {
    g_return_val_if_fail(repr != NULL, NULL);
    gchar const *id = repr->attribute("id");
    g_return_val_if_fail(id != NULL, NULL);
    SPObject *object=sp_desktop_document(_desktop)->getObjectById(id);
    g_return_val_if_fail(object != NULL, NULL);
    return object;
}

guint Selection::numberOfLayers() {
    GSList const *items = const_cast<Selection *>(this)->itemList();
    GSList *layers = NULL;
    for (GSList const *iter = items; iter != NULL; iter = iter->next) {
        SPObject *layer = desktop()->layerForObject(SP_OBJECT(iter->data));
        if (g_slist_find (layers, layer) == NULL) {
            layers = g_slist_prepend (layers, layer);
        }
    }
    guint ret = g_slist_length (layers);
    g_slist_free (layers);
    return ret;
}

guint Selection::numberOfParents() {
    GSList const *items = const_cast<Selection *>(this)->itemList();
    GSList *parents = NULL;
    for (GSList const *iter = items; iter != NULL; iter = iter->next) {
        SPObject *parent = SP_OBJECT_PARENT(iter->data);
        if (g_slist_find (parents, parent) == NULL) {
            parents = g_slist_prepend (parents, parent);
        }
    }
    guint ret = g_slist_length (parents);
    g_slist_free (parents);
    return ret;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
