#ifndef SEEN_ROUND_H
#define SEEN_ROUND_H

#include <cmath>

namespace Inkscape {

/** Returns x rounded to the nearest integer.  It is unspecified what happens
    if x is half way between two integers: we may in future use rint/round
    on platforms that have them.  If you depend on a particular rounding
    behaviour, then please change this documentation accordingly.
**/
inline double
round(double const x)
{
    return std::floor( x + .5 );
}

}

#endif /* !SEEN_ROUND_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
