/*
 * Unit test main.
 *
 * Author:
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2015 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "gtest/gtest.h"

namespace {

// Ensure that a known positive test works
TEST(PreTest, WorldIsSane)
{
    EXPECT_EQ(4, 2 + 2);
}

// Example of type casting to avoid compile warnings.


} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
