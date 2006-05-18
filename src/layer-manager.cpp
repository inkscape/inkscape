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
#include "xml/node.h"

namespace Inkscape {

LayerManager::LayerManager(SPDesktop *desktop)
: _desktop(desktop), _document(NULL)
{
//    _layer_connection = desktop->connectCurrentLayerChanged( sigc::hide<0>( sigc::mem_fun(*this, &LayerManager::_rebuild) ) );

    sigc::bound_mem_functor1<void, Inkscape::LayerManager, SPDocument*> first = sigc::mem_fun(*this, &LayerManager::_setDocument);

    // This next line has problems on gcc 4.0.2
    sigc::slot<void, SPDocument*> base2 = first;

    sigc::slot<void,SPDesktop*,SPDocument*> slot2 = sigc::hide<0>( base2 );
    _document_connection = desktop->connectDocumentReplaced( slot2 );

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
    GSList const *layers=sp_document_get_resource_list(_document, "layer");
    SPObject *root=_desktop->currentRoot();
    if ( root ) {
        _addOne(root);

        for ( GSList const *iter=layers ; iter ; iter = iter->next ) {
            SPObject *layer=static_cast<SPObject *>(iter->data);

            for ( SPObject* curr = layer; curr && (curr != root) ; curr = SP_OBJECT_PARENT(curr) ) {
                if ( (curr != root) && root->isAncestorOf(curr) && !includes(curr) ) {
                    // Filter out objects in the middle of being deleted

                    // Such may have been the cause of bug 1339397.
                    // See http://sourceforge.net/tracker/index.php?func=detail&aid=1339397&group_id=93438&atid=604306
                    SPObject const *higher = curr;
                    while ( higher && (SP_OBJECT_PARENT(higher) != root) ) {
                        higher = SP_OBJECT_PARENT(higher);
                    }
                    Inkscape::XML::Node* node = higher ? SP_OBJECT_REPR(higher) : 0;
                    if ( node && node->parent() ) {
                        _addOne(curr);
                    }
                }
            }
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
