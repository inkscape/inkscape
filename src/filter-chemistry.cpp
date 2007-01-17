#define __SP_FILTER_CHEMISTRY_C__

/*
 * Various utility methods for filters
 *
 * Authors:
 *   Hugo Rodrigues
 *   bulia byak
 *
 * Copyright (C) 2006 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "style.h"
#include "document-private.h"
#include "desktop-style.h"

#include "sp-filter.h"
#include "sp-gaussian-blur.h"
#include "svg/css-ostringstream.h"

#include "xml/repr.h"

/**
 * Creates a filter with blur primitive of specified radius for an item with the given matrix expansion, width and height
 */
SPFilter *
new_filter_gaussian_blur (SPDocument *document, gdouble radius, double expansion, double expansionX, double expansionY, double width, double height)
{
    g_return_val_if_fail(document != NULL, NULL);

    SPDefs *defs = (SPDefs *) SP_DOCUMENT_DEFS(document);

    // create a new filter
    Inkscape::XML::Node *repr;
    repr = sp_repr_new("svg:filter");
    repr->setAttribute("inkscape:collect", "always");

    double rx = radius * (expansionY != 0? (expansion / expansionY) : 1);
    double ry = radius * (expansionX != 0? (expansion / expansionX) : 1);

    if (width != 0 && height != 0 && (2 * rx > width * 0.1 || 2 * ry > height * 0.1)) {
        // If not within the default 10% margin (see
        // http://www.w3.org/TR/SVG11/filters.html#FilterEffectsRegion), specify margins
        double xmargin = 2 * (rx) / width;
        double ymargin = 2 * (ry) / height;

        // TODO: set it in UserSpaceOnUse instead?
        sp_repr_set_svg_double(repr, "x", -xmargin);
        sp_repr_set_svg_double(repr, "width", 1 + 2 * xmargin);
        sp_repr_set_svg_double(repr, "y", -ymargin);
        sp_repr_set_svg_double(repr, "height", 1 + 2 * ymargin);
    }

    //create feGaussianBlur node
    Inkscape::XML::Node *b_repr;
    b_repr = sp_repr_new("svg:feGaussianBlur");
    b_repr->setAttribute("inkscape:collect", "always");
    
    double stdDeviation = radius;
    if (expansion != 0)
        stdDeviation /= expansion;

    //set stdDeviation attribute
    sp_repr_set_svg_double(b_repr, "stdDeviation", stdDeviation);
    
    //set feGaussianBlur as child of filter node
    repr->appendChild(b_repr);
    Inkscape::GC::release(b_repr);
    
    // Append the new filter node to defs
    SP_OBJECT_REPR(defs)->appendChild(repr);
    Inkscape::GC::release(repr);

    // get corresponding object
    SPFilter *f = SP_FILTER( document->getObjectByRepr(repr) );
    SPGaussianBlur *b = SP_GAUSSIANBLUR( document->getObjectByRepr(b_repr) );
    
    g_assert(f != NULL);
    g_assert(SP_IS_FILTER(f));
    g_assert(b != NULL);
    g_assert(SP_IS_GAUSSIANBLUR(b));

    return f;
}

/**
 * Creates a filter with blur primitive of specified radius for the given item
 */
SPFilter *
new_filter_gaussian_blur_from_item (SPDocument *document, SPItem *item, gdouble radius)
{
    NR::Rect const r = sp_item_bbox_desktop(item);
    double width = r.extent(NR::X);
    double height = r.extent(NR::Y);

    NR::Matrix i2d = sp_item_i2d_affine (item);

    return (new_filter_gaussian_blur (document, radius, i2d.expansion(), i2d.expansionX(), i2d.expansionY(), width, height));
}

void remove_filter (SPObject *item, bool recursive)
{
	SPCSSAttr *css = sp_repr_css_attr_new ();
	sp_repr_css_unset_property (css, "filter");
	if (recursive)
		sp_repr_css_change_recursive(SP_OBJECT_REPR(item), css, "style");
	else
		sp_repr_css_change (SP_OBJECT_REPR(item), css, "style");
      sp_repr_css_attr_unref (css);
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
