#ifndef SEEN_INKSCAPE_LAYER_MODEL_H
#define SEEN_INKSCAPE_LAYER_MODEL_H

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   John Bintz <jcoswell@coswellproductions.org>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org>get
 *   Abhishek Sharma
 *   Eric Greveson <eric@greveson.co.uk>
 *
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 2006 John Bintz
 * Copyright (C) 1999-2013 authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

#include <sigc++/sigc++.h>
#include <cstddef>

class SPDocument;
class SPObject;

namespace Inkscape {
  
class ObjectHierarchy;

namespace XML {
    class Node;
}

/**
 * The layer model for a document.
 *
 * This class represents the layer model for a document, typically (but
 * not necessarily) displayed in an SPDesktop.
 *
 * It also implements its own asynchronous notification signals that
 * UI elements can listen to.
 */
class LayerModel
{
    SPDocument *_doc;
    Inkscape::ObjectHierarchy *_layer_hierarchy;
    unsigned int _display_key;

public:
    /** Construct a layer model */
    LayerModel();

    /** Destructor */
    ~LayerModel();

    // Set document
    void setDocument(SPDocument *doc);

    // Set display key. For GUI apps.
    void setDisplayKey(unsigned int display_key);

    // Get the document that this layer model refers to. May be NULL.
    SPDocument *getDocument();

    // TODO look into making these return a more specific subclass:
    SPObject *currentRoot() const;
    SPObject *currentLayer() const;

    void reset();
    void setCurrentLayer(SPObject *object);
    void toggleLayerSolo(SPObject *object);
    void toggleHideAllLayers(bool hide);
    void toggleLockAllLayers(bool lock);
    void toggleLockOtherLayers(SPObject *object);
    SPObject *layerForObject(SPObject *object);
    bool isLayer(SPObject *object) const;
    
    sigc::signal<void, SPObject *> _layer_activated_signal;
    sigc::signal<void, SPObject *> _layer_deactivated_signal;
    sigc::signal<void, SPObject *, SPObject *> _layer_changed_signal;
};

} // namespace Inkscape

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
