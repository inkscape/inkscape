#ifndef SP_FECONVOLVEMATRIX_H_SEEN
#define SP_FECONVOLVEMATRIX_H_SEEN

/** \file
 * SVG <feConvolveMatrix> implementation, see ConvolveMatrix.cpp.
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

#include "sp-filter.h"
#include "convolvematrix-fns.h"
#include "number-opt-number.h"
#include "display/nr-filter-convolve-matrix.h"
#include <vector>

/* FeConvolveMatrix base class */
class SPFeConvolveMatrixClass;

struct SPFeConvolveMatrix : public SPFilterPrimitive {
    /* CONVOLVEMATRIX ATTRIBUTES */
    NumberOptNumber order;
    std::vector<gdouble> kernelMatrix;
    double divisor, bias;
    int targetX, targetY;
    Inkscape::Filters::FilterConvolveMatrixEdgeMode edgeMode;
    NumberOptNumber kernelUnitLength;
    bool preserveAlpha;
    //some helper variables:
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
