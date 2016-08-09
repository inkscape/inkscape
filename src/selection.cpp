/*
 * Per-desktop selection container
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *   Andrius R. <knutux@gmail.com>
 *   Abhishek Sharma
 *   Adrian Boguszewski
 *
 * Copyright (C) 2016 Adrian Boguszewski
 * Copyright (C) 2006 Andrius R.
 * Copyright (C) 2004-2005 MenTaLguY
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#endif

#include "inkscape.h"
#include "xml/repr.h"
#include "preferences.h"

#include "sp-shape.h"
#include "sp-path.h"
#include "desktop.h"
#include "document.h"

#define SP_SELECTION_UPDATE_PRIORITY (G_PRIORITY_HIGH_IDLE + 1)

namespace Inkscape {

Selection::Selection(LayerModel *layers, SPDesktop *desktop):
    ObjectSet(desktop),
    _layers(layers),
    _selection_context(NULL),
    _flags(0),
    _idle(0)
{
}

Selection::~Selection() {
    _layers = NULL;
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

gboolean Selection::_emit_modified(Selection *selection)
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
    INKSCAPE.selection_modified(this, flags);
    _modified_signal.emit(this, flags);
}

void Selection::_emitChanged(bool persist_selection_context/* = false */) {
    if (persist_selection_context) {
        if (NULL == _selection_context) {
            _selection_context = _layers->currentLayer();
            sp_object_ref(_selection_context, NULL);
            _context_release_connection = _selection_context->connectRelease(sigc::mem_fun(*this, &Selection::_releaseContext));
        }
    } else {
        _releaseContext(_selection_context);
    }

    INKSCAPE.selection_changed(this);
    _changed_signal.emit(this);
}

void Selection::_releaseContext(SPObject *obj)
{
    if (NULL == _selection_context || _selection_context != obj)
        return;

    _context_release_connection.disconnect();

    sp_object_unref(_selection_context, NULL);
    _selection_context = NULL;
}

SPObject *Selection::activeContext() {
    if (NULL != _selection_context)
        return _selection_context;
    return _layers->currentLayer();
}

void Selection::set(SPObject *object, bool persist_selection_context) {
    ObjectSet::set(object);
    _emitChanged(persist_selection_context);
}

void Selection::setReprList(std::vector<XML::Node*> const &list) {
    clear();

    for (std::vector<XML::Node*>::const_reverse_iterator iter = list.rbegin(); iter != list.rend(); ++iter) {
        SPObject *obj = _objectForXMLNode(*iter);
        if (obj) {
            add(obj);
        }
    }

    _emitChanged();
}

std::vector<Inkscape::SnapCandidatePoint> Selection::getSnapPoints(SnapPreferences const *snapprefs) const {
    std::vector<Inkscape::SnapCandidatePoint> p;

    if (snapprefs != NULL){
        SnapPreferences snapprefs_dummy = *snapprefs; // create a local copy of the snapping prefs
        snapprefs_dummy.setTargetSnappable(Inkscape::SNAPTARGET_ROTATION_CENTER, false); // locally disable snapping to the item center
        auto items = const_cast<Selection *>(this)->items();
        for (auto iter = items.begin(); iter != items.end(); ++iter) {
            SPItem *this_item = *iter;
            this_item->getSnappoints(p, &snapprefs_dummy);
            
            //Include the transformation origin for snapping
            //For a selection or group only the overall center is considered, not for each item individually
            if (snapprefs->isTargetSnappable(Inkscape::SNAPTARGET_ROTATION_CENTER)) {
                p.push_back(Inkscape::SnapCandidatePoint(this_item->getCenter(), SNAPSOURCE_ROTATION_CENTER));
            }
        }
    }

    return p;
}

SPObject *Selection::_objectForXMLNode(Inkscape::XML::Node *repr) const {
    g_return_val_if_fail(repr != NULL, NULL);
    gchar const *id = repr->attribute("id");
    g_return_val_if_fail(id != NULL, NULL);
    SPObject *object=_layers->getDocument()->getObjectById(id);
    g_return_val_if_fail(object != NULL, NULL);
    return object;
}

size_t Selection::numberOfLayers() {
    auto items = this->items();
    std::set<SPObject*> layers;
    for (auto iter = items.begin(); iter != items.end(); ++iter) {
        SPObject *layer = _layers->layerForObject(*iter);
        layers.insert(layer);
    }

    return layers.size();
}

size_t Selection::numberOfParents() {
    auto items = this->items();
    std::set<SPObject*> parents;
    for (auto iter = items.begin(); iter != items.end(); ++iter) {
        SPObject *parent = (*iter)->parent;
        parents.insert(parent);
    }
    return parents.size();
}

void Selection::_emitSignals() {
    _emitChanged();
}

void Selection::_connectSignals(SPObject *object) {
    _modified_connections[object] = object->connectModified(sigc::mem_fun(*this, &Selection::_schedule_modified));
}

void Selection::_releaseSignals(SPObject *object) {
    _modified_connections[object].disconnect();
    _modified_connections.erase(object);
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
