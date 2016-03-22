#include <cxxtest/TestSuite.h>

#include "svg/svg-length.h"
#include <glib.h>
#include <utility>

// function internal to svg-length.cpp:
gchar const *sp_svg_length_get_css_units(SVGLength::Unit unit);

class SvgLengthTest : public CxxTest::TestSuite
{
private:
    struct test_t {
        char const* str; SVGLength::Unit unit; float value; float computed;
    };
    struct testd_t {
        char const* str; double val; int prec; int minexp;
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
            SVGLength len;
            TSM_ASSERT(absolute_tests[i].str , len.read(absolute_tests[i].str));
            TSM_ASSERT_EQUALS(absolute_tests[i].str , len.unit , absolute_tests[i].unit);
            TSM_ASSERT_EQUALS(absolute_tests[i].str , len.value , absolute_tests[i].value);
            TSM_ASSERT_EQUALS(absolute_tests[i].str , len.computed , absolute_tests[i].computed);
        }
        for(size_t i=0; i<G_N_ELEMENTS(relative_tests); i++) {
            SVGLength len;
            TSM_ASSERT(relative_tests[i].str , len.read(relative_tests[i].str));
            len.update(7,13,19);
            TSM_ASSERT_EQUALS(relative_tests[i].str , len.unit , relative_tests[i].unit);
            TSM_ASSERT_EQUALS(relative_tests[i].str , len.value , relative_tests[i].value);
            TSM_ASSERT_EQUALS(relative_tests[i].str , len.computed , relative_tests[i].computed);
        }
        for(size_t i=0; i<G_N_ELEMENTS(fail_tests); i++) {
            SVGLength len;
            TSM_ASSERT(fail_tests[i] , !len.read(fail_tests[i]));
        }
    }

    void testReadOrUnset()
    {
        for(size_t i=0; i<G_N_ELEMENTS(absolute_tests); i++) {
            SVGLength len;
            len.readOrUnset(absolute_tests[i].str);
            TSM_ASSERT_EQUALS(absolute_tests[i].str , len.unit , absolute_tests[i].unit);
            TSM_ASSERT_EQUALS(absolute_tests[i].str , len.value , absolute_tests[i].value);
            TSM_ASSERT_EQUALS(absolute_tests[i].str , len.computed , absolute_tests[i].computed);
        }
        for(size_t i=0; i<G_N_ELEMENTS(relative_tests); i++) {
            SVGLength len;
            len.readOrUnset(relative_tests[i].str);
            len.update(7,13,19);
            TSM_ASSERT_EQUALS(relative_tests[i].str , len.unit , relative_tests[i].unit);
            TSM_ASSERT_EQUALS(relative_tests[i].str , len.value , relative_tests[i].value);
            TSM_ASSERT_EQUALS(relative_tests[i].str , len.computed , relative_tests[i].computed);
        }
        for(size_t i=0; i<G_N_ELEMENTS(fail_tests); i++) {
            SVGLength len;
            len.readOrUnset(fail_tests[i], SVGLength::INCH, 123, 456);
            TSM_ASSERT_EQUALS(fail_tests[i] , len.unit , SVGLength::INCH);
            TSM_ASSERT_EQUALS(fail_tests[i] , len.value , 123);
            TSM_ASSERT_EQUALS(fail_tests[i] , len.computed , 456);
        }
    }

    void testReadAbsolute()
    {
        for(size_t i=0; i<G_N_ELEMENTS(absolute_tests); i++) {
            SVGLength len;
            TSM_ASSERT(absolute_tests[i].str , len.readAbsolute(absolute_tests[i].str));
            TSM_ASSERT_EQUALS(absolute_tests[i].str , len.unit , absolute_tests[i].unit);
            TSM_ASSERT_EQUALS(absolute_tests[i].str , len.value , absolute_tests[i].value);
            TSM_ASSERT_EQUALS(absolute_tests[i].str , len.computed , absolute_tests[i].computed);
        }
        for(size_t i=0; i<G_N_ELEMENTS(relative_tests); i++) {
            SVGLength len;
            TSM_ASSERT(relative_tests[i].str , !len.readAbsolute(relative_tests[i].str));
        }
        for(size_t i=0; i<G_N_ELEMENTS(fail_tests); i++) {
            SVGLength len;
            TSM_ASSERT(fail_tests[i] , !len.readAbsolute(fail_tests[i]));
        }
    }

    void testEnumMappedToString()
    {
        for ( int i = (static_cast<int>(SVGLength::NONE) + 1); i <= static_cast<int>(SVGLength::LAST_UNIT); i++ ) {
            SVGLength::Unit target = static_cast<SVGLength::Unit>(i);
            // PX is a special case where we don't have a unit string
            if ( (target != SVGLength::PX) ) {
                gchar const* val = sp_svg_length_get_css_units(target);
                TSM_ASSERT_DIFFERS(i, val, "");
            }
        }
    }

    // Ensure that all unit suffix strings used are allowed by SVG
    void testStringsAreValidSVG()
    {
        gchar const* valid[] = {"", "em", "ex", "px", "pt", "pc", "cm", "mm", "in", "%"};
        std::set<std::string> validStrings(valid, valid + G_N_ELEMENTS(valid));
        for ( int i = (static_cast<int>(SVGLength::NONE) + 1); i <= static_cast<int>(SVGLength::LAST_UNIT); i++ ) {
            SVGLength::Unit target = static_cast<SVGLength::Unit>(i);
            gchar const* val = sp_svg_length_get_css_units(target);
            TSM_ASSERT(i, validStrings.find(std::string(val)) != validStrings.end());
        }
    }

    // Ensure that all unit suffix strings allowed by SVG are covered by enum
    void testValidSVGStringsSupported()
    {
        // Note that "px" is ommitted from the list, as it will be assumed to be so if not explicitly set.
        gchar const* valid[] = {"em", "ex", "pt", "pc", "cm", "mm", "in", "%"};
        std::set<std::string> validStrings(valid, valid + G_N_ELEMENTS(valid));
        for ( int i = (static_cast<int>(SVGLength::NONE) + 1); i <= static_cast<int>(SVGLength::LAST_UNIT); i++ ) {
            SVGLength::Unit target = static_cast<SVGLength::Unit>(i);
            gchar const* val = sp_svg_length_get_css_units(target);
            std::set<std::string>::iterator iter = validStrings.find(std::string(val));
            if (iter != validStrings.end()) {
                validStrings.erase(iter);
            }
        }
        TSM_ASSERT_EQUALS(validStrings, validStrings.size(), 0u);
    }

    void testPlaces()
    {
        testd_t const precTests[] = {
            {"760", 761.92918978947023, 2, -8},
            {"761.9", 761.92918978947023, 4, -8},
        };

        for ( size_t i = 0; i < G_N_ELEMENTS(precTests); i++ ) {
            char buf[256] = {0};
            memset(buf, 0xCC, sizeof(buf)); // Make it easy to detect an overrun.
            unsigned int retval = sp_svg_number_write_de( buf, sizeof(buf), precTests[i].val, precTests[i].prec, precTests[i].minexp );
            TSM_ASSERT_EQUALS("Number of chars written", retval, strlen(precTests[i].str));
            TSM_ASSERT_EQUALS("Numeric string written", std::string(buf), std::string(precTests[i].str));
            TSM_ASSERT_EQUALS(std::string("Buffer overrun ") + precTests[i].str, '\xCC', buf[retval + 1]);
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
    {"100pt",        SVGLength::PT  , 100        ,  400.0/3.0},
    {"1e2pt",        SVGLength::PT  , 100        ,  400.0/3.0},
    {"3pc",          SVGLength::PC  ,   3        ,  48},
    {"-3.5pc",       SVGLength::PC  ,  -3.5      ,  -3.5*16.0},
    {"1.2345678mm",  SVGLength::MM  ,   1.2345678,   1.2345678f*96.0/25.4}, // TODO: More precise constants? (a 7 digit constant when the default precision is 8 digits?)
    {"123.45678cm", SVGLength::CM   , 123.45678  , 123.45678f*96.0/2.54},   // Note that svg_length_read is casting the result from g_ascii_strtod to float.
    {"73.162987in",  SVGLength::INCH,  73.162987 ,  73.162987f*96.0/1.00}};
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
