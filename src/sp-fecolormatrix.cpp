#define __SP_FECOLORMATRIX_CPP__

/** \file
 * SVG <feColorMatrix> implementation.
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

#include "attributes.h"
#include "svg/svg.h"
#include "sp-fecolormatrix.h"
#include "xml/repr.h"


/* FeColorMatrix base class */

static void sp_feColorMatrix_class_init(SPFeColorMatrixClass *klass);
static void sp_feColorMatrix_init(SPFeColorMatrix *feColorMatrix);

static void sp_feColorMatrix_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_feColorMatrix_release(SPObject *object);
static void sp_feColorMatrix_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feColorMatrix_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_feColorMatrix_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);
static void sp_feColorMatrix_build_renderer(SPFilterPrimitive *primitive, NR::Filter *filter);

static SPFilterPrimitiveClass *feColorMatrix_parent_class;

GType
sp_feColorMatrix_get_type()
{
    static GType feColorMatrix_type = 0;

    if (!feColorMatrix_type) {
        GTypeInfo feColorMatrix_info = {
            sizeof(SPFeColorMatrixClass),
            NULL, NULL,
            (GClassInitFunc) sp_feColorMatrix_class_init,
            NULL, NULL,
            sizeof(SPFeColorMatrix),
            16,
            (GInstanceInitFunc) sp_feColorMatrix_init,
            NULL,    /* value_table */
        };
        feColorMatrix_type = g_type_register_static(SP_TYPE_FILTER_PRIMITIVE, "SPFeColorMatrix", &feColorMatrix_info, (GTypeFlags)0);
    }
    return feColorMatrix_type;
}

static void
sp_feColorMatrix_class_init(SPFeColorMatrixClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;
    SPFilterPrimitiveClass *sp_primitive_class = (SPFilterPrimitiveClass *)klass;
    
    feColorMatrix_parent_class = (SPFilterPrimitiveClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_feColorMatrix_build;
    sp_object_class->release = sp_feColorMatrix_release;
    sp_object_class->write = sp_feColorMatrix_write;
    sp_object_class->set = sp_feColorMatrix_set;
    sp_object_class->update = sp_feColorMatrix_update;
    sp_primitive_class->build_renderer = sp_feColorMatrix_build_renderer;
}

static void
sp_feColorMatrix_init(SPFeColorMatrix *feColorMatrix)
{
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeColorMatrix variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_feColorMatrix_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) feColorMatrix_parent_class)->build) {
        ((SPObjectClass *) feColorMatrix_parent_class)->build(object, document, repr);
    }

    /*LOAD ATTRIBUTES FROM REPR HERE*/
}

/**
 * Drops any allocated memory.
 */
static void
sp_feColorMatrix_release(SPObject *object)
{
    if (((SPObjectClass *) feColorMatrix_parent_class)->release)
        ((SPObjectClass *) feColorMatrix_parent_class)->release(object);
}

/**
 * Sets a specific value in the SPFeColorMatrix.
 */
static void
sp_feColorMatrix_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeColorMatrix *feColorMatrix = SP_FECOLORMATRIX(object);
    (void)feColorMatrix;

    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
        default:
            if (((SPObjectClass *) feColorMatrix_parent_class)->set)
                ((SPObjectClass *) feColorMatrix_parent_class)->set(object, key, value);
            break;
    }

}

/**
 * Receives update notifications.
 */
static void
sp_feColorMatrix_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    if (((SPObjectClass *) feColorMatrix_parent_class)->update) {
        ((SPObjectClass *) feColorMatrix_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_feColorMatrix_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    // Inkscape-only object, not copied during an "plain SVG" dump:
    if (flags & SP_OBJECT_WRITE_EXT) {
        if (repr) {
            // is this sane?
            repr->mergeFrom(SP_OBJECT_REPR(object), "id");
        } else {
            repr = SP_OBJECT_REPR(object)->duplicate(NULL); // FIXME
        }
    }

    if (((SPObjectClass *) feColorMatrix_parent_class)->write) {
        ((SPObjectClass *) feColorMatrix_parent_class)->write(object, repr, flags);
    }

    return repr;
}

static void sp_feColorMatrix_build_renderer(SPFilterPrimitive *primitive, NR::Filter *filter) {
    g_assert(primitive != NULL);
    g_assert(filter != NULL);

    SPFeColorMatrix *sp_colormatrix = SP_FECOLORMATRIX(primitive);

    int primitive_n = filter->add_primitive(NR::NR_FILTER_COLORMATRIX);
    NR::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    NR::FilterColorMatrix *nr_colormatrix = dynamic_cast<NR::FilterColorMatrix*>(nr_primitive);
    g_assert(nr_colormatrix != NULL);

    sp_filter_primitive_renderer_common(primitive, nr_primitive);
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
