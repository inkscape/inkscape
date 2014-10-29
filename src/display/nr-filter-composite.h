#ifndef SEEN_NR_FILTER_COMPOSITE_H
#define SEEN_NR_FILTER_COMPOSITE_H

/*
 * feComposite filter effect renderer
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "filters/composite.h"
#include "display/nr-filter-primitive.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"

namespace Inkscape {
namespace Filters {

class FilterComposite : public FilterPrimitive {
public:
    FilterComposite();
    static FilterPrimitive *create();
    virtual ~FilterComposite();

    virtual void render_cairo(FilterSlot &);
    virtual bool can_handle_affine(Geom::Affine const &);
    virtual double complexity(Geom::Affine const &ctm);

    virtual void set_input(int input);
    virtual void set_input(int input, int slot);

    void set_operator(FeCompositeOperator op);
    void set_arithmetic(double k1, double k2, double k3, double k4);

private:
    FeCompositeOperator op;
    double k1, k2, k3, k4;
    int _input2;
};

} /* namespace Filters */
} /* namespace Inkscape */

#endif /* __NR_FILTER_COMPOSITE_H__ */
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
