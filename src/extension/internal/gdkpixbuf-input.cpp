#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "document-private.h"
#include <dir-util.h>
#include "prefs-utils.h"
#include "extension/system.h"
#include "gdkpixbuf-input.h"

namespace Inkscape {

namespace IO {
GdkPixbuf* pixbuf_new_from_file( char const *utf8name, GError **error );
}

namespace Extension {
namespace Internal {

SPDocument *
GdkpixbufInput::open(Inkscape::Extension::Input *mod, char const *uri)
{
    SPDocument *doc = sp_document_new(NULL, TRUE, TRUE);
    sp_document_set_undo_sensitive(doc, FALSE); // no need to undo in this temporary document
    GdkPixbuf *pb = Inkscape::IO::pixbuf_new_from_file( uri, NULL );
    Inkscape::XML::Node *rdoc = sp_document_repr_root(doc);
    gchar const *docbase = rdoc->attribute("sodipodi:docbase");
    gchar const *relname = sp_relative_path_from_path(uri, docbase);

    if (pb) {         /* We are readable */
        Inkscape::XML::Node *repr = NULL;

        double width = gdk_pixbuf_get_width(pb);
        double height = gdk_pixbuf_get_height(pb);
        gchar const *str = gdk_pixbuf_get_option( pb, "Inkscape::DpiX" );
        if ( str )
        {
            gint dpi = atoi(str);
            if ( dpi > 0 && dpi != 72 )
            {
                double scale = 72.0 / (double)dpi;
                width *= scale;
            }
        }
        str = gdk_pixbuf_get_option( pb, "Inkscape::DpiY" );
        if ( str )
        {
            gint dpi = atoi(str);
            if ( dpi > 0 && dpi != 72 )
            {
                double scale = 72.0 / (double)dpi;
                height *= scale;
            }
        }

        if (prefs_get_int_attribute("options.importbitmapsasimages", "value", 1) == 1) {
            // import as <image>
            repr = sp_repr_new("svg:image");
            repr->setAttribute("xlink:href", relname);
            repr->setAttribute("sodipodi:absref", uri);

            sp_repr_set_svg_double(repr, "width", width);
            sp_repr_set_svg_double(repr, "height", height);

        } else {
            // import as pattern-filled rect
            Inkscape::XML::Node *pat = sp_repr_new("svg:pattern");
            pat->setAttribute("inkscape:collect", "always");
            pat->setAttribute("patternUnits", "userSpaceOnUse");
            sp_repr_set_svg_double(pat, "width", width);
            sp_repr_set_svg_double(pat, "height", height);
            SP_OBJECT_REPR(SP_DOCUMENT_DEFS(doc))->appendChild(pat);
            gchar const *pat_id = pat->attribute("id");
            SPObject *pat_object = doc->getObjectById(pat_id);

            Inkscape::XML::Node *im = sp_repr_new("svg:image");
            im->setAttribute("xlink:href", relname);
            im->setAttribute("sodipodi:absref", uri);
            sp_repr_set_svg_double(im, "width", width);
            sp_repr_set_svg_double(im, "height", height);
            SP_OBJECT_REPR(pat_object)->addChild(im, NULL);

            repr = sp_repr_new("svg:rect");
            repr->setAttribute("style", g_strdup_printf("stroke:none;fill:url(#%s)", pat_id));
            sp_repr_set_svg_double(repr, "width", width);
            sp_repr_set_svg_double(repr, "height", height);
        }

        SP_DOCUMENT_ROOT(doc)->appendChildRepr(repr);
        Inkscape::GC::release(repr);
        gdk_pixbuf_unref(pb);
        // restore undo, as now this document may be shown to the user if a bitmap was opened
        sp_document_set_undo_sensitive(doc, TRUE);
    } else {
        printf("GdkPixbuf loader failed\n");
    }

    return doc;
}


void
GdkpixbufInput::init(void)
{
    GSList * formatlist, * formatlisthead;

    /* \todo I'm not sure if I need to free this list */
    for (formatlist = formatlisthead = gdk_pixbuf_get_formats();
         formatlist != NULL;
         formatlist = g_slist_next(formatlist)) {

        GdkPixbufFormat *pixformat = (GdkPixbufFormat *)formatlist->data;

        gchar *name =        gdk_pixbuf_format_get_name(pixformat);
        gchar *description = gdk_pixbuf_format_get_description(pixformat);
        gchar **extensions =  gdk_pixbuf_format_get_extensions(pixformat);
        gchar **mimetypes =   gdk_pixbuf_format_get_mime_types(pixformat);

        for (int i = 0; extensions[i] != NULL; i++) {
        for (int j = 0; mimetypes[j] != NULL; j++) {

            /* thanks but no thanks, we'll handle SVG extensions... */
            if (strcmp(extensions[i], "svg") == 0) {
                continue;
            }
            if (strcmp(extensions[i], "svgz") == 0) {
                continue;
            }
            if (strcmp(extensions[i], "svg.gz") == 0) {
                continue;
            }

            gchar *xmlString = g_strdup_printf(
                "<inkscape-extension>\n"
                    "<name>%s GDK pixbuf Input</name>\n"
                    "<id>org.inkscape.input.gdkpixbuf.%s</id>\n"
                    "<input>\n"
                        "<extension>.%s</extension>\n"
                        "<mimetype>%s</mimetype>\n"
                        "<filetypename>%s (*.%s)</filetypename>\n"
                        "<filetypetooltip>%s</filetypetooltip>\n"
                    "</input>\n"
                "</inkscape-extension>",
                name,
                extensions[i],
                extensions[i],
                mimetypes[j],
                name,
                extensions[i],
                description
                );

            Inkscape::Extension::build_from_mem(xmlString, new GdkpixbufInput());
            g_free(xmlString);
        }}

        g_free(name);
        g_free(description);
        g_strfreev(mimetypes);
        g_strfreev(extensions);
    }

    g_slist_free(formatlisthead);
}

} } }  /* namespace Inkscape, Extension, Implementation */

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
