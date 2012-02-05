/** \file
 * Superclass for all the filter primitives
 *
 */
/*
 * Authors:
 *   Kees Cook <kees@outflux.net>
 *   Niko Kiirala <niko@kiirala.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2004-2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>

#include "attributes.h"
#include "sp-filter-primitive.h"
#include "xml/repr.h"
#include "sp-filter.h"
#include "display/nr-filter-primitive.h"
#include "display/nr-filter-types.h"

/* FilterPrimitive base class */

static void sp_filter_primitive_class_init(SPFilterPrimitiveClass *klass);
static void sp_filter_primitive_init(SPFilterPrimitive *filter_primitive);

static void sp_filter_primitive_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_filter_primitive_release(SPObject *object);
static void sp_filter_primitive_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_filter_primitive_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_filter_primitive_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static SPObjectClass *filter_primitive_parent_class;

GType sp_filter_primitive_get_type()
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

static void sp_filter_primitive_class_init(SPFilterPrimitiveClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)(klass);

    filter_primitive_parent_class = static_cast<SPObjectClass *>(g_type_class_peek_parent(klass));

    sp_object_class->build = sp_filter_primitive_build;
    sp_object_class->release = sp_filter_primitive_release;
    sp_object_class->write = sp_filter_primitive_write;
    sp_object_class->set = sp_filter_primitive_set;
    sp_object_class->update = sp_filter_primitive_update;
    
    /* This should never be called on this base class, but only on derived
     * classes. */
    klass->build_renderer = NULL;
}

static void sp_filter_primitive_init(SPFilterPrimitive *filter_primitive)
{
    filter_primitive->image_in = Inkscape::Filters::NR_FILTER_SLOT_NOT_SET;
    filter_primitive->image_out = Inkscape::Filters::NR_FILTER_SLOT_NOT_SET;

    // We must keep track if a value is set or not, if not set then the region defaults to 0%, 0%,
    // 100%, 100% ("x", "y", "width", "height") of the -> filter <- region.  If set then
    // percentages are in terms of bounding box or viewbox, depending on value of "primitiveUnits"

    // NB: SVGLength.set takes prescaled percent values: 1 means 100%
    filter_primitive->x.unset(SVGLength::PERCENT, 0, 0);
    filter_primitive->y.unset(SVGLength::PERCENT, 0, 0);
    filter_primitive->width.unset(SVGLength::PERCENT, 1, 0);
    filter_primitive->height.unset(SVGLength::PERCENT, 1, 0);
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFilterPrimitive variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_filter_primitive_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if ((static_cast<SPObjectClass *>(filter_primitive_parent_class))->build) {
        (static_cast<SPObjectClass *>(filter_primitive_parent_class))->build(object, document, repr);
    }

    object->readAttr( "in" );
    object->readAttr( "result" );
    object->readAttr( "x" );
    object->readAttr( "y" );
    object->readAttr( "width" );
    object->readAttr( "height" );
}

/**
 * Drops any allocated memory.
 */
static void sp_filter_primitive_release(SPObject *object)
{
    /* deal with our children and our selves here */
    if ((static_cast<SPObjectClass *>(filter_primitive_parent_class))->release)
        (static_cast<SPObjectClass *>(filter_primitive_parent_class))->release(object);
}

/**
 * Sets a specific value in the SPFilterPrimitive.
 */
static void
sp_filter_primitive_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFilterPrimitive *filter_primitive = SP_FILTER_PRIMITIVE(object);
    (void)filter_primitive;
    int image_nr;
    switch (key) {
        case SP_ATTR_IN:
            if (value) {
                image_nr = sp_filter_primitive_read_in(filter_primitive, value);
            } else {
                image_nr = Inkscape::Filters::NR_FILTER_SLOT_NOT_SET;
            }
            if (image_nr != filter_primitive->image_in) {
                filter_primitive->image_in = image_nr;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_RESULT:
            if (value) {
                image_nr = sp_filter_primitive_read_result(filter_primitive, value);
            } else {
                image_nr = Inkscape::Filters::NR_FILTER_SLOT_NOT_SET;
            }
            if (image_nr != filter_primitive->image_out) {
                filter_primitive->image_out = image_nr;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;

        /* Filter primitive sub-region */
        case SP_ATTR_X:
            filter_primitive->x.readOrUnset(value);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_Y:
            filter_primitive->y.readOrUnset(value);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_WIDTH:
            filter_primitive->width.readOrUnset(value);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_HEIGHT:
            filter_primitive->height.readOrUnset(value);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
    }

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

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        object->readAttr( "in" );
        object->readAttr( "result" );
        object->readAttr( "x" );
        object->readAttr( "y" );
        object->readAttr( "width" );
        object->readAttr( "height" );
    }

    if (((SPObjectClass *) filter_primitive_parent_class)->update) {
        ((SPObjectClass *) filter_primitive_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_filter_primitive_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    SPFilterPrimitive *prim = SP_FILTER_PRIMITIVE(object);
    SPFilter *parent = SP_FILTER(object->parent);

    if (!repr) {
        repr = object->getRepr()->duplicate(doc);
    }

    gchar const *in_name = sp_filter_name_for_image(parent, prim->image_in);
    repr->setAttribute("in", in_name);

    gchar const *out_name = sp_filter_name_for_image(parent, prim->image_out);
    repr->setAttribute("result", out_name);

    /* Do we need to add x,y,width,height? */
    if (((SPObjectClass *) filter_primitive_parent_class)->write) {
        ((SPObjectClass *) filter_primitive_parent_class)->write(object, doc, repr, flags);
    }

    return repr;
}

int sp_filter_primitive_read_in(SPFilterPrimitive *prim, gchar const *name)
{
    if (!name) return Inkscape::Filters::NR_FILTER_SLOT_NOT_SET;
    // TODO: are these case sensitive or not? (assumed yes)
    switch (name[0]) {
        case 'S':
            if (strcmp(name, "SourceGraphic") == 0)
                return Inkscape::Filters::NR_FILTER_SOURCEGRAPHIC;
            if (strcmp(name, "SourceAlpha") == 0)
                return Inkscape::Filters::NR_FILTER_SOURCEALPHA;
            if (strcmp(name, "StrokePaint") == 0)
                return Inkscape::Filters::NR_FILTER_STROKEPAINT;
            break;
        case 'B':
            if (strcmp(name, "BackgroundImage") == 0)
                return Inkscape::Filters::NR_FILTER_BACKGROUNDIMAGE;
            if (strcmp(name, "BackgroundAlpha") == 0)
                return Inkscape::Filters::NR_FILTER_BACKGROUNDALPHA;
            break;
        case 'F':
            if (strcmp(name, "FillPaint") == 0)
                return Inkscape::Filters::NR_FILTER_FILLPAINT;
            break;
    }

    SPFilter *parent = SP_FILTER(prim->parent);
    int ret = sp_filter_get_image_name(parent, name);
    if (ret >= 0) return ret;

    return Inkscape::Filters::NR_FILTER_SLOT_NOT_SET;
}

int sp_filter_primitive_read_result(SPFilterPrimitive *prim, gchar const *name)
{
    SPFilter *parent = SP_FILTER(prim->parent);
    int ret = sp_filter_get_image_name(parent, name);
    if (ret >= 0) return ret;

    ret = sp_filter_set_image_name(parent, name);
    if (ret >= 0) return ret;

    return Inkscape::Filters::NR_FILTER_SLOT_NOT_SET;
}

/**
 * Gives name for output of previous filter. Makes things clearer when prim
 * is a filter with two or more inputs. Returns the slot number of result
 * of previous primitive, or NR_FILTER_SOURCEGRAPHIC if this is the first
 * primitive.
 */
int sp_filter_primitive_name_previous_out(SPFilterPrimitive *prim) {
    SPFilter *parent = SP_FILTER(prim->parent);
    SPObject *i = parent->children;
    while (i && i->next != prim) i = i->next;
    if (i) {
        SPFilterPrimitive *i_prim = SP_FILTER_PRIMITIVE(i);
        if (i_prim->image_out < 0) {
            Glib::ustring name = sp_filter_get_new_result_name(parent);
            int slot = sp_filter_set_image_name(parent, name.c_str());
            i_prim->image_out = slot;
            //XML Tree is being directly used while it shouldn't be.
            i_prim->getRepr()->setAttribute("result", name.c_str());
            return slot;
        } else {
            return i_prim->image_out;
        }
    }
    return Inkscape::Filters::NR_FILTER_SOURCEGRAPHIC;
}

/* Common initialization for filter primitives */
void sp_filter_primitive_renderer_common(SPFilterPrimitive *sp_prim, Inkscape::Filters::FilterPrimitive *nr_prim)
{
    g_assert(sp_prim != NULL);
    g_assert(nr_prim != NULL);

    
    nr_prim->set_input(sp_prim->image_in);
    nr_prim->set_output(sp_prim->image_out);

    /* TODO: place here code to handle input images, filter area etc. */
    nr_prim->set_subregion( sp_prim->x, sp_prim->y, sp_prim->width, sp_prim->height );
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
