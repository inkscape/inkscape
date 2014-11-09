#include <cxxtest/TestSuite.h>

#include "svg/svg.h"
#include "streq.h"
#include <2geom/affine.h>
#include <algorithm>
#include <glib.h>
#include <iostream>
#include <math.h>
#include <utility>

class SvgAffineTest : public CxxTest::TestSuite
{
private:
    struct test_t {
        char const * str;
        Geom::Affine matrix;
    };
    struct approx_equal_pred {
        bool operator()(Geom::Affine const &ref, Geom::Affine const &cm) const
        {
            double maxabsdiff = 0;
            for(size_t i=0; i<6; i++) {
                maxabsdiff = std::max(std::abs(ref[i]-cm[i]), maxabsdiff);
            }
            return maxabsdiff < 1e-14;
        }
    };
    static test_t const read_matrix_tests[3];
    static test_t const read_translate_tests[3];
    static test_t const read_scale_tests[3];
    static test_t const read_rotate_tests[4];
    static test_t const read_skew_tests[3];
    static char const * const read_fail_tests[25];
    static test_t const write_matrix_tests[2];
    static test_t const write_translate_tests[3];
    static test_t const write_scale_tests[3];
    static test_t const write_rotate_tests[3];
    static test_t const write_skew_tests[3];
public:
    SvgAffineTest() {
    }

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static SvgAffineTest *createSuite() { return new SvgAffineTest(); }
    static void destroySuite( SvgAffineTest *suite ) { delete suite; }

    void testReadIdentity()
    {
        char const* strs[] = {
            //0,
            "",
            "matrix(1,0,0,1,0,0)",
            "translate(0,0)",
            "scale(1,1)",
            "rotate(0,0,0)",
            "skewX(0)",
            "skewY(0)"};
        size_t n = G_N_ELEMENTS(strs);
        for(size_t i=0; i<n; i++) {
            Geom::Affine cm;
            TSM_ASSERT(strs[i] , sp_svg_transform_read(strs[i], &cm));
            TSM_ASSERT_EQUALS(strs[i] , Geom::identity() , cm);
        }
    }

    void testWriteIdentity()
    {
        TS_ASSERT_EQUALS(sp_svg_transform_write(Geom::identity()) , (void*)0)
    }

    void testReadMatrix()
    {
        for(size_t i=0; i<G_N_ELEMENTS(read_matrix_tests); i++) {
            Geom::Affine cm;
            TSM_ASSERT(read_matrix_tests[i].str , sp_svg_transform_read(read_matrix_tests[i].str, &cm));
            TSM_ASSERT_RELATION(read_matrix_tests[i].str , approx_equal_pred , read_matrix_tests[i].matrix , cm);
        }
    }

    void testReadTranslate()
    {
        for(size_t i=0; i<G_N_ELEMENTS(read_translate_tests); i++) {
            Geom::Affine cm;
            TSM_ASSERT(read_translate_tests[i].str , sp_svg_transform_read(read_translate_tests[i].str, &cm));
            TSM_ASSERT_RELATION(read_translate_tests[i].str , approx_equal_pred , read_translate_tests[i].matrix , cm);
        }
    }

    void testReadScale()
    {
        for(size_t i=0; i<G_N_ELEMENTS(read_scale_tests); i++) {
            Geom::Affine cm;
            TSM_ASSERT(read_scale_tests[i].str , sp_svg_transform_read(read_scale_tests[i].str, &cm));
            TSM_ASSERT_RELATION(read_scale_tests[i].str , approx_equal_pred , read_scale_tests[i].matrix , cm);
        }
    }

    void testReadRotate()
    {
        for(size_t i=0; i<G_N_ELEMENTS(read_rotate_tests); i++) {
            Geom::Affine cm;
            TSM_ASSERT(read_rotate_tests[i].str , sp_svg_transform_read(read_rotate_tests[i].str, &cm));
            TSM_ASSERT_RELATION(read_rotate_tests[i].str , approx_equal_pred , read_rotate_tests[i].matrix , cm);
        }
    }

    void testReadSkew()
    {
        for(size_t i=0; i<G_N_ELEMENTS(read_skew_tests); i++) {
            Geom::Affine cm;
            TSM_ASSERT(read_skew_tests[i].str , sp_svg_transform_read(read_skew_tests[i].str, &cm));
            TSM_ASSERT_RELATION(read_skew_tests[i].str , approx_equal_pred , read_skew_tests[i].matrix , cm);
        }
    }

    void testWriteMatrix()
    {
        for(size_t i=0; i<G_N_ELEMENTS(write_matrix_tests); i++) {
            char * str = sp_svg_transform_write(write_matrix_tests[i].matrix);
            TS_ASSERT_RELATION(streq_rel , str , write_matrix_tests[i].str);
            g_free(str);
        }
    }

    void testWriteTranslate()
    {
        for(size_t i=0; i<G_N_ELEMENTS(write_translate_tests); i++) {
            char * str = sp_svg_transform_write(write_translate_tests[i].matrix);
            TS_ASSERT_RELATION(streq_rel , str , write_translate_tests[i].str);
            g_free(str);
        }
    }

    void testWriteScale()
    {
        for(size_t i=0; i<G_N_ELEMENTS(write_scale_tests); i++) {
            char * str = sp_svg_transform_write(write_scale_tests[i].matrix);
            TS_ASSERT_RELATION(streq_rel , str , write_scale_tests[i].str);
            g_free(str);
        }
    }

    void testWriteRotate()
    {
        for(size_t i=0; i<G_N_ELEMENTS(write_rotate_tests); i++) {
            char * str = sp_svg_transform_write(write_rotate_tests[i].matrix);
            TS_ASSERT_RELATION(streq_rel , str , write_rotate_tests[i].str);
            g_free(str);
        }
    }

    void testWriteSkew()
    {
        for(size_t i=0; i<G_N_ELEMENTS(write_skew_tests); i++) {
            char * str = sp_svg_transform_write(write_skew_tests[i].matrix);
            TS_ASSERT_RELATION(streq_rel , str , write_skew_tests[i].str);
            g_free(str);
        }
    }

    void testReadConcatenation()
    {
        // NOTE: According to the SVG specification (see the syntax at http://www.w3.org/TR/SVG/coords.html#TransformAttribute
        //       there should be 1 or more comma-wsp sequences between transforms... This doesn't make sense and it seems
        //       likely that instead of a + they meant a ? (zero or one comma-wsp sequences).
        char const * str = "skewY(17)skewX(9)translate(7,13)scale(2)rotate(13)translate(3,5)";
        Geom::Affine ref(2.0199976232558053, 1.0674773585906016, -0.14125199392774669, 1.9055550612095459, 14.412730624347654, 28.499820929377454); // Precomputed using Mathematica
        Geom::Affine cm;
        TS_ASSERT(sp_svg_transform_read(str, &cm));
        TS_ASSERT_RELATION(approx_equal_pred , ref , cm);
    }

    void testReadFailures()
    {
        for(size_t i=0; i<G_N_ELEMENTS(read_fail_tests); i++) {
            Geom::Affine cm;
            TSM_ASSERT(read_fail_tests[i] , !sp_svg_transform_read(read_fail_tests[i], &cm));
        }
    }
};

static double const DEGREE = M_PI/180.;

SvgAffineTest::test_t const SvgAffineTest::read_matrix_tests[3] = {
    {"matrix(0,0,0,0,0,0)",Geom::Affine(0,0,0,0,0,0)},
    {" matrix(1,2,3,4,5,6)",Geom::Affine(1,2,3,4,5,6)},
    {"matrix (1 2 -3,-4,5e6,-6e-7)",Geom::Affine(1,2,-3,-4,5e6,-6e-7)}};
SvgAffineTest::test_t const SvgAffineTest::read_translate_tests[3] = {
    {"translate(1)",Geom::Affine(1,0,0,1,1,0)},
    {"translate(1,1)",Geom::Affine(1,0,0,1,1,1)},
    {"translate(-1e3 .123e2)",Geom::Affine(1,0,0,1,-1e3,.123e2)}};
SvgAffineTest::test_t const SvgAffineTest::read_scale_tests[3] = {
    {"scale(2)",Geom::Affine(2,0,0,2,0,0)},
    {"scale(2,3)",Geom::Affine(2,0,0,3,0,0)},
    {"scale(0.1e-2 -.475e0)",Geom::Affine(0.1e-2,0,0,-.475e0,0,0)}};
SvgAffineTest::test_t const SvgAffineTest::read_rotate_tests[4] = {
    {"rotate(13 )",Geom::Affine(cos(13.*DEGREE),sin(13.*DEGREE),-sin(13.*DEGREE),cos(13.*DEGREE),0,0)},
    {"rotate(-13)",Geom::Affine(cos(-13.*DEGREE),sin(-13.*DEGREE),-sin(-13.*DEGREE),cos(-13.*DEGREE),0,0)},
    {"rotate(373)",Geom::Affine(cos(13.*DEGREE),sin(13.*DEGREE),-sin(13.*DEGREE),cos(13.*DEGREE),0,0)},
    {"rotate(13,7,11)",Geom::Affine(cos(13.*DEGREE),sin(13.*DEGREE),-sin(13.*DEGREE),cos(13.*DEGREE),(1-cos(13.*DEGREE))*7+sin(13.*DEGREE)*11,(1-cos(13.*DEGREE))*11-sin(13.*DEGREE)*7)}};
SvgAffineTest::test_t const SvgAffineTest::read_skew_tests[3] = {
    {"skewX( 30)",Geom::Affine(1,0,tan(30.*DEGREE),1,0,0)},
    {"skewX(-30)",Geom::Affine(1,0,tan(-30.*DEGREE),1,0,0)},
    {"skewY(390)",Geom::Affine(1,tan(30.*DEGREE),0,1,0,0)}};
char const * const SvgAffineTest::read_fail_tests[25] = {
    "matrix((1,2,3,4,5,6)",
    "matrix((1,2,3,4,5,6))",
    "matrix(1,2,3,4,5,6))",
    "matrix(,1,2,3,4,5,6)",
    "matrix(1,2,3,4,5,6,)",
    "matrix(1,2,3,4,5,)",
    "matrix(1,2,3,4,5)",
    "matrix(1,2,3,4,5e6-3)", // Here numbers HAVE to be separated by a comma-wsp sequence
    "matrix(1,2,3,4,5e6.3)", // Here numbers HAVE to be separated by a comma-wsp sequence
    "translate()",
    "translate(,)",
    "translate(1,)",
    "translate(1,6,)",
    "translate(1,6,0)",
    "scale()",
    "scale(1,6,2)",
    "rotate()",
    "rotate(1,6)",
    "rotate(1,6,)",
    "rotate(1,6,3,4)",
    "skewX()",
    "skewX(-)",
    "skewX(.)",
    "skewY(,)",
    "skewY(1,2)"};

SvgAffineTest::test_t const SvgAffineTest::write_matrix_tests[2] = {
    {"matrix(1,2,3,4,5,6)",Geom::Affine(1,2,3,4,5,6)},
    {"matrix(-1,2123,3,0.4,1e-8,1e20)",Geom::Affine(-1,2.123e3,3+1e-14,0.4,1e-8,1e20)}};
SvgAffineTest::test_t const SvgAffineTest::write_translate_tests[3] = {
    {"translate(1,1)",Geom::Affine(1,0,0,1,1,1)},
    {"translate(1)",Geom::Affine(1,0,0,1,1,0)},
    {"translate(-1345,0.123)",Geom::Affine(1,0,0,1,-1.345e3,.123)}};
SvgAffineTest::test_t const SvgAffineTest::write_scale_tests[3] = {
    {"scale(0)",Geom::Affine(0,0,0,0,0,0)},
    {"scale(7)",Geom::Affine(7,0,0,7,0,0)},
    {"scale(2,3)",Geom::Affine(2,0,0,3,0,0)}};
SvgAffineTest::test_t const SvgAffineTest::write_rotate_tests[3] = {
    {"rotate(13)",Geom::Affine(cos(13.*DEGREE),sin(13.*DEGREE),-sin(13.*DEGREE),cos(13.*DEGREE),0,0)},
    {"rotate(-13,7,11)",Geom::Affine(cos(-13.*DEGREE),sin(-13.*DEGREE),-sin(-13.*DEGREE),cos(-13.*DEGREE),(1-cos(-13.*DEGREE))*7+sin(-13.*DEGREE)*11,(1-cos(-13.*DEGREE))*11-sin(-13.*DEGREE)*7)},
    {"rotate(-34.5,6.7,89)",Geom::Affine(cos(-34.5*DEGREE),sin(-34.5*DEGREE),-sin(-34.5*DEGREE),cos(-34.5*DEGREE),(1-cos(-34.5*DEGREE))*6.7+sin(-34.5*DEGREE)*89,(1-cos(-34.5*DEGREE))*89-sin(-34.5*DEGREE)*6.7)}};
SvgAffineTest::test_t const SvgAffineTest::write_skew_tests[3] = {
    {"skewX(30)",Geom::Affine(1,0,tan(30.*DEGREE),1,0,0)},
    {"skewX(-30)",Geom::Affine(1,0,tan(-30.*DEGREE),1,0,0)},
    {"skewY(30)",Geom::Affine(1,tan(30.*DEGREE),0,1,0,0)}};

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
