#ifndef SP_FECOMPOSITE_H_SEEN
#define SP_FECOMPOSITE_H_SEEN

/** \file
 * SVG <feComposite> implementation, see Composite.cpp.
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
#include "composite-fns.h"

enum FeCompositeOperator {
    // Default value is 'over', but let's distinquish specifying the
    // default and implicitely using the default
    COMPOSITE_DEFAULT,
    COMPOSITE_OVER,
    COMPOSITE_IN,
    COMPOSITE_OUT,
    COMPOSITE_ATOP,
    COMPOSITE_XOR,
    COMPOSITE_ARITHMETIC,
    COMPOSITE_ENDOPERATOR
};

/* FeComposite base class */
class SPFeCompositeClass;

struct SPFeComposite : public SPFilterPrimitive {
    FeCompositeOperator composite_operator;
    double k1, k2, k3, k4;
    int in2;
};

struct SPFeCompositeClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_feComposite_get_type();


#endif /* !SP_FECOMPOSITE_H_SEEN */

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
