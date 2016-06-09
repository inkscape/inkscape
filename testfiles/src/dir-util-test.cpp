/*
 * Unit tests for dir utils.
 *
 * Author:
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2015 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "gtest/gtest.h"

#include <glib.h>

#include "dir-util.h"

namespace {


TEST(DirUtilTest, Base)
{
    char const* cases[][3] = {
#if defined(WIN32) || defined(__WIN32__)
        {"\\foo\\bar", "\\foo", "bar"},
        {"\\foo\\barney", "\\foo\\bar", "\\foo\\barney"},
        {"\\foo\\bar\\baz", "\\foo\\", "bar\\baz"},
        {"\\foo\\bar\\baz", "\\", "foo\\bar\\baz"},
        {"\\foo\\bar\\baz", "\\foo\\qux", "\\foo\\bar\\baz"},
#else
        {"/foo/bar", "/foo", "bar"},
        {"/foo/barney", "/foo/bar", "/foo/barney"},
        {"/foo/bar/baz", "/foo/", "bar/baz"},
        {"/foo/bar/baz", "/", "foo/bar/baz"},
        {"/foo/bar/baz", "/foo/qux", "/foo/bar/baz"},
#endif
    };

    for ( size_t i = 0; i < G_N_ELEMENTS(cases); i++ )
    {
        if ( cases[i][0] && cases[i][1] ) { // std::string can't use null.
            std::string  result = sp_relative_path_from_path( cases[i][0], cases[i][1] );
            ASSERT_FALSE( result.empty() );
            if ( !result.empty() )
            {
                ASSERT_EQ( std::string(cases[i][2]), result );
            }
        }
    }
}

} // namespace

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
