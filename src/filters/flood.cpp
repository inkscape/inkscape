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
#include "flood.h"
#include "xml/repr.h"
#include "helper-fns.h"
#include "svg/svg-color.h"

/* FeFlood base class */

static void sp_feFlood_class_init(SPFeFloodClass *klass);
static void sp_feFlood_init(SPFeFlood *feFlood);

static void sp_feFlood_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_feFlood_release(SPObject *object);
static void sp_feFlood_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feFlood_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_feFlood_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_feFlood_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter);

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
    SPFilterPrimitiveClass *sp_primitive_class = (SPFilterPrimitiveClass *)klass;

    feFlood_parent_class = (SPFilterPrimitiveClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_feFlood_build;
    sp_object_class->release = sp_feFlood_release;
    sp_object_class->write = sp_feFlood_write;
    sp_object_class->set = sp_feFlood_set;
    sp_object_class->update = sp_feFlood_update;
    sp_primitive_class->build_renderer = sp_feFlood_build_renderer;
}

static void
sp_feFlood_init(SPFeFlood *feFlood)
{
    feFlood->opacity = 1;
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
    sp_object_read_attr(object, "flood-opacity");
    sp_object_read_attr(object, "flood-color");
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
    (void)feFlood;
    gchar const *cend_ptr = NULL;
    gchar *end_ptr = NULL;
    guint32 read_color;
    double read_num;
    
    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
        case SP_PROP_FLOOD_COLOR:
            cend_ptr = NULL;
            read_color = sp_svg_read_color(value, &cend_ptr, 0xffffffff);
            if (cend_ptr && read_color != feFlood->color){
                feFlood->color = read_color;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_PROP_FLOOD_OPACITY:
            if (value) {
                read_num = g_ascii_strtod(value, &end_ptr);
                if (*end_ptr) {
                    g_warning("Unable to convert \"%s\" to number", value);
                    read_num = 1;
                }
            }
            else {
                read_num = 1;
            }
            if (read_num != feFlood->opacity){
                feFlood->opacity = read_num;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
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
sp_feFlood_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    /* TODO: Don't just clone, but create a new repr node and write all
     * relevant values into it */
    if (!repr) {
        repr = SP_OBJECT_REPR(object)->duplicate(doc);
    }

    if (((SPObjectClass *) feFlood_parent_class)->write) {
        ((SPObjectClass *) feFlood_parent_class)->write(object, doc, repr, flags);
    }

    return repr;
}

static void sp_feFlood_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter) {
    g_assert(primitive != NULL);
    g_assert(filter != NULL);

    SPFeFlood *sp_flood = SP_FEFLOOD(primitive);
    (void)sp_flood;

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_FLOOD);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterFlood *nr_flood = dynamic_cast<Inkscape::Filters::FilterFlood*>(nr_primitive);
    g_assert(nr_flood != NULL);

    sp_filter_primitive_renderer_common(primitive, nr_primitive);
    
    nr_flood->set_opacity(sp_flood->opacity);
    nr_flood->set_color(sp_flood->color);
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
