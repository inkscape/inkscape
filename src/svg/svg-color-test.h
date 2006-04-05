#include <cxxtest/TestSuite.h>
#include "svg/svg-color.h"
#include "svg/svg-icc-color.h"

class SVGColorTest : public CxxTest::TestSuite
{
public:
    void check_rgb24(unsigned const rgb24)
    {
        char css[8];
        sp_svg_write_color(css, sizeof(css), rgb24 << 8);
        TS_ASSERT_EQUALS(sp_svg_read_color(css, 0xff),
                         rgb24 << 8);
    }

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
            TS_ASSERT_EQUALS( tmp, 0 );
            check_rgb24(rgb24);
        }

        /* And a few completely random ones. */
        for (unsigned i = 500; i--;) {  /* Arbitrary number of iterations. */
            unsigned const rgb24 = (rand() >> 4) & 0xffffff;
            check_rgb24(rgb24);
        }
    }

    void testReadColor()
    {
        gchar* val="#f0f";
        gchar const * end = 0;
        guint32 result = sp_svg_read_color( val, &end, 0x3 );
        TS_ASSERT_EQUALS( result, 0xff00ff00 );
        TS_ASSERT_LESS_THAN( val, end );
    }

    void testIccColor()
    {
        SVGICCColor tmp;
        gchar* str = "icc-color(named, 3)";
        gchar const * result = 0;

        bool parseRet = sp_svg_read_icc_color( str, &result, &tmp );
        TS_ASSERT( parseRet );
        TS_ASSERT_DIFFERS( str, result );
        TS_ASSERT_EQUALS( std::string("named"), tmp.colorProfile );
        TS_ASSERT_EQUALS( 1, tmp.colors.size() );


        gchar* badThing = "foodle";
        result = 0;
        parseRet = sp_svg_read_icc_color( badThing, &result, &tmp );
        TS_ASSERT( !parseRet );
        TS_ASSERT_EQUALS( badThing, result );
        TS_ASSERT_DIFFERS( std::string("named"), tmp.colorProfile );
        TS_ASSERT_EQUALS( 0, tmp.colors.size() );
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
