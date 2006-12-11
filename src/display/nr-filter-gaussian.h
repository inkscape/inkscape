#ifndef __NR_FILTER_GAUSSIAN_H__
#define __NR_FILTER_GAUSSIAN_H__

/*
 * Gaussian blur renderer
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *   bulia byak
 *   Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 *
 * Copyright (C) 2006 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-primitive.h"
#include "display/nr-filter-slot.h"
#include "libnr/nr-pixblock.h"
#include "libnr/nr-matrix.h"

enum {
    BLUR_QUALITY_BEST = 2,
    BLUR_QUALITY_BETTER = 1,
    BLUR_QUALITY_NORMAL = 0,
    BLUR_QUALITY_WORSE = -1,
    BLUR_QUALITY_WORST = -2
};

namespace NR {

class FilterGaussian : public FilterPrimitive {
public:
    FilterGaussian();
    static FilterPrimitive *create();
    virtual ~FilterGaussian();

    virtual int render(FilterSlot &slot, Matrix const &trans);
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

    int _kernel_size(double expansionX, double expansionY);
    void _make_kernel(double *kernel, double deviation, double expansion);
    int _effect_area_scr(double deviation, double expansion);
    int _effect_subsample_step(int scr_len_x, int quality);
    int _effect_subsample_step_log2(int scr_len_x, int quality);

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
