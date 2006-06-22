#ifndef __NR_FILTER_GAUSSIAN_H__
#define __NR_FILTER_GAUSSIAN_H__

/*
 * Gaussian blur renderer
 *
 * Author:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006 Niko Kiirala
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-primitive.h"
#include "libnr/nr-pixblock.h"
#include "libnr/nr-matrix.h"

namespace NR {

class FilterGaussian : public FilterPrimitive {
public:
    FilterGaussian();
    static FilterPrimitive *create();
    virtual ~FilterGaussian();

    virtual int render(NRPixBlock **pb, Matrix const &trans);
    virtual int get_enlarge(Matrix const &m);

    /**
     * Set the standard deviation value for gaussian blur. Deviation along
     * both axis is set to the provided value.
     * Negative value, NaN and infinity are considered an error and no
     * changes to filter state are made. If not set, default value of zero
     * is used, which means the filter results in transparent black image.
     */
    void set_deviation(double deviation);
    /**
     * Set the standard deviation value for gaussian blur. First parameter
     * sets the deviation alogn x-axis, second along y-axis.
     * Negative value, NaN and infinity are considered an error and no
     * changes to filter state are made. If not set, default value of zero
     * is used, which means the filter results in transparent black image.
     */
    void set_deviation(double x, double y);

private:
    double _deviation_x;
    double _deviation_y;

    int _kernel_size(Matrix const &trans);
    void _make_kernel(double *kernel, double deviation, double expansion);
    int _effect_area_scr_x(Matrix const &trans);
    int _effect_area_scr_y(Matrix const &trans);
    int _effect_subsample_step(int scr_len_x);
    int _effect_subsample_step_log2(int scr_len_x);

    inline int _min(int const a, int const b)
    {
        return ((a < b) ? a : b);
    }
    inline int _max(int const a, int const b)
    {
        return ((a > b) ? a : b);
    }
};


} /* namespace NR */




#endif /* __NR_FILTER_GAUSSIAN_H__ */
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
