
#ifndef SEEN_STYLE_TEST_H
#define SEEN_STYLE_TEST_H

#include <cxxtest/TestSuite.h>

#include "test-helpers.h"

#include "style.h"

class StyleTest : public CxxTest::TestSuite
{
public:
    SPDocument* _doc;

    StyleTest() :
        _doc(0)
    {
    }

    virtual ~StyleTest()
    {
        if ( _doc )
        {
            sp_document_unref( _doc );
            _doc = 0;
        }
    }

    static void createSuiteSubclass( StyleTest*& dst )
    {
        dst = new StyleTest();
    }

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static StyleTest *createSuite()
    {
        StyleTest* suite = Inkscape::createSuiteAndDocument<StyleTest>( createSuiteSubclass );
        return suite;
    }

    static void destroySuite( StyleTest *suite )
    {
        delete suite;
    }

    // ---------------------------------------------------------------
    // ---------------------------------------------------------------
    // ---------------------------------------------------------------

    void testOne()
    {
        struct TestCase {
            TestCase(gchar const* src, gchar const* dst = 0, gchar const* uri = 0) : src(src), dst(dst), uri(uri) {}
            gchar const* src;
            gchar const* dst;
            gchar const* uri;
        };

        TestCase cases[] = {
            TestCase("fill:none"),
            TestCase("fill:currentColor"),
            TestCase("fill:#ff00ff"),

            TestCase("fill:rgb(100%, 0%, 100%)", "fill:#ff00ff"),
            // TODO - fix this to preserve the string
            TestCase("fill:url(#painter) rgb(100%, 0%, 100%)",
                     "fill:url(#painter) #ff00ff", "#painter"),

            TestCase("fill:rgb(255, 0, 255)", "fill:#ff00ff"),
            // TODO - fix this to preserve the string
            TestCase("fill:url(#painter) rgb(255, 0, 255)",
                     "fill:url(#painter) #ff00ff", "#painter"),


//             TestCase("fill:#ff00ff icc-color(colorChange, 0.1, 0.5, 0.1)"),

            TestCase("fill:url(#painter)", 0, "#painter"),
            TestCase("fill:url(#painter) none", 0, "#painter"),
            TestCase("fill:url(#painter) currentColor", 0, "#painter"),
            TestCase("fill:url(#painter) #ff00ff", 0, "#painter"),
//             TestCase("fill:url(#painter) rgb(100%, 0%, 100%)", 0, "#painter"),
//             TestCase("fill:url(#painter) rgb(255, 0, 255)", 0, "#painter"),

            TestCase("fill:url(#painter) #ff00ff icc-color(colorChange, 0.1, 0.5, 0.1)", 0, "#painter"),

//             TestCase("fill:url(#painter) inherit", 0, "#painter"),
            TestCase("fill:inherit"),
            TestCase(0)
        };

        for ( gint i = 0; cases[i].src; i++ ) {
            SPStyle *style = sp_style_new(_doc);
            TS_ASSERT(style);
            if ( style ) {
                sp_style_merge_from_style_string( style, cases[i].src );

                if ( cases[i].uri ) {
                    TSM_ASSERT( cases[i].src, style->fill.value.href );
                    if ( style->fill.value.href ) {
                        TS_ASSERT_EQUALS( style->fill.value.href->getURI()->toString(), std::string(cases[i].uri) );
                    }
                } else {
                    TS_ASSERT( !style->fill.value.href || !style->fill.value.href->getObject() );
                }

                gchar *str0_set = sp_style_write_string( style, SP_STYLE_FLAG_IFSET );
                //printf("<<%s>>\n", str0_set);
                if ( cases[i].dst ) {
                    TS_ASSERT_EQUALS( std::string(str0_set), std::string(cases[i].dst) );
                } else {
                    TS_ASSERT_EQUALS( std::string(str0_set), std::string(cases[i].src) );
                }

                g_free(str0_set);
                sp_style_unref(style);
            }
        }
    }

};


#endif // SEEN_STYLE_TEST_H

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
