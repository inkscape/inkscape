
#ifndef SEEN_ROUND_TEST_H
#define SEEN_ROUND_TEST_H

#include <cxxtest/TestSuite.h>

#include <vector>
#include <round.h>

class RoundTest : public CxxTest::TestSuite
{
public:
    struct Case {
        double arg0;
        double ret;
    };

    std::vector<Case> nonneg_round_cases;
    std::vector<Case> nonpos_round_cases;

    RoundTest() :
        TestSuite()
    {
        Case cases[] = {
            { 5.0, 5.0 },
            { 0.0, 0.0 },
            { 5.4, 5.0 },
            { 5.6, 6.0 },
            { 1e-7, 0.0 },
            { 1e7 + .49, 1e7 },
            { 1e7 + .51, 1e7 + 1 },
            { 1e12 + .49, 1e12 },
            { 1e12 + .51, 1e12 + 1 },
            { 1e40, 1e40 }
        };

        for ( size_t i = 0; i < G_N_ELEMENTS(cases); i++ )
        {
            nonneg_round_cases.push_back( cases[i] );

            Case tmp = {-nonneg_round_cases[i].arg0, -nonneg_round_cases[i].ret};
            nonpos_round_cases.push_back( tmp );
        }
    }

    virtual ~RoundTest()
    {
    }
    
    static RoundTest *createSuite() { return new RoundTest(); }
    static void destroySuite( RoundTest *suite ) { delete suite; }

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------



    void testNonNegRound()
    {
        for ( size_t i = 0; i < nonneg_round_cases.size(); i++ )
        {
            double result = Inkscape::round( nonneg_round_cases[i].arg0 );
            TS_ASSERT_EQUALS( result, nonneg_round_cases[i].ret );
        }
    }

    void testNonPosRoung()
    {
        for ( size_t i = 0; i < nonpos_round_cases.size(); i++ )
        {
            double result = Inkscape::round( nonpos_round_cases[i].arg0 );
            TS_ASSERT_EQUALS( result, nonpos_round_cases[i].ret );
        }
    }

};


#endif // SEEN_ROUND_TEST_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

