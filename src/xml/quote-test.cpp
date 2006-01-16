/* Initial author: Peter Moulder.
   Hereby released into the Public Domain. */

#include <functional>
#include "utest/test-1ary-cases.h"

/* mental disclaims all responsibility for this evil idea for testing
   static functions.  The main disadvantages are that we retain any
   #define's and `using' directives of the included file. */
#include "quote.cpp"

struct streq_free2 {
    bool operator()(char const *exp, char *got)
    {
        bool const ret = (strcmp(exp, got) == 0);
        g_free(got);
        return ret;
    }
};

static bool
test_xml_quoted_strlen()
{
    utest_start("xml_quoted_strlen");
    struct Case1<char const *, size_t> cases[] = {
        {"", 0},
        {"x", 1},
        {"Foo", 3},
        {"\"", 6},
        {"&", 5},
        {"<", 4},
        {">", 4},
        {"a\"b", 8},
        {"a\"b<c>d;!@#$%^*(\\)?", 30}
    };
    test_1ary_cases<size_t, char const *, size_t, std::equal_to<size_t> >("xml_quoted_strlen",
                                                                          xml_quoted_strlen,
                                                                          G_N_ELEMENTS(cases),
                                                                          cases);
    return utest_end();
}

static bool
test_xml_quote_strdup()
{
    utest_start("xml_quote_strdup");
    struct Case1<char const *, char const *> cases[] = {
        {"", ""},
        {"x", "x"},
        {"Foo", "Foo"},
        {"\"", "&quot;"},
        {"&", "&amp;"},
        {"<", "&lt;"},
        {">", "&gt;"},
        {"a\"b<c>d;!@#$%^*(\\)?", "a&quot;b&lt;c&gt;d;!@#$%^*(\\)?"}
    };
    test_1ary_cases<char *, char const *, char const *, streq_free2>("xml_quote_strdup",
                                                                     xml_quote_strdup,
                                                                     G_N_ELEMENTS(cases),
                                                                     cases);
    return utest_end();
}

int main() {
    bool const succ = (test_xml_quoted_strlen()
                       && test_xml_quote_strdup());
    return ( succ
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
