

#include <2geom/point.h>
#include <2geom/sbasis.h>
#include <2geom/d2.h>

#include <vector>
#include <utility>


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


/*
 * find_intersection
 *
 *  input: A, B       - set of control points of two Bezier curve
 *  input: precision  - required precision of computation
 *  output: xs        - set of pairs of parameter values
 *                      at which crossing happens
 *
 *  This routine is based on the Bezier Clipping Algorithm,
 *  see: Sederberg - Computer Aided Geometric Design
 */
void find_intersections (std::vector< std::pair<double, double> > & xs,
                         std::vector<Point> const& A,
                         std::vector<Point> const& B,
                         double precision = 1e-5);

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
