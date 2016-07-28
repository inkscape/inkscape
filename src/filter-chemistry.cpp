/*
 * Various utility methods for filters
 *
 * Authors:
 *   Hugo Rodrigues
 *   bulia byak
 *   Niko Kiirala
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2006-2008 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <cstring>
#include <glibmm.h>

#include "style.h"
#include "document-private.h"
#include "desktop-style.h"

#include "filter-chemistry.h"
#include "filter-enums.h"

#include "filters/blend.h"
#include "filters/gaussian-blur.h"
#include "sp-filter.h"
#include "sp-filter-reference.h"
#include "svg/css-ostringstream.h"

#include "xml/repr.h"

/**
 * Count how many times the filter is used by the styles of o and its
 * descendants
 */
static guint count_filter_hrefs(SPObject *o, SPFilter *filter)
{
    if (!o)
        return 1;

    guint i = 0;

    SPStyle *style = o->style;
    if (style
        && style->filter.set
        && style->getFilter() == filter)
    {
        i ++;
    }

    for ( SPObject *child = o->firstChild(); child; child = child->getNext() ) {
        i += count_filter_hrefs(child, filter);
    }

    return i;
}

/**
 * Sets a suitable filter effects area according to given blur radius,
 * expansion and object size.
 */
static void set_filter_area(Inkscape::XML::Node *repr, gdouble radius,
                            double expansion, double expansionX,
                            double expansionY, double width, double height)
{
    // TODO: make this more generic, now assumed, that only the blur
    // being added can affect the required filter area

    double rx = radius * (expansionY != 0 ? (expansion / expansionY) : 1);
    double ry = radius * (expansionX != 0 ? (expansion / expansionX) : 1);

    if (width != 0 && height != 0) {
        // If not within the default 10% margin (see
        // http://www.w3.org/TR/SVG11/filters.html#FilterEffectsRegion), specify margins
        // The 2.4 is an empirical coefficient: at that distance the cutoff is practically invisible 
        // (the opacity at 2.4*radius is about 3e-3)
        double xmargin = 2.4 * (rx) / width;
        double ymargin = 2.4 * (ry) / height;

        // TODO: set it in UserSpaceOnUse instead?
        sp_repr_set_svg_double(repr, "x", -xmargin);
        sp_repr_set_svg_double(repr, "width", 1 + 2 * xmargin);
        sp_repr_set_svg_double(repr, "y", -ymargin);
        sp_repr_set_svg_double(repr, "height", 1 + 2 * ymargin);
    }
}

SPFilter *new_filter(SPDocument *document)
{
    g_return_val_if_fail(document != NULL, NULL);

    SPDefs *defs = document->getDefs();

    Inkscape::XML::Document *xml_doc = document->getReprDoc();

    // create a new filter
    Inkscape::XML::Node *repr;
    repr = xml_doc->createElement("svg:filter");

    // Inkscape now supports both sRGB and linear color-interpolation-filters.
    // But, for the moment, keep sRGB as default value for new filters
    // (historically set to sRGB and doesn't require conversion between
    // filter cairo surfaces and other types of cairo surfaces).
    SPCSSAttr *css = sp_repr_css_attr_new();
    sp_repr_css_set_property(css, "color-interpolation-filters", "sRGB");
    sp_repr_css_change(repr, css, "style");
    sp_repr_css_attr_unref(css);

    // Append the new filter node to defs
    defs->appendChild(repr);
    Inkscape::GC::release(repr);

    // get corresponding object
    SPFilter *f = SP_FILTER( document->getObjectByRepr(repr) );
    
    
    g_assert(f != NULL);
    g_assert(SP_IS_FILTER(f));

    return f;
}

SPFilterPrimitive *
filter_add_primitive(SPFilter *filter, const Inkscape::Filters::FilterPrimitiveType type)
{
    Inkscape::XML::Document *xml_doc = filter->document->getReprDoc();

    //create filter primitive node
    Inkscape::XML::Node *repr;
    repr = xml_doc->createElement(FPConverter.get_key(type).c_str());

    // set default values
    switch(type) {
        case Inkscape::Filters::NR_FILTER_BLEND:
            repr->setAttribute("mode", "normal");
            break;
        case Inkscape::Filters::NR_FILTER_COLORMATRIX:
            break;
        case Inkscape::Filters::NR_FILTER_COMPONENTTRANSFER:
            break;
        case Inkscape::Filters::NR_FILTER_COMPOSITE:
            break;
        case Inkscape::Filters::NR_FILTER_CONVOLVEMATRIX:
            repr->setAttribute("order", "3 3");
            repr->setAttribute("kernelMatrix", "0 0 0 0 0 0 0 0 0");
            break;
        case Inkscape::Filters::NR_FILTER_DIFFUSELIGHTING:
            break;
        case Inkscape::Filters::NR_FILTER_DISPLACEMENTMAP:
            break;
        case Inkscape::Filters::NR_FILTER_FLOOD:
            break;
        case Inkscape::Filters::NR_FILTER_GAUSSIANBLUR:
            repr->setAttribute("stdDeviation", "1");
            break;
        case Inkscape::Filters::NR_FILTER_IMAGE:
            break;
        case Inkscape::Filters::NR_FILTER_MERGE:
            break;
        case Inkscape::Filters::NR_FILTER_MORPHOLOGY:
            break;
        case Inkscape::Filters::NR_FILTER_OFFSET:
            repr->setAttribute("dx", "0");
            repr->setAttribute("dy", "0");
            break;
        case Inkscape::Filters::NR_FILTER_SPECULARLIGHTING:
            break;
        case Inkscape::Filters::NR_FILTER_TILE:
            break;
        case Inkscape::Filters::NR_FILTER_TURBULENCE:
            break;
        default:
            break;
    }

    //set primitive as child of filter node
    // XML tree being used directly while/where it shouldn't be...
    filter->appendChild(repr);
    Inkscape::GC::release(repr);
    
    // get corresponding object
    SPFilterPrimitive *prim = SP_FILTER_PRIMITIVE( filter->document->getObjectByRepr(repr) );
 
    g_assert(prim != NULL);
    g_assert(SP_IS_FILTER_PRIMITIVE(prim));

    return prim;
}

/**
 * Creates a filter with blur primitive of specified radius for an item with the given matrix expansion, width and height
 */
SPFilter *
new_filter_gaussian_blur (SPDocument *document, gdouble radius, double expansion, double expansionX, double expansionY, double width, double height)
{
    g_return_val_if_fail(document != NULL, NULL);

    SPDefs *defs = document->getDefs();

    Inkscape::XML::Document *xml_doc = document->getReprDoc();

    // create a new filter
    Inkscape::XML::Node *repr;
    repr = xml_doc->createElement("svg:filter");
    //repr->setAttribute("inkscape:collect", "always");

    set_filter_area(repr, radius, expansion, expansionX, expansionY,
                    width, height);

    /* Inkscape now supports both sRGB and linear color-interpolation-filters.  
     * But, for the moment, keep sRGB as default value for new filters.
     * historically set to sRGB and doesn't require conversion between
     * filter cairo surfaces and other types of cairo surfaces. lp:1127103 */
    SPCSSAttr *css = sp_repr_css_attr_new();                                    
    sp_repr_css_set_property(css, "color-interpolation-filters", "sRGB");       
    sp_repr_css_change(repr, css, "style");                                     
    sp_repr_css_attr_unref(css);

    //create feGaussianBlur node
    Inkscape::XML::Node *b_repr;
    b_repr = xml_doc->createElement("svg:feGaussianBlur");
    //b_repr->setAttribute("inkscape:collect", "always");
    
    double stdDeviation = radius;
    if (expansion != 0)
        stdDeviation /= expansion;

    //set stdDeviation attribute
    sp_repr_set_svg_double(b_repr, "stdDeviation", stdDeviation);
    
    //set feGaussianBlur as child of filter node
    repr->appendChild(b_repr);
    Inkscape::GC::release(b_repr);
    
    // Append the new filter node to defs
    defs->appendChild(repr);
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
 * Creates a simple filter with a blend primitive and a blur primitive of specified radius for
 * an item with the given matrix expansion, width and height
 */
static SPFilter *
new_filter_blend_gaussian_blur (SPDocument *document, const char *blendmode, gdouble radius, double expansion,
                                double expansionX, double expansionY, double width, double height)
{
    g_return_val_if_fail(document != NULL, NULL);

    SPDefs *defs = document->getDefs();

    Inkscape::XML::Document *xml_doc = document->getReprDoc();

    // create a new filter
    Inkscape::XML::Node *repr;
    repr = xml_doc->createElement("svg:filter");
    repr->setAttribute("inkscape:collect", "always");

    /* Inkscape now supports both sRGB and linear color-interpolation-filters.  
     * But, for the moment, keep sRGB as default value for new filters. 
     * historically set to sRGB and doesn't require conversion between
     * filter cairo surfaces and other types of cairo surfaces. lp:1127103 */
    SPCSSAttr *css = sp_repr_css_attr_new();                                    
    sp_repr_css_set_property(css, "color-interpolation-filters", "sRGB");       
    sp_repr_css_change(repr, css, "style");                                     
    sp_repr_css_attr_unref(css);

    // Append the new filter node to defs
    defs->appendChild(repr);
    Inkscape::GC::release(repr);
 
    // get corresponding object
    SPFilter *f = SP_FILTER( document->getObjectByRepr(repr) );

    // Gaussian blur primitive
    if(radius != 0) {
        set_filter_area(repr, radius, expansion, expansionX, expansionY, width, height);

        //create feGaussianBlur node
        Inkscape::XML::Node *b_repr;
        b_repr = xml_doc->createElement("svg:feGaussianBlur");
        b_repr->setAttribute("inkscape:collect", "always");
        
        double stdDeviation = radius;
        if (expansion != 0)
            stdDeviation /= expansion;
        
        //set stdDeviation attribute
        sp_repr_set_svg_double(b_repr, "stdDeviation", stdDeviation);
     
        //set feGaussianBlur as child of filter node
        repr->appendChild(b_repr);
        Inkscape::GC::release(b_repr);

        SPGaussianBlur *b = SP_GAUSSIANBLUR( document->getObjectByRepr(b_repr) );
        g_assert(b != NULL);
        g_assert(SP_IS_GAUSSIANBLUR(b));
    }
    // Blend primitive
    if(strcmp(blendmode, "normal")) {
        Inkscape::XML::Node *b_repr;
        b_repr = xml_doc->createElement("svg:feBlend");
        b_repr->setAttribute("inkscape:collect", "always");
        b_repr->setAttribute("mode", blendmode);
        b_repr->setAttribute("in2", "BackgroundImage");

        // set feBlend as child of filter node
        repr->appendChild(b_repr);
        Inkscape::GC::release(b_repr);

        // Enable background image buffer for document
        Inkscape::XML::Node *root = b_repr->root();
        if (!root->attribute("enable-background")) {
            root->setAttribute("enable-background", "new");
        }

        SPFeBlend *b = SP_FEBLEND(document->getObjectByRepr(b_repr));
        g_assert(b != NULL);
        g_assert(SP_IS_FEBLEND(b));
    }
    
    g_assert(f != NULL);
    g_assert(SP_IS_FILTER(f));
 
    return f;
}

/**
 * Creates a simple filter for the given item with blend and blur primitives, using the
 * specified mode and radius, respectively
 */
SPFilter *
new_filter_simple_from_item (SPDocument *document, SPItem *item, const char *mode, gdouble radius)
{
    Geom::OptRect const r = item->desktopGeometricBounds();

    double width;
    double height;
    if (r) {
        width = r->dimensions()[Geom::X];
        height= r->dimensions()[Geom::Y];
    } else {
        width = height = 0;
    }

    Geom::Affine i2dt (item->i2dt_affine () );

    return (new_filter_blend_gaussian_blur (document, mode, radius, i2dt.descrim(), i2dt.expansionX(), i2dt.expansionY(), width, height));
}

/**
 * Modifies the gaussian blur applied to the item.
 * If no filters are applied to given item, creates a new blur filter.
 * If a filter is applied and it contains a blur, modify that blur.
 * If the filter doesn't contain blur, a blur is added to the filter.
 * Should there be more references to modified filter, that filter is
 * duplicated, so that other elements referring that filter are not modified.
 */
/* TODO: this should be made more generic, not just for blurs */
SPFilter *modify_filter_gaussian_blur_from_item(SPDocument *document, SPItem *item,
                                                gdouble radius)
{
    if (!item->style || !item->style->filter.set) {
        return new_filter_simple_from_item(document, item, "normal", radius);
    }

    SPFilter *filter = SP_FILTER(item->style->getFilter());
    if (!filter) {
        // We reach here when filter.set is true, but the href is not found in the document
        return new_filter_simple_from_item(document, item, "normal", radius);
    }

    Inkscape::XML::Document *xml_doc = document->getReprDoc();

    // If there are more users for this filter, duplicate it
    if (filter->hrefcount > count_filter_hrefs(item, filter)) {
        Inkscape::XML::Node *repr = item->style->getFilter()->getRepr()->duplicate(xml_doc);
        SPDefs *defs = document->getDefs();
        defs->appendChild(repr);

        filter = SP_FILTER( document->getObjectByRepr(repr) );
        Inkscape::GC::release(repr);
    }

    // Determine the required standard deviation value
    Geom::Affine i2d (item->i2dt_affine ());
    double expansion = i2d.descrim();
    double stdDeviation = radius;
    if (expansion != 0)
        stdDeviation /= expansion;

    // Get the object size
    Geom::OptRect const r = item->desktopGeometricBounds();
    double width;
    double height;
    if (r) {
        width = r->dimensions()[Geom::X];
        height= r->dimensions()[Geom::Y];
    } else {
        width = height = 0;
    }

    // Set the filter effects area
    Inkscape::XML::Node *repr = item->style->getFilter()->getRepr();
    set_filter_area(repr, radius, expansion, i2d.expansionX(),
                    i2d.expansionY(), width, height);

    // Search for gaussian blur primitives. If found, set the stdDeviation
    // of the first one and return.
    Inkscape::XML::Node *primitive = repr->firstChild();
    while (primitive) {
        if (strcmp("svg:feGaussianBlur", primitive->name()) == 0) {
            sp_repr_set_svg_double(primitive, "stdDeviation",
                                   stdDeviation);
            return filter;
        }
        primitive = primitive->next();
    }

    // If there were no gaussian blur primitives, create a new one

    //create feGaussianBlur node
    Inkscape::XML::Node *b_repr;
    b_repr = xml_doc->createElement("svg:feGaussianBlur");
    //b_repr->setAttribute("inkscape:collect", "always");
    
    //set stdDeviation attribute
    sp_repr_set_svg_double(b_repr, "stdDeviation", stdDeviation);
    
    //set feGaussianBlur as child of filter node
    filter->getRepr()->appendChild(b_repr);
    Inkscape::GC::release(b_repr);

    return filter;
}

void remove_filter (SPObject *item, bool recursive)
{
    SPCSSAttr *css = sp_repr_css_attr_new();
    sp_repr_css_unset_property(css, "filter");
    if (recursive) {
        sp_repr_css_change_recursive(item->getRepr(), css, "style");
    } else {
        sp_repr_css_change(item->getRepr(), css, "style");
    }
    sp_repr_css_attr_unref(css);
}

/**
 * Removes the first feGaussianBlur from the filter attached to given item.
 * Should this leave us with an empty filter, remove that filter.
 */
/* TODO: the removed filter primitive may had had a named result image, so
 * after removing, the filter may be in erroneous state, this situation should
 * be handled gracefully */
void remove_filter_gaussian_blur (SPObject *item)
{
    if (item->style && item->style->filter.set && item->style->getFilter()) {
        // Search for the first blur primitive and remove it. (if found)
        Inkscape::XML::Node *repr = item->style->getFilter()->getRepr();
        Inkscape::XML::Node *primitive = repr->firstChild();
        while (primitive) {
            if (strcmp("svg:feGaussianBlur", primitive->name()) == 0) {
                sp_repr_unparent(primitive);
                break;
            }
            primitive = primitive->next();
        }

        // If there are no more primitives left in this filter, discard it.
        if (repr->childCount() == 0) {
            remove_filter(item, false);
        }
    }
}

bool filter_is_single_gaussian_blur(SPFilter *filter)
{
    return (filter->firstChild() && 
            (filter->firstChild() == filter->lastChild()) &&
            SP_IS_GAUSSIANBLUR(filter->firstChild()));
}

double get_single_gaussian_blur_radius(SPFilter *filter)
{
    if (filter->firstChild() && 
        (filter->firstChild() == filter->lastChild()) &&
        SP_IS_GAUSSIANBLUR(filter->firstChild())) {

        SPGaussianBlur *gb = SP_GAUSSIANBLUR(filter->firstChild());
        double x = gb->stdDeviation.getNumber();
        double y = gb->stdDeviation.getOptNumber();
        if (x > 0 && y > 0) {
            return MAX(x, y);
        }
        return x;
    }
    return 0.0;
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
