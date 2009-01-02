#ifndef __NR_FILTER_COLOR_MATRIX_H__
#define __NR_FILTER_COLOR_MATRIX_H__

/*
 * feColorMatrix filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <felipe.sanches@gmail.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-primitive.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"
#include<vector>

namespace Inkscape {
namespace Filters {

enum FilterColorMatrixType {
    COLORMATRIX_MATRIX,
    COLORMATRIX_SATURATE,
    COLORMATRIX_HUEROTATE,
    COLORMATRIX_LUMINANCETOALPHA,
    COLORMATRIX_ENDTYPE
};

class FilterColorMatrix : public FilterPrimitive {
public:
    FilterColorMatrix();
    static FilterPrimitive *create();
    virtual ~FilterColorMatrix();

    virtual int render(FilterSlot &slot, FilterUnits const &units);
    virtual void area_enlarge(NRRectL &area, Geom::Matrix const &trans);
    virtual void set_type(FilterColorMatrixType type);
    virtual void set_value(gdouble value);
    virtual void set_values(std::vector<gdouble> &values);
private:
    std::vector<gdouble> values;
    gdouble value;
    FilterColorMatrixType type;
};

} /* namespace Filters */
} /* namespace Inkscape */

#endif /* __NR_FILTER_COLOR_MATRIX_H__ */
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
