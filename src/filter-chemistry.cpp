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
 * Creates a filter with blur primitive of specified stdDeviation
 */
SPFilter *
new_filter_gaussian_blur (SPDocument *document, gdouble stdDeviation)
{
    g_return_val_if_fail(document != NULL, NULL);

    SPDefs *defs = (SPDefs *) SP_DOCUMENT_DEFS(document);

    // create a new private filter
    Inkscape::XML::Node *repr;
    repr = sp_repr_new("svg:filter");
    repr->setAttribute("inkscape:collect", "always");

    //create feGaussianBlur node
    Inkscape::XML::Node *b_repr;
    b_repr = sp_repr_new("svg:feGaussianBlur");
    b_repr->setAttribute("inkscape:collect", "always");
    
    //set stdDeviation attribute
    Inkscape::CSSOStringStream os;
    os << stdDeviation;
    b_repr->setAttribute("stdDeviation", os.str().c_str());
    
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
