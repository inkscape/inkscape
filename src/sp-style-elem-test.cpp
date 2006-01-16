#include "attributes.h"
#include "document.h"
#include "inkscape-private.h"
#include "sp-style-elem.h"
#include "streq.h"
#include "utest/utest.h"
#include "xml/repr.h"

/// Dummy functions to keep linker happy
int sp_main_gui (int, char const**) { return 0; }
int sp_main_console (int, char const**) { return 0; }

static bool
test_style_elem()
{
    utest_start("SPStyleElem");
//#if 0
    UTEST_TEST("init") {
        SPStyleElem *style_elem = static_cast<SPStyleElem *>(g_object_new(SP_TYPE_STYLE_ELEM, NULL));
        UTEST_ASSERT(!style_elem->is_css);
        UTEST_ASSERT(style_elem->media.print);
        UTEST_ASSERT(style_elem->media.screen);
        g_object_unref(style_elem);
    }
//#endif

    /* Create the global inkscape object. */
    static_cast<void>(g_object_new(inkscape_get_type(), NULL));

//#if 0
    SPDocument *doc = sp_document_new_dummy();

    UTEST_TEST("sp_object_set(\"type\")") {
        SPStyleElem *style_elem = static_cast<SPStyleElem *>(g_object_new(SP_TYPE_STYLE_ELEM, NULL));
        SP_OBJECT(style_elem)->document = doc;
        sp_object_set(SP_OBJECT(style_elem), SP_ATTR_TYPE, "something unrecognized");
        UTEST_ASSERT(!style_elem->is_css);
        sp_object_set(SP_OBJECT(style_elem), SP_ATTR_TYPE, "text/css");
        UTEST_ASSERT(style_elem->is_css);
        sp_object_set(SP_OBJECT(style_elem), SP_ATTR_TYPE, "atext/css");
        UTEST_ASSERT(!style_elem->is_css);
        sp_object_set(SP_OBJECT(style_elem), SP_ATTR_TYPE, "text/cssx");
        UTEST_ASSERT(!style_elem->is_css);
        g_object_unref(style_elem);
    }

    UTEST_TEST("write") {
        SPStyleElem *style_elem = SP_STYLE_ELEM(g_object_new(SP_TYPE_STYLE_ELEM, NULL));
        SP_OBJECT(style_elem)->document = doc;
        sp_object_set(SP_OBJECT(style_elem), SP_ATTR_TYPE, "text/css");
        Inkscape::XML::Node *repr = sp_repr_new("svg:style");
        SP_OBJECT(style_elem)->updateRepr(repr, SP_OBJECT_WRITE_ALL);
        {
            gchar const *typ = repr->attribute("type");
            UTEST_ASSERT(streq(typ, "text/css"));
        }
        g_object_unref(style_elem);
    }

    UTEST_TEST("build") {
        SPStyleElem &style_elem = *SP_STYLE_ELEM(g_object_new(SP_TYPE_STYLE_ELEM, NULL));
        Inkscape::XML::Node *const repr = sp_repr_new("svg:style");
        repr->setAttribute("type", "text/css");
        sp_object_invoke_build(&style_elem, doc, repr, false);
        UTEST_ASSERT(style_elem.is_css);
        UTEST_ASSERT(style_elem.media.print);
        UTEST_ASSERT(style_elem.media.screen);

        /* Some checks relevant to the read_content test below. */
        {
            g_assert(doc->style_cascade);
            CRStyleSheet const *const stylesheet = cr_cascade_get_sheet(doc->style_cascade, ORIGIN_AUTHOR);
            g_assert(stylesheet);
            g_assert(stylesheet->statements == NULL);
        }

        g_object_unref(&style_elem);
        Inkscape::GC::release(repr);
    }

    UTEST_TEST("read_content") {
        SPStyleElem &style_elem = *SP_STYLE_ELEM(g_object_new(SP_TYPE_STYLE_ELEM, NULL));
        Inkscape::XML::Node *const repr = sp_repr_new("svg:style");
        repr->setAttribute("type", "text/css");
        Inkscape::XML::Node *const content_repr = sp_repr_new_text(".myclass { }");
        repr->addChild(content_repr, NULL);
        sp_object_invoke_build(&style_elem, doc, repr, false);
        UTEST_ASSERT(style_elem.is_css);
        UTEST_ASSERT(doc->style_cascade);
        CRStyleSheet const *const stylesheet = cr_cascade_get_sheet(doc->style_cascade, ORIGIN_AUTHOR);
        UTEST_ASSERT(stylesheet != NULL);
        UTEST_ASSERT(stylesheet->statements != NULL);
        g_object_unref(&style_elem);
        Inkscape::GC::release(repr);
    }
//#endif
    return utest_end();
}

int main()
{
    g_type_init();
    Inkscape::GC::init();
    return ( test_style_elem()
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
