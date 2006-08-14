#ifndef SP_GAUSSIANBLUR_H_SEEN
#define SP_GAUSSIANBLUR_H_SEEN

/** \file
 * SVG <gaussianBlur> implementation, see sp-gaussianBlur.cpp.
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
#include "sp-gaussian-blur-fns.h"

/* GaussianBlur base class */
class SPGaussianBlurClass;

struct SPGaussianBlur : public SPFilterPrimitive {
    /** stdDeviation attribute */
    NumberOptNumber stdDeviation;
};

struct SPGaussianBlurClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_gaussianBlur_get_type();


#endif /* !SP_GAUSSIANBLUR_H_SEEN */

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
