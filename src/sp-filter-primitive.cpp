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

#include "display/nr-filter-primitive.h"
#include "display/nr-filter-types.h"

#include "attributes.h"
#include "style.h"
#include "sp-filter-primitive.h"
#include "xml/repr.h"
#include "sp-filter.h"
#include "sp-item.h"


// CPPIFY: Make pure virtual.
//void SPFilterPrimitive::build_renderer(Inkscape::Filters::Filter* filter) {
// throw;
//}

SPFilterPrimitive::SPFilterPrimitive() : SPObject() {
    this->image_in = Inkscape::Filters::NR_FILTER_SLOT_NOT_SET;
    this->image_out = Inkscape::Filters::NR_FILTER_SLOT_NOT_SET;

    // We must keep track if a value is set or not, if not set then the region defaults to 0%, 0%,
    // 100%, 100% ("x", "y", "width", "height") of the -> filter <- region.  If set then
    // percentages are in terms of bounding box or viewbox, depending on value of "primitiveUnits"

    // NB: SVGLength.set takes prescaled percent values: 1 means 100%
    this->x.unset(SVGLength::PERCENT, 0, 0);
    this->y.unset(SVGLength::PERCENT, 0, 0);
    this->width.unset(SVGLength::PERCENT, 1, 0);
    this->height.unset(SVGLength::PERCENT, 1, 0);
}

SPFilterPrimitive::~SPFilterPrimitive() {
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFilterPrimitive variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
void SPFilterPrimitive::build(SPDocument *document, Inkscape::XML::Node *repr) {
    SPFilterPrimitive* object = this;

    object->readAttr( "style" ); // struct not derived from SPItem, we need to do this ourselves.
    object->readAttr( "in" );
    object->readAttr( "result" );
    object->readAttr( "x" );
    object->readAttr( "y" );
    object->readAttr( "width" );
    object->readAttr( "height" );

    SPObject::build(document, repr);
}

/**
 * Drops any allocated memory.
 */
void SPFilterPrimitive::release() {
    SPObject::release();
}

/**
 * Sets a specific value in the SPFilterPrimitive.
 */
void SPFilterPrimitive::set(unsigned int key, gchar const *value) {

    int image_nr;
    switch (key) {
        case SP_ATTR_IN:
            if (value) {
                image_nr = sp_filter_primitive_read_in(this, value);
            } else {
                image_nr = Inkscape::Filters::NR_FILTER_SLOT_NOT_SET;
            }
            if (image_nr != this->image_in) {
                this->image_in = image_nr;
                this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_RESULT:
            if (value) {
                image_nr = sp_filter_primitive_read_result(this, value);
            } else {
                image_nr = Inkscape::Filters::NR_FILTER_SLOT_NOT_SET;
            }
            if (image_nr != this->image_out) {
                this->image_out = image_nr;
                this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;

        /* Filter primitive sub-region */
        case SP_ATTR_X:
            this->x.readOrUnset(value);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_Y:
            this->y.readOrUnset(value);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_WIDTH:
            this->width.readOrUnset(value);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_HEIGHT:
            this->height.readOrUnset(value);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
    }

    /* See if any parents need this value. */
    SPObject::set(key, value);
}

/**
 * Receives update notifications.
 */
void SPFilterPrimitive::update(SPCtx *ctx, guint flags) {

    SPItemCtx *ictx = (SPItemCtx *) ctx;

    // Do here since we know viewport (Bounding box case handled during rendering)
    SPFilter *parent = SP_FILTER(this->parent);

    if( parent->primitiveUnits == SP_FILTER_UNITS_USERSPACEONUSE ) {
        if (this->x.unit == SVGLength::PERCENT) {
            this->x._set = true;
            this->x.computed = this->x.value * ictx->viewport.width();
        }

        if (this->y.unit == SVGLength::PERCENT) {
            this->y._set = true;
            this->y.computed = this->y.value * ictx->viewport.height();
        }

        if (this->width.unit == SVGLength::PERCENT) {
            this->width._set = true;
            this->width.computed = this->width.value * ictx->viewport.width();
        }

        if (this->height.unit == SVGLength::PERCENT) {
            this->height._set = true;
            this->height.computed = this->height.value * ictx->viewport.height();
        }
    }

    SPObject::update(ctx, flags);
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
Inkscape::XML::Node* SPFilterPrimitive::write(Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags) {
    SPFilterPrimitive* object = this;

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
    SPObject::write(doc, repr, flags);

    return repr;
}

int sp_filter_primitive_read_in(SPFilterPrimitive *prim, gchar const *name)
{
    if (!name || !prim){
        return Inkscape::Filters::NR_FILTER_SLOT_NOT_SET;
    }
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
    // We don't know current viewport or bounding box, this is wrong approach.
    nr_prim->set_subregion( sp_prim->x, sp_prim->y, sp_prim->width, sp_prim->height );

    // Give renderer access to filter properties
    nr_prim->setStyle( sp_prim->style );
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
