#ifndef SEEN_SP_STYLE_ELEM_TEST_H
#define SEEN_SP_STYLE_ELEM_TEST_H

#include <cxxtest/TestSuite.h>

#include "test-helpers.h"

#include "sp-style-elem.h"
#include "xml/repr.h"

class SPStyleElemTest : public CxxTest::TestSuite
{
public:
    SPDocument* _doc;

    SPStyleElemTest() :
        _doc(0)
    {
    }

    virtual ~SPStyleElemTest()
    {
        if ( _doc )
        {
            _doc->doUnref();
        }
    }

    static void createSuiteSubclass( SPStyleElemTest *& dst )
    {
        SPStyleElem *style_elem = new SPStyleElem();

        if ( style_elem ) {
            TS_ASSERT(!style_elem->is_css);
            TS_ASSERT(style_elem->media.print);
            TS_ASSERT(style_elem->media.screen);
            delete style_elem;

            dst = new SPStyleElemTest();
        }
    }

    static SPStyleElemTest *createSuite()
    {
        return Inkscape::createSuiteAndDocument<SPStyleElemTest>( createSuiteSubclass );
    }

    static void destroySuite( SPStyleElemTest *suite ) { delete suite; }

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------


    void testSetType()
    {
        SPStyleElem *style_elem = new SPStyleElem();
        SP_OBJECT(style_elem)->document = _doc;

        SP_OBJECT(style_elem)->setKeyValue( SP_ATTR_TYPE, "something unrecognized");
        TS_ASSERT( !style_elem->is_css );

        SP_OBJECT(style_elem)->setKeyValue( SP_ATTR_TYPE, "text/css");
        TS_ASSERT( style_elem->is_css );

        SP_OBJECT(style_elem)->setKeyValue( SP_ATTR_TYPE, "atext/css");
        TS_ASSERT( !style_elem->is_css );

        SP_OBJECT(style_elem)->setKeyValue( SP_ATTR_TYPE, "text/cssx");
        TS_ASSERT( !style_elem->is_css );

        delete style_elem;
    }

    void testWrite()
    {
        TS_ASSERT( _doc );
        TS_ASSERT( _doc->getReprDoc() );
        if ( !_doc->getReprDoc() ) {
            return; // evil early return
        }

        SPStyleElem *style_elem = new SPStyleElem();
        SP_OBJECT(style_elem)->document = _doc;

        SP_OBJECT(style_elem)->setKeyValue( SP_ATTR_TYPE, "text/css");
        Inkscape::XML::Node *repr = _doc->getReprDoc()->createElement("svg:style");
        SP_OBJECT(style_elem)->updateRepr(_doc->getReprDoc(), repr, SP_OBJECT_WRITE_ALL);
        {
            gchar const *typ = repr->attribute("type");
            TS_ASSERT( typ != NULL );
            if ( typ )
            {
                TS_ASSERT_EQUALS( std::string(typ), std::string("text/css") );
            }
        }

        delete style_elem;
    }

    void testBuild()
    {
        TS_ASSERT( _doc );
        TS_ASSERT( _doc->getReprDoc() );
        if ( !_doc->getReprDoc() ) {
            return; // evil early return
        }

        SPStyleElem *style_elem = new SPStyleElem();
        Inkscape::XML::Node *const repr = _doc->getReprDoc()->createElement("svg:style");
        repr->setAttribute("type", "text/css");
        style_elem->invoke_build( _doc, repr, false);
        TS_ASSERT( style_elem->is_css );
        TS_ASSERT( style_elem->media.print );
        TS_ASSERT( style_elem->media.screen );

        /* Some checks relevant to the read_content test below. */
        {
            g_assert(_doc->style_cascade);
            CRStyleSheet const *const stylesheet = cr_cascade_get_sheet(_doc->style_cascade, ORIGIN_AUTHOR);
            g_assert(stylesheet);
            g_assert(stylesheet->statements == NULL);
        }

        delete style_elem;
        Inkscape::GC::release(repr);
    }

    void testReadContent()
    {
        TS_ASSERT( _doc );
        TS_ASSERT( _doc->getReprDoc() );
        if ( !_doc->getReprDoc() ) {
            return; // evil early return
        }

        SPStyleElem *style_elem = new SPStyleElem();
        Inkscape::XML::Node *const repr = _doc->getReprDoc()->createElement("svg:style");
        repr->setAttribute("type", "text/css");
        Inkscape::XML::Node *const content_repr = _doc->getReprDoc()->createTextNode(".myclass { }");
        repr->addChild(content_repr, NULL);
        style_elem->invoke_build(_doc, repr, false);
        TS_ASSERT( style_elem->is_css );
        TS_ASSERT( _doc->style_cascade );
        CRStyleSheet const *const stylesheet = cr_cascade_get_sheet(_doc->style_cascade, ORIGIN_AUTHOR);
        TS_ASSERT(stylesheet != NULL);
        TS_ASSERT(stylesheet->statements != NULL);

        delete style_elem;
        Inkscape::GC::release(repr);
    }

};


#endif // SEEN_SP_STYLE_ELEM_TEST_H

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
