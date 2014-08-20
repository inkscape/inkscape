#include <cxxtest/TestSuite.h>
#include "streq.h"

/* Initial author: Peter Moulder.
   Hereby released into the Public Domain. */

#include <cstring>
#include <functional>

#include "quote.h"

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
            char* str = xml_quote_strdup(cases[i].s1);
            TS_ASSERT_RELATION( streq_rel, cases[i].s2, str );
            g_free(str);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
