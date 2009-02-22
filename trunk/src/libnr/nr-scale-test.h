#include <cxxtest/TestSuite.h>

#include <libnr/nr-scale.h>
#include <libnr/nr-scale-ops.h>

class NrScaleTest : public CxxTest::TestSuite
{
public:

    NrScaleTest() :
        sa( 1.5, 2.0 ),
        b( -2.0, 3.0 ),
        sb( b )
    {
    }
    virtual ~NrScaleTest() {}

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static NrScaleTest *createSuite() { return new NrScaleTest(); }
    static void destroySuite( NrScaleTest *suite ) { delete suite; }

    NR::scale const sa;
    NR::Point const b;
    NR::scale const sb;



    void testXY_CtorArrayOperator(void)
    {
        TS_ASSERT_EQUALS( sa[NR::X], 1.5 );
        TS_ASSERT_EQUALS( sa[NR::Y], 2.0 );
        TS_ASSERT_EQUALS( sa[0u], 1.5 );
        TS_ASSERT_EQUALS( sa[1u], 2.0 );
    }


    void testCopyCtor_AssignmentOp_NotEquals(void)
    {
        NR::scale const sa_copy(sa);
        TS_ASSERT_EQUALS( sa, sa_copy );
        TS_ASSERT(!( sa != sa_copy ));
        TS_ASSERT( sa != sb );
    }

    void testAssignmentOp(void)
    {
        NR::scale sa_eq(sb);
        sa_eq = sa;
        TS_ASSERT_EQUALS( sa, sa_eq );
    }

    void testPointCtor(void)
    {
        TS_ASSERT_EQUALS( sb[NR::X], b[NR::X] );
        TS_ASSERT_EQUALS( sb[NR::Y], b[NR::Y] );
    }

    void testOpStarPointScale(void)
    {
        NR::Point const ab( b * sa );
        TS_ASSERT_EQUALS( ab, NR::Point(-3.0, 6.0) );
    }

    void testOpStarScaleScale(void)
    {
        NR::scale const sab( sa * sb );
        TS_ASSERT_EQUALS( sab, NR::scale(-3.0, 6.0) );
    }

    void testOpDivScaleScale(void)
    {
        NR::scale const sa_b( sa / sb );
        NR::scale const exp_sa_b(-0.75, 2./3.);
        TS_ASSERT_EQUALS( sa_b[0], exp_sa_b[0] );
//      TS_ASSERT_EQUALS( fabs( sa_b[1] - exp_sa_b[1] ) < 1e-10 );
        TS_ASSERT_DELTA( sa_b[1], exp_sa_b[1], 1e-10 );
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
