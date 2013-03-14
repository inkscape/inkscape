/** @file
 * @brief SVG composite filter effect
 *//*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef SP_FECOMPOSITE_H_SEEN
#define SP_FECOMPOSITE_H_SEEN

#include "sp-filter-primitive.h"

#define SP_TYPE_FECOMPOSITE (sp_feComposite_get_type())
#define SP_FECOMPOSITE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FECOMPOSITE, SPFeComposite))
#define SP_FECOMPOSITE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_FECOMPOSITE, SPFeCompositeClass))
#define SP_IS_FECOMPOSITE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FECOMPOSITE))
#define SP_IS_FECOMPOSITE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_FECOMPOSITE))

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
