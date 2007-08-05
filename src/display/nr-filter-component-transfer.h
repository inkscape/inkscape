#ifndef __NR_FILTER_COMPONENT_TRANSFER_H__
#define __NR_FILTER_COMPONENT_TRANSFER_H__

/*
 * feComponentTransfer filter primitive renderer
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

namespace NR {

class FilterComponentTransfer : public FilterPrimitive {
public:
    FilterComponentTransfer();
    static FilterPrimitive *create();
    virtual ~FilterComponentTransfer();

    virtual int render(FilterSlot &slot, Matrix const &trans);
    virtual void area_enlarge(NRRectL &area, Matrix const &trans);
};

} /* namespace NR */

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
