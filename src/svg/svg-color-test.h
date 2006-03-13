#include <cxxtest/TestSuite.h>
#include "svg/svg-color.h"

static void
test_rgb24(unsigned const rgb24)
{
    char css[8];
    sp_svg_write_color(css, sizeof(css), rgb24 << 8);
    TS_ASSERT_EQUALS(sp_svg_read_color(css, 0xff),
                     rgb24 << 8);
}

static void
test_sp_svg_write_color()
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
        assert(tmp == 0);
        test_rgb24(rgb24);
    }

    /* And a few completely random ones. */
    for (unsigned i = 500; i--;) {  /* Arbitrary number of iterations. */
        unsigned const rgb24 = (rand() >> 4) & 0xffffff;
        test_rgb24(rgb24);
    }
}

class SVGColorTest : public CxxTest::TestSuite
{
public:
    void testWrite()
    {
        test_sp_svg_write_color();
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
