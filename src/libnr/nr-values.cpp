#define __NR_VALUES_C__

#include <libnr/nr-rect-l.h>
#include <libnr/nr-matrix-fns.h>


/*
The following predefined objects are for reference
and comparison.
*/
NR::Matrix NR_MATRIX_IDENTITY = NR::identity();
NRRect   NR_RECT_EMPTY(NR_HUGE, NR_HUGE, -NR_HUGE, -NR_HUGE);
NRRectL  NR_RECT_L_EMPTY =
       {NR_HUGE_L, NR_HUGE_L, -NR_HUGE_L, -NR_HUGE_L};
NRRectL  NR_RECT_S_EMPTY =
       {NR_HUGE_S, NR_HUGE_S, -NR_HUGE_S, -NR_HUGE_S};

/** component_vectors[i] is like $e_i$ in common mathematical usage;
    or equivalently $I_i$ (where $I$ is the identity matrix). */
NR::Point const component_vectors[] = {NR::Point(1., 0.),
				       NR::Point(0., 1.)};

