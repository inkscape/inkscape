#ifndef SP_FEMORPHOLOGY_H_SEEN
#define SP_FEMORPHOLOGY_H_SEEN

/** \file
 * SVG <feMorphology> implementation, see Morphology.cpp.
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-filter.h"
#include "morphology-fns.h"
#include "number-opt-number.h"
#include "display/nr-filter.h"
#include "display/nr-filter-morphology.h"


/* FeMorphology base class */
class SPFeMorphologyClass;

struct SPFeMorphology : public SPFilterPrimitive {
    /** MORPHOLOGY ATTRIBUTES HERE */
    NR::FilterMorphologyOperator Operator;
    NumberOptNumber radius;
};

struct SPFeMorphologyClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_feMorphology_get_type();


#endif /* !SP_FEMORPHOLOGY_H_SEEN */

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
