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

#include <gtkmm.h>

#include "inkgc/gc-core.h"
#include "inkscape.h"

namespace {

// Ensure that a known positive test works
TEST(PreTest, WorldIsSane)
{
    EXPECT_EQ(4, 2 + 2);
}

// Example of type casting to avoid compile warnings.


} // namespace

int main(int argc, char **argv) {

    // setup general environment
#if !GLIB_CHECK_VERSION(2,36,0)
    g_type_init();
#endif

    // If possible, unit tests shouldn't require a GUI session
    // since this won't generally be available in auto-builders
    // int tmpArgc = 1;
    // char const *tmp[] = {"foo", ""};
    // char **tmpArgv = const_cast<char **>(tmp);
    // Gtk::Main(tmpArgc, tmpArgv);

    Inkscape::GC::init();

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
