#include <cxxtest/TestSuite.h>

#include "svg/svg-length.h"
#include <glib.h>
#include <utility>

class SvgLengthTest : public CxxTest::TestSuite
{
private:
    struct test_t {
        char const* str; SVGLength::Unit unit; float value; float computed;
    };
    static test_t const absolute_tests[12];
    static test_t const relative_tests[3];
    static char const * fail_tests[8];
public:
    SvgLengthTest() {
    }

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static SvgLengthTest *createSuite() { return new SvgLengthTest(); }
    static void destroySuite( SvgLengthTest *suite ) { delete suite; }

    void testRead()
    {
        for(size_t i=0; i<G_N_ELEMENTS(absolute_tests); i++) {
            SVGLength l;
            TSM_ASSERT(absolute_tests[i].str , l.read(absolute_tests[i].str));
            TSM_ASSERT_EQUALS(absolute_tests[i].str , l.unit , absolute_tests[i].unit);
            TSM_ASSERT_EQUALS(absolute_tests[i].str , l.value , absolute_tests[i].value);
            TSM_ASSERT_EQUALS(absolute_tests[i].str , l.computed , absolute_tests[i].computed);
        }
        for(size_t i=0; i<G_N_ELEMENTS(relative_tests); i++) {
            SVGLength l;
            TSM_ASSERT(relative_tests[i].str , l.read(relative_tests[i].str));
            l.update(7,13,19);
            TSM_ASSERT_EQUALS(relative_tests[i].str , l.unit , relative_tests[i].unit);
            TSM_ASSERT_EQUALS(relative_tests[i].str , l.value , relative_tests[i].value);
            TSM_ASSERT_EQUALS(relative_tests[i].str , l.computed , relative_tests[i].computed);
        }
        for(size_t i=0; i<G_N_ELEMENTS(fail_tests); i++) {
            SVGLength l;
            TSM_ASSERT(fail_tests[i] , !l.read(fail_tests[i]));
        }
    }

    void testReadOrUnset()
    {
        for(size_t i=0; i<G_N_ELEMENTS(absolute_tests); i++) {
            SVGLength l;
            l.readOrUnset(absolute_tests[i].str);
            TSM_ASSERT_EQUALS(absolute_tests[i].str , l.unit , absolute_tests[i].unit);
            TSM_ASSERT_EQUALS(absolute_tests[i].str , l.value , absolute_tests[i].value);
            TSM_ASSERT_EQUALS(absolute_tests[i].str , l.computed , absolute_tests[i].computed);
        }
        for(size_t i=0; i<G_N_ELEMENTS(relative_tests); i++) {
            SVGLength l;
            l.readOrUnset(relative_tests[i].str);
            l.update(7,13,19);
            TSM_ASSERT_EQUALS(relative_tests[i].str , l.unit , relative_tests[i].unit);
            TSM_ASSERT_EQUALS(relative_tests[i].str , l.value , relative_tests[i].value);
            TSM_ASSERT_EQUALS(relative_tests[i].str , l.computed , relative_tests[i].computed);
        }
        for(size_t i=0; i<G_N_ELEMENTS(fail_tests); i++) {
            SVGLength l;
            l.readOrUnset(fail_tests[i], SVGLength::INCH, 123, 456);
            TSM_ASSERT_EQUALS(fail_tests[i] , l.unit , SVGLength::INCH);
            TSM_ASSERT_EQUALS(fail_tests[i] , l.value , 123);
            TSM_ASSERT_EQUALS(fail_tests[i] , l.computed , 456);
        }
    }

    void testReadAbsolute()
    {
        for(size_t i=0; i<G_N_ELEMENTS(absolute_tests); i++) {
            SVGLength l;
            TSM_ASSERT(absolute_tests[i].str , l.readAbsolute(absolute_tests[i].str));
            TSM_ASSERT_EQUALS(absolute_tests[i].str , l.unit , absolute_tests[i].unit);
            TSM_ASSERT_EQUALS(absolute_tests[i].str , l.value , absolute_tests[i].value);
            TSM_ASSERT_EQUALS(absolute_tests[i].str , l.computed , absolute_tests[i].computed);
        }
        for(size_t i=0; i<G_N_ELEMENTS(relative_tests); i++) {
            SVGLength l;
            TSM_ASSERT(relative_tests[i].str , !l.readAbsolute(relative_tests[i].str));
        }
        for(size_t i=0; i<G_N_ELEMENTS(fail_tests); i++) {
            SVGLength l;
            TSM_ASSERT(fail_tests[i] , !l.readAbsolute(fail_tests[i]));
        }
    }

    // TODO: More tests
};

SvgLengthTest::test_t const SvgLengthTest::absolute_tests[12] = {
    {"0",            SVGLength::NONE,   0        ,   0},
    {"1",            SVGLength::NONE,   1        ,   1},
    {"1.00001",      SVGLength::NONE,   1.00001  ,   1.00001},
    {"1px",          SVGLength::PX  ,   1        ,   1},
    {".1px",         SVGLength::PX  ,   0.1      ,   0.1},
    {"100pt",        SVGLength::PT  , 100        , 125},
    {"1e2pt",        SVGLength::PT  , 100        , 125},
    {"3pc",          SVGLength::PC  ,   3        ,  45},
    {"-3.5pc",       SVGLength::PC  ,  -3.5      ,  -3.5*15.},
    {"1.2345678mm",  SVGLength::MM  ,   1.2345678,   1.2345678*3.543307}, // TODO: More precise constants? (a 7 digit constant when the default precision is 8 digits?)
    {"123.45678cm", SVGLength::CM   , 123.45678  , 123.45678*35.43307},
    {"73.162987in",  SVGLength::INCH,  73.162987 ,  73.162987*90}};
SvgLengthTest::test_t const SvgLengthTest::relative_tests[3] = {
    {"123em", SVGLength::EM,      123, 123. *  7.},
    {"123ex", SVGLength::EX,      123, 123. * 13.},
    {"123%",  SVGLength::PERCENT, 1.23, 1.23 * 19.}};
char const * SvgLengthTest::fail_tests[8] = {
    "123 px",
    "123e",
    "123e+m",
    "123ec",
    "123pxt",
    "--123",
    "",
    "px"};

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
