#ifndef _SBASIS_TO_BEZIER
#define _SBASIS_TO_BEZIER

#include <2geom/d2.h>
#include <2geom/path.h>

#include <vector>

namespace Geom {

void sbasis_to_bezier (Bezier & bz, SBasis const& sb, size_t sz = 0);
void sbasis_to_bezier (std::vector<Point> & bz, D2<SBasis> const& sb, size_t sz = 0);
void bezier_to_sbasis (SBasis & sb, Bezier const& bz);
void bezier_to_sbasis (D2<SBasis> & sb, std::vector<Point> const& bz);


#if 0
// this produces a degree k bezier from a degree k sbasis
Bezier
sbasis_to_bezier(SBasis const &B, unsigned q = 0);

// inverse
SBasis bezier_to_sbasis(Bezier const &B);


std::vector<Geom::Point>
sbasis_to_bezier(D2<SBasis> const &B, unsigned q = 0);
#endif


std::vector<Path> path_from_piecewise(Piecewise<D2<SBasis> > const &B, double tol, bool only_cubicbeziers = false);

Path path_from_sbasis(D2<SBasis> const &B, double tol, bool only_cubicbeziers = false);
inline Path cubicbezierpath_from_sbasis(D2<SBasis> const &B, double tol)
    { return path_from_sbasis(B, tol, true); }

} // end namespace Geom



#endif

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
