#include <cxxtest/TestSuite.h>

#include <cstdlib>
#include <glib.h>

#include "uri.h"


class RebaseHrefsTest : public CxxTest::TestSuite
{
    Inkscape::XML::Document *document;
    Inkscape::XML::Node *a, *b, *c, *root;

public:

    RebaseHrefsTest()
    {
        Inkscape::GC::init();

        document = sp_repr_document_new("test");
        root = document->root();

        a = document->createElement("a");
        b = document->createElement("b");
        c = document->createElement("c");
    }
    virtual ~RebaseHrefsTest() {}

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static RebaseHrefsTest *createSuite() { return new RebaseHrefsTest(); }
    static void destroySuite( RebaseHrefsTest *suite ) { delete suite; }


    void dump_str(gchar const *str, gchar const *prefix)
    {
        Glib::ustring tmp;
        tmp = prefix;
        tmp += " [";
        size_t const total = strlen(str);
        for (unsigned i = 0; i < total; i++) {
            gchar *const tmp2 = g_strdup_printf(" %02x", (0x0ff & str[i]));
            tmp += tmp2;
            g_free(tmp2);
        }

        tmp += "]";
        g_message("%s", tmp.c_str());
    }

    void testFlipples()
    {
        using Inkscape::URI;
        using Inkscape::MalformedURIException;

        gchar const* things[] = {
            "data:foo,bar",
            "http://www.google.com/image.png",
            "ftp://ssd.com/doo",
            "/foo/dee/bar.svg",
            "foo.svg",
            "file:/foo/dee/bar.svg",
            "file:///foo/dee/bar.svg",
            "file:foo.svg",
            "/foo/bar\xe1\x84\x92.svg",
            "file:///foo/bar\xe1\x84\x92.svg",
            "file:///foo/bar%e1%84%92.svg",
            "/foo/bar%e1%84%92.svg",
            "bar\xe1\x84\x92.svg",
            "bar%e1%84%92.svg",
            NULL
        };
        g_message("+------");
        for ( int i = 0; things[i]; i++ )
        {
            try
            {
                URI uri(things[i]);
                gboolean isAbs = g_path_is_absolute( things[i] );
                gchar *str = uri.toString();
                g_message( "abs:%d  isRel:%d  scheme:[%s]  path:[%s][%s]   uri[%s] / [%s]", (int)isAbs,
                           (int)uri.isRelative(),
                           uri.getScheme(),
                           uri.getPath(),
                           uri.getOpaque(),
                           things[i],
                           str );
                g_free(str);
            }
            catch ( MalformedURIException err )
            {
                dump_str( things[i], "MalformedURIException" );
                xmlChar *redo = xmlURIEscape((xmlChar const *)things[i]);
                g_message("    gone from [%s] to [%s]", things[i], redo );
                if ( redo == NULL )
                {
                    URI again = URI::fromUtf8( things[i] );
                    g_message("     uri from [%s] to [%s]", things[i], again.toString() );
                    gboolean isAbs = g_path_is_absolute( things[i] );
                    gchar *str = again.toString();
                    g_message( "abs:%d  isRel:%d  scheme:[%s]  path:[%s][%s]   uri[%s] / [%s]", (int)isAbs,
                               (int)again.isRelative(),
                               again.getScheme(),
                               again.getPath(),
                               again.getOpaque(),
                               things[i],
                               str );
                    g_free(str);
                    g_message("    ----");
                }
            }
        }
        g_message("+------");
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
