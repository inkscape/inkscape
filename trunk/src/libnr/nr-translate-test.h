#include <cxxtest/TestSuite.h>

#include <libnr/nr-point-ops.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-point-matrix-ops.h>
#include <libnr/nr-translate.h>
#include <libnr/nr-translate-ops.h>

class NrTranslateTest : public CxxTest::TestSuite
{
public:

    NrTranslateTest() :
        b( -2.0, 3.0 ),
        tb( b ),
        tc( -3.0, -2.0 ),
        tbc( tb * tc ),
        t_id( 0.0, 0.0 ),
        m_id( NR::identity() )
    {
    }
    virtual ~NrTranslateTest() {}

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static NrTranslateTest *createSuite() { return new NrTranslateTest(); }
    static void destroySuite( NrTranslateTest *suite ) { delete suite; }

    NR::Point const b;
    NR::translate const tb;
    NR::translate const tc;
    NR::translate const tbc;
    NR::translate const t_id;
    NR::Matrix const m_id;


    void testCtorsArrayOperator(void)
    {
        TS_ASSERT_EQUALS( tc[NR::X], -3.0 );
        TS_ASSERT_EQUALS( tc[NR::Y], -2.0 );

        TS_ASSERT_EQUALS( tb[0], b[NR::X] );
        TS_ASSERT_EQUALS( tb[1], b[NR::Y] );
    }

    void testAssignmentOperator(void)
    {
        NR::translate tb_eq(tc);
        tb_eq = tb;
        TS_ASSERT_EQUALS( tb, tb_eq );
        TS_ASSERT_DIFFERS( tb_eq, tc );
    }

    void testOpStarTranslateTranslate(void)
    {
        TS_ASSERT_EQUALS( tbc.offset, NR::Point(-5.0, 1.0) );
        TS_ASSERT_EQUALS( tbc.offset, ( tc * tb ).offset );
        TS_ASSERT_EQUALS( NR::Matrix(tbc), NR::Matrix(tb) * NR::Matrix(tc) );
    }

    void testOpStarPointTranslate(void)
    {
        TS_ASSERT_EQUALS( tbc.offset, b * tc );
        TS_ASSERT_EQUALS( b * tc, b * NR::Matrix(tc) );
    }

    void testIdentity(void)
    {
        TS_ASSERT_EQUALS( b * t_id, b );
        TS_ASSERT_EQUALS( NR::Matrix(t_id), m_id );
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
