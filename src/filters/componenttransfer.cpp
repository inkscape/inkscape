#define __SP_FECOMPONENTTRANSFER_CPP__

/** \file
 * SVG <feComponentTransfer> implementation.
 *
 */
/*
 * Authors:
 *   hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>

#include "document.h"
#include "attributes.h"
#include "svg/svg.h"
#include "componenttransfer.h"
#include "componenttransfer-funcnode.h"
#include "xml/repr.h"
#include "display/nr-filter-component-transfer.h"

/* FeComponentTransfer base class */

static void sp_feComponentTransfer_class_init(SPFeComponentTransferClass *klass);
static void sp_feComponentTransfer_init(SPFeComponentTransfer *feComponentTransfer);

static void sp_feComponentTransfer_build(SPObject *object, Document *document, Inkscape::XML::Node *repr);
static void sp_feComponentTransfer_release(SPObject *object);
static void sp_feComponentTransfer_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feComponentTransfer_update(SPObject *object, SPCtx *ctx, guint flags);
static void sp_feComponentTransfer_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter);
static void sp_feComponentTransfer_remove_child(SPObject *object, Inkscape::XML::Node *child);
static void sp_feComponentTransfer_child_added(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref);
static Inkscape::XML::Node *sp_feComponentTransfer_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static SPFilterPrimitiveClass *feComponentTransfer_parent_class;

GType
sp_feComponentTransfer_get_type()
{
    static GType feComponentTransfer_type = 0;

    if (!feComponentTransfer_type) {
        GTypeInfo feComponentTransfer_info = {
            sizeof(SPFeComponentTransferClass),
            NULL, NULL,
            (GClassInitFunc) sp_feComponentTransfer_class_init,
            NULL, NULL,
            sizeof(SPFeComponentTransfer),
            16,
            (GInstanceInitFunc) sp_feComponentTransfer_init,
            NULL,    /* value_table */
        };
        feComponentTransfer_type = g_type_register_static(SP_TYPE_FILTER_PRIMITIVE, "SPFeComponentTransfer", &feComponentTransfer_info, (GTypeFlags)0);
    }
    return feComponentTransfer_type;
}

static void
sp_feComponentTransfer_class_init(SPFeComponentTransferClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;
    SPFilterPrimitiveClass *sp_primitive_class = (SPFilterPrimitiveClass *)klass;
    feComponentTransfer_parent_class = (SPFilterPrimitiveClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_feComponentTransfer_build;
    sp_object_class->release = sp_feComponentTransfer_release;
    sp_object_class->write = sp_feComponentTransfer_write;
    sp_object_class->set = sp_feComponentTransfer_set;
    sp_object_class->update = sp_feComponentTransfer_update;
    sp_object_class->child_added = sp_feComponentTransfer_child_added;
    sp_object_class->remove_child = sp_feComponentTransfer_remove_child;

    sp_primitive_class->build_renderer = sp_feComponentTransfer_build_renderer;
}

static void
sp_feComponentTransfer_init(SPFeComponentTransfer */*feComponentTransfer*/)
{}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeComponentTransfer variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_feComponentTransfer_build(SPObject *object, Document *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) feComponentTransfer_parent_class)->build) {
        ((SPObjectClass *) feComponentTransfer_parent_class)->build(object, document, repr);
    }

    /*LOAD ATTRIBUTES FROM REPR HERE*/

    //do we need this?
    sp_document_add_resource(document, "feComponentTransfer", object);
}

static void sp_feComponentTransfer_children_modified(SPFeComponentTransfer *sp_componenttransfer)
{
    if (sp_componenttransfer->renderer) {
        bool set[4] = {false, false, false, false};
        SPObject* node = sp_componenttransfer->children;
        for(;node;node=node->next){
            int i=4;
            if (SP_IS_FEFUNCR(node)) i=0;
            if (SP_IS_FEFUNCG(node)) i=1;
            if (SP_IS_FEFUNCB(node)) i=2;
            if (SP_IS_FEFUNCA(node)) i=3;
            if (i==4) {
                g_warning("Unrecognized channel for component transfer.");
                break;
            }
            sp_componenttransfer->renderer->type[i] = ((SPFeFuncNode *) node)->type;
            sp_componenttransfer->renderer->tableValues[i] = ((SPFeFuncNode *) node)->tableValues;
            sp_componenttransfer->renderer->slope[i] = ((SPFeFuncNode *) node)->slope;
            sp_componenttransfer->renderer->intercept[i] = ((SPFeFuncNode *) node)->intercept;
            sp_componenttransfer->renderer->amplitude[i] = ((SPFeFuncNode *) node)->amplitude;
            sp_componenttransfer->renderer->exponent[i] = ((SPFeFuncNode *) node)->exponent;
            sp_componenttransfer->renderer->offset[i] = ((SPFeFuncNode *) node)->offset;
            set[i] = true;
        }
        // Set any types not explicitly set to the identity transform
        for(int i=0;i<4;i++) {
            if (!set[i]) {
                sp_componenttransfer->renderer->type[i] = Inkscape::Filters::COMPONENTTRANSFER_TYPE_IDENTITY;
            }
        }
    }
}

/**
 * Callback for child_added event.
 */
static void
sp_feComponentTransfer_child_added(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
    g_warning("child_added");
    SPFeComponentTransfer *f = SP_FECOMPONENTTRANSFER(object);

    if (((SPObjectClass *) feComponentTransfer_parent_class)->child_added)
        (* ((SPObjectClass *) feComponentTransfer_parent_class)->child_added)(object, child, ref);

    sp_feComponentTransfer_children_modified(f);
    object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
}


/**
 * Callback for remove_child event.
 */
static void
sp_feComponentTransfer_remove_child(SPObject *object, Inkscape::XML::Node *child)
{
    SPFeComponentTransfer *f = SP_FECOMPONENTTRANSFER(object);

    if (((SPObjectClass *) feComponentTransfer_parent_class)->remove_child)
        (* ((SPObjectClass *) feComponentTransfer_parent_class)->remove_child)(object, child);

    sp_feComponentTransfer_children_modified(f);
    object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Drops any allocated memory.
 */
static void
sp_feComponentTransfer_release(SPObject *object)
{
    if (((SPObjectClass *) feComponentTransfer_parent_class)->release)
        ((SPObjectClass *) feComponentTransfer_parent_class)->release(object);
}

/**
 * Sets a specific value in the SPFeComponentTransfer.
 */
static void
sp_feComponentTransfer_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeComponentTransfer *feComponentTransfer = SP_FECOMPONENTTRANSFER(object);
    (void)feComponentTransfer;

    switch(key) {
        /*DEAL WITH SETTING ATTRIBUTES HERE*/
        default:
            if (((SPObjectClass *) feComponentTransfer_parent_class)->set)
                ((SPObjectClass *) feComponentTransfer_parent_class)->set(object, key, value);
            break;
    }
}

/**
 * Receives update notifications.
 */
static void
sp_feComponentTransfer_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    if (((SPObjectClass *) feComponentTransfer_parent_class)->update) {
        ((SPObjectClass *) feComponentTransfer_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_feComponentTransfer_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    /* TODO: Don't just clone, but create a new repr node and write all
     * relevant values into it */
    if (!repr) {
        repr = SP_OBJECT_REPR(object)->duplicate(doc);
    }

    if (((SPObjectClass *) feComponentTransfer_parent_class)->write) {
        ((SPObjectClass *) feComponentTransfer_parent_class)->write(object, doc, repr, flags);
    }

    return repr;
}

static void sp_feComponentTransfer_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter) {
    g_assert(primitive != NULL);
    g_assert(filter != NULL);

    SPFeComponentTransfer *sp_componenttransfer = SP_FECOMPONENTTRANSFER(primitive);

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_COMPONENTTRANSFER);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterComponentTransfer *nr_componenttransfer = dynamic_cast<Inkscape::Filters::FilterComponentTransfer*>(nr_primitive);
    g_assert(nr_componenttransfer != NULL);

    sp_componenttransfer->renderer = nr_componenttransfer;
    sp_filter_primitive_renderer_common(primitive, nr_primitive);


    sp_feComponentTransfer_children_modified(sp_componenttransfer);    //do we need it?!
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
