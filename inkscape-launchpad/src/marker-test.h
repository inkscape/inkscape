/** @file
 * @brief Unit tests for SVG marker handling
 */
/* Authors:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * This file is released into the public domain.
 */

#include <cxxtest/TestSuite.h>

#include "sp-marker-loc.h"

class MarkerTest : public CxxTest::TestSuite
{
public:

    void testMarkerLoc()
    {
        // code depends on these *exact* values, so check them here.
        TS_ASSERT_EQUALS(SP_MARKER_LOC, 0);
        TS_ASSERT_EQUALS(SP_MARKER_LOC_START, 1);
        TS_ASSERT_EQUALS(SP_MARKER_LOC_MID, 2);
        TS_ASSERT_EQUALS(SP_MARKER_LOC_END, 3);
        TS_ASSERT_EQUALS(SP_MARKER_LOC_QTY, 4);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
