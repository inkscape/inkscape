#include <cmath>
#include <glib/gmacros.h>

#include <libnr/nr-rotate-fns.h>
#include <utest/utest.h>

int main(int /*argc*/, char */*argv*/[])
{
    utest_start("rotate-fns");

    UTEST_TEST("rotate_degrees") {
        double const d[] = {
            0, 90, 180, 270, 360, 45, 45.01, 44.99, 134, 135, 136, 314, 315, 317, 359, 361
        };
        for (unsigned i = 0; i < G_N_ELEMENTS(d); ++i) {
            double const degrees = d[i];
            NR::rotate const rot(rotate_degrees(degrees));
            NR::rotate const rot_approx( M_PI * ( degrees / 180. ) );
            UTEST_ASSERT(rotate_equalp(rot, rot_approx, 1e-12));

            NR::rotate const rot_inv(rotate_degrees(-degrees));
            NR::rotate const rot_compl(rotate_degrees(360 - degrees));
            UTEST_ASSERT(rotate_equalp(rot_inv, rot_compl, 1e-12));

            UTEST_ASSERT(!rotate_equalp(rot, rotate_degrees(degrees + 1), 1e-5));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
