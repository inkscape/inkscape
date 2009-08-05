
#ifndef SEEN_SP_GRADIENT_TEST_H
#define SEEN_SP_GRADIENT_TEST_H

#include "document-using-test.h"


#include "sp-gradient.h"
#include "svg/svg.h"
#include "xml/repr.h"
#include <2geom/transforms.h>
#include "helper/geom.h"

class SPGradientTest : public DocumentUsingTest
{
public:
    Document* _doc;

    SPGradientTest() :
        _doc(0)
    {
    }

    virtual ~SPGradientTest()
    {
        if ( _doc )
        {
            sp_document_unref( _doc );
        }
    }

    static void createSuiteSubclass( SPGradientTest *& dst )
    {
        SPGradient *gr = static_cast<SPGradient *>(g_object_new(SP_TYPE_GRADIENT, NULL));
        if ( gr ) {
            UTEST_ASSERT(gr->gradientTransform.isIdentity());
            UTEST_ASSERT(gr->gradientTransform == Geom::identity());
            g_object_unref(gr);

            dst = new SPGradientTest();
        }
    }

    static SPGradientTest *createSuite()
    {
        return Inkscape::createSuiteAndDocument<SPGradientTest>( createSuiteSubclass );
    }

    static void destroySuite( SPGradientTest *suite ) { delete suite; }

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

    void testSetGradientTransform()
    {
        SPGradient *gr = static_cast<SPGradient *>(g_object_new(SP_TYPE_GRADIENT, NULL));
        SP_OBJECT(gr)->document = _doc;

        sp_object_set(SP_OBJECT(gr), SP_ATTR_GRADIENTTRANSFORM, "translate(5, 8)");
        TS_ASSERT_EQUALS( gr->gradientTransform, Geom::Matrix(Geom::Translate(5, 8)) );

        sp_object_set(SP_OBJECT(gr), SP_ATTR_GRADIENTTRANSFORM, "");
        TS_ASSERT_EQUALS( gr->gradientTransform, Geom::identity() );

        sp_object_set(SP_OBJECT(gr), SP_ATTR_GRADIENTTRANSFORM, "rotate(90)");
        TS_ASSERT_EQUALS( gr->gradientTransform, Geom::Matrix(rotate_degrees(90)) );

        g_object_unref(gr);
    }


    void testWrite()
    {
        SPGradient *gr = static_cast<SPGradient *>(g_object_new(SP_TYPE_GRADIENT, NULL));
        SP_OBJECT(gr)->document = _doc;

        sp_object_set(SP_OBJECT(gr), SP_ATTR_GRADIENTTRANSFORM, "matrix(0, 1, -1, 0, 0, 0)");
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(_doc);
        Inkscape::XML::Node *repr = xml_doc->createElement("svg:radialGradient");
        SP_OBJECT(gr)->updateRepr(repr, SP_OBJECT_WRITE_ALL);
        {
            gchar const *tr = repr->attribute("gradientTransform");
            Geom::Matrix svd;
            bool const valid = sp_svg_transform_read(tr, &svd);
            TS_ASSERT( valid );
            TS_ASSERT_EQUALS( svd, Geom::Matrix(rotate_degrees(90)) );
        }

        g_object_unref(gr);
    }


    void testGetG2dGetGs2dSetGs2d()
    {
        SPGradient *gr = static_cast<SPGradient *>(g_object_new(SP_TYPE_GRADIENT, NULL));
        SP_OBJECT(gr)->document = _doc;
        Geom::Matrix const grXform(2, 1,
                                 1, 3,
                                 4, 6);
        gr->gradientTransform = grXform;
        Geom::Rect const unit_rect(Geom::Point(0, 0), Geom::Point(1, 1));
        {
            Geom::Matrix const g2d(sp_gradient_get_g2d_matrix(gr, Geom::identity(), unit_rect));
            Geom::Matrix const gs2d(sp_gradient_get_gs2d_matrix(gr, Geom::identity(), unit_rect));
            TS_ASSERT_EQUALS( g2d, Geom::identity() );
            TS_ASSERT( Geom::matrix_equalp(gs2d, gr->gradientTransform * g2d, 1e-12) );

            sp_gradient_set_gs2d_matrix(gr, Geom::identity(), unit_rect, gs2d);
            TS_ASSERT( Geom::matrix_equalp(gr->gradientTransform, grXform, 1e-12) );
        }

        gr->gradientTransform = grXform;
        Geom::Matrix const funny(2, 3,
                               4, 5,
                               6, 7);
        {
            Geom::Matrix const g2d(sp_gradient_get_g2d_matrix(gr, funny, unit_rect));
            Geom::Matrix const gs2d(sp_gradient_get_gs2d_matrix(gr, funny, unit_rect));
            TS_ASSERT_EQUALS( g2d, funny );
            TS_ASSERT( Geom::matrix_equalp(gs2d, gr->gradientTransform * g2d, 1e-12) );

            sp_gradient_set_gs2d_matrix(gr, funny, unit_rect, gs2d);
            TS_ASSERT( Geom::matrix_equalp(gr->gradientTransform, grXform, 1e-12) );
        }

        gr->gradientTransform = grXform;
        Geom::Rect const larger_rect(Geom::Point(5, 6), Geom::Point(8, 10));
        {
            Geom::Matrix const g2d(sp_gradient_get_g2d_matrix(gr, funny, larger_rect));
            Geom::Matrix const gs2d(sp_gradient_get_gs2d_matrix(gr, funny, larger_rect));
            TS_ASSERT_EQUALS( g2d, Geom::Matrix(3, 0,
                                              0, 4,
                                              5, 6) * funny );
            TS_ASSERT( Geom::matrix_equalp(gs2d, gr->gradientTransform * g2d, 1e-12) );

            sp_gradient_set_gs2d_matrix(gr, funny, larger_rect, gs2d);
            TS_ASSERT( Geom::matrix_equalp(gr->gradientTransform, grXform, 1e-12) );

            sp_object_set(SP_OBJECT(gr), SP_ATTR_GRADIENTUNITS, "userSpaceOnUse");
            Geom::Matrix const user_g2d(sp_gradient_get_g2d_matrix(gr, funny, larger_rect));
            Geom::Matrix const user_gs2d(sp_gradient_get_gs2d_matrix(gr, funny, larger_rect));
            TS_ASSERT_EQUALS( user_g2d, funny );
            TS_ASSERT( Geom::matrix_equalp(user_gs2d, gr->gradientTransform * user_g2d, 1e-12) );
        }
        g_object_unref(gr);
    }

};


#endif // SEEN_SP_GRADIENT_TEST_H

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
