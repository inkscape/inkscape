#include <utest/utest.h>
#include "extract-uri.h"
#include <string.h>
#include <glib.h>

struct Case {
    char const *input;
    char const *exp;
};

static void test_extract_uri_case(Case const &c)
{
    char * const p = extract_uri(c.input);
    UTEST_TEST(c.input) {
        UTEST_ASSERT( ( p == NULL ) == ( c.exp == NULL ) );
        if (p) {
            UTEST_ASSERT( strcmp(p, c.exp) == 0 );
        }
    }
    g_free(p);
}

int main(int argc, char *argv[])
{
    utest_start("extract_uri");

    Case const cases[] = {
        { "url(#foo)", "#foo" },
        { "url  foo  ", "foo" },
        { "url", NULL },
        { "url ", NULL },
        { "url()", NULL },
        { "url ( ) ", NULL },
        { "url foo bar ", "foo bar" }
    };

    for(unsigned i = 0; i < G_N_ELEMENTS(cases); ++i) {
        Case const &c = cases[i];
        test_extract_uri_case(c);
    }

    return ( utest_end()
             ? EXIT_SUCCESS
             : EXIT_FAILURE );
}

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
