/*
 * Editable view implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   John Bintz <jcoswell@coswellproductions.org>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2007 Jon A. Cruz
 * Copyright (C) 2006-2008 Johan Engelen
 * Copyright (C) 2006 John Bintz
 * Copyright (C) 2004 MenTaLguY
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "document.h"
#include "layer-fns.h"
#include "layer-model.h"
#include "object-hierarchy.h"
#include "sp-defs.h"
#include "sp-item.h"
#include "sp-item-group.h"
#include "sp-object.h"
#include "sp-root.h"
#include <glib.h>
#include <glibmm/i18n.h>
#include <sigc++/functors/ptr_fun.h>

// Callbacks
static void _layer_activated(SPObject *layer, Inkscape::LayerModel *layer_model);
static void _layer_deactivated(SPObject *layer, Inkscape::LayerModel *layer_model);
static void _layer_changed(SPObject *top, SPObject *bottom, Inkscape::LayerModel *layer_model);

namespace Inkscape {

LayerModel::LayerModel()
    : _doc( 0 )
    , _layer_hierarchy( 0 )
    , _display_key( 0 )
{
}

LayerModel::~LayerModel()
{
    if (_layer_hierarchy) {
        delete _layer_hierarchy;
//        _layer_hierarchy = NULL; //this should be here, but commented to find other bug somewhere else.
    }
}

void LayerModel::setDocument(SPDocument *doc)
{
    _doc = doc;
    if (_layer_hierarchy) {
        _layer_hierarchy->clear();
        delete _layer_hierarchy;
    }
    _layer_hierarchy = new Inkscape::ObjectHierarchy(NULL);
    _layer_hierarchy->connectAdded(sigc::bind(sigc::ptr_fun(_layer_activated), this));
    _layer_hierarchy->connectRemoved(sigc::bind(sigc::ptr_fun(_layer_deactivated), this));
    _layer_hierarchy->connectChanged(sigc::bind(sigc::ptr_fun(_layer_changed), this));
    _layer_hierarchy->setTop(doc->getRoot());
}

void LayerModel::setDisplayKey(unsigned int display_key)
{
    _display_key = display_key;
}

SPDocument *LayerModel::getDocument()
{
    return _doc;
}

/**
 * Returns current root (=bottom) layer.
 */
SPObject *LayerModel::currentRoot() const
{
    return _layer_hierarchy ? _layer_hierarchy->top() : NULL;
}

/**
 * Returns current top layer.
 */
SPObject *LayerModel::currentLayer() const
{
    return _layer_hierarchy ? _layer_hierarchy->bottom() : NULL;
}

/** 
 * Resets the bottom layer to the current root
 */
void LayerModel::reset() {
    if (_layer_hierarchy) {
        _layer_hierarchy->setBottom(currentRoot());
    }
}

/**
 * Sets the current layer of the desktop.
 *
 * Make \a object the top layer.
 */
void LayerModel::setCurrentLayer(SPObject *object) {
    g_return_if_fail(SP_IS_GROUP(object));
    g_return_if_fail( currentRoot() == object || (currentRoot() && currentRoot()->isAncestorOf(object)) );
    // printf("Set Layer to ID: %s\n", object->getId());
    _layer_hierarchy->setBottom(object);
}

void LayerModel::toggleHideAllLayers(bool hide) {

    for ( SPObject* obj = Inkscape::previous_layer(currentRoot(), currentRoot()); obj; obj = Inkscape::previous_layer(currentRoot(), obj) ) {
        SP_ITEM(obj)->setHidden(hide);
    }
}

void LayerModel::toggleLockAllLayers(bool lock) {

    for ( SPObject* obj = Inkscape::previous_layer(currentRoot(), currentRoot()); obj; obj = Inkscape::previous_layer(currentRoot(), obj) ) {
        SP_ITEM(obj)->setLocked(lock);
    }
}

void LayerModel::toggleLockOtherLayers(SPObject *object) {
    g_return_if_fail(SP_IS_GROUP(object));
    g_return_if_fail( currentRoot() == object || (currentRoot() && currentRoot()->isAncestorOf(object)) );

    bool othersLocked = false;
    std::vector<SPObject*> layers;
    for ( SPObject* obj = Inkscape::next_layer(currentRoot(), object); obj; obj = Inkscape::next_layer(currentRoot(), obj) ) {
        // Dont lock any ancestors, since that would in turn lock the layer as well
        if (!obj->isAncestorOf(object)) {
            layers.push_back(obj);
            othersLocked |= !SP_ITEM(obj)->isLocked();
        }
    }
    for ( SPObject* obj = Inkscape::previous_layer(currentRoot(), object); obj; obj = Inkscape::previous_layer(currentRoot(), obj) ) {
        if (!obj->isAncestorOf(object)) {
            layers.push_back(obj);
            othersLocked |= !SP_ITEM(obj)->isLocked();
        }
    }

    SPItem *item = SP_ITEM(object);
    if ( item->isLocked() ) {
        item->setLocked(false);
    }

    for ( std::vector<SPObject*>::iterator it = layers.begin(); it != layers.end(); ++it ) {
        SP_ITEM(*it)->setLocked(othersLocked);
    }
}


void LayerModel::toggleLayerSolo(SPObject *object) {
    g_return_if_fail(SP_IS_GROUP(object));
    g_return_if_fail( currentRoot() == object || (currentRoot() && currentRoot()->isAncestorOf(object)) );

    bool othersShowing = false;
    std::vector<SPObject*> layers;
    for ( SPObject* obj = Inkscape::next_layer(currentRoot(), object); obj; obj = Inkscape::next_layer(currentRoot(), obj) ) {
        // Don't hide ancestors, since that would in turn hide the layer as well
        if (!obj->isAncestorOf(object)) {
            layers.push_back(obj);
            othersShowing |= !SP_ITEM(obj)->isHidden();
        }
    }
    for ( SPObject* obj = Inkscape::previous_layer(currentRoot(), object); obj; obj = Inkscape::previous_layer(currentRoot(), obj) ) {
        if (!obj->isAncestorOf(object)) {
            layers.push_back(obj);
            othersShowing |= !SP_ITEM(obj)->isHidden();
        }
    }


    SPItem *item = SP_ITEM(object);
    if ( item->isHidden() ) {
        item->setHidden(false);
    }

    for ( std::vector<SPObject*>::iterator it = layers.begin(); it != layers.end(); ++it ) {
        SP_ITEM(*it)->setHidden(othersShowing);
    }
}

/**
 * Return layer that contains \a object.
 */
SPObject *LayerModel::layerForObject(SPObject *object) {
    g_return_val_if_fail(object != NULL, NULL);

    SPObject *root=currentRoot();
    object = object->parent;
    while ( object && object != root && !isLayer(object) ) {
        // Objects in defs have no layer and are NOT in the root layer
        if(SP_IS_DEFS(object))
            return NULL;
        object = object->parent;
    }
    return object;
}

/**
 * True if object is a layer.
 */
bool LayerModel::isLayer(SPObject *object) const {
    return ( SP_IS_GROUP(object)
             && ( SP_GROUP(object)->effectiveLayerMode(_display_key)
                  == SPGroup::LAYER ) );
}

} // namespace Inkscape


/// Callback
static void
_layer_activated(SPObject *layer, Inkscape::LayerModel *layer_model) {
    g_return_if_fail(SP_IS_GROUP(layer));
    layer_model->_layer_activated_signal.emit(layer);
}

/// Callback
static void
_layer_deactivated(SPObject *layer, Inkscape::LayerModel *layer_model) {
    g_return_if_fail(SP_IS_GROUP(layer));
    layer_model->_layer_deactivated_signal.emit(layer);
}

/// Callback
static void
_layer_changed(SPObject *top, SPObject *bottom, Inkscape::LayerModel *layer_model)
{
    layer_model->_layer_changed_signal.emit (top, bottom);
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
