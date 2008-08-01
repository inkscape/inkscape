#include <utest/utest.h>
#include <libnr/nr-point-ops.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-point-matrix-ops.h>
#include <libnr/nr-translate.h>
#include <libnr/nr-translate-ops.h>
using NR::X;
using NR::Y;


int main(int /*argc*/, char */*argv*/[])
{
    utest_start("translate");

    NR::Point const b(-2.0, 3.0);
    NR::translate const tb(b);
    NR::translate const tc(-3.0, -2.0);
    UTEST_TEST("constructors, operator[]") {
        UTEST_ASSERT( tc[X] == -3.0 && tc[Y] == -2.0 );
        UTEST_ASSERT( tb[0] == b[X] && tb[1] == b[Y] );
    }

    UTEST_TEST("operator=") {
        NR::translate tb_eq(tc);
        tb_eq = tb;
        UTEST_ASSERT( tb == tb_eq );
        UTEST_ASSERT( tb_eq != tc );
    }

    NR::translate const tbc( tb * tc );
    UTEST_TEST("operator*(translate, translate)") {
        UTEST_ASSERT( tbc.offset == NR::Point(-5.0, 1.0) );
        UTEST_ASSERT( tbc.offset == ( tc * tb ).offset );
        UTEST_ASSERT( NR::Matrix(tbc) == NR::Matrix(tb) * NR::Matrix(tc) );
    }

    UTEST_TEST("operator*(Point, translate)") {
        UTEST_ASSERT( tbc.offset == b * tc );
        UTEST_ASSERT( b * tc == b * NR::Matrix(tc) );
    }

    NR::translate const t_id(0.0, 0.0);
    NR::Matrix const m_id(NR::identity());
    UTEST_TEST("identity") {
        UTEST_ASSERT( b * t_id == b );
        UTEST_ASSERT( NR::Matrix(t_id) == m_id );
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
