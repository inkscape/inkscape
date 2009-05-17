#define __SP_FEDISPLACEMENTMAP_CPP__

/** \file
 * SVG <feDisplacementMap> implementation.
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
#include "displacementmap.h"
#include "xml/repr.h"
#include "display/nr-filter-displacement-map.h"
#include "helper-fns.h"

/* FeDisplacementMap base class */

static void sp_feDisplacementMap_class_init(SPFeDisplacementMapClass *klass);
static void sp_feDisplacementMap_init(SPFeDisplacementMap *feDisplacementMap);

static void sp_feDisplacementMap_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_feDisplacementMap_release(SPObject *object);
static void sp_feDisplacementMap_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feDisplacementMap_update(SPObject *object, SPCtx *ctx, guint flags);
static void sp_feDisplacementMap_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter);
static Inkscape::XML::Node *sp_feDisplacementMap_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static SPFilterPrimitiveClass *feDisplacementMap_parent_class;

GType
sp_feDisplacementMap_get_type()
{
    static GType feDisplacementMap_type = 0;

    if (!feDisplacementMap_type) {
        GTypeInfo feDisplacementMap_info = {
            sizeof(SPFeDisplacementMapClass),
            NULL, NULL,
            (GClassInitFunc) sp_feDisplacementMap_class_init,
            NULL, NULL,
            sizeof(SPFeDisplacementMap),
            16,
            (GInstanceInitFunc) sp_feDisplacementMap_init,
            NULL,    /* value_table */
        };
        feDisplacementMap_type = g_type_register_static(SP_TYPE_FILTER_PRIMITIVE, "SPFeDisplacementMap", &feDisplacementMap_info, (GTypeFlags)0);
    }
    return feDisplacementMap_type;
}

static void
sp_feDisplacementMap_class_init(SPFeDisplacementMapClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;
    SPFilterPrimitiveClass *sp_primitive_class = (SPFilterPrimitiveClass *)klass;

    feDisplacementMap_parent_class = (SPFilterPrimitiveClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_feDisplacementMap_build;
    sp_object_class->release = sp_feDisplacementMap_release;
    sp_object_class->write = sp_feDisplacementMap_write;
    sp_object_class->set = sp_feDisplacementMap_set;
    sp_object_class->update = sp_feDisplacementMap_update;
    sp_primitive_class->build_renderer = sp_feDisplacementMap_build_renderer;
}

static void
sp_feDisplacementMap_init(SPFeDisplacementMap *feDisplacementMap)
{
    feDisplacementMap->scale=0;
    feDisplacementMap->xChannelSelector = DISPLACEMENTMAP_CHANNEL_ALPHA;
    feDisplacementMap->yChannelSelector = DISPLACEMENTMAP_CHANNEL_ALPHA;
    feDisplacementMap->in2 = Inkscape::Filters::NR_FILTER_SLOT_NOT_SET;
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeDisplacementMap variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_feDisplacementMap_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) feDisplacementMap_parent_class)->build) {
        ((SPObjectClass *) feDisplacementMap_parent_class)->build(object, document, repr);
    }

    /*LOAD ATTRIBUTES FROM REPR HERE*/
    sp_object_read_attr(object, "scale");
    sp_object_read_attr(object, "in2");
    sp_object_read_attr(object, "xChannelSelector");
    sp_object_read_attr(object, "yChannelSelector");

    /* Unlike normal in, in2 is required attribute. Make sure, we can call
     * it by some name. */
    SPFeDisplacementMap *disp = SP_FEDISPLACEMENTMAP(object);
    if (disp->in2 == Inkscape::Filters::NR_FILTER_SLOT_NOT_SET ||
        disp->in2 == Inkscape::Filters::NR_FILTER_UNNAMED_SLOT)
    {
        SPFilter *parent = SP_FILTER(object->parent);
        disp->in2 = sp_filter_primitive_name_previous_out(disp);
        repr->setAttribute("in2", sp_filter_name_for_image(parent, disp->in2));
    }
}

/**
 * Drops any allocated memory.
 */
static void
sp_feDisplacementMap_release(SPObject *object)
{
    if (((SPObjectClass *) feDisplacementMap_parent_class)->release)
        ((SPObjectClass *) feDisplacementMap_parent_class)->release(object);
}

static FilterDisplacementMapChannelSelector sp_feDisplacementMap_readChannelSelector(gchar const *value)
{
    if (!value) return DISPLACEMENTMAP_CHANNEL_ALPHA;
    switch (value[0]) {
        case 'R':
            return DISPLACEMENTMAP_CHANNEL_RED;
            break;
        case 'G':
            return DISPLACEMENTMAP_CHANNEL_GREEN;
            break;
        case 'B':
            return DISPLACEMENTMAP_CHANNEL_BLUE;
            break;
        case 'A':
            return DISPLACEMENTMAP_CHANNEL_ALPHA;
            break;
        default:
            // error
            g_warning("Invalid attribute for Channel Selector. Valid modes are 'R', 'G', 'B' or 'A'");
            break;
    }
    return DISPLACEMENTMAP_CHANNEL_ALPHA; //default is Alpha Channel
}

/**
 * Sets a specific value in the SPFeDisplacementMap.
 */
static void
sp_feDisplacementMap_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeDisplacementMap *feDisplacementMap = SP_FEDISPLACEMENTMAP(object);
    (void)feDisplacementMap;
    int input;
    double read_num;
    FilterDisplacementMapChannelSelector read_selector;
    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
        case SP_ATTR_XCHANNELSELECTOR:
            read_selector = sp_feDisplacementMap_readChannelSelector(value);
            if (read_selector != feDisplacementMap->xChannelSelector){
                feDisplacementMap->xChannelSelector = read_selector;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_YCHANNELSELECTOR:
            read_selector = sp_feDisplacementMap_readChannelSelector(value);
            if (read_selector != feDisplacementMap->yChannelSelector){
                feDisplacementMap->yChannelSelector = read_selector;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_SCALE:
            read_num = value ? helperfns_read_number(value) : 0;
            if (read_num != feDisplacementMap->scale) {
                feDisplacementMap->scale = read_num;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_IN2:
            input = sp_filter_primitive_read_in(feDisplacementMap, value);
            if (input != feDisplacementMap->in2) {
                feDisplacementMap->in2 = input;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        default:
            if (((SPObjectClass *) feDisplacementMap_parent_class)->set)
                ((SPObjectClass *) feDisplacementMap_parent_class)->set(object, key, value);
            break;
    }

}

/**
 * Receives update notifications.
 */
static void
sp_feDisplacementMap_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    /* Unlike normal in, in2 is required attribute. Make sure, we can call
     * it by some name. */
    SPFeDisplacementMap *disp = SP_FEDISPLACEMENTMAP(object);
    if (disp->in2 == Inkscape::Filters::NR_FILTER_SLOT_NOT_SET ||
        disp->in2 == Inkscape::Filters::NR_FILTER_UNNAMED_SLOT)
    {
        SPFilter *parent = SP_FILTER(object->parent);
        disp->in2 = sp_filter_primitive_name_previous_out(disp);
        object->repr->setAttribute("in2", sp_filter_name_for_image(parent, disp->in2));
    }

    if (((SPObjectClass *) feDisplacementMap_parent_class)->update) {
        ((SPObjectClass *) feDisplacementMap_parent_class)->update(object, ctx, flags);
    }
}

static char const * get_channelselector_name(FilterDisplacementMapChannelSelector selector) {
    switch(selector) {
        case DISPLACEMENTMAP_CHANNEL_RED:
            return "R";
        case DISPLACEMENTMAP_CHANNEL_GREEN:
            return "G";
        case DISPLACEMENTMAP_CHANNEL_BLUE:
            return "B";
        case DISPLACEMENTMAP_CHANNEL_ALPHA:
            return "A";
        default:
            return 0;
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_feDisplacementMap_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    SPFeDisplacementMap *disp = SP_FEDISPLACEMENTMAP(object);
    SPFilter *parent = SP_FILTER(object->parent);

    if (!repr) {
        repr = doc->createElement("svg:feDisplacementMap");
    }

    gchar const *out_name = sp_filter_name_for_image(parent, disp->in2);
    if (out_name) {
        repr->setAttribute("in2", out_name);
    } else {
        SPObject *i = parent->children;
        while (i && i->next != object) i = i->next;
        SPFilterPrimitive *i_prim = SP_FILTER_PRIMITIVE(i);
        out_name = sp_filter_name_for_image(parent, i_prim->image_out);
        repr->setAttribute("in2", out_name);
        if (!out_name) {
            g_warning("Unable to set in2 for feDisplacementMap");
        }
    }

    sp_repr_set_svg_double(repr, "scale", disp->scale);
    repr->setAttribute("xChannelSelector",
                       get_channelselector_name(disp->xChannelSelector));
    repr->setAttribute("yChannelSelector",
                       get_channelselector_name(disp->yChannelSelector));

    if (((SPObjectClass *) feDisplacementMap_parent_class)->write) {
        ((SPObjectClass *) feDisplacementMap_parent_class)->write(object, doc, repr, flags);
    }

    return repr;
}

static void sp_feDisplacementMap_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter) {
    g_assert(primitive != NULL);
    g_assert(filter != NULL);

    SPFeDisplacementMap *sp_displacement_map = SP_FEDISPLACEMENTMAP(primitive);

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_DISPLACEMENTMAP);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterDisplacementMap *nr_displacement_map = dynamic_cast<Inkscape::Filters::FilterDisplacementMap*>(nr_primitive);
    g_assert(nr_displacement_map != NULL);

    sp_filter_primitive_renderer_common(primitive, nr_primitive);

    nr_displacement_map->set_input(1, sp_displacement_map->in2);
    nr_displacement_map->set_scale(sp_displacement_map->scale);
    nr_displacement_map->set_channel_selector(0, sp_displacement_map->xChannelSelector);
    nr_displacement_map->set_channel_selector(1, sp_displacement_map->yChannelSelector);
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
