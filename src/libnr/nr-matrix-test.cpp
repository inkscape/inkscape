#include <utest/utest.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-matrix-rotate-ops.h>
#include <libnr/nr-matrix-scale-ops.h>
#include <libnr/nr-point-matrix-ops.h>
#include <libnr/nr-rotate.h>
#include <libnr/nr-rotate-ops.h>
#include <libnr/nr-scale-ops.h>
#include <libnr/nr-scale-translate-ops.h>
#include <libnr/nr-translate.h>
#include <libnr/nr-translate-ops.h>
#include <libnr/nr-translate-scale-ops.h>
using NR::Matrix;
using NR::X;
using NR::Y;

inline bool point_equalp(NR::Point const &a, NR::Point const &b)
{
    return ( NR_DF_TEST_CLOSE(a[X], b[X], 1e-5) &&
             NR_DF_TEST_CLOSE(a[Y], b[Y], 1e-5)   );
}

int main(int argc, char *argv[])
{
    int rc = EXIT_SUCCESS;

    Matrix const m_id(NR::identity());
    NR::rotate const r_id(NR::Point(1, 0));
    NR::translate const t_id(0, 0);

    utest_start("Matrix");

    Matrix const c16(1.0, 2.0,
                     3.0, 4.0,
                     5.0, 6.0);
    UTEST_TEST("basic constructors, operator=") {
        Matrix const c16_copy(c16);
        Matrix c16_eq(m_id);
        c16_eq = c16;
        for(unsigned i = 0; i < 6; ++i) {
            UTEST_ASSERT( c16[i] == 1.0 + i );
            UTEST_ASSERT( c16[i] == c16_copy[i] );
            UTEST_ASSERT( c16[i] == c16_eq[i] );
            UTEST_ASSERT( m_id[i] == double( i == 0 || i == 3 ) );
        }
    }

    UTEST_TEST("scale constructor") {
        NR::scale const s(2.0, 3.0);
        NR::Matrix const ms(s);
        NR::Point const p(5.0, 7.0);
        UTEST_ASSERT( p * s == NR::Point(10.0, 21.0) );
        UTEST_ASSERT( p * ms == NR::Point(10.0, 21.0) );
    }

    NR::rotate const r86(NR::Point(.8, .6));
    NR::Matrix const mr86(r86);
    UTEST_TEST("rotate constructor") {
        NR::Point const p0(1.0, 0.0);
        NR::Point const p90(0.0, 1.0);
        UTEST_ASSERT( p0 * r86 == NR::Point(.8, .6) );
        UTEST_ASSERT( p0 * mr86 == NR::Point(.8, .6) );
        UTEST_ASSERT( p90 * r86 == NR::Point(-.6, .8) );
        UTEST_ASSERT( p90 * mr86 == NR::Point(-.6, .8) );
        UTEST_ASSERT(matrix_equalp(Matrix( r86 * r86 ),
                                   mr86 * mr86,
                                   1e-14));
    }

    NR::translate const t23(2.0, 3.0);
    UTEST_TEST("translate constructor") {
        NR::Matrix const mt23(t23);
        NR::Point const b(-2.0, 3.0);
        UTEST_ASSERT( b * t23 == b * mt23 );
    }

    NR::scale const s_id(1.0, 1.0);
    UTEST_TEST("test_identity") {
        UTEST_ASSERT(m_id.test_identity());
        UTEST_ASSERT(Matrix(t_id).test_identity());
        UTEST_ASSERT(!(Matrix(NR::translate(-2, 3)).test_identity()));
        UTEST_ASSERT(Matrix(r_id).test_identity());
        NR::rotate const rot180(NR::Point(-1, 0));
        UTEST_ASSERT(!(Matrix(rot180).test_identity()));
        UTEST_ASSERT(Matrix(s_id).test_identity());
        UTEST_ASSERT(!(Matrix(NR::scale(1.0, 0.0)).test_identity()));
        UTEST_ASSERT(!(Matrix(NR::scale(0.0, 1.0)).test_identity()));
        UTEST_ASSERT(!(Matrix(NR::scale(1.0, -1.0)).test_identity()));
        UTEST_ASSERT(!(Matrix(NR::scale(-1.0, -1.0)).test_identity()));
    }

    UTEST_TEST("inverse") {
        UTEST_ASSERT( m_id.inverse() == m_id );
        UTEST_ASSERT( Matrix(t23).inverse() == Matrix(NR::translate(-2.0, -3.0)) );
        NR::scale const s2(-4.0, 2.0);
        NR::scale const sp5(-.25, .5);
        UTEST_ASSERT( Matrix(s2).inverse() == Matrix(sp5) );
        UTEST_ASSERT( Matrix(sp5).inverse() == Matrix(s2) );
    }

    UTEST_TEST("elliptic quadratic form") {
        NR::Matrix const aff(1.0, 1.0,
                             0.0, 1.0,
                             5.0, 6.0);
        NR::Matrix const invaff = aff.inverse();
        UTEST_ASSERT( invaff[1] == -1.0 );

        NR::Matrix const ef(elliptic_quadratic_form(invaff));
        NR::Matrix const exp_ef(2, -1,
                                -1, 1,
                                0, 0);
        UTEST_ASSERT( ef == exp_ef );
    }

    UTEST_TEST("Matrix * rotate") {
        NR::Matrix const ma(2.0, -1.0,
                            4.0, 4.0,
                            -0.5, 2.0);
        NR::Matrix const a_r86( ma * r86 );
        NR::Matrix const ma1( a_r86 * r86.inverse() );
        UTEST_ASSERT(matrix_equalp(ma1, ma, 1e-12));
        NR::Matrix const exp_a_r86( 2*.8 + -1*-.6,  2*.6 + -1*.8,
                                    4*.8 + 4*-.6,   4*.6 + 4*.8,
                                    -.5*.8 + 2*-.6, -.5*.6 + 2*.8 );
        UTEST_ASSERT(matrix_equalp(a_r86, exp_a_r86, 1e-12));
    }

    UTEST_TEST("translate*scale, scale*translate") {
        NR::translate const t2n4(2, -4);
        NR::scale const sn2_8(-2, 8);
        NR::Matrix const exp_ts(-2, 0,
                                0,  8,
                                -4, -32);
        NR::Matrix const exp_st(-2, 0,
                                0,  8,
                                2, -4);
        UTEST_ASSERT( exp_ts == t2n4 * sn2_8 );
        UTEST_ASSERT( exp_st == sn2_8 * t2n4 );
    }

    UTEST_TEST("Matrix * scale") {
        NR::Matrix const ma(2.0, -1.0,
                            4.0, 4.0,
                            -0.5, 2.0);
        NR::scale const sn2_8(-2, 8);
        NR::Matrix const exp_as(-4, -8,
                                -8, 32,
                                1,  16);
        UTEST_ASSERT( ma * sn2_8 == exp_as );
    }

    if (!utest_end()) {
        rc = EXIT_FAILURE;
    }

    return rc;
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
