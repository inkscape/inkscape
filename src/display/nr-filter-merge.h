#ifndef SEEN_NR_FILTER_MERGE_H
#define SEEN_NR_FILTER_MERGE_H

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
#include "display/nr-filter-primitive.h"

namespace Inkscape {
namespace Filters {

class FilterMerge : public FilterPrimitive {
public:
    FilterMerge();
    static FilterPrimitive *create();
    virtual ~FilterMerge();

    virtual void render_cairo(FilterSlot &);
    virtual bool can_handle_affine(Geom::Affine const &);
    virtual double complexity(Geom::Affine const &ctm);
    virtual bool uses_background();

    virtual void set_input(int input);
    virtual void set_input(int input, int slot);

private:
    std::vector<int> _input_image;
};

} /* namespace Filters */
} /* namespace Inkscape */

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
