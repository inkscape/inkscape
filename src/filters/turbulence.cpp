#define __SP_FETURBULENCE_CPP__

/** \file
 * SVG <feTurbulence> implementation.
 *
 */
/*
 * Authors:
 *   Felipe Corrêa da Silva Sanches <felipe.sanches@gmail.com>
 *   hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2007 Felipe Sanches
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "attributes.h"
#include "svg/svg.h"
#include "turbulence.h"
#include "helper-fns.h"
#include "xml/repr.h"
#include <string.h>

#include "display/nr-filter.h"
#include "display/nr-filter-turbulence.h"

/* FeTurbulence base class */

static void sp_feTurbulence_class_init(SPFeTurbulenceClass *klass);
static void sp_feTurbulence_init(SPFeTurbulence *feTurbulence);

static void sp_feTurbulence_build(SPObject *object, Document *document, Inkscape::XML::Node *repr);
static void sp_feTurbulence_release(SPObject *object);
static void sp_feTurbulence_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feTurbulence_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_feTurbulence_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_feTurbulence_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter);

static SPFilterPrimitiveClass *feTurbulence_parent_class;

GType
sp_feTurbulence_get_type()
{
    static GType feTurbulence_type = 0;

    if (!feTurbulence_type) {
        GTypeInfo feTurbulence_info = {
            sizeof(SPFeTurbulenceClass),
            NULL, NULL,
            (GClassInitFunc) sp_feTurbulence_class_init,
            NULL, NULL,
            sizeof(SPFeTurbulence),
            16,
            (GInstanceInitFunc) sp_feTurbulence_init,
            NULL,    /* value_table */
        };
        feTurbulence_type = g_type_register_static(SP_TYPE_FILTER_PRIMITIVE, "SPFeTurbulence", &feTurbulence_info, (GTypeFlags)0);
    }
    return feTurbulence_type;
}

static void
sp_feTurbulence_class_init(SPFeTurbulenceClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;
    SPFilterPrimitiveClass * sp_primitive_class = (SPFilterPrimitiveClass *)klass;

    feTurbulence_parent_class = (SPFilterPrimitiveClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_feTurbulence_build;
    sp_object_class->release = sp_feTurbulence_release;
    sp_object_class->write = sp_feTurbulence_write;
    sp_object_class->set = sp_feTurbulence_set;
    sp_object_class->update = sp_feTurbulence_update;

    sp_primitive_class->build_renderer = sp_feTurbulence_build_renderer;
}

static void
sp_feTurbulence_init(SPFeTurbulence *feTurbulence)
{
    feTurbulence->updated=false;
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeTurbulence variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_feTurbulence_build(SPObject *object, Document *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) feTurbulence_parent_class)->build) {
        ((SPObjectClass *) feTurbulence_parent_class)->build(object, document, repr);
    }

    /*LOAD ATTRIBUTES FROM REPR HERE*/
    sp_object_read_attr(object, "baseFrequency");
    sp_object_read_attr(object, "numOctaves");
    sp_object_read_attr(object, "seed");
    sp_object_read_attr(object, "stitchTiles");
    sp_object_read_attr(object, "type");
}

/**
 * Drops any allocated memory.
 */
static void
sp_feTurbulence_release(SPObject *object)
{
    if (((SPObjectClass *) feTurbulence_parent_class)->release)
        ((SPObjectClass *) feTurbulence_parent_class)->release(object);
}

static bool sp_feTurbulence_read_stitchTiles(gchar const *value){
    if (!value) return false; // 'noStitch' is default
    switch(value[0]){
        case 's':
            if (strncmp(value, "stitch", 6) == 0) return true;
            break;
        case 'n':
            if (strncmp(value, "noStitch", 8) == 0) return false;
            break;
    }
    return false; // 'noStitch' is default
}

static Inkscape::Filters::FilterTurbulenceType sp_feTurbulence_read_type(gchar const *value){
    if (!value) return Inkscape::Filters::TURBULENCE_TURBULENCE; // 'turbulence' is default
    switch(value[0]){
        case 'f':
            if (strncmp(value, "fractalNoise", 12) == 0) return Inkscape::Filters::TURBULENCE_FRACTALNOISE;
            break;
        case 't':
            if (strncmp(value, "turbulence", 10) == 0) return Inkscape::Filters::TURBULENCE_TURBULENCE;
            break;
    }
    return Inkscape::Filters::TURBULENCE_TURBULENCE; // 'turbulence' is default
}

/**
 * Sets a specific value in the SPFeTurbulence.
 */
static void
sp_feTurbulence_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeTurbulence *feTurbulence = SP_FETURBULENCE(object);
    (void)feTurbulence;

    int read_int;
    double read_num;
    bool read_bool;
    Inkscape::Filters::FilterTurbulenceType read_type;

    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/

        case SP_ATTR_BASEFREQUENCY:
            feTurbulence->baseFrequency.set(value);
                //From SVG spec: If two <number>s are provided, the first number represents a base frequency in the X direction and the second value represents a base frequency in the Y direction. If one number is provided, then that value is used for both X and Y.
            if (feTurbulence->baseFrequency.optNumIsSet() == false)
                feTurbulence->baseFrequency.setOptNumber(feTurbulence->baseFrequency.getNumber());
            feTurbulence->updated = false;
            object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_NUMOCTAVES:
            read_int = value ? (int)floor(helperfns_read_number(value)) : 1;
            if (read_int != feTurbulence->numOctaves){
                feTurbulence->numOctaves = read_int;
                feTurbulence->updated = false;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_SEED:
            read_num = value ? helperfns_read_number(value) : 0;
            if (read_num != feTurbulence->seed){
                feTurbulence->seed = read_num;
                feTurbulence->updated = false;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_STITCHTILES:
            read_bool = sp_feTurbulence_read_stitchTiles(value);
            if (read_bool != feTurbulence->stitchTiles){
                feTurbulence->stitchTiles = read_bool;
                feTurbulence->updated = false;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_TYPE:
            read_type = sp_feTurbulence_read_type(value);
            if (read_type != feTurbulence->type){
                feTurbulence->type = read_type;
                feTurbulence->updated = false;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        default:
            if (((SPObjectClass *) feTurbulence_parent_class)->set)
                ((SPObjectClass *) feTurbulence_parent_class)->set(object, key, value);
            break;
    }

}

/**
 * Receives update notifications.
 */
static void
sp_feTurbulence_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    if (((SPObjectClass *) feTurbulence_parent_class)->update) {
        ((SPObjectClass *) feTurbulence_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_feTurbulence_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    /* TODO: Don't just clone, but create a new repr node and write all
     * relevant values into it */
    if (!repr) {
        repr = SP_OBJECT_REPR(object)->duplicate(doc);
    }

    if (((SPObjectClass *) feTurbulence_parent_class)->write) {
        ((SPObjectClass *) feTurbulence_parent_class)->write(object, doc, repr, flags);
    }

    /* turbulence doesn't take input */
    repr->setAttribute("in", 0);

    return repr;
}

static void sp_feTurbulence_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter) {
    g_assert(primitive != NULL);
    g_assert(filter != NULL);

    SPFeTurbulence *sp_turbulence = SP_FETURBULENCE(primitive);

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_TURBULENCE);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterTurbulence *nr_turbulence = dynamic_cast<Inkscape::Filters::FilterTurbulence*>(nr_primitive);
    g_assert(nr_turbulence != NULL);

    sp_filter_primitive_renderer_common(primitive, nr_primitive);

    nr_turbulence->set_baseFrequency(0, sp_turbulence->baseFrequency.getNumber());
    nr_turbulence->set_baseFrequency(1, sp_turbulence->baseFrequency.getOptNumber());
    nr_turbulence->set_numOctaves(sp_turbulence->numOctaves);
    nr_turbulence->set_seed(sp_turbulence->seed);
    nr_turbulence->set_stitchTiles(sp_turbulence->stitchTiles);
    nr_turbulence->set_type(sp_turbulence->type);
    nr_turbulence->set_updated(sp_turbulence->updated);
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
