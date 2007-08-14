#ifndef __NR_FILTER_TURBULENCE_H__
#define __NR_FILTER_TURBULENCE_H__

/*
 * feTurbulence filter primitive renderer
 *
 * Authors:
 *   Felipe Sanches <felipe.sanches@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-primitive.h"
#include "display/nr-filter-slot.h"

namespace NR {

enum FilterTurbulenceType {
    TURBULENCE_FRACTALNOISE,
    TURBULENCE_TURBULENCE,
    TURBULENCE_ENDTYPE
};

class FilterTurbulence : public FilterPrimitive {
public:
    FilterTurbulence();
    static FilterPrimitive *create();
    virtual ~FilterTurbulence();

    virtual int render(FilterSlot &slot, Matrix const &trans);
    virtual void update_pixbuffer(FilterSlot &slot);
    
    virtual void set_baseFrequency(int axis, double freq);
    virtual void set_numOctaves(int num);
    virtual void set_seed(double s);
    virtual void set_stitchTiles(bool st);
    virtual void set_type(FilterTurbulenceType t);
    virtual void set_updated(bool u);
private:
    double XbaseFrequency, YbaseFrequency;
    int numOctaves;
    double seed;
    bool stitchTiles;
    FilterTurbulenceType type;
    bool updated;
    NRPixBlock *pix;
    unsigned char *pix_data;
};

} /* namespace NR */

#endif /* __NR_FILTER_TURBULENCE_H__ */
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
