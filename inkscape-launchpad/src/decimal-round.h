#ifndef SEEN_DECIMAL_ROUND_H
#define SEEN_DECIMAL_ROUND_H

#include <cmath>

#include "round.h"


namespace Inkscape {

/** Returns x rounded to the nearest \a nplaces decimal places.

    Implemented in terms of Inkscape::round, i.e. we make no guarantees as to what happens if x is
    half way between two rounded numbers.  Add a note to the documentation if you care which
    candidate is chosen for such case (round towards -infinity, +infinity, 0, or "even").

    Note: nplaces is the number of decimal places without using scientific (e) notation, not the
    number of significant figures.  This function may not be suitable for values of x whose
    magnitude is so far from 1 that one would want to use scientific (e) notation.

    The current implementation happens to allow nplaces to be negative: e.g. nplaces=-2 means
    rounding to a multiple of 100.  May or may not be useful.
**/
inline double
decimal_round(double const x, int const nplaces)
{
    double const multiplier = std::pow(10.0, nplaces);
    return Inkscape::round( x * multiplier ) / multiplier;
}

}

#endif /* !SEEN_DECIMAL_ROUND_H */

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
