/** \file
 * SVG <gaussianBlur> implementation.
 *
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2006,2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "attributes.h"
#include "svg/svg.h"
#include "filters/gaussian-blur.h"
#include "xml/repr.h"

#include "display/nr-filter.h"
#include "display/nr-filter-primitive.h"
#include "display/nr-filter-gaussian.h"
#include "display/nr-filter-types.h"

SPGaussianBlur::SPGaussianBlur() : SPFilterPrimitive() {
}

SPGaussianBlur::~SPGaussianBlur() {
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPGaussianBlur variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
void SPGaussianBlur::build(SPDocument *document, Inkscape::XML::Node *repr) {
	SPFilterPrimitive::build(document, repr);

    this->readAttr( "stdDeviation" );
}

/**
 * Drops any allocated memory.
 */
void SPGaussianBlur::release() {
	SPFilterPrimitive::release();
}

/**
 * Sets a specific value in the SPGaussianBlur.
 */
void SPGaussianBlur::set(unsigned int key, gchar const *value) {
    switch(key) {
        case SP_ATTR_STDDEVIATION:
            this->stdDeviation.set(value);
            this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        default:
        	SPFilterPrimitive::set(key, value);
            break;
    }
}

/**
 * Receives update notifications.
 */
void SPGaussianBlur::update(SPCtx *ctx, guint flags) {
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        this->readAttr( "stdDeviation" );
    }

    SPFilterPrimitive::update(ctx, flags);
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
Inkscape::XML::Node* SPGaussianBlur::write(Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags) {
    /* TODO: Don't just clone, but create a new repr node and write all
     * relevant values into it */
    if (!repr) {
        repr = this->getRepr()->duplicate(doc);
    }

    SPFilterPrimitive::write(doc, repr, flags);

    return repr;
}

void  sp_gaussianBlur_setDeviation(SPGaussianBlur *blur, float num)
{
    blur->stdDeviation.setNumber(num);
}

void  sp_gaussianBlur_setDeviation(SPGaussianBlur *blur, float num, float optnum)
{
    blur->stdDeviation.setNumber(num);
    blur->stdDeviation.setOptNumber(optnum);
}

void SPGaussianBlur::build_renderer(Inkscape::Filters::Filter* filter) {
    int handle = filter->add_primitive(Inkscape::Filters::NR_FILTER_GAUSSIANBLUR);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(handle);
    Inkscape::Filters::FilterGaussian *nr_blur = dynamic_cast<Inkscape::Filters::FilterGaussian*>(nr_primitive);

    sp_filter_primitive_renderer_common(this, nr_primitive);

    gfloat num = this->stdDeviation.getNumber();

    if (num >= 0.0) {
        gfloat optnum = this->stdDeviation.getOptNumber();

        if(optnum >= 0.0) {
            nr_blur->set_deviation((double) num, (double) optnum);
        } else {
            nr_blur->set_deviation((double) num);
        }
    }
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
