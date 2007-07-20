#ifndef __NR_LIGHT_TYPES_H__
#define __NR_LIGHT_TYPES_H__

namespace NR {

enum LightType{
    NO_LIGHT = 0,
    DISTANT_LIGHT,
    POINT_LIGHT,
    SPOT_LIGHT
};

} /* namespace NR */

#endif // __NR_LIGHT_TYPES_H__
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
