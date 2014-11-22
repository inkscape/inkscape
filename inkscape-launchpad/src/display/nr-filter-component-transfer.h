#ifndef SEEN_NR_FILTER_COMPONENT_TRANSFER_H
#define SEEN_NR_FILTER_COMPONENT_TRANSFER_H

/*
 * feComponentTransfer filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <vector>
#include "display/nr-filter-primitive.h"

namespace Inkscape {
namespace Filters {

class FilterSlot;

enum FilterComponentTransferType {
    COMPONENTTRANSFER_TYPE_IDENTITY,
    COMPONENTTRANSFER_TYPE_TABLE,
    COMPONENTTRANSFER_TYPE_DISCRETE,
    COMPONENTTRANSFER_TYPE_LINEAR,
    COMPONENTTRANSFER_TYPE_GAMMA,
    COMPONENTTRANSFER_TYPE_ERROR
};

class FilterComponentTransfer : public FilterPrimitive {
public:
    FilterComponentTransfer();
    static FilterPrimitive *create();
    virtual ~FilterComponentTransfer();

    virtual void render_cairo(FilterSlot &slot);
    virtual bool can_handle_affine(Geom::Affine const &);
    virtual double complexity(Geom::Affine const &ctm);

    FilterComponentTransferType type[4];
    std::vector<double> tableValues[4];
    double slope[4];
    double intercept[4];
    double amplitude[4];
    double exponent[4];
    double offset[4];
};

} /* namespace Filters */
} /* namespace Inkscape */

#endif /* __NR_FILTER_COMPONENT_TRANSFER_H__ */
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
