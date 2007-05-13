
#ifndef SEEN_STYLE_TEST_H
#define SEEN_STYLE_TEST_H

#include <cxxtest/TestSuite.h>

#include "style.h"

class StyleTest : public CxxTest::TestSuite
{
public:

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
            TestCase("fill:rgb(255, 0, 255)", "fill:#ff00ff"),

//             TestCase("fill:#ff00ff icc-color(colorChange, 0.1, 0.5, 0.1)"),

            TestCase("fill:url(#painter)", 0, "#painter"),
//             TestCase("fill:url(#painter) none", 0, "#painter"),
//             TestCase("fill:url(#painter) currentColor", 0, "#painter"),
//             TestCase("fill:url(#painter) #ff00ff", 0, "#painter"),
//             TestCase("fill:url(#painter) rgb(100%, 0%, 100%)", 0, "#painter"),
//             TestCase("fill:url(#painter) rgb(255, 0, 255)", 0, "#painter"),

//             TestCase("fill:url(#painter) #ff00ff icc-color(colorChange, 0.1, 0.5, 0.1)",
//             "fill:url(#painter) #ff00ff icc-color(colorChange, 0.10000000000000001, 0.50000000000000000, 0.10000000000000001)", "#painter"),

//             TestCase("fill:url(#painter) inherit", 0, "#painter"),
            TestCase("fill:inherit"),
            TestCase(0)
        };

        for ( gint i = 0; cases[i].src; i++ ) {
            SPStyle *style = sp_style_new();
            TS_ASSERT(style);
            if ( style ) {
                sp_style_merge_from_style_string( style, cases[i].src );

                if ( cases[i].uri ) {
                    TS_ASSERT( style->fill.value.paint.uri );
                    if ( style->fill.value.paint.uri ) {
                        TS_ASSERT_EQUALS( std::string(style->fill.value.paint.uri), std::string(cases[i].uri) );
                    }
                } else {
                    TS_ASSERT( !style->fill.value.paint.uri );
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
