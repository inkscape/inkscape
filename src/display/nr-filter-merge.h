#ifndef __NR_FILTER_MERGE_H__
#define __NR_FILTER_MERGE_H__

/*
 * feMerge filter effect renderer
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <vector>

#include "filters/merge.h"
#include "display/nr-filter-primitive.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"
#include "libnr/nr-matrix.h"

namespace NR {

class FilterMerge : public FilterPrimitive {
public:
    FilterMerge();
    static FilterPrimitive *create();
    virtual ~FilterMerge();

    virtual int render(FilterSlot &slot, FilterUnits const &units);

    virtual void set_input(int input);
    virtual void set_input(int input, int slot);

private:
    std::vector<int> _input_image;
};

} /* namespace NR */

#endif /* __NR_FILTER_MERGE_H__ */
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
