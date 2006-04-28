
#ifndef SEEN_EXTRACT_URI_TEST_H
#define SEEN_EXTRACT_URI_TEST_H

#include <cxxtest/TestSuite.h>

#include "extract-uri.h"

class ExtractURITest : public CxxTest::TestSuite
{
public:
    void testBase()
    {
        char const* cases[][2] = {
            { "url(#foo)", "#foo" },
            { "url  foo  ", "foo" },
            { "url", NULL },
            { "url ", NULL },
            { "url()", NULL },
            { "url ( ) ", NULL },
            { "url foo bar ", "foo bar" }
        };

        for ( size_t i = 0; i < G_N_ELEMENTS(cases); i++ )
        {
            char const* result = extract_uri( cases[i][0] );

            TS_ASSERT_EQUALS( ( result == NULL ), ( cases[i][1] == NULL ) );
            if ( result )
            {
                TS_ASSERT_EQUALS( std::string(result), std::string(cases[i][1]) );
            }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
