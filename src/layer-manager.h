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

#include "document-subset.h"
#include "gc-finalized.h"
#include "gc-soft-ptr.h"

class SPDesktop;
class SPDocument;

namespace Inkscape {

class LayerManager : public DocumentSubset,
                     public GC::Finalized
{
public:
    LayerManager(SPDesktop *desktop);

private:

    void _setDocument(SPDocument *document);
    void _rebuild();

    sigc::connection _layer_connection;
    sigc::connection _document_connection;
    sigc::connection _resource_connection;

    GC::soft_ptr<SPDesktop> _desktop;
    SPDocument *_document;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
