#include <cxxtest/TestSuite.h>
#include "svg/stringstream.h"
#include <2geom/point.h>

template<typename T>
static void
svg_test_datum(T const x, std::string const &exp_str)
{
    Inkscape::SVGOStringStream s;
    s << x;
    TS_ASSERT_EQUALS(s.str(), exp_str);
}

static void
svg_test_float(float const x, std::string const &exp_str)
{
    svg_test_datum(x, exp_str);
    svg_test_datum((double) x, exp_str);
}

class StringStreamTest : public CxxTest::TestSuite
{
public:

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static StringStreamTest *createSuite() { return new StringStreamTest(); }
    static void destroySuite( StringStreamTest *suite ) { delete suite; }

    void testFloats()
    {
        svg_test_float(4.5, "4.5");
        svg_test_float(4.0, "4");
        svg_test_float(0.0, "0");
        svg_test_float(-3.75, "-3.75");
        svg_test_float(-2.0625, "-2.0625");
        svg_test_float(-0.0625, "-0.0625");
        svg_test_float(30.0, "30");
        svg_test_float(12345678.0, "12345678");
        svg_test_float(3e9, "3e+009");
        svg_test_float(-3.5e9, "-3.5e+009");
        svg_test_float(32768e9, "3.2768e+013");
        svg_test_float(-10.5, "-10.5");
    }

    void testOtherTypes()
    {
        svg_test_datum('3', "3");
        svg_test_datum('x', "x");
        svg_test_datum((unsigned char) '$', "$");
        svg_test_datum((signed char) 'Z', "Z");
        svg_test_datum("  my string  ", "  my string  ");
        svg_test_datum((signed char const *) "023", "023");
        svg_test_datum((unsigned char const *) "023", "023");
        svg_test_datum(Geom::Point(1.23, 3.45), "1.23,3.45");
    }

    void testConcat()
    {
        Inkscape::SVGOStringStream s;
        s << "hello, ";
        s << -53.5;
        TS_ASSERT_EQUALS(s.str(), std::string("hello, -53.5"));
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
