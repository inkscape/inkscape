#define __SP_FEFLOOD_CPP__

/** \file
 * SVG <feFlood> implementation.
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
#include "sp-feflood.h"
#include "xml/repr.h"


/* FeFlood base class */

static void sp_feFlood_class_init(SPFeFloodClass *klass);
static void sp_feFlood_init(SPFeFlood *feFlood);

static void sp_feFlood_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_feFlood_release(SPObject *object);
static void sp_feFlood_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feFlood_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_feFlood_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

static SPFilterPrimitiveClass *feFlood_parent_class;

GType
sp_feFlood_get_type()
{
    static GType feFlood_type = 0;

    if (!feFlood_type) {
        GTypeInfo feFlood_info = {
            sizeof(SPFeFloodClass),
            NULL, NULL,
            (GClassInitFunc) sp_feFlood_class_init,
            NULL, NULL,
            sizeof(SPFeFlood),
            16,
            (GInstanceInitFunc) sp_feFlood_init,
            NULL,    /* value_table */
        };
        feFlood_type = g_type_register_static(SP_TYPE_FILTER_PRIMITIVE, "SPFeFlood", &feFlood_info, (GTypeFlags)0);
    }
    return feFlood_type;
}

static void
sp_feFlood_class_init(SPFeFloodClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;

    feFlood_parent_class = (SPFilterPrimitiveClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_feFlood_build;
    sp_object_class->release = sp_feFlood_release;
    sp_object_class->write = sp_feFlood_write;
    sp_object_class->set = sp_feFlood_set;
    sp_object_class->update = sp_feFlood_update;
}

static void
sp_feFlood_init(SPFeFlood *feFlood)
{
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeFlood variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_feFlood_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) feFlood_parent_class)->build) {
        ((SPObjectClass *) feFlood_parent_class)->build(object, document, repr);
    }

    /*LOAD ATTRIBUTES FROM REPR HERE*/
}

/**
 * Drops any allocated memory.
 */
static void
sp_feFlood_release(SPObject *object)
{
    if (((SPObjectClass *) feFlood_parent_class)->release)
        ((SPObjectClass *) feFlood_parent_class)->release(object);
}

/**
 * Sets a specific value in the SPFeFlood.
 */
static void
sp_feFlood_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeFlood *feFlood = SP_FEFLOOD(object);

    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
        default:
            if (((SPObjectClass *) feFlood_parent_class)->set)
                ((SPObjectClass *) feFlood_parent_class)->set(object, key, value);
            break;
    }

}

/**
 * Receives update notifications.
 */
static void
sp_feFlood_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    if (((SPObjectClass *) feFlood_parent_class)->update) {
        ((SPObjectClass *) feFlood_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_feFlood_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    // Inkscape-only object, not copied during an "plain SVG" dump:
    if (flags & SP_OBJECT_WRITE_EXT) {
        if (repr) {
            // is this sane?
            repr->mergeFrom(SP_OBJECT_REPR(object), "id");
        } else {
            repr = SP_OBJECT_REPR(object)->duplicate();
        }
    }

    if (((SPObjectClass *) feFlood_parent_class)->write) {
        ((SPObjectClass *) feFlood_parent_class)->write(object, repr, flags);
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
