#include "sbasis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "d2.h"

namespace Geom {

std::vector<std::pair<double, double> > 
find_intersections( D2<SBasis> const & A, 
                    D2<SBasis> const & B);

std::vector<std::pair<double, double> > 
find_self_intersections(D2<SBasis> const & A);

// Bezier form
std::vector<std::pair<double, double> > 
find_intersections( std::vector<Point> const & A, 
                    std::vector<Point> const & B);

std::vector<std::pair<double, double> > 
find_self_intersections(std::vector<Point> const & A);

};

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
