#ifndef SP_FILTER_H_SEEN
#define SP_FILTER_H_SEEN

/** \file
 * SVG <filter> implementation, see sp-filter.cpp.
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006,2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "number-opt-number.h"
#include "sp-object.h"
#include "sp-filter-units.h"
#include "sp-filter-fns.h"
#include "svg/svg-length.h"
#include "display/nr-filter.h"

/* Filter base class */

/* MACROS DEFINED IN FILE sp-filter-fns.h */

struct SPFilterReference;

class SPFilter;
class SPFilterClass;

struct SPFilter : public SPObject {

    /** filterUnits attribute */
    SPFilterUnits filterUnits;
    guint filterUnits_set : 1;
    /** primitiveUnits attribute */
    SPFilterUnits primitiveUnits;
    guint primitiveUnits_set : 1;
    /** X attribute */    
    SVGLength x;
    /** Y attribute */
    SVGLength y;
    /** WIDTH attribute */
    SVGLength width;
    /** HEIGHT attribute */
    SVGLength height;
    /** FILTERRES attribute */
    NumberOptNumber filterRes;
    /** HREF attribute */
    SPFilterReference *href;

    int _primitive_count;
    int _primitive_table_size;
    SPFilterPrimitive ** _primitives;
    NR::Filter *_renderer;
};

struct SPFilterClass {
    SPObjectClass parent_class;
};

/*
 * Initializes the given NR::Filter object as a renderer for this
 * SPFilter object.
 */
void sp_filter_build_renderer(SPFilter *sp_filter, NR::Filter *nr_filter);

/*
 * Returns the number of filter primitives in this SPFilter object.
 */
int sp_filter_primitive_count(SPFilter *filter);

#endif /* !SP_FILTER_H_SEEN */

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
