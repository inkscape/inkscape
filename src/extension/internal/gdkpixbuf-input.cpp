#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "document-private.h"
#include <dir-util.h>
#include "extension/system.h"
#include "gdkpixbuf-input.h"
#include "selection-chemistry.h"

namespace Inkscape {

namespace IO {
GdkPixbuf* pixbuf_new_from_file( char const *utf8name, GError **error );
}

namespace Extension {
namespace Internal {

Document *
GdkpixbufInput::open(Inkscape::Extension::Input */*mod*/, char const *uri)
{
    PDocument *doc = NULL;
    GdkPixbuf *pb = Inkscape::IO::pixbuf_new_from_file( uri, NULL );

    if (pb) {         /* We are readable */
        doc = sp_document_new(NULL, TRUE, TRUE);
        bool saved = sp_document_get_undo_sensitive(doc);
        sp_document_set_undo_sensitive(doc, false); // no need to undo in this temporary document

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

        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);
        // import as <image>
        repr = xml_doc->createElement("svg:image");

        // convert filename to uri
        gchar* _uri = g_filename_to_uri(uri, NULL, NULL);
        if(_uri) {
            repr->setAttribute("xlink:href", _uri);
            g_free(_uri);
        } else {
            repr->setAttribute("xlink:href", uri);
        }
        /* impl: doc->base is currently NULL, so we can use uri for href whether it's absolute
         * or relative.  The href will get rewritten by rebase_hrefs if by chance uri is relative
         * and doc gets saved to a different directory.
         *
         * We don't bother setting sodipodi:absref, as we assume it's never useful to have
         * sodipodi:absref with the same value as xlink:href, and rebase_hrefs will provide
         * sodipodi:absref values where necessary. */

        sp_repr_set_svg_double(repr, "width", width);
        sp_repr_set_svg_double(repr, "height", height);

        SP_DOCUMENT_ROOT(doc)->appendChildRepr(repr);
        Inkscape::GC::release(repr);
        gdk_pixbuf_unref(pb);
        //alter the canvas size to fit the image size
        fit_canvas_to_drawing(doc);
        // restore undo, as now this document may be shown to the user if a bitmap was opened
        sp_document_set_undo_sensitive(doc, saved);
    } else {
        printf("GdkPixbuf loader failed\n");
    }

    return doc;
}

#include "clear-n_.h"

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
                "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
                    "<name>" N_("%s GDK pixbuf Input") "</name>\n"
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
