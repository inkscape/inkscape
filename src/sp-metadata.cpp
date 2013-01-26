/*
 * SVG <metadata> implementation
 *
 * Authors:
 *   Kees Cook <kees@outflux.net>
 *
 * Copyright (C) 2004 Kees Cook
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "sp-metadata.h"
#include "xml/node-iterators.h"
#include "document.h"

#include "sp-item-group.h"
#include "sp-root.h"

#define noDEBUG_METADATA
#ifdef DEBUG_METADATA
# define debug(f, a...) { g_print("%s(%d) %s:", \
                                  __FILE__,__LINE__,__FUNCTION__); \
                          g_print(f, ## a); \
                          g_print("\n"); \
                        }
#else
# define debug(f, a...) /**/
#endif

/* Metadata base class */

static void sp_metadata_build (SPObject * object, SPDocument * document, Inkscape::XML::Node * repr);
static void sp_metadata_release (SPObject *object);
static void sp_metadata_set (SPObject *object, unsigned int key, const gchar *value);
static void sp_metadata_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_metadata_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

G_DEFINE_TYPE(SPMetadata, sp_metadata, SP_TYPE_OBJECT);

static void
sp_metadata_class_init (SPMetadataClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;

    sp_object_class->build = sp_metadata_build;
    sp_object_class->release = sp_metadata_release;
    sp_object_class->write = sp_metadata_write;
    sp_object_class->set = sp_metadata_set;
    sp_object_class->update = sp_metadata_update;
}

static void
sp_metadata_init (SPMetadata *metadata)
{
    (void)metadata;
    debug("0x%08x",(unsigned int)metadata);
}

namespace {

void strip_ids_recursively(Inkscape::XML::Node *node) {
    using Inkscape::XML::NodeSiblingIterator;
    if ( node->type() == Inkscape::XML::ELEMENT_NODE ) {
        node->setAttribute("id", NULL);
    }
    for ( NodeSiblingIterator iter=node->firstChild() ; iter ; ++iter ) {
        strip_ids_recursively(iter);
    }
}

}

/**
 * Reads the Inkscape::XML::Node, and initializes SPMetadata variables.
 *
 * For this to get called, our name must be associated with
 * a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void sp_metadata_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    using Inkscape::XML::NodeSiblingIterator;

    debug("0x%08x",(unsigned int)object);

    /* clean up our mess from earlier versions; elements under rdf:RDF should not
     * have id= attributes... */
    static GQuark const rdf_root_name=g_quark_from_static_string("rdf:RDF");
    for ( NodeSiblingIterator iter=repr->firstChild() ; iter ; ++iter ) {
        if ( (GQuark)iter->code() == rdf_root_name ) {
            strip_ids_recursively(iter);
        }
    }

    if (((SPObjectClass *) sp_metadata_parent_class)->build)
        ((SPObjectClass *) sp_metadata_parent_class)->build (object, document, repr);
}

/**
 * Drops any allocated memory.
 */
static void sp_metadata_release(SPObject *object)
{
    debug("0x%08x",(unsigned int)object);

    // handle ourself

    if (((SPObjectClass *) sp_metadata_parent_class)->release)
        ((SPObjectClass *) sp_metadata_parent_class)->release (object);
}

/**
 * Sets a specific value in the SPMetadata.
 */
static void sp_metadata_set(SPObject *object, unsigned int key, const gchar *value)
{
    debug("0x%08x %s(%u): '%s'",(unsigned int)object,
          sp_attribute_name(key),key,value);
    SP_METADATA(object); // ensures the object is of the proper type.

    // see if any parents need this value
    if (reinterpret_cast<SPObjectClass *>(sp_metadata_parent_class)->set) {
        reinterpret_cast<SPObjectClass *>(sp_metadata_parent_class)->set(object, key, value);
    }
}

/**
 * Receives update notifications.
 */
static void sp_metadata_update(SPObject *object, SPCtx *ctx, guint flags)
{
    debug("0x%08x",(unsigned int)object);
    //SPMetadata *metadata = SP_METADATA(object);

    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something? */

    }

    if (((SPObjectClass *) sp_metadata_parent_class)->update)
        ((SPObjectClass *) sp_metadata_parent_class)->update(object, ctx, flags);
}

/**
 * Writes it's settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *sp_metadata_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    debug("0x%08x",(unsigned int)object);
    //SPMetadata *metadata = SP_METADATA(object);

    if ( repr != object->getRepr() ) {
        if (repr) {
            repr->mergeFrom(object->getRepr(), "id");
        } else {
            repr = object->getRepr()->duplicate(doc);
        }
    }

    if (((SPObjectClass *) sp_metadata_parent_class)->write) {
        ((SPObjectClass *) sp_metadata_parent_class)->write(object, doc, repr, flags);
    }

    return repr;
}

/**
 * Retrieves the metadata object associated with a document.
 */
SPMetadata *sp_document_metadata(SPDocument *document)
{
    SPObject *nv;

    g_return_val_if_fail (document != NULL, NULL);

    nv = sp_item_group_get_child_by_name( document->getRoot(), NULL,
                                        "metadata");
    g_assert (nv != NULL);

    return (SPMetadata *)nv;
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
