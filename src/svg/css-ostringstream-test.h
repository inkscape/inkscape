#include <cxxtest/TestSuite.h>
#include "svg/css-ostringstream.h"

template<typename T>
static void
css_test_datum(T const x, std::string const &exp_str)
{
    Inkscape::CSSOStringStream s;
    s << x;
    TS_ASSERT_EQUALS(s.str(), exp_str);
}

static void
css_test_float(float const x, std::string const &exp_str)
{
    css_test_datum(x, exp_str);
    css_test_datum((double) x, exp_str);
}

class CSSOStringStreamTest : public CxxTest::TestSuite
{
public:

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static CSSOStringStreamTest *createSuite() { return new CSSOStringStreamTest(); }
    static void destroySuite( CSSOStringStreamTest *suite ) { delete suite; }

    void testFloats()
    {
        css_test_float(4.5, "4.5");
        css_test_float(4.0, "4");
        css_test_float(0.0, "0");
        css_test_float(-3.75, "-3.75");
        css_test_float(-2.0625, "-2.0625");
        css_test_float(-0.0625, "-0.0625");
        css_test_float(30.0, "30");
        css_test_float(12345678.0, "12345678");
        css_test_float(3e9, "3000000000");
        css_test_float(-3.5e9, "-3500000000");
        css_test_float(3e-7, "0.0000003");
        css_test_float(3e-8, "0.00000003");
        css_test_float(3e-9, "0");
        css_test_float(32768e9, "32768000000000");
        css_test_float(-10.5, "-10.5");
    }

    void testOtherTypes()
    {
        css_test_datum('3', "3");
        css_test_datum('x', "x");
        css_test_datum((unsigned char) '$', "$");
        css_test_datum((signed char) 'Z', "Z");
        css_test_datum("  my string  ", "  my string  ");
        css_test_datum((signed char const *) "023", "023");
        css_test_datum((unsigned char const *) "023", "023");
    }

    void testConcat()
    {
        Inkscape::CSSOStringStream s;
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
