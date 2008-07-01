#include <cxxtest/TestSuite.h>

/* Initial author: Peter Moulder.
   Hereby released into the Public Domain. */

#include <cstring>
#include <functional>

/* mental disclaims all responsibility for this evil idea for testing
   static functions.  The main disadvantages are that we retain any
   #define's and `using' directives of the included file. */
#include "quote.cpp"

struct streq_free2 {
    bool operator()(char const *exp, char *got) const
    {
        bool const ret = (strcmp(exp, got) == 0);
        g_free(got);
        return ret;
    }
};

class XmlQuoteTest : public CxxTest::TestSuite
{
public:

    XmlQuoteTest()
    {
    }
    virtual ~XmlQuoteTest() {}

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static XmlQuoteTest *createSuite() { return new XmlQuoteTest(); }
    static void destroySuite( XmlQuoteTest *suite ) { delete suite; }

    void testXmlQuotedStrlen()
    {
        struct {
            char const *s;
            size_t len;
        } cases[] = {
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
        for(size_t i=0; i<G_N_ELEMENTS(cases); i++) {
            TS_ASSERT_EQUALS( xml_quoted_strlen(cases[i].s) , cases[i].len );
        }
    }

    void testXmlQuoteStrdup()
    {
        struct {
            char const * s1;
            char const * s2;
        } cases[] = {
            {"", ""},
            {"x", "x"},
            {"Foo", "Foo"},
            {"\"", "&quot;"},
            {"&", "&amp;"},
            {"<", "&lt;"},
            {">", "&gt;"},
            {"a\"b<c>d;!@#$%^*(\\)?", "a&quot;b&lt;c&gt;d;!@#$%^*(\\)?"}
        };
        for(size_t i=0; i<G_N_ELEMENTS(cases); i++) {
            TS_ASSERT_RELATION( streq_free2, cases[i].s2, xml_quote_strdup(cases[i].s1) );
        }
    }
};

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
