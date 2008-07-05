#include <cxxtest/TestSuite.h>

#include "svg/svg-length.h"
#include <glib.h>
#include <utility>

class SvgLengthTest : public CxxTest::TestSuite
{
private:
public:
    SvgLengthTest() {
    }

    void testRead()
    {
        struct test_t {
            char const* str; float computed;
            test_t(char const* str, float computed) : str(str), computed(computed) {}
        };
        test_t tests[] = {
            test_t("0",0),
            test_t("1",1),
            test_t("1.00001",1.00001),
            test_t("1px",1),
            test_t(".1px",0.1)};
        size_t n = G_N_ELEMENTS(tests);
        for(size_t i=0; i<n; i++) {
            SVGLength l;
            TSM_ASSERT(tests[i].str , l.read(tests[i].str));
            TSM_ASSERT_EQUALS(tests[i].str , l.computed , tests[i].computed);
        }
    }

    // TODO: More tests
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
