#include <cxxtest/TestSuite.h>

#include <glib/gmacros.h>
#include <cmath>

#include "libnr/in-svg-plane.h"
#include "2geom/isnan.h"

class InSvgPlaneTest : public CxxTest::TestSuite
{
public:

    InSvgPlaneTest() :
        setupValid(true),
        p3n4( 3.0, -4.0 ),
        p0(0.0, 0.0),
        small( pow(2.0, -1070) ),
        inf( 1e400 ),
        nan( inf - inf ),
        small_left( -small, 0.0 ),
        small_n3_4( -3.0 * small, 4.0 * small ),
        part_nan( 3., nan )
    {
        setupValid &= IS_NAN(nan);
        setupValid &= !IS_NAN(small);
    }
    virtual ~InSvgPlaneTest() {}

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static InSvgPlaneTest *createSuite() { return new InSvgPlaneTest(); }
    static void destroySuite( InSvgPlaneTest *suite ) { delete suite; }

// Called before each test in this suite
    void setUp()
    {
        TS_ASSERT( setupValid );
    }

    bool setupValid;
    NR::Point const p3n4;
    NR::Point const p0;
    double const small;
    double const inf;
    double const nan;
    NR::Point const small_left;
    NR::Point const small_n3_4;
    NR::Point const part_nan;


    void testInSvgPlane(void)
    {
        TS_ASSERT( in_svg_plane(p3n4) );
        TS_ASSERT( in_svg_plane(p0) );
        TS_ASSERT( in_svg_plane(small_left) );
        TS_ASSERT( in_svg_plane(small_n3_4) );
        TS_ASSERT_DIFFERS( nan, nan );
        TS_ASSERT( !in_svg_plane(NR::Point(nan, 3.)) );
        TS_ASSERT( !in_svg_plane(NR::Point(inf, nan)) );
        TS_ASSERT( !in_svg_plane(NR::Point(0., -inf)) );
        double const xs[] = {inf, -inf, nan, 1., -2., small, -small};
        for (unsigned i = 0; i < G_N_ELEMENTS(xs); ++i) {
            for (unsigned j = 0; j < G_N_ELEMENTS(xs); ++j) {
                TS_ASSERT_EQUALS( in_svg_plane(NR::Point(xs[i], xs[j])),
                                  (fabs(xs[i]) < inf &&
                                   fabs(xs[j]) < inf   ) );
            }
        }
    }
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
