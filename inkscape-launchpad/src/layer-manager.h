/*
 * Inkscape::LayerManager - a view of a document's layers, relative
 *                          to a particular desktop
 *
 * Copyright 2006  MenTaLguY  <mental@rydia.net>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_LAYER_MANAGER_H
#define SEEN_INKSCAPE_LAYER_MANAGER_H

#include <vector>
#include <map>

#include "document-subset.h"
#include "gc-finalized.h"
#include "inkgc/gc-soft-ptr.h"

class SPDesktop;
class SPDocument;

namespace Inkscape {

class LayerManager : public DocumentSubset,
                     public GC::Finalized
{
public:
    LayerManager(SPDesktop *desktop);
    virtual ~LayerManager();

    void setCurrentLayer( SPObject* obj );
    void renameLayer( SPObject* obj, char const *label, bool uniquify );
    Glib::ustring getNextLayerName( SPObject* obj, char const *label);

    sigc::connection connectCurrentLayerChanged(const sigc::slot<void, SPObject *> & slot) {
        return _layer_changed_signal.connect(slot);
    }

    sigc::connection connectLayerDetailsChanged(const sigc::slot<void, SPObject *> & slot) {
        return _details_changed_signal.connect(slot);
    }

private:
    friend class LayerWatcher;
    class LayerWatcher;

    void _objectModified( SPObject* obj, unsigned int flags );
    void _setDocument(SPDocument *document);
    void _rebuild();
    void _selectedLayerChanged(SPObject *layer);

    sigc::connection _layer_connection;
    sigc::connection _document_connection;
    sigc::connection _resource_connection;

    GC::soft_ptr<SPDesktop> _desktop;
    SPDocument *_document;

    std::vector<LayerWatcher*> _watchers;

    sigc::signal<void, SPObject *>     _layer_changed_signal;
    sigc::signal<void, SPObject *>     _details_changed_signal;
};

}

#endif
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
