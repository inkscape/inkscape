#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <glib/gprintf.h>
#include <glibmm/i18n.h>
#include "document-private.h"
#include <dir-util.h>
#include "extension/input.h"
#include "extension/system.h"
#include "gdkpixbuf-input.h"
#include "selection-chemistry.h"
#include "sp-image.h"

namespace Inkscape {

namespace IO {
GdkPixbuf* pixbuf_new_from_file( char const *utf8name, GError **error );
}

namespace Extension {
namespace Internal {

static std::set<Glib::ustring> create_lossy_set()
{
    std::set<Glib::ustring> lossy;
    lossy.insert(".jpg");
    lossy.insert(".jpeg");
    return lossy;
}

SPDocument *
GdkpixbufInput::open(Inkscape::Extension::Input *mod, char const *uri)
{
    bool embed = (strcmp(mod->get_param_optiongroup("link"), "embed") == 0);

    SPDocument *doc = NULL;
    GdkPixbuf *pb = Inkscape::IO::pixbuf_new_from_file( uri, NULL );
    static std::set<Glib::ustring> lossy = create_lossy_set();

    if (pb) {         /* We are readable */
        bool is_lossy;
        Glib::ustring mime_type, ext;
        Glib::ustring u = uri;
        std::size_t dotpos = u.rfind('.');
        if (dotpos != Glib::ustring::npos) {
            ext = u.substr(dotpos, Glib::ustring::npos);
        }

        // HACK: replace with something better based on GIO
        if (!ext.empty() && lossy.find(ext) != lossy.end()) {
            is_lossy = true;
            mime_type = "image/jpeg";
        } else {
            is_lossy = false;
            mime_type = "image/png";
        }

        doc = sp_document_new(NULL, TRUE, TRUE);
        bool saved = sp_document_get_undo_sensitive(doc);
        sp_document_set_undo_sensitive(doc, false); // no need to undo in this temporary document

        double width = gdk_pixbuf_get_width(pb);
        double height = gdk_pixbuf_get_height(pb);
        gchar const *str = gdk_pixbuf_get_option( pb, "Inkscape::DpiX" );
        if ( str ) {
            gint dpi = atoi(str);
            if ( dpi > 0 && dpi != 72 ) {
                double scale = 72.0 / (double)dpi;
                width *= scale;
            }
        }
        str = gdk_pixbuf_get_option( pb, "Inkscape::DpiY" );
        if ( str ) {
            gint dpi = atoi(str);
            if ( dpi > 0 && dpi != 72 ) {
                double scale = 72.0 / (double)dpi;
                height *= scale;
            }
        }

        // Create image node
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);
        Inkscape::XML::Node *image_node = xml_doc->createElement("svg:image");
        sp_repr_set_svg_double(image_node, "width", width);
        sp_repr_set_svg_double(image_node, "height", height);

        if (embed) {
            sp_embed_image(image_node, pb, mime_type);
        } else {
            // convert filename to uri
            gchar* _uri = g_filename_to_uri(uri, NULL, NULL);
            if(_uri) {
                image_node->setAttribute("xlink:href", _uri);
                g_free(_uri);
            } else {
                image_node->setAttribute("xlink:href", uri);
            }
        }

        g_object_unref(pb);

        // Add it to the current layer
        SP_DOCUMENT_ROOT(doc)->appendChildRepr(image_node);
        Inkscape::GC::release(image_node);
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
            gchar *caption = g_strdup_printf(_("%s GDK pixbuf Input"), name);
            gchar *xmlString = g_strdup_printf(
                "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
                    "<name>%s</name>\n"
                    "<id>org.inkscape.input.gdkpixbuf.%s</id>\n"
                    "<param name='link' type='optiongroup'  appearance='full' _gui-text='" N_("Link or embed image:") "' >\n"
                        "<_option value='embed'>" N_("embed") "</_option>\n"
                        "<_option value='link'>" N_("link") "</_option>\n"
                    "</param>\n"
                    "<_param name='help' type='description'>" N_("Embed results in stand-alone, larger SVG files. Link references a file outside this SVG document and all files must be moved together.") "</_param>\n"
                    "<input>\n"
                        "<extension>.%s</extension>\n"
                        "<mimetype>%s</mimetype>\n"
                        "<filetypename>%s (*.%s)</filetypename>\n"
                        "<filetypetooltip>%s</filetypetooltip>\n"
                    "</input>\n"
                "</inkscape-extension>",
                caption,
                extensions[i],
                extensions[i],
                mimetypes[j],
                name,
                extensions[i],
                description
                );

            Inkscape::Extension::build_from_mem(xmlString, new GdkpixbufInput());
            g_free(xmlString);
            g_free(caption);
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
