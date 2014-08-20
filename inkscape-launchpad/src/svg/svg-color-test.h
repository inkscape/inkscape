#include <cxxtest/TestSuite.h>
#include <cassert>
#include <cstdlib>

#include "preferences.h"
#include "svg/svg-color.h"
#include "svg/svg-icc-color.h"

class SVGColorTest : public CxxTest::TestSuite
{
    struct simpleIccCase {
        unsigned numEntries;
        bool shouldPass;
        char const* name;
        char const* str;
    };

public:
    void check_rgb24(unsigned const rgb24)
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        char css[8];
        prefs->setBool("/options/svgoutput/usenamedcolors", false);
        sp_svg_write_color(css, sizeof(css), rgb24 << 8);
        TS_ASSERT_EQUALS(sp_svg_read_color(css, 0xff),
                         rgb24 << 8);
        prefs->setBool("/options/svgoutput/usenamedcolors", true);
        sp_svg_write_color(css, sizeof(css), rgb24 << 8);
        TS_ASSERT_EQUALS(sp_svg_read_color(css, 0xff),
                         rgb24 << 8);
    }

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static SVGColorTest *createSuite() { return new SVGColorTest(); }
    static void destroySuite( SVGColorTest *suite ) { delete suite; }

    void testWrite()
    {
        unsigned const components[] = {0, 0x80, 0xff, 0xc0, 0x77};
        unsigned const nc = G_N_ELEMENTS(components);
        for (unsigned i = nc*nc*nc; i--;) {
            unsigned tmp = i;
            unsigned rgb24 = 0;
            for (unsigned c = 0; c < 3; ++c) {
                unsigned const component = components[tmp % nc];
                rgb24 = (rgb24 << 8) | component;
                tmp /= nc;
            }
            assert( tmp == 0 );
            check_rgb24(rgb24);
        }

        /* And a few completely random ones. */
        for (unsigned i = 500; i--;) {  /* Arbitrary number of iterations. */
            unsigned const rgb24 = (std::rand() >> 4) & 0xffffff;
            check_rgb24(rgb24);
        }
    }

    void testReadColor()
    {
        gchar const* val[] = {"#f0f", "#ff00ff", "rgb(255,0,255)", "fuchsia"};
        size_t const n = sizeof(val)/sizeof(*val);
        for(size_t i=0; i<n; i++) {
            gchar const* end = 0;
            guint32 result = sp_svg_read_color( val[i], &end, 0x3 );
            TS_ASSERT_EQUALS( result, 0xff00ff00 );
            TS_ASSERT_LESS_THAN( val[i], end );
        }
    }

    void testIccColor()
    {
        simpleIccCase cases[] = {
            {1, true, "named", "icc-color(named, 3)"},
            {0, false, "", "foodle"},
            {1, true, "a", "icc-color(a, 3)"},
            {4, true, "named", "icc-color(named, 3, 0, 0.1, 2.5)"},
            {0, false, "", "icc-color(named, 3"},
            {0, false, "", "icc-color(space named, 3)"},
            {0, false, "", "icc-color(tab\tnamed, 3)"},
            {0, false, "", "icc-color(0name, 3)"},
            {0, false, "", "icc-color(-name, 3)"},
            {1, true, "positive", "icc-color(positive, +3)"},
            {1, true, "negative", "icc-color(negative, -3)"},
            {1, true, "positive", "icc-color(positive, +0.1)"},
            {1, true, "negative", "icc-color(negative, -0.1)"},
            {0, false, "", "icc-color(named, value)"},
            {1, true, "hyphen-name", "icc-color(hyphen-name, 1)"},
            {1, true, "under_name", "icc-color(under_name, 1)"},
        };

        for ( size_t i = 0; i < G_N_ELEMENTS(cases); i++ ) {
            SVGICCColor tmp;
            gchar const* str = cases[i].str;
            gchar const* result = 0;

            std::string testDescr( cases[i].str );

            bool parseRet = sp_svg_read_icc_color( str, &result, &tmp );
            TSM_ASSERT_EQUALS( testDescr, parseRet, cases[i].shouldPass );
            TSM_ASSERT_EQUALS( testDescr, tmp.colors.size(), cases[i].numEntries );
            if ( cases[i].shouldPass ) {
                TSM_ASSERT_DIFFERS( testDescr, str, result );
                TSM_ASSERT_EQUALS( testDescr, tmp.colorProfile, std::string(cases[i].name) );
            } else {
                TSM_ASSERT_EQUALS( testDescr, str, result );
                TSM_ASSERT( testDescr, tmp.colorProfile.empty() );
            }
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
