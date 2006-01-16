#include <libnr/nr-matrix-fns.h>

namespace NR {

Matrix elliptic_quadratic_form(Matrix const &m) {
    double const od = m[0] * m[1]  +  m[2] * m[3];
    return Matrix((m[0]*m[0] + m[1]*m[1]), od,
                  od, (m[2]*m[2] + m[3]*m[3]),
                  0, 0);
/* def quadratic_form((a, b), (c, d)):
   return ((a*a + c*c), a*c+b*d),(a*c+b*d, (b*b + d*d)) */
}

Eigen::Eigen(Matrix const &m) {
    double const B = -m[0] - m[3];
    double const C = m[0]*m[3] - m[1]*m[2];
    double const center = -B/2.0;
    double const delta = sqrt(B*B-4*C)/2.0;
    values = Point(center + delta, center - delta);
    for (int i = 0; i < 2; i++) {
        vectors[i] = unit_vector(rot90(Point(m[0]-values[i], m[1])));
    }
}

/** Returns just the scale/rotate/skew part of the matrix without the translation part. */
Matrix transform(Matrix const &m) {
    Matrix const ret(m[0], m[1],
                     m[2], m[3],
                     0, 0);
    return ret;
}

translate get_translation(Matrix const &m) {
    return translate(m[4], m[5]);
}

void matrix_print(const gchar *say, Matrix const &m)
{ 
    printf ("%s %g %g %g %g %g %g\n", say, m[0], m[1], m[2], m[3], m[4], m[5]);
}

}  // namespace NR


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
