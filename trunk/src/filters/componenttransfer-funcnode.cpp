#define __SP_FEFUNCNODE_CPP__

/** \file
 * SVG <funcR>, <funcG>, <funcB> and <funcA> implementations.
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *   Felipe Corrêa da Silva Sanches <felipe.sanches@gmail.com>
 *
 * Copyright (C) 2006, 2007, 2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib.h>

#include "attributes.h"
#include "document.h"
#include "componenttransfer.h"
#include "componenttransfer-funcnode.h"
#include "display/nr-filter-component-transfer.h"
#include "xml/repr.h"
#include "helper-fns.h"

#define SP_MACROS_SILENT
#include "macros.h"

/* FeFuncNode class */

static void sp_fefuncnode_class_init(SPFeFuncNodeClass *klass);
static void sp_fefuncnode_init(SPFeFuncNode *fefuncnode);

static void sp_fefuncnode_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_fefuncnode_release(SPObject *object);
static void sp_fefuncnode_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_fefuncnode_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_fefuncnode_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static SPObjectClass *feFuncNode_parent_class;

GType
sp_fefuncR_get_type()
{
    static GType fefuncnode_type = 0;

    if (!fefuncnode_type) {
        GTypeInfo fefuncnode_info = {
            sizeof(SPFeFuncNodeClass),
            NULL, NULL,
            (GClassInitFunc) sp_fefuncnode_class_init,
            NULL, NULL,
            sizeof(SPFeFuncNode),
            16,
            (GInstanceInitFunc) sp_fefuncnode_init,
            NULL,    /* value_table */
        };
        fefuncnode_type = g_type_register_static(SP_TYPE_OBJECT, "SPFeFuncR", &fefuncnode_info, (GTypeFlags)0);
    }
    return fefuncnode_type;
}

GType
sp_fefuncG_get_type()
{
    static GType fefuncnode_type = 0;

    if (!fefuncnode_type) {
        GTypeInfo fefuncnode_info = {
            sizeof(SPFeFuncNodeClass),
            NULL, NULL,
            (GClassInitFunc) sp_fefuncnode_class_init,
            NULL, NULL,
            sizeof(SPFeFuncNode),
            16,
            (GInstanceInitFunc) sp_fefuncnode_init,
            NULL,    /* value_table */
        };
        fefuncnode_type = g_type_register_static(SP_TYPE_OBJECT, "SPFeFuncG", &fefuncnode_info, (GTypeFlags)0);
    }
    return fefuncnode_type;
}

GType
sp_fefuncB_get_type()
{
    static GType fefuncnode_type = 0;

    if (!fefuncnode_type) {
        GTypeInfo fefuncnode_info = {
            sizeof(SPFeFuncNodeClass),
            NULL, NULL,
            (GClassInitFunc) sp_fefuncnode_class_init,
            NULL, NULL,
            sizeof(SPFeFuncNode),
            16,
            (GInstanceInitFunc) sp_fefuncnode_init,
            NULL,    /* value_table */
        };
        fefuncnode_type = g_type_register_static(SP_TYPE_OBJECT, "SPFeFuncB", &fefuncnode_info, (GTypeFlags)0);
    }
    return fefuncnode_type;
}

GType
sp_fefuncA_get_type()
{
    static GType fefuncnode_type = 0;

    if (!fefuncnode_type) {
        GTypeInfo fefuncnode_info = {
            sizeof(SPFeFuncNodeClass),
            NULL, NULL,
            (GClassInitFunc) sp_fefuncnode_class_init,
            NULL, NULL,
            sizeof(SPFeFuncNode),
            16,
            (GInstanceInitFunc) sp_fefuncnode_init,
            NULL,    /* value_table */
        };
        fefuncnode_type = g_type_register_static(SP_TYPE_OBJECT, "SPFeFuncA", &fefuncnode_info, (GTypeFlags)0);
    }
    return fefuncnode_type;
}

static void
sp_fefuncnode_class_init(SPFeFuncNodeClass *klass)
{

    SPObjectClass *sp_object_class = (SPObjectClass *)klass;

    feFuncNode_parent_class = (SPObjectClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_fefuncnode_build;
    sp_object_class->release = sp_fefuncnode_release;
    sp_object_class->write = sp_fefuncnode_write;
    sp_object_class->set = sp_fefuncnode_set;
    sp_object_class->update = sp_fefuncnode_update;
}

static void
sp_fefuncnode_init(SPFeFuncNode *fefuncnode)
{
    fefuncnode->type = Inkscape::Filters::COMPONENTTRANSFER_TYPE_IDENTITY;
    //fefuncnode->tableValues = NULL;
    fefuncnode->slope = 1;
    fefuncnode->intercept = 0;
    fefuncnode->amplitude = 1;
    fefuncnode->exponent = 1;
    fefuncnode->offset = 0;
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPDistantLight variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_fefuncnode_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) feFuncNode_parent_class)->build) {
        ((SPObjectClass *) feFuncNode_parent_class)->build(object, document, repr);
    }

    //Read values of key attributes from XML nodes into object.
    sp_object_read_attr(object, "type");
    sp_object_read_attr(object, "tableValues");
    sp_object_read_attr(object, "slope");
    sp_object_read_attr(object, "intercept");
    sp_object_read_attr(object, "amplitude");
    sp_object_read_attr(object, "exponent");
    sp_object_read_attr(object, "offset");


//is this necessary?
    sp_document_add_resource(document, "fefuncnode", object); //maybe feFuncR, fefuncG, feFuncB and fefuncA ?
}

/**
 * Drops any allocated memory.
 */
static void
sp_fefuncnode_release(SPObject *object)
{
    //SPFeFuncNode *fefuncnode = SP_FEFUNCNODE(object);

    if (SP_OBJECT_DOCUMENT(object)) {
        /* Unregister ourselves */
        sp_document_remove_resource(SP_OBJECT_DOCUMENT(object), "fefuncnode", SP_OBJECT(object));
    }

//TODO: release resources here
}

static Inkscape::Filters::FilterComponentTransferType sp_feComponenttransfer_read_type(gchar const *value){
    if (!value) return Inkscape::Filters::COMPONENTTRANSFER_TYPE_ERROR; //type attribute is REQUIRED.
    switch(value[0]){
        case 'i':
            if (strncmp(value, "identity", 8) == 0) return Inkscape::Filters::COMPONENTTRANSFER_TYPE_IDENTITY;
            break;
        case 't':
            if (strncmp(value, "table", 5) == 0) return Inkscape::Filters::COMPONENTTRANSFER_TYPE_TABLE;
            break;
        case 'd':
            if (strncmp(value, "discrete", 8) == 0) return Inkscape::Filters::COMPONENTTRANSFER_TYPE_DISCRETE;
            break;
        case 'l':
            if (strncmp(value, "linear", 6) == 0) return Inkscape::Filters::COMPONENTTRANSFER_TYPE_LINEAR;
            break;
        case 'g':
            if (strncmp(value, "gamma", 5) == 0) return Inkscape::Filters::COMPONENTTRANSFER_TYPE_GAMMA;
            break;
    }
    return Inkscape::Filters::COMPONENTTRANSFER_TYPE_ERROR; //type attribute is REQUIRED.
}

/**
 * Sets a specific value in the SPFeFuncNode.
 */
static void
sp_fefuncnode_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeFuncNode *feFuncNode = SP_FEFUNCNODE(object);
    Inkscape::Filters::FilterComponentTransferType type;
    double read_num;
    switch(key) {
        case SP_ATTR_TYPE:
            type = sp_feComponenttransfer_read_type(value);
            if(type != feFuncNode->type) {
                feFuncNode->type = type;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_TABLEVALUES:
            if (value){
                feFuncNode->tableValues = helperfns_read_vector(value);
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_SLOPE:
            read_num = helperfns_read_number(value);
            if (read_num != feFuncNode->slope) {
                feFuncNode->slope = read_num;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_INTERCEPT:
            read_num = helperfns_read_number(value);
            if (read_num != feFuncNode->intercept) {
                feFuncNode->intercept = read_num;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_AMPLITUDE:
            read_num = helperfns_read_number(value);
            if (read_num != feFuncNode->amplitude) {
                feFuncNode->amplitude = read_num;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_EXPONENT:
            read_num = helperfns_read_number(value);
            if (read_num != feFuncNode->exponent) {
                feFuncNode->exponent = read_num;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_OFFSET:
            read_num = helperfns_read_number(value);
            if (read_num != feFuncNode->offset) {
                feFuncNode->offset = read_num;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        default:
            if (((SPObjectClass *) feFuncNode_parent_class)->set)
                ((SPObjectClass *) feFuncNode_parent_class)->set(object, key, value);
            break;
    }
}

/**
 *  * Receives update notifications.
 *   */
static void
sp_fefuncnode_update(SPObject *object, SPCtx *ctx, guint flags)
{
    SPFeFuncNode *feFuncNode = SP_FEFUNCNODE(object);
    (void)feFuncNode;

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        /* do something to trigger redisplay, updates? */
        //TODO
        //sp_object_read_attr(object, "azimuth");
        //sp_object_read_attr(object, "elevation");
    }

    if (((SPObjectClass *) feFuncNode_parent_class)->update) {
        ((SPObjectClass *) feFuncNode_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_fefuncnode_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    SPFeFuncNode *fefuncnode = SP_FEFUNCNODE(object);

    if (!repr) {
        repr = SP_OBJECT_REPR(object)->duplicate(doc);
    }

    (void)fefuncnode;
    /*
TODO: I'm not sure what to do here...

    if (fefuncnode->azimuth_set)
        sp_repr_set_css_double(repr, "azimuth", fefuncnode->azimuth);
    if (fefuncnode->elevation_set)
        sp_repr_set_css_double(repr, "elevation", fefuncnode->elevation);*/

    if (((SPObjectClass *) feFuncNode_parent_class)->write) {
        ((SPObjectClass *) feFuncNode_parent_class)->write(object, doc, repr, flags);
    }

    return repr;
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
