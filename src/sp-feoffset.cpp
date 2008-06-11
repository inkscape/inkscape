#define __SP_FEOFFSET_CPP__

/** \file
 * SVG <feOffset> implementation.
 *
 */
/*
 * Authors:
 *   hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006,2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "attributes.h"
#include "svg/svg.h"
#include "sp-feoffset.h"
#include "helper-fns.h"
#include "xml/repr.h"
#include "display/nr-filter-offset.h"

/* FeOffset base class */

static void sp_feOffset_class_init(SPFeOffsetClass *klass);
static void sp_feOffset_init(SPFeOffset *feOffset);

static void sp_feOffset_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_feOffset_release(SPObject *object);
static void sp_feOffset_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feOffset_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_feOffset_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_feOffset_build_renderer(SPFilterPrimitive *primitive, NR::Filter *filter);

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
    SPFilterPrimitiveClass *sp_primitive_class = (SPFilterPrimitiveClass *)klass;

    feOffset_parent_class = (SPFilterPrimitiveClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_feOffset_build;
    sp_object_class->release = sp_feOffset_release;
    sp_object_class->write = sp_feOffset_write;
    sp_object_class->set = sp_feOffset_set;
    sp_object_class->update = sp_feOffset_update;

    sp_primitive_class->build_renderer = sp_feOffset_build_renderer;
}

static void
sp_feOffset_init(SPFeOffset *feOffset)
{
    feOffset->dx = 0;
    feOffset->dy = 0;
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

    sp_object_read_attr(object, "dx");
    sp_object_read_attr(object, "dy");
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

    double read_num;
    switch(key) {
        case SP_ATTR_DX:
            read_num = helperfns_read_number(value);
            if (read_num != feOffset->dx) {
                feOffset->dx = read_num;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_DY:
            read_num = helperfns_read_number(value);
            if (read_num != feOffset->dy) {
                feOffset->dy = read_num;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
            
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
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        sp_object_read_attr(object, "dx");
        sp_object_read_attr(object, "dy");
    }

    if (((SPObjectClass *) feOffset_parent_class)->update) {
        ((SPObjectClass *) feOffset_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_feOffset_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    // Inkscape-only object, not copied during an "plain SVG" dump:
    if (flags & SP_OBJECT_WRITE_EXT) {
        if (repr) {
            // is this sane?
            // Not. Causes coredumps.
            // repr->mergeFrom(SP_OBJECT_REPR(object), "id");
        } else {
            repr = SP_OBJECT_REPR(object)->duplicate(doc);
        }
    }

    if (((SPObjectClass *) feOffset_parent_class)->write) {
        ((SPObjectClass *) feOffset_parent_class)->write(object, doc, repr, flags);
    }

    return repr;
}

static void sp_feOffset_build_renderer(SPFilterPrimitive *primitive, NR::Filter *filter) {
    g_assert(primitive != NULL);
    g_assert(filter != NULL);

    SPFeOffset *sp_offset = SP_FEOFFSET(primitive);

    int primitive_n = filter->add_primitive(NR::NR_FILTER_OFFSET);
    NR::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    NR::FilterOffset *nr_offset = dynamic_cast<NR::FilterOffset*>(nr_primitive);
    g_assert(nr_offset != NULL);

    sp_filter_primitive_renderer_common(primitive, nr_primitive);

    nr_offset->set_dx(sp_offset->dx);
    nr_offset->set_dy(sp_offset->dy);
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
