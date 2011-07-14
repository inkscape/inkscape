#define __NR_VALUES_C__

#include "libnr/nr-values.h"
#include "libnr/nr-rect.h"
#include "libnr/nr-rect-l.h"

/*
The following predefined objects are for reference
and comparison.
*/
NRRect   NR_RECT_EMPTY(NR_HUGE, NR_HUGE, -NR_HUGE, -NR_HUGE);
NRRectL  NR_RECT_L_EMPTY(NR_HUGE_L, NR_HUGE_L, -NR_HUGE_L, -NR_HUGE_L);

/** component_vectors[i] is like $e_i$ in common mathematical usage;
    or equivalently $I_i$ (where $I$ is the identity matrix). */
Geom::Point const component_vectors[] = {Geom::Point(1., 0.),
				       Geom::Point(0., 1.)};

