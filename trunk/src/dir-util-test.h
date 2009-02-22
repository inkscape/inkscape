
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
            {"/foo/bar", "/foo", "bar"},
            {"/foo/barney", "/foo/bar", "/foo/barney"},
            {"/foo/bar/baz", "/foo/", "bar/baz"},
            {"/foo/bar/baz", "/", "foo/bar/baz"},
            {"/foo/bar/baz", "/foo/qux", "/foo/bar/baz"},
            {"/foo", NULL, "/foo"}
        };

        for ( size_t i = 0; i < G_N_ELEMENTS(cases); i++ )
        {
            char const* result = sp_relative_path_from_path( cases[i][0], cases[i][1] );
            TS_ASSERT( result );
            TS_ASSERT( cases[i][2] );
            if ( result && cases[i][2] )
            {
                TS_ASSERT_EQUALS( std::string(result), std::string(cases[i][2]) );
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
