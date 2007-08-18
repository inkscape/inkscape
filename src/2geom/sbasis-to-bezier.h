#ifndef _SBASIS_TO_BEZIER
#define _SBASIS_TO_BEZIER

#include "d2.h"
#include "path.h"

namespace Geom{
// this produces a degree k bezier from a degree k sbasis
std::vector<double>
sbasis_to_bezier(SBasis const &B, unsigned q = 0);

std::vector<Geom::Point>
sbasis_to_bezier(D2<SBasis> const &B, unsigned q = 0);

std::vector<Path> path_from_piecewise(Piecewise<D2<SBasis> > const &B, double tol);

Path path_from_sbasis(D2<SBasis> const &B, double tol);

};
#endif
