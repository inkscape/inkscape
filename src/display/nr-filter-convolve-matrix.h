#ifndef __NR_FILTER_CONVOLVE_MATRIX_H__
#define __NR_FILTER_CONVOLVE_MATRIX_H__

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
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"
#include "libnr/nr-rect-l.h"
#include <vector>

namespace Inkscape {
namespace Filters {

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

    virtual int render(FilterSlot &slot, FilterUnits const &units);
    virtual void area_enlarge(NRRectL &area, Geom::Matrix const &trans);
    virtual FilterTraits get_input_traits();

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
    std::vector<gdouble> kernelMatrix;
    int targetX, targetY;
    int orderX, orderY;
    gdouble divisor, bias;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
