
#ifndef SEEN_MOD_360_TEST_H
#define SEEN_MOD_360_TEST_H

#include <cxxtest/TestSuite.h>
#include <2geom/math-utils.h>
#include "mod360.h"


class Mod360Test : public CxxTest::TestSuite
{
public:
    static double inf() { return INFINITY; }
    static double nan() { return ((double)INFINITY) - ((double)INFINITY); }

    void testMod360()
    {
        double cases[][2] = {
            {0, 0},
            {10, 10},
            {360, 0},
            {361, 1},
            {-1, 359},
            {-359, 1},
            {-360, -0},
            {-361, 359},
            {inf(), 0},
            {-inf(), 0},
            {nan(), 0},
            {720, 0},
            {-721, 359},
            {-1000, 80}
        };

        for ( unsigned i = 0; i < G_N_ELEMENTS(cases); i++ ) {
            double result = mod360( cases[i][0] );
            TS_ASSERT_EQUALS( cases[i][1], result );
        }
    }

};


#endif // SEEN_MOD_360_TEST_H

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

