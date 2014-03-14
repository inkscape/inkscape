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
            _doc->doUnref();
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

// General tests (in general order of appearance in sp_style_read), SPIPaint tested above
            TestCase("visibility:hidden"),                       // SPIEnum
            TestCase("visibility:collapse"),
            TestCase("visibility:visible"),
            TestCase("display:none"),                            // SPIEnum
            TestCase("overflow:visible"),                        // SPIEnum
            TestCase("overflow:auto"),                           // SPIEnum

            // Not directly read
            TestCase("font:bold 12px Arial",
                     "font-size:12px;font-style:normal;font-variant:normal;font-weight:bold;font-family:Arial"),
            // line-height not read in
            //TestCase("font:bold 12px/24px 'Times New Roman'",
            //         "font-size:12px;font-style:normal;font-variant:normal;font-weight:bold;line-height:24px;font-family:Times New Roman"),
            TestCase("font-family:sans-serif"),                  // SPIString, text_private
            TestCase("font-family:Arial"),
            TestCase("font-variant:normal;font-stretch:normal;-inkscape-font-specification:Nimbus Roman No9 L Bold Italic"),
            // Needs to be fixed (quotes should be around each font-family):
            TestCase("font-family:Georgia, 'Minion Web'","font-family:'Georgia, \"Minion Web\"'"),
            TestCase("font-size:12",     "font-size:12px"),      // SPIFontSize
            TestCase("font-size:12px"),
            TestCase("font-size:12pt",   "font-size:15px"),
            TestCase("font-size:medium"),
            TestCase("font-size:smaller"),
            TestCase("font-style:italic"),                       // SPIEnum
            TestCase("font-variant:small-caps"),                 // SPIEnum
            TestCase("font-weight:100"),                         // SPIEnum
            TestCase("font-weight:normal"),
            TestCase("font-weight:bolder"),
            TestCase("font-stretch:condensed"),                  // SPIEnum

            // Should be moved down
            TestCase("text-indent:12em"),                        // SPILength?
            TestCase("text-align:center"),                       // SPIEnum
            TestCase("text-decoration: underline"),              // SPITextDecoration
            TestCase("text-decoration: underline wavy #0000ff"), // SPITextDecoration CSS3
            TestCase("text-decoration: overline double #ff0000"),

            // Should be moved up
            TestCase("line-height:24px"),                        // SPILengthOrNormal
            TestCase("line-height:1.5"),
            TestCase("letter-spacing:2px"),                      // SPILengthOrNormal
            TestCase("word-spacing:2px"),                        // SPILengthOrNormal
            TestCase("word-spacing:normal"),
            TestCase("text-transform:lowercase"),                // SPIEnum
            // ...
            TestCase("baseline-shift:baseline"),                 // SPIBaselineShift
            TestCase("baseline-shift:sub"),
            TestCase("baseline-shift:12.5%"),
            TestCase("baseline-shift:2px"),

            TestCase("opacity:0.1"),                             // SPIScale24
            // ...
            TestCase("stroke-width:2px"),                        // SPILength
            TestCase("stroke-linecap:round"),                    // SPIEnum
            TestCase("stroke-linejoin:round"),                   // SPIEnum
            TestCase("stroke-miterlimit:4"),                     // SPIFloat
            TestCase("marker:url(#Arrow)"),                      // SPIString
            TestCase("marker-start:url(#Arrow)"),
            TestCase("marker-mid:url(#Arrow)"),
            TestCase("marker-end:url(#Arrow)"),
            TestCase("stroke-opacity:0.5"),                      // SPIScale24
            TestCase("stroke-dasharray:0, 1, 0, 1"),             // SPIDashArray
            TestCase("stroke-dasharray:0 1 0 1","stroke-dasharray:0, 1, 0, 1"),
            TestCase("stroke-dasharray:0  1  2  3","stroke-dasharray:0, 1, 2, 3"),
            TestCase("stroke-dashoffset:13"),                    // SPILength
            TestCase("stroke-dashoffset:10px"),
            // ...
            //TestCase("filter:url(#myfilter)"),                   // SPIFilter segfault in read
            TestCase("filter:inherit"),

            TestCase("opacity:0.1;fill:#ff0000;stroke:#0000ff;stroke-width:2px"),
            TestCase("opacity:0.1;fill:#ff0000;stroke:#0000ff;stroke-width:2px;stroke-dasharray:1, 2, 3, 4;stroke-dashoffset:15"),


#ifdef WITH_SVG2
            TestCase("paint-order:stroke"),                      // SPIPaintOrder
            TestCase("paint-order:normal"),
            TestCase("paint-order: markers stroke   fill", "paint-order:markers stroke fill"),
#endif
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
                    //std::cout << "  " << std::string(str0_set) << " " << std::string(cases[i].dst) << std::endl;
                    TS_ASSERT_EQUALS( std::string(str0_set), std::string(cases[i].dst) );
                } else {
                    //std::cout << "  " << std::string(str0_set) << " " << std::string(cases[i].src) << std::endl;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
