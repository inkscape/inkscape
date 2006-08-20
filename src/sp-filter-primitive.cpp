#define __SP_FILTER_PRIMITIVE_CPP__

/** \file
 * Superclass for all the filter primitives
 *
 */
/*
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

#include "attributes.h"
#include "sp-filter-primitive.h"
#include "xml/repr.h"


/* FilterPrimitive base class */

static void sp_filter_primitive_class_init(SPFilterPrimitiveClass *klass);
static void sp_filter_primitive_init(SPFilterPrimitive *filter_primitive);

static void sp_filter_primitive_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_filter_primitive_release(SPObject *object);
static void sp_filter_primitive_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_filter_primitive_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_filter_primitive_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

static SPObjectClass *filter_primitive_parent_class;

GType
sp_filter_primitive_get_type()
{
    static GType filter_primitive_type = 0;

    if (!filter_primitive_type) {
        GTypeInfo filter_primitive_info = {
            sizeof(SPFilterPrimitiveClass),
            NULL, NULL,
            (GClassInitFunc) sp_filter_primitive_class_init,
            NULL, NULL,
            sizeof(SPFilterPrimitive),
            16,
            (GInstanceInitFunc) sp_filter_primitive_init,
            NULL,    /* value_table */
        };
        filter_primitive_type = g_type_register_static(SP_TYPE_OBJECT, "SPFilterPrimitive", &filter_primitive_info, (GTypeFlags)0);
    }
    return filter_primitive_type;
}

static void
sp_filter_primitive_class_init(SPFilterPrimitiveClass *klass)
{
    //GObjectClass *gobject_class = (GObjectClass *)klass;
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;

    filter_primitive_parent_class = (SPObjectClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_filter_primitive_build;
    sp_object_class->release = sp_filter_primitive_release;
    sp_object_class->write = sp_filter_primitive_write;
    sp_object_class->set = sp_filter_primitive_set;
    sp_object_class->update = sp_filter_primitive_update;
}

static void
sp_filter_primitive_init(SPFilterPrimitive *filter_primitive)
{
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFilterPrimitive variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_filter_primitive_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) filter_primitive_parent_class)->build) {
        ((SPObjectClass *) filter_primitive_parent_class)->build(object, document, repr);
    }

}

/**
 * Drops any allocated memory.
 */
static void
sp_filter_primitive_release(SPObject *object)
{
    /* deal with our children and our selves here */
    if (((SPObjectClass *) filter_primitive_parent_class)->release)
        ((SPObjectClass *) filter_primitive_parent_class)->release(object);
}

/**
 * Sets a specific value in the SPFilterPrimitive.
 */
static void
sp_filter_primitive_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFilterPrimitive *filter_primitive = SP_FILTER_PRIMITIVE(object);

    /* See if any parents need this value. */
    if (((SPObjectClass *) filter_primitive_parent_class)->set) {
        ((SPObjectClass *) filter_primitive_parent_class)->set(object, key, value);
    }
}

/**
 * Receives update notifications.
 */
static void
sp_filter_primitive_update(SPObject *object, SPCtx *ctx, guint flags)
{
    //SPFilterPrimitive *filter_primitive = SP_FILTER_PRIMITIVE(object);

    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */
    }

    if (((SPObjectClass *) filter_primitive_parent_class)->update) {
        ((SPObjectClass *) filter_primitive_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_filter_primitive_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    //SPFilterPrimitive *filterPrimitive = SP_FILTER_PRIMITIVE(object);

    // Inkscape-only object, not copied during an "plain SVG" dump:
    if (flags & SP_OBJECT_WRITE_EXT) {
        if (repr) {
            // is this sane?
            repr->mergeFrom(SP_OBJECT_REPR(object), "id");
        } else {
            repr = SP_OBJECT_REPR(object)->duplicate();
        }
    }

    if (((SPObjectClass *) filter_primitive_parent_class)->write) {
        ((SPObjectClass *) filter_primitive_parent_class)->write(object, repr, flags);
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
