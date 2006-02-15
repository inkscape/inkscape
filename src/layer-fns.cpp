/*
 * Inkscape::SelectionDescriber - shows messages describing selection
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "document.h"
#include "sp-item-group.h"
#include "xml/repr.h"
#include "algorithms/find-last-if.h"

namespace Inkscape {

/**
 *  Creates a new layer.  Advances to the next layer id indicated
 *  by the string "layerNN", then creates a new group object of
 *  that id with attribute inkscape:groupmode='layer', and finally
 *  appends the new group object to \a root after object \a layer.
 *
 *  \pre \a root should be either \a layer or an ancestor of it
 */
SPObject *create_layer(SPObject *root, SPObject *layer) {
    SPDocument *document=SP_OBJECT_DOCUMENT(root);

    static int layer_suffix=1;
    gchar *id=NULL;
    do {
        g_free(id);
        id = g_strdup_printf("layer%d", layer_suffix++);
    } while (document->getObjectById(id));

    Inkscape::XML::Node *repr=sp_repr_new("svg:g");
    repr->setAttribute("inkscape:groupmode", "layer");
    repr->setAttribute("id", id);
    g_free(id);

    if ( root == layer ) {
        SP_OBJECT_REPR(root)->appendChild(repr);
    } else {
        Inkscape::XML::Node *layer_repr=SP_OBJECT_REPR(layer);
        sp_repr_parent(layer_repr)->addChild(repr, layer_repr);
    }

    return document->getObjectByRepr(repr);
}

namespace {

bool is_layer(SPObject &object) {
    return SP_IS_GROUP(&object) &&
           SP_GROUP(&object)->layerMode() == SPGroup::LAYER;
}

SPObject *next_sibling_layer(SPObject *layer) {
    using std::find_if;

    return find_if<SPObject::SiblingIterator>(
        SP_OBJECT_NEXT(layer), NULL, &is_layer
    );
}

SPObject *previous_sibling_layer(SPObject *layer) {
    using Inkscape::Algorithms::find_last_if;

    SPObject *sibling(find_last_if<SPObject::SiblingIterator>(
        SP_OBJECT_PARENT(layer)->firstChild(), layer, &is_layer
    ));

    return ( sibling != layer ) ? sibling : NULL;
}

SPObject *first_descendant_layer(SPObject *layer) {
    using std::find_if;

    SPObject *first_descendant=NULL;
    while (layer) {
        layer = find_if<SPObject::SiblingIterator>(
            layer->firstChild(), NULL, &is_layer
        );
        if (layer) {
            first_descendant = layer;
        }
    }

    return first_descendant;
}

SPObject *last_child_layer(SPObject *layer) {
    using Inkscape::Algorithms::find_last_if;

    return find_last_if<SPObject::SiblingIterator>(
        layer->firstChild(), NULL, &is_layer
    );
}

SPObject *last_elder_layer(SPObject *root, SPObject *layer) {
    using Inkscape::Algorithms::find_last_if;

    while ( layer != root ) {
        SPObject *sibling(previous_sibling_layer(layer));
        if (sibling) {
            return sibling;
        }
        layer = SP_OBJECT_PARENT(layer);
    }

    return NULL;
}

}

/** Finds the next layer under \a root, relative to \a layer in
 *  depth-first order.
 *
 *  @returns NULL if there are no further layers under \a root
 */
SPObject *next_layer(SPObject *root, SPObject *layer) {
    using std::find_if;

    g_return_val_if_fail(layer != NULL, NULL);

    SPObject *sibling(next_sibling_layer(layer));
    if (sibling) {
        SPObject *descendant(first_descendant_layer(sibling));
        if (descendant) {
            return descendant;
        } else {
            return sibling;
        }
    } else {
        SPObject *parent=SP_OBJECT_PARENT(layer);
        if ( parent != root ) {
            return parent;
        } else {
            return NULL;
        }
    }
}


/** Finds the previous layer under \a root, relative to \a layer in
 *  depth-first order.
 *
 *  @returns NULL if there are no prior layers under \a root.
 */
SPObject *previous_layer(SPObject *root, SPObject *layer) {
    using Inkscape::Algorithms::find_last_if;

    g_return_val_if_fail(layer != NULL, NULL);

    SPObject *child(last_child_layer(layer));
    if (child) {
        return child;
    } else if ( layer != root ) {
        SPObject *sibling(previous_sibling_layer(layer));
        if (sibling) {
            return sibling;
        } else {
            return last_elder_layer(root, SP_OBJECT_PARENT(layer));
        }
    }

    return NULL;
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
