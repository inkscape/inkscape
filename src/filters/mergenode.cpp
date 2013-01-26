/** \file
 * feMergeNode implementation. A feMergeNode contains the name of one
 * input image for feMerge.
 */
/*
 * Authors:
 *   Kees Cook <kees@outflux.net>
 *   Niko Kiirala <niko@kiirala.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2004,2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "attributes.h"
#include "xml/repr.h"
#include "filters/mergenode.h"
#include "filters/merge.h"
#include "display/nr-filter-types.h"

static void sp_feMergeNode_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_feMergeNode_release(SPObject *object);
static void sp_feMergeNode_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feMergeNode_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_feMergeNode_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

G_DEFINE_TYPE(SPFeMergeNode, sp_feMergeNode, SP_TYPE_OBJECT);

static void
sp_feMergeNode_class_init(SPFeMergeNodeClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;

    sp_object_class->build = sp_feMergeNode_build;
    sp_object_class->release = sp_feMergeNode_release;
    sp_object_class->write = sp_feMergeNode_write;
    sp_object_class->set = sp_feMergeNode_set;
    sp_object_class->update = sp_feMergeNode_update;
}

static void
sp_feMergeNode_init(SPFeMergeNode *feMergeNode)
{
    feMergeNode->input = Inkscape::Filters::NR_FILTER_SLOT_NOT_SET;
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeMergeNode variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_feMergeNode_build(SPObject *object, SPDocument */*document*/, Inkscape::XML::Node */*repr*/)
{
    object->readAttr( "in" );
}

/**
 * Drops any allocated memory.
 */
static void
sp_feMergeNode_release(SPObject *object)
{
    /* deal with our children and our selves here */

    if (((SPObjectClass *) sp_feMergeNode_parent_class)->release)
        ((SPObjectClass *) sp_feMergeNode_parent_class)->release(object);
}

/**
 * Sets a specific value in the SPFeMergeNode.
 */
static void
sp_feMergeNode_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeMergeNode *feMergeNode = SP_FEMERGENODE(object);
    SPFeMerge *parent = SP_FEMERGE(object->parent);

    if (key == SP_ATTR_IN) {
        int input = sp_filter_primitive_read_in(parent, value);
        if (input != feMergeNode->input) {
            feMergeNode->input = input;
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
    }

    /* See if any parents need this value. */
    if (((SPObjectClass *) sp_feMergeNode_parent_class)->set) {
        ((SPObjectClass *) sp_feMergeNode_parent_class)->set(object, key, value);
    }
}

/**
 * Receives update notifications.
 */
static void
sp_feMergeNode_update(SPObject *object, SPCtx *ctx, guint flags)
{
    //SPFeMergeNode *feMergeNode = SP_FEMERGENODE(object);

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
    }

    if (((SPObjectClass *) sp_feMergeNode_parent_class)->update) {
        ((SPObjectClass *) sp_feMergeNode_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_feMergeNode_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    //SPFeMergeNode *feMergeNode = SP_FEMERGENODE(object);

    // Inkscape-only object, not copied during an "plain SVG" dump:
    if (flags & SP_OBJECT_WRITE_EXT) {
        if (repr) {
            // is this sane?
            //repr->mergeFrom(object->getRepr(), "id");
        } else {
            repr = object->getRepr()->duplicate(doc);
        }
    }

    if (((SPObjectClass *) sp_feMergeNode_parent_class)->write) {
        ((SPObjectClass *) sp_feMergeNode_parent_class)->write(object, doc, repr, flags);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
