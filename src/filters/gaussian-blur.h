/** @file
 * @brief SVG Gaussian blur filter effect
 *//*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SP_GAUSSIANBLUR_H_SEEN
#define SP_GAUSSIANBLUR_H_SEEN

#include "sp-filter-primitive.h"
#include "number-opt-number.h"

#define SP_TYPE_GAUSSIANBLUR (sp_gaussianBlur_get_type())
#define SP_GAUSSIANBLUR(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_GAUSSIANBLUR, SPGaussianBlur))
#define SP_GAUSSIANBLUR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_GAUSSIANBLUR, SPGaussianBlurClass))
#define SP_IS_GAUSSIANBLUR(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_GAUSSIANBLUR))
#define SP_IS_GAUSSIANBLUR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_GAUSSIANBLUR))

struct SPGaussianBlur : public SPFilterPrimitive {
    /** stdDeviation attribute */
    NumberOptNumber stdDeviation;
};

/* GaussianBlur base class */
struct SPGaussianBlurClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_gaussianBlur_get_type();
void  sp_gaussianBlur_setDeviation(SPGaussianBlur *blur, float num);
void  sp_gaussianBlur_setDeviation(SPGaussianBlur *blur, float num, float optnum);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
