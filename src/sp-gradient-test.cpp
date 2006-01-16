#include "attributes.h"
#include "inkscape-private.h"
#include "sp-gradient.h"
#include "sp-object.h"
#include "document.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-matrix-fns.h"
#include "libnr/nr-matrix-ops.h"
#include "libnr/nr-rect.h"
#include "libnr/nr-rotate-fns.h"
#include "svg/svg.h"
#include "utest/utest.h"
#include "xml/repr.h"

/// Dummy functions to keep linker happy
int sp_main_gui (int, char const**) { return 0; }
int sp_main_console (int, char const**) { return 0; }

static bool
test_gradient()
{
    utest_start("gradient");
    UTEST_TEST("init") {
        SPGradient *gr = static_cast<SPGradient *>(g_object_new(SP_TYPE_GRADIENT, NULL));
        UTEST_ASSERT(gr->gradientTransform.test_identity());
        UTEST_ASSERT(gr->gradientTransform == NR::identity());
        g_object_unref(gr);
    }

    /* Create the global inkscape object. */
    static_cast<void>(g_object_new(inkscape_get_type(), NULL));


    SPDocument *doc = sp_document_new_dummy();

    UTEST_TEST("sp_object_set(\"gradientTransform\")") {
        SPGradient *gr = static_cast<SPGradient *>(g_object_new(SP_TYPE_GRADIENT, NULL));
        SP_OBJECT(gr)->document = doc;
        sp_object_set(SP_OBJECT(gr), SP_ATTR_GRADIENTTRANSFORM, "translate(5, 8)");
        UTEST_ASSERT(gr->gradientTransform == NR::Matrix(NR::translate(5, 8)));
        sp_object_set(SP_OBJECT(gr), SP_ATTR_GRADIENTTRANSFORM, "");
        UTEST_ASSERT(gr->gradientTransform == NR::identity());
        sp_object_set(SP_OBJECT(gr), SP_ATTR_GRADIENTTRANSFORM, "rotate(90)");
        UTEST_ASSERT(gr->gradientTransform == NR::Matrix(rotate_degrees(90)));
        g_object_unref(gr);
    }

    UTEST_TEST("write") {
        SPGradient *gr = static_cast<SPGradient *>(g_object_new(SP_TYPE_GRADIENT, NULL));
        SP_OBJECT(gr)->document = doc;
        sp_object_set(SP_OBJECT(gr), SP_ATTR_GRADIENTTRANSFORM, "matrix(0, 1, -1, 0, 0, 0)");
        Inkscape::XML::Node *repr = sp_repr_new("svg:radialGradient");
        SP_OBJECT(gr)->updateRepr(repr, SP_OBJECT_WRITE_ALL);
        {
            gchar const *tr = repr->attribute("gradientTransform");
            NR::Matrix svd;
            bool const valid = sp_svg_transform_read(tr, &svd);
            UTEST_ASSERT(valid);
            UTEST_ASSERT(svd == NR::Matrix(rotate_degrees(90)));
        }
        g_object_unref(gr);
    }

    UTEST_TEST("get_g2d, get_gs2d, set_gs2d") {
        SPGradient *gr = static_cast<SPGradient *>(g_object_new(SP_TYPE_GRADIENT, NULL));
        SP_OBJECT(gr)->document = doc;
        NR::Matrix const grXform(2, 1,
                                 1, 3,
                                 4, 6);
        gr->gradientTransform = grXform;
        NR::Rect const unit_rect(NR::Point(0, 0), NR::Point(1, 1));
        {
            NR::Matrix const g2d(sp_gradient_get_g2d_matrix(gr, NR::identity(), unit_rect));
            NR::Matrix const gs2d(sp_gradient_get_gs2d_matrix(gr, NR::identity(), unit_rect));
            UTEST_ASSERT(g2d == NR::identity());
            UTEST_ASSERT(NR::matrix_equalp(gs2d, gr->gradientTransform * g2d, 1e-12));

            sp_gradient_set_gs2d_matrix(gr, NR::identity(), unit_rect, gs2d);
            UTEST_ASSERT(NR::matrix_equalp(gr->gradientTransform, grXform, 1e-12));
        }

        gr->gradientTransform = grXform;
        NR::Matrix const funny(2, 3,
                               4, 5,
                               6, 7);
        {
            NR::Matrix const g2d(sp_gradient_get_g2d_matrix(gr, funny, unit_rect));
            NR::Matrix const gs2d(sp_gradient_get_gs2d_matrix(gr, funny, unit_rect));
            UTEST_ASSERT(g2d == funny);
            UTEST_ASSERT(NR::matrix_equalp(gs2d, gr->gradientTransform * g2d, 1e-12));

            sp_gradient_set_gs2d_matrix(gr, funny, unit_rect, gs2d);
            UTEST_ASSERT(NR::matrix_equalp(gr->gradientTransform, grXform, 1e-12));
        }

        gr->gradientTransform = grXform;
        NR::Rect const larger_rect(NR::Point(5, 6), NR::Point(8, 10));
        {
            NR::Matrix const g2d(sp_gradient_get_g2d_matrix(gr, funny, larger_rect));
            NR::Matrix const gs2d(sp_gradient_get_gs2d_matrix(gr, funny, larger_rect));
            UTEST_ASSERT(g2d == NR::Matrix(3, 0,
                                           0, 4,
                                           5, 6) * funny);
            UTEST_ASSERT(NR::matrix_equalp(gs2d, gr->gradientTransform * g2d, 1e-12));

            sp_gradient_set_gs2d_matrix(gr, funny, larger_rect, gs2d);
            UTEST_ASSERT(NR::matrix_equalp(gr->gradientTransform, grXform, 1e-12));

            sp_object_set(SP_OBJECT(gr), SP_ATTR_GRADIENTUNITS, "userSpaceOnUse");
            NR::Matrix const user_g2d(sp_gradient_get_g2d_matrix(gr, funny, larger_rect));
            NR::Matrix const user_gs2d(sp_gradient_get_gs2d_matrix(gr, funny, larger_rect));
            UTEST_ASSERT(user_g2d == funny);
            UTEST_ASSERT(NR::matrix_equalp(user_gs2d, gr->gradientTransform * user_g2d, 1e-12));
        }
        g_object_unref(gr);
    }

    return utest_end();
}

int main()
{
    g_type_init();
    Inkscape::GC::init();
    return ( test_gradient()
             ? EXIT_SUCCESS
             : EXIT_FAILURE );
}

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
