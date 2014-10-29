#ifndef SEEN_NR_FILTER_CONVOLVE_MATRIX_H
#define SEEN_NR_FILTER_CONVOLVE_MATRIX_H

/*
 * feConvolveMatrix filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-primitive.h"
#include <vector>

namespace Inkscape {
namespace Filters {

class FilterSlot;

enum FilterConvolveMatrixEdgeMode {
    CONVOLVEMATRIX_EDGEMODE_DUPLICATE,
    CONVOLVEMATRIX_EDGEMODE_WRAP,
    CONVOLVEMATRIX_EDGEMODE_NONE,
    CONVOLVEMATRIX_EDGEMODE_ENDTYPE
};

class FilterConvolveMatrix : public FilterPrimitive {
public:
    FilterConvolveMatrix();
    static FilterPrimitive *create();
    virtual ~FilterConvolveMatrix();

    virtual void render_cairo(FilterSlot &slot);
    virtual void area_enlarge(Geom::IntRect &area, Geom::Affine const &trans);
    virtual double complexity(Geom::Affine const &ctm);

    void set_targetY(int coord);
    void set_targetX(int coord);
    void set_orderY(int coord);
    void set_orderX(int coord);
    void set_kernelMatrix(std::vector<gdouble>& km);
    void set_bias(double b);
    void set_divisor(double d);
    void set_edgeMode(FilterConvolveMatrixEdgeMode mode);    
    void set_preserveAlpha(bool pa);

private:
    std::vector<double> kernelMatrix;
    int targetX, targetY;
    int orderX, orderY;
    double divisor, bias;
    int dx, dy, kernelUnitLength;
    FilterConvolveMatrixEdgeMode edgeMode;
    bool preserveAlpha;
};

} /* namespace Filters */
} /* namespace Inkscape */

#endif /* __NR_FILTER_CONVOLVE_MATRIX_H__ */
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
