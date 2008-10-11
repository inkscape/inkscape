#define __SP_FILTER_CHEMISTRY_C__

/*
 * Various utility methods for filters
 *
 * Authors:
 *   Hugo Rodrigues
 *   bulia byak
 *   Niko Kiirala
 *
 * Copyright (C) 2006-2008 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "style.h"
#include "document-private.h"
#include "desktop-style.h"

#include "filter-chemistry.h"
#include "filter-enums.h"

#include "sp-feblend.h"
#include "sp-filter.h"
#include "sp-filter-reference.h"
#include "sp-gaussian-blur.h"
#include "svg/css-ostringstream.h"
#include "libnr/nr-matrix-fns.h"

#include "xml/repr.h"

/**
 * Count how many times the filter is used by the styles of o and its
 * descendants
 */
static guint
count_filter_hrefs(SPObject *o, SPFilter *filter)
{
    if (!o)
        return 1;

    guint i = 0;

    SPStyle *style = SP_OBJECT_STYLE(o);
    if (style
        && style->filter.set
        && style->getFilter() == filter)
    {
        i ++;
    }

    for (SPObject *child = sp_object_first_child(o);
         child != NULL; child = SP_OBJECT_NEXT(child)) {
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

    if (width != 0 && height != 0 && (2.4 * rx > width * 0.1 || 2.4 * ry > height * 0.1)) {
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

    SPDefs *defs = (SPDefs *) SP_DOCUMENT_DEFS(document);

    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(document);

    // create a new filter
    Inkscape::XML::Node *repr;
    repr = xml_doc->createElement("svg:filter");

    // Append the new filter node to defs
    SP_OBJECT_REPR(defs)->appendChild(repr);
    Inkscape::GC::release(repr);

    // get corresponding object
    SPFilter *f = SP_FILTER( document->getObjectByRepr(repr) );
    
    
    g_assert(f != NULL);
    g_assert(SP_IS_FILTER(f));

    return f;
}

SPFilterPrimitive *
filter_add_primitive(SPFilter *filter, const NR::FilterPrimitiveType type)
{
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(filter->document);

    //create filter primitive node
    Inkscape::XML::Node *repr;
    repr = xml_doc->createElement(FPConverter.get_key(type).c_str());

    // set default values
    switch(type) {
        case NR::NR_FILTER_BLEND:
            repr->setAttribute("blend", "normal");
            break;
        case NR::NR_FILTER_COLORMATRIX:
            break;
        case NR::NR_FILTER_COMPONENTTRANSFER:
            break;
        case NR::NR_FILTER_COMPOSITE:
            break;
        case NR::NR_FILTER_CONVOLVEMATRIX:
            repr->setAttribute("order", "3 3");
            repr->setAttribute("kernelMatrix", "0 0 0 0 0 0 0 0 0");
            break;
        case NR::NR_FILTER_DIFFUSELIGHTING:
            break;
        case NR::NR_FILTER_DISPLACEMENTMAP:
            break;
        case NR::NR_FILTER_FLOOD:
            break;
        case NR::NR_FILTER_GAUSSIANBLUR:
            repr->setAttribute("stdDeviation", "1");
            break;
        case NR::NR_FILTER_IMAGE:
            break;
        case NR::NR_FILTER_MERGE:
            break;
        case NR::NR_FILTER_MORPHOLOGY:
            break;
        case NR::NR_FILTER_OFFSET:
            repr->setAttribute("dx", "0");
            repr->setAttribute("dy", "0");
            break;
        case NR::NR_FILTER_SPECULARLIGHTING:
            break;
        case NR::NR_FILTER_TILE:
            break;
        case NR::NR_FILTER_TURBULENCE:
            break;
        default:
            break;
    }

    //set primitive as child of filter node
    filter->repr->appendChild(repr);
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

    SPDefs *defs = (SPDefs *) SP_DOCUMENT_DEFS(document);

    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(document);

    // create a new filter
    Inkscape::XML::Node *repr;
    repr = xml_doc->createElement("svg:filter");
    //repr->setAttribute("inkscape:collect", "always");

    set_filter_area(repr, radius, expansion, expansionX, expansionY,
                    width, height);

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
 * Creates a simple filter with a blend primitive and a blur primitive of specified radius for
 * an item with the given matrix expansion, width and height
 */
SPFilter *
new_filter_blend_gaussian_blur (SPDocument *document, const char *blendmode, gdouble radius, double expansion,
                                double expansionX, double expansionY, double width, double height)
{
    g_return_val_if_fail(document != NULL, NULL);

    SPDefs *defs = (SPDefs *) SP_DOCUMENT_DEFS(document);

    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(document);

    // create a new filter
    Inkscape::XML::Node *repr;
    repr = xml_doc->createElement("svg:filter");
    repr->setAttribute("inkscape:collect", "always");

    // Append the new filter node to defs
    SP_OBJECT_REPR(defs)->appendChild(repr);
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
    boost::optional<Geom::Rect> const r = sp_item_bbox_desktop(item, SPItem::GEOMETRIC_BBOX);

    double width;
    double height;
    if (r) {
        width = r->dimensions()[Geom::X];
        height= r->dimensions()[Geom::Y];
    } else {
        width = height = 0;
    }

    Geom::Matrix i2d (sp_item_i2d_affine (item) );

    return (new_filter_blend_gaussian_blur (document, mode, radius, i2d.descrim(), i2d.expansionX(), i2d.expansionY(), width, height));
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
SPFilter *
modify_filter_gaussian_blur_from_item(SPDocument *document, SPItem *item,
                                      gdouble radius)
{
    if (!item->style || !item->style->filter.set) {
        return new_filter_simple_from_item(document, item, "normal", radius);
    }

    SPFilter *filter = SP_FILTER(item->style->getFilter());
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(document);

    // If there are more users for this filter, duplicate it
    if (SP_OBJECT_HREFCOUNT(filter) > count_filter_hrefs(item, filter)) {
        Inkscape::XML::Node *repr;
        repr = SP_OBJECT_REPR(item->style->getFilter())->duplicate(xml_doc);
        SPDefs *defs = (SPDefs *) SP_DOCUMENT_DEFS(document);
        SP_OBJECT_REPR(defs)->appendChild(repr);

        filter = SP_FILTER( document->getObjectByRepr(repr) );
        Inkscape::GC::release(repr);
    }

    // Determine the required standard deviation value
    NR::Matrix i2d (sp_item_i2d_affine (item));
    double expansion = NR::expansion(i2d);
    double stdDeviation = radius;
    if (expansion != 0)
        stdDeviation /= expansion;

    // Get the object size
    boost::optional<Geom::Rect> const r = sp_item_bbox_desktop(item, SPItem::GEOMETRIC_BBOX);
    double width;
    double height;
    if (r) {
        width = r->dimensions()[Geom::X];
        height= r->dimensions()[Geom::Y];
    } else {
        width = height = 0;
    }

    // Set the filter effects area
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(item->style->getFilter());
    set_filter_area(repr, radius, expansion, NR::expansionX(i2d),
                    NR::expansionY(i2d), width, height);

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
    SP_OBJECT_REPR(filter)->appendChild(b_repr);
    Inkscape::GC::release(b_repr);

    return filter;
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
        Inkscape::XML::Node *repr = SP_OBJECT_REPR(item->style->getFilter());
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
    return (SP_OBJECT(filter)->firstChild() && 
            SP_OBJECT(filter)->firstChild() == SP_OBJECT(filter)->lastChild() &&
            SP_IS_GAUSSIANBLUR(SP_OBJECT(filter)->firstChild()));
}

double get_single_gaussian_blur_radius(SPFilter *filter)
{
    if (SP_OBJECT(filter)->firstChild() && 
        SP_OBJECT(filter)->firstChild() == SP_OBJECT(filter)->lastChild() &&
        SP_IS_GAUSSIANBLUR(SP_OBJECT(filter)->firstChild())) {

        SPGaussianBlur *gb = SP_GAUSSIANBLUR(SP_OBJECT(filter)->firstChild());
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
