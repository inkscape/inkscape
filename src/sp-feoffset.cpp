#define __SP_FEOFFSET_CPP__

/** \file
 * SVG <feOffset> implementation.
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
#include "sp-feoffset.h"
#include "xml/repr.h"


/* FeOffset base class */

static void sp_feOffset_class_init(SPFeOffsetClass *klass);
static void sp_feOffset_init(SPFeOffset *feOffset);

static void sp_feOffset_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_feOffset_release(SPObject *object);
static void sp_feOffset_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feOffset_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_feOffset_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

static SPFilterPrimitiveClass *feOffset_parent_class;

GType
sp_feOffset_get_type()
{
    static GType feOffset_type = 0;

    if (!feOffset_type) {
        GTypeInfo feOffset_info = {
            sizeof(SPFeOffsetClass),
            NULL, NULL,
            (GClassInitFunc) sp_feOffset_class_init,
            NULL, NULL,
            sizeof(SPFeOffset),
            16,
            (GInstanceInitFunc) sp_feOffset_init,
            NULL,    /* value_table */
        };
        feOffset_type = g_type_register_static(SP_TYPE_FILTER_PRIMITIVE, "SPFeOffset", &feOffset_info, (GTypeFlags)0);
    }
    return feOffset_type;
}

static void
sp_feOffset_class_init(SPFeOffsetClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;

    feOffset_parent_class = (SPFilterPrimitiveClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_feOffset_build;
    sp_object_class->release = sp_feOffset_release;
    sp_object_class->write = sp_feOffset_write;
    sp_object_class->set = sp_feOffset_set;
    sp_object_class->update = sp_feOffset_update;
}

static void
sp_feOffset_init(SPFeOffset *feOffset)
{
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeOffset variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_feOffset_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) feOffset_parent_class)->build) {
        ((SPObjectClass *) feOffset_parent_class)->build(object, document, repr);
    }

    /*LOAD ATTRIBUTES FROM REPR HERE*/
}

/**
 * Drops any allocated memory.
 */
static void
sp_feOffset_release(SPObject *object)
{
    if (((SPObjectClass *) feOffset_parent_class)->release)
        ((SPObjectClass *) feOffset_parent_class)->release(object);
}

/**
 * Sets a specific value in the SPFeOffset.
 */
static void
sp_feOffset_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeOffset *feOffset = SP_FEOFFSET(object);
    (void)feOffset;

    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
        default:
            if (((SPObjectClass *) feOffset_parent_class)->set)
                ((SPObjectClass *) feOffset_parent_class)->set(object, key, value);
            break;
    }

}

/**
 * Receives update notifications.
 */
static void
sp_feOffset_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    if (((SPObjectClass *) feOffset_parent_class)->update) {
        ((SPObjectClass *) feOffset_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_feOffset_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    // Inkscape-only object, not copied during an "plain SVG" dump:
    if (flags & SP_OBJECT_WRITE_EXT) {
        if (repr) {
            // is this sane?
            repr->mergeFrom(SP_OBJECT_REPR(object), "id");
        } else {
            repr = SP_OBJECT_REPR(object)->duplicate(repr->document());
        }
    }

    if (((SPObjectClass *) feOffset_parent_class)->write) {
        ((SPObjectClass *) feOffset_parent_class)->write(object, repr, flags);
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
