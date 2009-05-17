#define __SP_FETILE_CPP__

/** \file
 * SVG <feTile> implementation.
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
#include "tile.h"
#include "xml/repr.h"


/* FeTile base class */

static void sp_feTile_class_init(SPFeTileClass *klass);
static void sp_feTile_init(SPFeTile *feTile);

static void sp_feTile_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_feTile_release(SPObject *object);
static void sp_feTile_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feTile_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_feTile_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_feTile_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter);

static SPFilterPrimitiveClass *feTile_parent_class;

GType
sp_feTile_get_type()
{
    static GType feTile_type = 0;

    if (!feTile_type) {
        GTypeInfo feTile_info = {
            sizeof(SPFeTileClass),
            NULL, NULL,
            (GClassInitFunc) sp_feTile_class_init,
            NULL, NULL,
            sizeof(SPFeTile),
            16,
            (GInstanceInitFunc) sp_feTile_init,
            NULL,    /* value_table */
        };
        feTile_type = g_type_register_static(SP_TYPE_FILTER_PRIMITIVE, "SPFeTile", &feTile_info, (GTypeFlags)0);
    }
    return feTile_type;
}

static void
sp_feTile_class_init(SPFeTileClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;
    SPFilterPrimitiveClass *sp_primitive_class = (SPFilterPrimitiveClass *)klass;

    feTile_parent_class = (SPFilterPrimitiveClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_feTile_build;
    sp_object_class->release = sp_feTile_release;
    sp_object_class->write = sp_feTile_write;
    sp_object_class->set = sp_feTile_set;
    sp_object_class->update = sp_feTile_update;
    sp_primitive_class->build_renderer = sp_feTile_build_renderer;
}

static void
sp_feTile_init(SPFeTile */*feTile*/)
{
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeTile variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_feTile_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) feTile_parent_class)->build) {
        ((SPObjectClass *) feTile_parent_class)->build(object, document, repr);
    }

    /*LOAD ATTRIBUTES FROM REPR HERE*/
}

/**
 * Drops any allocated memory.
 */
static void
sp_feTile_release(SPObject *object)
{
    if (((SPObjectClass *) feTile_parent_class)->release)
        ((SPObjectClass *) feTile_parent_class)->release(object);
}

/**
 * Sets a specific value in the SPFeTile.
 */
static void
sp_feTile_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeTile *feTile = SP_FETILE(object);
    (void)feTile;

    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
        default:
            if (((SPObjectClass *) feTile_parent_class)->set)
                ((SPObjectClass *) feTile_parent_class)->set(object, key, value);
            break;
    }

}

/**
 * Receives update notifications.
 */
static void
sp_feTile_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    if (((SPObjectClass *) feTile_parent_class)->update) {
        ((SPObjectClass *) feTile_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_feTile_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    /* TODO: Don't just clone, but create a new repr node and write all
     * relevant values into it */
    if (!repr) {
        repr = SP_OBJECT_REPR(object)->duplicate(doc);
    }

    if (((SPObjectClass *) feTile_parent_class)->write) {
        ((SPObjectClass *) feTile_parent_class)->write(object, doc, repr, flags);
    }

    return repr;
}

static void sp_feTile_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter) {
    g_assert(primitive != NULL);
    g_assert(filter != NULL);

    SPFeTile *sp_tile = SP_FETILE(primitive);
    (void)sp_tile;

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_TILE);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterTile *nr_tile = dynamic_cast<Inkscape::Filters::FilterTile*>(nr_primitive);
    g_assert(nr_tile != NULL);

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
