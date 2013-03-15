/** @file
 * @brief SVG matrix convolution filter effect
 */
/*
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef SP_FECONVOLVEMATRIX_H_SEEN
#define SP_FECONVOLVEMATRIX_H_SEEN

#include <vector>
#include "sp-filter-primitive.h"
#include "number-opt-number.h"
#include "display/nr-filter-convolve-matrix.h"

#define SP_TYPE_FECONVOLVEMATRIX (sp_feConvolveMatrix_get_type())
#define SP_FECONVOLVEMATRIX(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FECONVOLVEMATRIX, SPFeConvolveMatrix))
#define SP_FECONVOLVEMATRIX_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_FECONVOLVEMATRIX, SPFeConvolveMatrixClass))
#define SP_IS_FECONVOLVEMATRIX(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FECONVOLVEMATRIX))
#define SP_IS_FECONVOLVEMATRIX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_FECONVOLVEMATRIX))

struct SPFeConvolveMatrix : public SPFilterPrimitive {
    NumberOptNumber order;
    std::vector<gdouble> kernelMatrix;
    double divisor, bias;
    int targetX, targetY;
    Inkscape::Filters::FilterConvolveMatrixEdgeMode edgeMode;
    NumberOptNumber kernelUnitLength;
    bool preserveAlpha;

    bool targetXIsSet;
    bool targetYIsSet;
    bool divisorIsSet;
    bool kernelMatrixIsSet;
};

struct SPFeConvolveMatrixClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_feConvolveMatrix_get_type();

#endif /* !SP_FECONVOLVEMATRIX_H_SEEN */

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
