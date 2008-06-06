#include <glib/gmacros.h>
#include <cmath>

#include "libnr/in-svg-plane.h"
#include "utest/utest.h"
#include "isnan.h"

int main(int argc, char *argv[])
{
    utest_start("in-svg-plane.h");

    NR::Point const p3n4(3.0, -4.0);
    NR::Point const p0(0.0, 0.0);
    double const small = pow(2.0, -1070);
    double const inf = 1e400;
    double const nan = inf - inf;

    NR::Point const small_left(-small, 0.0);
    NR::Point const small_n3_4(-3.0 * small, 4.0 * small);
    NR::Point const part_nan(3., nan);

    assert(IS_NAN(nan));
    assert(!IS_NAN(small));

    UTEST_TEST("in_svg_plane") {
        UTEST_ASSERT(in_svg_plane(p3n4));
        UTEST_ASSERT(in_svg_plane(p0));
        UTEST_ASSERT(in_svg_plane(small_left));
        UTEST_ASSERT(in_svg_plane(small_n3_4));
        UTEST_ASSERT(nan != nan);
        UTEST_ASSERT(!in_svg_plane(NR::Point(nan, 3.)));
        UTEST_ASSERT(!in_svg_plane(NR::Point(inf, nan)));
        UTEST_ASSERT(!in_svg_plane(NR::Point(0., -inf)));
        double const xs[] = {inf, -inf, nan, 1., -2., small, -small};
        for (unsigned i = 0; i < G_N_ELEMENTS(xs); ++i) {
            for (unsigned j = 0; j < G_N_ELEMENTS(xs); ++j) {
                UTEST_ASSERT( in_svg_plane(NR::Point(xs[i], xs[j]))
                              == (fabs(xs[i]) < inf &&
                                  fabs(xs[j]) < inf   ) );
            }
        }
    }

    return ( utest_end()
             ? EXIT_SUCCESS
             : EXIT_FAILURE );
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
