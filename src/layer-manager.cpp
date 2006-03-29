/*
 * Inkscape::LayerManager - a view of a document's layers, relative
 *                          to a particular desktop
 *
 * Copyright 2006  MenTaLguY  <mental@rydia.net>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <sigc++/functors/mem_fun.h>
#include <sigc++/adaptors/hide.h>
#include "gc-managed.h"
#include "gc-finalized.h"
#include "document.h"
#include "desktop.h"
#include "layer-manager.h"
#include "ui/view/view.h"
#include "sp-object.h"

namespace Inkscape {

LayerManager::LayerManager(SPDesktop *desktop)
: _desktop(desktop), _document(NULL)
{
    _layer_connection = desktop->connectCurrentLayerChanged(sigc::hide<0>(sigc::mem_fun(*this, &LayerManager::_rebuild)));
    _document_connection = desktop->connectDocumentReplaced(sigc::hide<0>(sigc::mem_fun(*this, &LayerManager::_setDocument)));
    _setDocument(desktop->doc());
}

void LayerManager::_setDocument(SPDocument *document) {
    if (_document) {
        _resource_connection.disconnect();
    }
    _document = document;
    if (document) {
        _resource_connection = sp_document_resources_changed_connect(document, "layer", sigc::mem_fun(*this, &LayerManager::_rebuild));
    }
    _rebuild();
}

void LayerManager::_rebuild() {
    _clear();
    GSList const *layers=sp_document_get_resource_list(_document, "layers");
    for ( GSList const *iter=layers ; iter ; iter = iter->next ) {
        SPObject *layer=static_cast<SPObject *>(iter->data);
        _addOne(layer);
    }
    SPObject *root=_desktop->currentRoot();
    SPObject *layer=_desktop->currentLayer();
    for ( ; layer ; layer = SP_OBJECT_PARENT(layer) ) {
        if (!includes(layer)) {
            _addOne(layer);
        }
        if ( layer == root ) {
            break;
        }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
