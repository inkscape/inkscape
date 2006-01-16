#include <utest/utest.h>
#include "mod360.h"
#include <glib.h>
#include <math.h>

/** Note: doesn't distinguish between 0.0 and -0.0. */
static bool same_double(double const x, double const y)
{
    return ( ( isnan(x) && isnan(y) )
             || ( x == y ) );
}

int main(int argc, char **argv)
{
    double const inf = 1e400;
    double const nan = inf - inf;

    utest_start("mod360.cpp");

    UTEST_TEST("same_double") {
        double const sd_cases[] = {inf, -inf, nan, 0.0, -1.0, 1.0, 8.0};
        for (unsigned i = 0; i < G_N_ELEMENTS(sd_cases); ++i) {
            for (unsigned j = 0; j < G_N_ELEMENTS(sd_cases); ++j) {
                UTEST_ASSERT( same_double(sd_cases[i], sd_cases[j])
                              == ( i == j ) );
            }
        }
    }

    UTEST_TEST("mod360") {
        struct Case {
            double x;
            double y;
        } const cases[] = {
            {0, 0},
            {10, 10},
            {360, 0},
            {361, 1},
            {-1, 359},
            {-359, 1},
            {-360, -0},
            {-361, 359},
            {inf, 0},
            {-inf, 0},
            {nan, 0},
            {720, 0},
            {-721, 359},
            {-1000, 80}
        };
        for(unsigned i = 0; i < G_N_ELEMENTS(cases); ++i) {
            Case const &c = cases[i];
            UTEST_ASSERT(same_double(mod360(c.x), c.y));
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
