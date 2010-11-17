
#ifndef SEEN_EXTRACT_URI_TEST_H
#define SEEN_EXTRACT_URI_TEST_H

#include <cxxtest/TestSuite.h>

#include "extract-uri.h"

class ExtractURITest : public CxxTest::TestSuite
{
public:
    void checkOne( char const* str, char const* expected )
    {
        gchar* result = extract_uri( str );
        TS_ASSERT_EQUALS( ( result == NULL ), ( expected == NULL ) );
        if ( result && expected ) {
            TS_ASSERT_EQUALS( std::string(result), std::string(expected) );
        } else if ( result ) {
            TS_FAIL( std::string("Expected null, found (") + result + ")" );
        } else if ( expected ) {
            TS_FAIL( std::string("Expected (") + expected + "), found null" );
        }
        g_free( result );
    }

    void testBase()
    {
        char const* cases[][2] = {
            { "url(#foo)", "#foo" },
            { "url  foo  ", NULL },
            { "url", NULL },
            { "url ", NULL },
            { "url()", NULL },
            { "url ( ) ", NULL },
            { "url foo bar ", NULL },
        };

        for ( size_t i = 0; i < G_N_ELEMENTS(cases); i++ )
        {
            checkOne( cases[i][0], cases[i][1] );
        }
    }

    void testWithTrailing()
    {
        char const* cases[][2] = {
            { "url(#foo) bar", "#foo" },
            { "url() bar", NULL },
            { "url ( ) bar ", NULL }
        };

        for ( size_t i = 0; i < G_N_ELEMENTS(cases); i++ )
        {
            checkOne( cases[i][0], cases[i][1] );
        }
    }

    void testQuoted()
    {
        char const* cases[][2] = {
            { "url('#foo')", "#foo" },
            { "url(\"#foo\")", "#foo" },
            { "url('#f o o')", "#f o o" },
            { "url(\"#f o o\")", "#f o o" },
            { "url('#fo\"o')", "#fo\"o" },
            { "url(\"#fo'o\")", "#fo'o" },
        };

        for ( size_t i = 0; i < G_N_ELEMENTS(cases); i++ )
        {
            checkOne( cases[i][0], cases[i][1] );
        }
    }

};

#endif // SEEN_EXTRACT_URI_TEST_H

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
