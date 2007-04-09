#define __SP_FEBLEND_CPP__

/** \file
 * SVG <feBlend> implementation.
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
#include "sp-feblend.h"
#include "xml/repr.h"


/* FeBlend base class */

static void sp_feBlend_class_init(SPFeBlendClass *klass);
static void sp_feBlend_init(SPFeBlend *feBlend);

static void sp_feBlend_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_feBlend_release(SPObject *object);
static void sp_feBlend_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feBlend_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_feBlend_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

static SPFilterPrimitiveClass *feBlend_parent_class;

GType
sp_feBlend_get_type()
{
    static GType feBlend_type = 0;

    if (!feBlend_type) {
        GTypeInfo feBlend_info = {
            sizeof(SPFeBlendClass),
            NULL, NULL,
            (GClassInitFunc) sp_feBlend_class_init,
            NULL, NULL,
            sizeof(SPFeBlend),
            16,
            (GInstanceInitFunc) sp_feBlend_init,
            NULL,    /* value_table */
        };
        feBlend_type = g_type_register_static(SP_TYPE_FILTER_PRIMITIVE, "SPFeBlend", &feBlend_info, (GTypeFlags)0);
    }
    return feBlend_type;
}

static void
sp_feBlend_class_init(SPFeBlendClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;

    feBlend_parent_class = (SPFilterPrimitiveClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_feBlend_build;
    sp_object_class->release = sp_feBlend_release;
    sp_object_class->write = sp_feBlend_write;
    sp_object_class->set = sp_feBlend_set;
    sp_object_class->update = sp_feBlend_update;
}

static void
sp_feBlend_init(SPFeBlend *feBlend)
{
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeBlend variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_feBlend_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) feBlend_parent_class)->build) {
        ((SPObjectClass *) feBlend_parent_class)->build(object, document, repr);
    }

    /*LOAD ATTRIBUTES FROM REPR HERE*/
}

/**
 * Drops any allocated memory.
 */
static void
sp_feBlend_release(SPObject *object)
{
    if (((SPObjectClass *) feBlend_parent_class)->release)
        ((SPObjectClass *) feBlend_parent_class)->release(object);
}

/**
 * Sets a specific value in the SPFeBlend.
 */
static void
sp_feBlend_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeBlend *feBlend = SP_FEBLEND(object);
    (void)feBlend;

    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
        default:
            if (((SPObjectClass *) feBlend_parent_class)->set)
                ((SPObjectClass *) feBlend_parent_class)->set(object, key, value);
            break;
    }

}

/**
 * Receives update notifications.
 */
static void
sp_feBlend_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    if (((SPObjectClass *) feBlend_parent_class)->update) {
        ((SPObjectClass *) feBlend_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_feBlend_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
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

    if (((SPObjectClass *) feBlend_parent_class)->write) {
        ((SPObjectClass *) feBlend_parent_class)->write(object, repr, flags);
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
