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

    // Reading and writing style string
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

            TestCase("color:#ff0000"),
            TestCase("color:blue",             "color:#0000ff"),
            // TestCase("color:currentColor"),  SVG 1.1 does not allow color value 'currentColor'

            // Font shorthand
            TestCase("font:bold 12px Arial",
                     "font-style:normal;font-variant:normal;font-weight:bold;font-stretch:normal;font-size:12px;line-height:normal;font-family:Arial"),
            TestCase("font:bold 12px/24px 'Times New Roman'",
                     "font-style:normal;font-variant:normal;font-weight:bold;font-stretch:normal;font-size:12px;line-height:24px;font-family:\'Times New Roman\'"),
            // From CSS 3 Fonts (examples):
            TestCase("font: 12pt/15pt sans-serif",
                     "font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-size:16px;line-height:15pt;font-family:sans-serif"),
            TestCase("font: 80% sans-serif",
                     "font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-size:80%;line-height:normal;font-family:sans-serif"),
            TestCase("font: x-large/110% 'new century schoolbook', serif",
                     "font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-size:x-large;line-height:110%;font-family:\'new century schoolbook\', serif"),
            TestCase("font: bold italic large Palatino, serif",
                     "font-style:italic;font-variant:normal;font-weight:bold;font-stretch:normal;font-size:large;line-height:normal;font-family:Palatino, serif"),
            TestCase("font: normal small-caps 120%/120% fantasy",
                     "font-style:normal;font-variant:small-caps;font-weight:normal;font-stretch:normal;font-size:120%;line-height:120%;font-family:fantasy"),
            TestCase("font: condensed oblique 12pt 'Helvetica Neue', serif;",
                     "font-style:oblique;font-variant:normal;font-weight:normal;font-stretch:condensed;font-size:16px;line-height:normal;font-family:\'Helvetica Neue\', serif"),

            TestCase("font-family:sans-serif"),                  // SPIString, text_private
            TestCase("font-family:Arial"),
            // TestCase("font-variant:normal;font-stretch:normal;-inkscape-font-specification:Nimbus Roman No9 L Bold Italic"),

            // Needs to be fixed (quotes should be around each font-family):
            TestCase("font-family:Georgia, 'Minion Web'","font-family:Georgia, \'Minion Web\'"),
            TestCase("font-size:12",     "font-size:12px"),      // SPIFontSize
            TestCase("font-size:12px"),
            TestCase("font-size:12pt",   "font-size:16px"),
            TestCase("font-size:medium"),
            TestCase("font-size:smaller"),
            TestCase("font-style:italic"),                       // SPIEnum
            TestCase("font-variant:small-caps"),                 // SPIEnum
            TestCase("font-weight:100"),                         // SPIEnum
            TestCase("font-weight:normal"),
            TestCase("font-weight:bolder"),
            TestCase("font-stretch:condensed"),                  // SPIEnum

            TestCase("font-variant-ligatures:none"),             // SPILigatures
            TestCase("font-variant-ligatures:normal"),
            TestCase("font-variant-ligatures:no-common-ligatures"),
            TestCase("font-variant-ligatures:discretionary-ligatures"),
            TestCase("font-variant-ligatures:historical-ligatures"),
            TestCase("font-variant-ligatures:no-contextual"),
            TestCase("font-variant-ligatures:common-ligatures", "font-variant-ligatures:normal"),
            TestCase("font-variant-ligatures:contextual", "font-variant-ligatures:normal"),
            TestCase("font-variant-ligatures:no-common-ligatures historical-ligatures"),
            TestCase("font-variant-ligatures:historical-ligatures no-contextual"),
            TestCase("font-variant-position:normal"),
            TestCase("font-variant-position:sub"),
            TestCase("font-variant-position:super"),
            TestCase("font-variant-caps:normal"),
            TestCase("font-variant-caps:small-caps"),
            TestCase("font-variant-caps:all-small-caps"),
            TestCase("font-variant-numeric:normal"),
            TestCase("font-variant-numeric:lining-nums"),
            TestCase("font-variant-numeric:oldstyle-nums"),
            TestCase("font-variant-numeric:proportional-nums"),
            TestCase("font-variant-numeric:tabular-nums"),
            TestCase("font-variant-numeric:diagonal-fractions"),
            TestCase("font-variant-numeric:stacked-fractions"),
            TestCase("font-variant-numeric:ordinal"),
            TestCase("font-variant-numeric:slashed-zero"),
            TestCase("font-variant-numeric:tabular-nums slashed-zero"),
            TestCase("font-variant-numeric:tabular-nums proportional-nums", "font-variant-numeric:proportional-nums"),

            // Should be moved down
            TestCase("text-indent:12em"),                        // SPILength?
            TestCase("text-align:center"),                       // SPIEnum

            // SPITextDecoration
            // The default value for 'text-decoration-color' is 'currentColor', but
            // we cannot set the default to that value yet. (We need to switch
            // SPIPaint to SPIColor and then add the ability to set default.)
            // TestCase("text-decoration: underline",
            //          "text-decoration: underline;text-decoration-line: underline;text-decoration-color:currentColor"),
            // TestCase("text-decoration: overline underline",
            //          "text-decoration: underline overline;text-decoration-line: underline overline;text-decoration-color:currentColor"),

            TestCase("text-decoration: underline wavy #0000ff",
                     "text-decoration: underline;text-decoration-line: underline;text-decoration-style:wavy;text-decoration-color:#0000ff"),
            TestCase("text-decoration: double overline underline #ff0000",
                     "text-decoration: underline overline;text-decoration-line: underline overline;text-decoration-style:double;text-decoration-color:#ff0000"),

            // SPITextDecorationLine
            TestCase("text-decoration-line: underline",
                     "text-decoration: underline;text-decoration-line: underline"),

            // SPITextDecorationStyle
            TestCase("text-decoration-style:solid"),
            TestCase("text-decoration-style:dotted"),

            // SPITextDecorationColor
            TestCase("text-decoration-color:#ff00ff"),

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
            // std::cout << "Test one: " << i << std::endl;
            SPStyle style(_doc);
            style.mergeString( cases[i].src );
            if ( cases[i].uri ) {
                TSM_ASSERT( cases[i].src, style.fill.value.href );
                if ( style.fill.value.href ) {
                    TS_ASSERT_EQUALS( style.fill.value.href->getURI()->toString(), std::string(cases[i].uri) );
                }
            } else {
                TS_ASSERT( !style.fill.value.href || !style.fill.value.href->getObject() );
            }

            std::string str0_set = style.write(SP_STYLE_FLAG_IFSET );

            if ( cases[i].dst ) {
                // std::cout << "  " << str0_set << " " << std::string(cases[i].dst) << std::endl;
                TS_ASSERT_EQUALS( str0_set, std::string(cases[i].dst) );
            } else {
                // std::cout << "  " << str0_set << " " << std::string(cases[i].src) << std::endl;
                TS_ASSERT_EQUALS( str0_set, std::string(cases[i].src) );
            }
        }
    }

    // Testing operator==
    void testTwo()
    {
        struct TestCase {
            TestCase(gchar const* src, gchar const* dst, bool match) :
                src(src), dst(dst), match(match) {}
            gchar const* src;
            gchar const* dst;
            bool         match;
        };

        TestCase cases[] = {

            // SPIFloat
            TestCase("stroke-miterlimit:4",     "stroke-miterlimit:4",      true ),
            TestCase("stroke-miterlimit:4",     "stroke-miterlimit:2",      false),
            TestCase("stroke-miterlimit:4",     "",                         true ), // Default

            // SPIScale24
            TestCase("opacity:0.3",             "opacity:0.3",              true ),
            TestCase("opacity:0.3",             "opacity:0.6",              false),
            TestCase("opacity:1.0",             "",                         true ), // Default

            // SPILength
            TestCase("text-indent:3",           "text-indent:3",            true ),
            TestCase("text-indent:6",           "text-indent:3",            false),
            TestCase("text-indent:6px",         "text-indent:3",            false),
            TestCase("text-indent:1px",         "text-indent:12pc",         false),
            TestCase("text-indent:2ex",         "text-indent:2ex",          false),

            // SPILengthOrNormal
            TestCase("letter-spacing:normal",   "letter-spacing:normal",    true ),
            TestCase("letter-spacing:2",        "letter-spacing:normal",    false),
            TestCase("letter-spacing:normal",   "letter-spacing:2",         false),
            TestCase("letter-spacing:5px",      "letter-spacing:5px",       true ),
            TestCase("letter-spacing:10px",     "letter-spacing:5px",       false),
            TestCase("letter-spacing:10em",     "letter-spacing:10em",      false),

            // SPIEnum
            TestCase("text-anchor:start",       "text-anchor:start",        true ),
            TestCase("text-anchor:start",       "text-anchor:middle",       false),
            TestCase("text-anchor:start",       "",                         true ), // Default
            TestCase("text-anchor:start",       "text-anchor:junk",         true ), // Bad value

            TestCase("font-weight:normal",      "font-weight:400",          true ),
            TestCase("font-weight:bold",        "font-weight:700",          true ),


            // SPIString and SPIFontString
            TestCase("font-family:Arial",       "font-family:Arial",        true ),
            TestCase("font-family:A B",         "font-family:A B",          true ),
            TestCase("font-family:A B",         "font-family:A C",          false),
            // Default is not set by class... value is NULL which cannot be compared
            // TestCase("font-family:sans-serif",  "",                       true ), // Default

            // SPIColor
            TestCase("color:blue",              "color:blue",               true ),
            TestCase("color:blue",              "color:red",                false),
            TestCase("color:red",               "color:#ff0000",            true ),

            // SPIPaint
            TestCase("fill:blue",               "fill:blue",               true ),
            TestCase("fill:blue",               "fill:red",                false),
            TestCase("fill:currentColor",       "fill:currentColor",       true ),
            TestCase("fill:url(#xxx)",          "fill:url(#xxx)",          true ),
            // Needs URL defined as in test 1
            //TestCase("fill:url(#xxx)",          "fill:url(#yyy)",          false),

            // SPIPaintOrder
            TestCase("paint-order:markers",     "paint-order:markers",     true ),
            TestCase("paint-order:markers",     "paint-order:stroke",      false),
            //TestCase("paint-order:fill stroke markers", "",                true ), // Default
            TestCase("paint-order:normal",      "paint-order:normal",      true ),
            //TestCase("paint-order:fill stroke markers", "paint-order:normal", true ),

            // SPIDashArray
            TestCase("stroke-dasharray:0 1 2 3","stroke-dasharray:0 1 2 3",true ),
            TestCase("stroke-dasharray:0 1",    "stroke-dasharray:0 2",    false),

            // SPIFilter

            // SPIFontSize
            TestCase("font-size:12px",          "font-size:12px",          true ),
            TestCase("font-size:12px",          "font-size:24px",          false),
            TestCase("font-size:12ex",          "font-size:24ex",          false),
            TestCase("font-size:medium",        "font-size:medium",        true ),
            TestCase("font-size:medium",        "font-size:large",         false),

            // SPIBaselineShift
            TestCase("baseline-shift:baseline", "baseline-shift:baseline", true ),
            TestCase("baseline-shift:sub",      "baseline-shift:sub",      true ),
            TestCase("baseline-shift:sub",      "baseline-shift:super",    false),
            TestCase("baseline-shift:baseline", "baseline-shift:sub",      false),
            TestCase("baseline-shift:10px",     "baseline-shift:10px",     true ),
            TestCase("baseline-shift:10px",     "baseline-shift:12px",     false),


            // SPITextDecorationLine
            TestCase("text-decoration-line:underline", "text-decoration-line:underline", true ),
            TestCase("text-decoration-line:underline", "text-decoration-line:overline",  false),
            TestCase("text-decoration-line:underline overline", "text-decoration-line:underline overline", true ),
            TestCase("text-decoration-line:none",      "", true ), // Default


            // SPITextDecorationStyle
            TestCase("text-decoration-style:solid",    "text-decoration-style:solid", true ),
            TestCase("text-decoration-style:dotted",   "text-decoration-style:solid", false),
            TestCase("text-decoration-style:solid",    "",   true ), // Default

            // SPITextDecoration
            TestCase("text-decoration:underline",          "text-decoration:underline", true ),
            TestCase("text-decoration:underline",          "text-decoration:overline",  false),
            TestCase("text-decoration:underline overline","text-decoration:underline overline",true ),
            TestCase("text-decoration:overline underline","text-decoration:underline overline",true ),
            // TestCase("text-decoration:none",               "text-decoration-color:currentColor", true ), // Default


            // Terminate
            TestCase(0,0,0)
        };
        for ( gint i = 0; cases[i].src; i++ ) {
            // std::cout << "Test two: " << i << std::endl;
            SPStyle style_src(_doc);
            SPStyle style_dst(_doc);

            style_src.mergeString( cases[i].src );
            style_dst.mergeString( cases[i].dst );
            
            // std::cout << "Test:" << std::endl;
            // std::cout << "  C: |" << cases[i].src << "|   |" << cases[i].dst << "|" << std::endl;
            // std::cout << "  S: |" << style_src.write( SP_STYLE_FLAG_IFSET, NULL ) << "|   |"
            //           << style_dst.write( SP_STYLE_FLAG_IFSET, NULL ) << "|" <<std::endl;
            TS_ASSERT( (style_src == style_dst) == cases[i].match );
            // std::cout << "End Test\n" << std::endl;
        }
    }


    // Test of cascade
    void testThree()
    {
        struct TestCase {
            TestCase(gchar const* parent, gchar const* child, gchar const* result) :
                parent(parent), child(child), result(result) {}
            gchar const* parent;
            gchar const* child;
            gchar const* result;
        };

        TestCase cases[] = {

            // SPIFloat
            TestCase("stroke-miterlimit:6",   "stroke-miterlimit:2",   "stroke-miterlimit:2"    ),
            TestCase("stroke-miterlimit:6",   "",                      "stroke-miterlimit:6"    ),
            TestCase("",                      "stroke-miterlimit:2",   "stroke-miterlimit:2"    ),

            // SPIScale24
            TestCase("opacity:0.3",           "opacity:0.3",            "opacity:0.3"           ),
            TestCase("opacity:0.3",           "opacity:0.6",            "opacity:0.6"           ),
            // 'opacity' does not inherit
            TestCase("opacity:0.3",           "",                       "opacity:1.0"           ),
            TestCase("",                      "opacity:0.3",            "opacity:0.3"           ),
            TestCase("opacity:0.5",           "opacity:inherit",        "opacity:0.5"           ),
            TestCase("",                      "",                       "opacity:1.0"           ),

            // SPILength
            TestCase("text-indent:3",         "text-indent:3",          "text-indent:3"         ),
            TestCase("text-indent:6",         "text-indent:3",          "text-indent:3"         ),
            TestCase("text-indent:6px",       "text-indent:3",          "text-indent:3"         ),
            TestCase("text-indent:1px",       "text-indent:12pc",       "text-indent:12pc"      ),
            // ex, em cannot be equal
            //TestCase("text-indent:2ex",       "text-indent:2ex",        "text-indent:2ex"       ),
            TestCase("text-indent:3",         "",                       "text-indent:3"         ),
            TestCase("text-indent:3",         "text-indent:inherit",    "text-indent:3"         ),

            // SPILengthOrNormal
            TestCase("letter-spacing:normal", "letter-spacing:normal",  "letter-spacing:normal" ), 
            TestCase("letter-spacing:2",      "letter-spacing:normal",  "letter-spacing:normal" ), 
            TestCase("letter-spacing:normal", "letter-spacing:2",       "letter-spacing:2"      ),
            TestCase("letter-spacing:5px",    "letter-spacing:5px",     "letter-spacing:5px"    ),
            TestCase("letter-spacing:10px",   "letter-spacing:5px",     "letter-spacing:5px"    ),
            // ex, em cannot be equal
            // TestCase("letter-spacing:10em",   "letter-spacing:10em",    "letter-spacing:10em"   ),    

            // SPIEnum
            TestCase("text-anchor:start",     "text-anchor:start",      "text-anchor:start"     ),
            TestCase("text-anchor:start",     "text-anchor:middle",     "text-anchor:middle"    ),
            TestCase("text-anchor:start",     "",                       "text-anchor:start"     ),
            TestCase("text-anchor:start",     "text-anchor:junk",       "text-anchor:start"     ),
            TestCase("text-anchor:end",       "text-anchor:inherit",    "text-anchor:end"       ),

            TestCase("font-weight:400",       "font-weight:400",        "font-weight:400"       ),
            TestCase("font-weight:400",       "font-weight:700",        "font-weight:700"       ),
            TestCase("font-weight:400",       "font-weight:bolder",     "font-weight:700"       ),
            TestCase("font-weight:700",       "font-weight:bolder",     "font-weight:900"       ),
            TestCase("font-weight:400",       "font-weight:lighter",    "font-weight:100"       ),
            TestCase("font-weight:200",       "font-weight:lighter",    "font-weight:100"       ),

            TestCase("font-stretch:condensed","font-stretch:expanded",  "font-stretch:expanded" ),
            TestCase("font-stretch:condensed","font-stretch:wider",     "font-stretch:semi-condensed" ),

            // SPIString and SPIFontString

            // SPIPaint

            // SPIPaintOrder

            // SPIDashArray

            // SPIFilter

            // SPIFontSize

            // SPIBaselineShift


            // SPITextDecorationLine
            TestCase("text-decoration-line:overline", "text-decoration-line:underline",
                     "text-decoration-line:underline"          ),

            // SPITextDecorationStyle

            // SPITextDecoration

            // Terminate
            TestCase(0,0,0)
        };
        for ( gint i = 0; cases[i].parent; i++ ) {
            // std::cout << "Test three: " << i << std::endl;
            SPStyle style_parent(_doc);
            SPStyle style_child( _doc);
            SPStyle style_result(_doc);

            style_parent.mergeString( cases[i].parent );
            style_child.mergeString(  cases[i].child );
            style_result.mergeString( cases[i].result );

            // std::cout << "Test:" << std::endl;
            // std::cout << " Input: ";
            // std::cout << "  Parent: " << cases[i].parent
            //           << "  Child: "  << cases[i].child
            //           << "  Result: " << cases[i].result << std::endl;
            // std::cout << " Write: ";
            // std::cout << "  Parent: " << style_parent.write( SP_STYLE_FLAG_IFSET ) 
            //           << "  Child: "  << style_child.write( SP_STYLE_FLAG_IFSET ) 
            //           << "  Result: " << style_result.write( SP_STYLE_FLAG_IFSET ) << std::endl;

            style_child.cascade( &style_parent );

            TS_ASSERT(style_child == style_result );

            // std::cout << "End Test: *************\n" << std::endl;
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
