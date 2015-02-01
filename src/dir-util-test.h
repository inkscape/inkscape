
#ifndef SEEN_DIR_UTIL_TEST_H
#define SEEN_DIR_UTIL_TEST_H

#include <cxxtest/TestSuite.h>

#include "dir-util.h"

class DirUtilTest : public CxxTest::TestSuite
{
public:
    void testBase()
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
                TS_ASSERT( !result.empty() );
                if ( !result.empty() )
                {
                    TS_ASSERT_EQUALS( result, std::string(cases[i][2]) );
                }
            }
        }
    }

};

#endif // SEEN_DIR_UTIL_TEST_H

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
