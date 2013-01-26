/** \file
 * SVG <feMorphology> implementation.
 *
 */
/*
 * Authors:
 *   Felipe Sanches <juca@members.fsf.org>
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>

#include "attributes.h"
#include "svg/svg.h"
#include "morphology.h"
#include "xml/repr.h"
#include "display/nr-filter.h"
#include "display/nr-filter-morphology.h"

/* FeMorphology base class */
static void sp_feMorphology_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_feMorphology_release(SPObject *object);
static void sp_feMorphology_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feMorphology_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_feMorphology_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_feMorphology_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter);

G_DEFINE_TYPE(SPFeMorphology, sp_feMorphology, SP_TYPE_FILTER_PRIMITIVE);

static void
sp_feMorphology_class_init(SPFeMorphologyClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;
    SPFilterPrimitiveClass *sp_primitive_class = (SPFilterPrimitiveClass *)klass;

    sp_object_class->build = sp_feMorphology_build;
    sp_object_class->release = sp_feMorphology_release;
    sp_object_class->write = sp_feMorphology_write;
    sp_object_class->set = sp_feMorphology_set;
    sp_object_class->update = sp_feMorphology_update;
    sp_primitive_class->build_renderer = sp_feMorphology_build_renderer;
}

static void
sp_feMorphology_init(SPFeMorphology *feMorphology)
{
    //Setting default values:
    feMorphology->radius.set("0");
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeMorphology variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_feMorphology_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) sp_feMorphology_parent_class)->build) {
        ((SPObjectClass *) sp_feMorphology_parent_class)->build(object, document, repr);
    }

    /*LOAD ATTRIBUTES FROM REPR HERE*/
    object->readAttr( "operator" );
    object->readAttr( "radius" );
}

/**
 * Drops any allocated memory.
 */
static void
sp_feMorphology_release(SPObject *object)
{
    if (((SPObjectClass *) sp_feMorphology_parent_class)->release)
        ((SPObjectClass *) sp_feMorphology_parent_class)->release(object);
}

static Inkscape::Filters::FilterMorphologyOperator sp_feMorphology_read_operator(gchar const *value){
    if (!value) return Inkscape::Filters::MORPHOLOGY_OPERATOR_ERODE; //erode is default
    switch(value[0]){
        case 'e':
            if (strncmp(value, "erode", 5) == 0) return Inkscape::Filters::MORPHOLOGY_OPERATOR_ERODE;
            break;
        case 'd':
            if (strncmp(value, "dilate", 6) == 0) return Inkscape::Filters::MORPHOLOGY_OPERATOR_DILATE;
            break;
    }
    return Inkscape::Filters::MORPHOLOGY_OPERATOR_ERODE; //erode is default
}

/**
 * Sets a specific value in the SPFeMorphology.
 */
static void
sp_feMorphology_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeMorphology *feMorphology = SP_FEMORPHOLOGY(object);
    (void)feMorphology;
    
    Inkscape::Filters::FilterMorphologyOperator read_operator;
    switch(key) {
    /*DEAL WITH SETTING ATTRIBUTES HERE*/
        case SP_ATTR_OPERATOR:
            read_operator = sp_feMorphology_read_operator(value);
            if (read_operator != feMorphology->Operator){
                feMorphology->Operator = read_operator;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_RADIUS:
            feMorphology->radius.set(value);
            //From SVG spec: If <y-radius> is not provided, it defaults to <x-radius>.
            if (feMorphology->radius.optNumIsSet() == false)
                feMorphology->radius.setOptNumber(feMorphology->radius.getNumber());
            object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        default:
            if (((SPObjectClass *) sp_feMorphology_parent_class)->set)
                ((SPObjectClass *) sp_feMorphology_parent_class)->set(object, key, value);
            break;
    }

}

/**
 * Receives update notifications.
 */
static void
sp_feMorphology_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    if (((SPObjectClass *) sp_feMorphology_parent_class)->update) {
        ((SPObjectClass *) sp_feMorphology_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_feMorphology_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    /* TODO: Don't just clone, but create a new repr node and write all
     * relevant values into it */
    if (!repr) {
        repr = object->getRepr()->duplicate(doc);
    }

    if (((SPObjectClass *) sp_feMorphology_parent_class)->write) {
        ((SPObjectClass *) sp_feMorphology_parent_class)->write(object, doc, repr, flags);
    }

    return repr;
}

static void sp_feMorphology_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter) {
    g_assert(primitive != NULL);
    g_assert(filter != NULL);

    SPFeMorphology *sp_morphology = SP_FEMORPHOLOGY(primitive);

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_MORPHOLOGY);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterMorphology *nr_morphology = dynamic_cast<Inkscape::Filters::FilterMorphology*>(nr_primitive);
    g_assert(nr_morphology != NULL);

    sp_filter_primitive_renderer_common(primitive, nr_primitive); 
    
    nr_morphology->set_operator(sp_morphology->Operator);
    nr_morphology->set_xradius( sp_morphology->radius.getNumber() );
    nr_morphology->set_yradius( sp_morphology->radius.getOptNumber() );
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
