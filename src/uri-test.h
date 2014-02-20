
#ifndef SEEN_URI_TEST_H
#define SEEN_URI_TEST_H
/*
 * Test uri.h
 *
 * Written to aid with refactoring the uri handling to support fullPath
 * and data URIs and also cover code which wasn't before tested.
 *
 * Copyright 2014 (c) BasisTech Boston
 *
 */
#include <cxxtest/TestSuite.h>

#include "uri.h"

using Inkscape::URI;

class URITest : public CxxTest::TestSuite
{
public:
    void stringTest( std::string result, std::string expected )
    {
        if ( !result.empty() && !expected.empty() ) {
            TS_ASSERT_EQUALS( result, expected );
        } else if ( result.empty() && !expected.empty() ) {
            TS_FAIL( std::string("Expected (") + expected + "), found null" );
        } else if ( !result.empty() && expected.empty() ) {
            TS_FAIL( std::string("Expected null, found (") + result + ")" );
        }
    }

    std::string ValueOrEmpty(const char* s) {
        return s == NULL ? std::string() : s;
    }

    void toStringTest( std::string uri, std::string expected ) {
        stringTest( URI(uri.c_str()).toString(), expected );
    }
    void pathTest( std::string uri, std::string expected ) {
        stringTest( ValueOrEmpty(URI(uri.c_str()).getPath()), expected );
    }

    void testToString()
    {
        char const* cases[][2] = {
            { "foo", "foo" },
            { "#foo", "#foo" },
            { "blah.svg#h", "blah.svg#h" },
            //{ "data:data", "data:data" },
        };

        for ( size_t i = 0; i < G_N_ELEMENTS(cases); i++ ) {
            toStringTest( std::string(cases[i][0]), std::string(cases[i][1]) );
        }
    }

    void testPath()
    {
        char const* cases[][2] = {
            { "foo.svg",     "foo.svg" },
            { "foo.svg#bar", "foo.svg" },
            { "#bar",        NULL },
            { "data:data",   NULL },
        };

        for ( size_t i = 0; i < G_N_ELEMENTS(cases); i++ ) {
            pathTest( ValueOrEmpty(cases[i][0]), ValueOrEmpty(cases[i][1]) );
        }
    }
    void testFullPath() {
        std::ofstream fhl("/tmp/cxxtest-uri.svg", std::ofstream::out);
        stringTest( URI("cxxtest-uri.svg").getFullPath("/tmp"), std::string("/tmp/cxxtest-uri.svg") );
        stringTest( URI("cxxtest-uri.svg").getFullPath("/usr/../tmp"), std::string("/tmp/cxxtest-uri.svg") );
    }

};

#endif // SEEN_URI_TEST_H

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
