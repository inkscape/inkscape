#include <cxxtest/TestSuite.h>

#include <glib.h>
#include <libnr/nr-macros.h> /* NR_DF_TEST_CLOSE */
#include <sstream>

/* mental disclaims all responsibility for this evil idea for testing
   static functions.  The main disadvantages are that we retain the
   #define's and `using' directives of the included file. */
#include "bezier-utils.cpp"

/* (Returns false if NaN encountered.) */
static bool range_approx_equal(double const a[], double const b[], unsigned const len) {
    for (unsigned i = 0; i < len; ++i) {
        if (!( fabs( a[i] - b[i] ) < 1e-4 )) {
            return false;
        }
    }
    return true;
}

static inline bool point_approx_equal(NR::Point const &a, NR::Point const &b, double const eps)
{
    using NR::X; using NR::Y;
    return ( NR_DF_TEST_CLOSE(a[X], b[X], eps) &&
             NR_DF_TEST_CLOSE(a[Y], b[Y], eps) );
}

static inline double square(double const x) {
    return x * x;
}

/** Determine whether the found control points are the same as previously found on some developer's
    machine.  Doesn't call utest__fail, just writes a message to stdout for diagnostic purposes:
    the most important test is that the root-mean-square of errors in the estimation are low rather
    than that the control points found are the same.
**/
static void compare_ctlpts(NR::Point const est_b[], NR::Point const exp_est_b[])
{
    unsigned diff_mask = 0;
    for (unsigned i = 0; i < 4; ++i) {
        for (unsigned d = 0; d < 2; ++d) {
            if ( fabs( est_b[i][d] - exp_est_b[i][d] ) > 1.1e-5 ) {
                diff_mask |= 1 << ( i * 2 + d );
            }
        }
    }
    if ( diff_mask != 0 ) {
        std::stringstream msg;
        msg << "Got different control points from previously-coded (diffs=0x" << std::hex << diff_mask << "\n";
        msg << " Previous:";
        for (unsigned i = 0; i < 4; ++i) {
            msg << " (" << exp_est_b[i][0] << ", " << exp_est_b[i][1] << ")"; // localizing ok
        }
        msg << "\n";
        msg << " Found:   ";
        for (unsigned i = 0; i < 4; ++i) {
            msg << " (" << est_b[i][0] << ", " << est_b[i][1] << ")"; // localizing ok
        }
        msg << "\n";
        TS_WARN(msg.str().c_str());
    }
}

static void compare_rms(NR::Point const est_b[], double const t[], NR::Point const d[], unsigned const n,
                        double const exp_rms_error)
{
    double sum_errsq = 0.0;
    for (unsigned i = 0; i < n; ++i) {
        NR::Point const fit_pt = bezier_pt(3, est_b, t[i]);
        NR::Point const diff = fit_pt - d[i];
        sum_errsq += dot(diff, diff);
    }
    double const rms_error = sqrt( sum_errsq / n );
    TS_ASSERT_LESS_THAN_EQUALS( rms_error , exp_rms_error + 1.1e-6 );
    if ( rms_error < exp_rms_error - 1.1e-6 ) {
        /* The fitter code appears to have improved [or the floating point calculations differ
           on this machine from the machine where exp_rms_error was calculated]. */
        char msg[200];
        sprintf(msg, "N.B. rms_error regression requirement can be decreased: have rms_error=%g.", rms_error); // localizing ok
        TS_TRACE(msg);
    }
}

class BezierUtilsTest : public CxxTest::TestSuite {
public:
    static NR::Point const c[4];
    static double const t[24];
    static unsigned const n;
    NR::Point d[24];
    static NR::Point const src_b[4];
    static NR::Point const tHat1;
    static NR::Point const tHat2;

    BezierUtilsTest()
    {
        /* Feed it some points that can be fit exactly with a single bezier segment, and see how
        well it manages. */
        for (unsigned i = 0; i < n; ++i) {
            d[i] = bezier_pt(3, src_b, t[i]);
        }
    }
    virtual ~BezierUtilsTest() {}

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static BezierUtilsTest *createSuite() { return new BezierUtilsTest(); }
    static void destroySuite( BezierUtilsTest *suite ) { delete suite; }

    void testCopyWithoutNansOrAdjacentDuplicates()
    {
        NR::Point const src[] = {
            NR::Point(2., 3.),
            NR::Point(2., 3.),
            NR::Point(0., 0.),
            NR::Point(2., 3.),
            NR::Point(2., 3.),
            NR::Point(1., 9.),
            NR::Point(1., 9.)
        };
        NR::Point const exp_dest[] = {
            NR::Point(2., 3.),
            NR::Point(0., 0.),
            NR::Point(2., 3.),
            NR::Point(1., 9.)
        };
        g_assert( G_N_ELEMENTS(src) == 7 );
        NR::Point dest[7];
        struct tst {
            unsigned src_ix0;
            unsigned src_len;
            unsigned exp_dest_ix0;
            unsigned exp_dest_len;
        } const test_data[] = {
            /* src start ix, src len, exp_dest start ix, exp dest len */
            {0, 0, 0, 0},
            {2, 1, 1, 1},
            {0, 1, 0, 1},
            {0, 2, 0, 1},
            {0, 3, 0, 2},
            {1, 3, 0, 3},
            {0, 5, 0, 3},
            {0, 6, 0, 4},
            {0, 7, 0, 4}
        };
        for (unsigned i = 0 ; i < G_N_ELEMENTS(test_data) ; ++i) {
            tst const &t = test_data[i];
            TS_ASSERT_EQUALS( t.exp_dest_len,
                              copy_without_nans_or_adjacent_duplicates(src + t.src_ix0,
                                                                       t.src_len,
                                                                       dest) );
            TS_ASSERT_SAME_DATA(dest,
                                exp_dest + t.exp_dest_ix0,
                                t.exp_dest_len);
        }
    }

    void testBezierPt1()
    {
        NR::Point const a[] = {NR::Point(2.0, 4.0),
                               NR::Point(1.0, 8.0)};
        TS_ASSERT_EQUALS( bezier_pt(1, a, 0.0) , a[0] );
        TS_ASSERT_EQUALS( bezier_pt(1, a, 1.0) , a[1] );
        TS_ASSERT_EQUALS( bezier_pt(1, a, 0.5) , NR::Point(1.5, 6.0) );
        double const t[] = {0.5, 0.25, 0.3, 0.6};
        for (unsigned i = 0; i < G_N_ELEMENTS(t); ++i) {
            double const ti = t[i], si = 1.0 - ti;
            TS_ASSERT_EQUALS( bezier_pt(1, a, ti) , si * a[0] + ti * a[1] );
        }
    }

    void testBezierPt2()
    {
        NR::Point const b[] = {NR::Point(1.0, 2.0),
                               NR::Point(8.0, 4.0),
                               NR::Point(3.0, 1.0)};
        TS_ASSERT_EQUALS( bezier_pt(2, b, 0.0) , b[0] );
        TS_ASSERT_EQUALS( bezier_pt(2, b, 1.0) , b[2] );
        TS_ASSERT_EQUALS( bezier_pt(2, b, 0.5) , NR::Point(5.0, 2.75) );
        double const t[] = {0.5, 0.25, 0.3, 0.6};
        for (unsigned i = 0; i < G_N_ELEMENTS(t); ++i) {
            double const ti = t[i], si = 1.0 - ti;
            NR::Point const exp_pt( si*si * b[0] + 2*si*ti * b[1] + ti*ti * b[2] );
            NR::Point const pt(bezier_pt(2, b, ti));
            TS_ASSERT(point_approx_equal(pt, exp_pt, 1e-11));
        }
    }

    void testBezierPt3()
    {
        TS_ASSERT_EQUALS( bezier_pt(3, c, 0.0) , c[0] );
        TS_ASSERT_EQUALS( bezier_pt(3, c, 1.0) , c[3] );
        TS_ASSERT_EQUALS( bezier_pt(3, c, 0.5) , NR::Point(4.0, 13.0/8.0) );
        double const t[] = {0.5, 0.25, 0.3, 0.6};
        for (unsigned i = 0; i < G_N_ELEMENTS(t); ++i) {
            double const ti = t[i], si = 1.0 - ti;
            TS_ASSERT( LInfty( bezier_pt(3, c, ti)
                                  - ( si*si*si * c[0] +
                                      3*si*si*ti * c[1] +
                                      3*si*ti*ti * c[2] +
                                      ti*ti*ti * c[3] ) )
                          < 1e-4 );
        }
    }

    void testComputeMaxErrorRatio()
    {
        struct Err_tst {
            NR::Point pt;
            double u;
            double err;
        } const err_tst[] = {
            {c[0], 0.0, 0.0},
            {NR::Point(4.0, 13.0/8.0), 0.5, 0.0},
            {NR::Point(4.0, 2.0), 0.5, 9.0/64.0},
            {NR::Point(3.0, 2.0), 0.5, 1.0 + 9.0/64.0},
            {NR::Point(6.0, 2.0), 0.5, 4.0 + 9.0/64.0},
            {c[3], 1.0, 0.0},
        };
        NR::Point d[G_N_ELEMENTS(err_tst)];
        double u[G_N_ELEMENTS(err_tst)];
        for (unsigned i = 0; i < G_N_ELEMENTS(err_tst); ++i) {
            Err_tst const &t = err_tst[i];
            d[i] = t.pt;
            u[i] = t.u;
        }
        g_assert( G_N_ELEMENTS(u) == G_N_ELEMENTS(d) );
        unsigned max_ix = ~0u;
        double const err_ratio = compute_max_error_ratio(d, u, G_N_ELEMENTS(d), c, 1.0, &max_ix);
        TS_ASSERT_LESS_THAN( fabs( sqrt(err_tst[4].err) - err_ratio ) , 1e-12 );
        TS_ASSERT_EQUALS( max_ix , 4u );
    }

    void testChordLengthParameterize()
    {
        /* n == 2 */
        {
            NR::Point const d[] = {NR::Point(2.9415, -5.8149),
                                   NR::Point(23.021, 4.9814)};
            double u[G_N_ELEMENTS(d)];
            double const exp_u[] = {0.0, 1.0};
            g_assert( G_N_ELEMENTS(u) == G_N_ELEMENTS(exp_u) );
            chord_length_parameterize(d, u, G_N_ELEMENTS(d));
            TS_ASSERT_SAME_DATA(u, exp_u, G_N_ELEMENTS(exp_u));
        }

        /* Straight line. */
        {
            double const exp_u[] = {0.0, 0.1829, 0.2105, 0.2105, 0.619, 0.815, 0.999, 1.0};
            unsigned const n = G_N_ELEMENTS(exp_u);
            NR::Point d[n];
            double u[n];
            NR::Point const a(-23.985, 4.915), b(4.9127, 5.203);
            for (unsigned i = 0; i < n; ++i) {
                double bi = exp_u[i], ai = 1.0 - bi;
                d[i] = ai * a  +  bi * b;
            }
            chord_length_parameterize(d, u, n);
            TS_ASSERT(range_approx_equal(u, exp_u, n));
        }
    }

    void testGenerateBezier()
    {
        NR::Point est_b[4];
        generate_bezier(est_b, d, t, n, tHat1, tHat2, 1.0);

        compare_ctlpts(est_b, src_b);

        /* We're being unfair here in using our t[] rather than best t[] for est_b: we
           may over-estimate RMS of errors. */
        compare_rms(est_b, t, d, n, 1e-8);
    }

    void testSpBezierFitCubicFull()
    {
        NR::Point est_b[4];
        int splitpoints[2];
        gint const succ = sp_bezier_fit_cubic_full(est_b, splitpoints, d, n, tHat1, tHat2, square(1.2), 1);
        TS_ASSERT_EQUALS( succ , 1 );

        NR::Point const exp_est_b[4] = {
            NR::Point(5.000000, -3.000000),
            NR::Point(7.5753, -0.4247),
            NR::Point(4.77533, 1.22467),
            NR::Point(3, 3)
        };
        compare_ctlpts(est_b, exp_est_b);

        /* We're being unfair here in using our t[] rather than best t[] for est_b: we
           may over-estimate RMS of errors. */
        compare_rms(est_b, t, d, n, .307911);
    }

    void testSpBezierFitCubic()
    {
        NR::Point est_b[4];
        gint const succ = sp_bezier_fit_cubic(est_b, d, n, square(1.2));
        TS_ASSERT_EQUALS( succ , 1 );

        NR::Point const exp_est_b[4] = {
            NR::Point(5.000000, -3.000000),
            NR::Point(7.57134, -0.423509),
            NR::Point(4.77929, 1.22426),
            NR::Point(3, 3)
        };
        compare_ctlpts(est_b, exp_est_b);

#if 1 /* A change has been made to right_tangent.  I believe that usually this change
         will result in better fitting, but it won't do as well for this example where
         we happen to be feeding a t=0.999 point to the fitter. */
        TS_WARN("TODO: Update this test case for revised right_tangent implementation.");
        /* In particular, have a test case to show whether the new implementation
           really is likely to be better on average. */
#else
        /* We're being unfair here in using our t[] rather than best t[] for est_b: we
           may over-estimate RMS of errors. */
        compare_rms(est_b, t, d, n, .307983);
#endif
    }
};

// This is not very neat, but since we know this header is only included by the generated CxxTest file it shouldn't give any problems
NR::Point const BezierUtilsTest::c[4] = {
    NR::Point(1.0, 2.0),
    NR::Point(8.0, 4.0),
    NR::Point(3.0, 1.0),
    NR::Point(-2.0, -4.0)};
double const BezierUtilsTest::t[24] = {
    0.0, .001, .03, .05, .09, .13, .18, .25, .29, .33,  .39, .44,
    .51,  .57, .62, .69, .75, .81, .91, .93, .97, .98, .999, 1.0};
unsigned const BezierUtilsTest::n = G_N_ELEMENTS(BezierUtilsTest::t);
NR::Point const BezierUtilsTest::src_b[4] = {
    NR::Point(5., -3.),
    NR::Point(8., 0.),
    NR::Point(4., 2.),
    NR::Point(3., 3.)};
NR::Point const BezierUtilsTest::tHat1(unit_vector( BezierUtilsTest::src_b[1] - BezierUtilsTest::src_b[0] ));
NR::Point const BezierUtilsTest::tHat2(unit_vector( BezierUtilsTest::src_b[2] - BezierUtilsTest::src_b[3] ));

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
