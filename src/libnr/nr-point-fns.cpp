#include <libnr/nr-point-fns.h>
#include <isnan.h>

using NR::Point;

/** Compute the L infinity, or maximum, norm of \a p. */
NR::Coord NR::LInfty(Point const &p) {
    NR::Coord const a(fabs(p[0]));
    NR::Coord const b(fabs(p[1]));
    return ( a < b || IS_NAN(b)
             ? b
             : a );
}

/** Returns true iff p is a zero vector, i.e.\ Point(0, 0).
 *
 *  (NaN is considered non-zero.)
 */
bool
NR::is_zero(Point const &p)
{
    return ( p[0] == 0 &&
             p[1] == 0   );
}

bool
NR::is_unit_vector(Point const &p)
{
    return fabs(1.0 - L2(p)) <= 1e-4;
    /* The tolerance of 1e-4 is somewhat arbitrary.  NR::Point::normalize is believed to return
       points well within this tolerance.  I'm not aware of any callers that want a small
       tolerance; most callers would be ok with a tolerance of 0.25. */
}

NR::Coord NR::atan2(Point const p) {
    return std::atan2(p[NR::Y], p[NR::X]);
}

/** Returns a version of \a a scaled to be a unit vector (within rounding error).
 *
 *  The current version tries to handle infinite coordinates gracefully,
 *  but it's not clear that any callers need that.
 *
 *  \pre a != Point(0, 0).
 *  \pre Neither coordinate is NaN.
 *  \post L2(ret) very near 1.0.
 */
Point NR::unit_vector(Point const &a)
{
    Point ret(a);
    ret.normalize();
    return ret;
}

NR::Point abs(NR::Point const &b)
{
    NR::Point ret;
    for ( int i = 0 ; i < 2 ; i++ ) {
        ret[i] = fabs(b[i]);
    }
    return ret;
}

NR::Point *
get_snap_vector (NR::Point p, NR::Point o, double snap, double initial)
{
    double r = NR::L2 (p - o);
    if (r < 1e-3)
        return NULL;
    double angle = NR::atan2 (p - o);
    // snap angle to snaps increments, starting from initial:
    double a_snapped = initial + floor((angle - initial)/snap + 0.5) * snap;
    // calculate the new position and subtract p to get the vector:
    return new NR::Point (o + r * NR::Point(cos(a_snapped), sin(a_snapped)) - p);
}

NR::Point
snap_vector_midpoint (NR::Point p, NR::Point begin, NR::Point end, double snap)
{
    double length = NR::L2(end - begin);
    NR::Point be = (end - begin) / length;
    double r = NR::dot(p - begin, be);

    if (r < 0.0) return begin;
    if (r > length) return end;

    double snapdist = length * snap;
    double r_snapped = (snap==0) ? r : floor(r/(snapdist + 0.5)) * snapdist;

    return (begin + r_snapped * be);
}

double
get_offset_between_points (NR::Point p, NR::Point begin, NR::Point end)
{
    double length = NR::L2(end - begin);
    NR::Point be = (end - begin) / length;
    double r = NR::dot(p - begin, be);

    if (r < 0.0) return 0.0;
    if (r > length) return 1.0;

    return (r / length);
}

NR::Point
project_on_linesegment(NR::Point const p, NR::Point const p1, NR::Point const p2) 
{
    // p_proj = projection of p on the linesegment running from p1 to p2
    // p_proj = p1 + u (p2 - p1)
    // calculate u according to "Minimum Distance between a Point and a Line"
    // see http://local.wasp.uwa.edu.au/~pbourke/geometry/pointline/        
    
    // Warning: projected points will not necessarily be in between the endpoints of the linesegments!  
    
    if (p1 == p2) { // to avoid div. by zero below
        return p;
    }
    
    NR::Point const d1(p-p1); // delta 1
    NR::Point const d2(p2-p1); // delta 2
    double const u = (d1[NR::X] * d2[NR::X] + d1[NR::Y] * d2[NR::Y]) / (NR::L2(d2) * NR::L2(d2));
    
    return (p1 + u*(p2-p1));
}

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
